/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "hpgp.h"
#include <string.h>

#define MMTYPE_OFFSET_BIT	2

#if !defined(HPGP_MMVER)
#define HPGP_MMVER		1
#endif

#if !defined(MIN)
#define MIN(a, b)		(((a) > (b))? (b) : (a))
#endif

#if !defined(ARRAY_COUNT)
#define ARRAY_COUNT(x)		(sizeof(x) / sizeof((x)[0]))
#endif

typedef size_t (*encoder_func_t)(struct hpgp_mme *mme,
		const void *data, size_t datasize);

struct encoder {
	hpgp_variant_t variant;
	hpgp_mmtype_t type;
	encoder_func_t func;
};

static void set_mmver(struct hpgp_frame *frame, uint8_t mmv)
{
	frame->mmv = mmv;
}

static void set_mmtype(struct hpgp_frame *frame, uint16_t mmtype)
{
	frame->mmtype = mmtype;
}

static void set_fmi(struct hpgp_mme *mme)
{
	mme->fmi = 0;
	mme->fmi_opt = 0;
}

static void apply_header(struct hpgp_frame *hpgp, uint16_t mmtype, uint8_t mmv)
{
	struct hpgp_mme *mme = (struct hpgp_mme *)hpgp->body;

	set_mmver(hpgp, mmv);
	set_mmtype(hpgp, mmtype);

	memset(mme, 0, sizeof(*mme));

	set_fmi(mme);
}

static void set_header(struct hpgp_frame *hpgp, hpgp_variant_t variant,
		hpgp_entity_t entity, hpgp_mmtype_t type)
{
	variant &= (1U << MMTYPE_OFFSET_BIT) - 1; /* 2 bits */
	entity &= 0x7; /* 3 bits */
	type &= 0x7ff; /* 11 bits */

	const uint16_t entity_shifted = entity << HPGP_MMTYPE_MSB_BIT;
	const uint16_t type_shifed = type << MMTYPE_OFFSET_BIT;
	const uint16_t combined = variant | entity_shifted | type_shifed;
	uint8_t mmv = 0;

	if (entity != HPGP_ENTITY_VENDOR && entity != HPGP_ENTITY_MANUFACTURE) {
		mmv = HPGP_MMVER;
	}

	apply_header(hpgp, combined, mmv);
}

static hpgp_variant_t get_variant(uint16_t mmtype)
{
	const uint16_t mask = (1U << MMTYPE_OFFSET_BIT) - 1; /* 2 bits */
	return (hpgp_variant_t)(mmtype & mask);
}

static hpgp_entity_t get_entity(uint16_t mmtype)
{
	return (hpgp_entity_t)(mmtype >> HPGP_MMTYPE_MSB_BIT);
}

static hpgp_mmtype_t get_mmtype(uint16_t mmtype)
{
	return (hpgp_mmtype_t)((mmtype >> MMTYPE_OFFSET_BIT) & 0x7ff);
}

static uint16_t mmtype_to_mmcode(hpgp_mmtype_t type)
{
	uint16_t base = HPGP_ENTITY_STA_STA;
	uint16_t offset = (uint16_t)type;

	if (type >= HPGP_MMTYPE_MAX) {
		return 0;
	} else if (type == HPGP_MMTYPE_DISCOVER_LIST) {
		base = HPGP_ENTITY_STA_CCO;
		offset = 0x05;
	} else if (type >= HPGP_MMTYPE_SLAC_PARM) {
		offset = (uint16_t)(0x19 + type - HPGP_MMTYPE_SLAC_PARM);
	} else if (type >= HPGP_MMTYPE_NW_STATS) {
		offset = (uint16_t)(0x12 + type - HPGP_MMTYPE_NW_STATS);
	} else if (type >= HPGP_MMTYPE_HFID) {
		offset = (uint16_t)(0x10 + type - HPGP_MMTYPE_HFID);
	} else if (type >= HPGP_MMTYPE_NW_INFO) {
		offset = (uint16_t)(0x0E + type - HPGP_MMTYPE_NW_INFO);
	} else if (type >= HPGP_MMTYPE_BRG_INFO) {
		offset = (uint16_t)(0x08 + type - HPGP_MMTYPE_BRG_INFO);
	}

	return (uint16_t)((base << HPGP_MMTYPE_MSB_BIT)
			+ (offset << MMTYPE_OFFSET_BIT));
}

static hpgp_mmtype_t mmcode_to_mmtype(uint16_t code)
{
	hpgp_entity_t msb = (hpgp_entity_t)(code >> HPGP_MMTYPE_MSB_BIT);
	uint16_t offset = (uint16_t)((code >> MMTYPE_OFFSET_BIT) & 0x7ff);

	switch (msb) {
	case HPGP_ENTITY_STA_CCO:
		if (offset == 5) {
			return HPGP_MMTYPE_DISCOVER_LIST;
		}
		break;
	case HPGP_ENTITY_STA_STA:
		if (offset >= 0x19) {
			offset = offset - 0x19 + HPGP_MMTYPE_SLAC_PARM;
		} else if (offset >= 0x12) {
			offset = offset - 0x12 + HPGP_MMTYPE_NW_STATS;
		} else if (offset >= 0x10) {
			offset = offset - 0x10 + HPGP_MMTYPE_HFID;
		} else if (offset >= 0x0E) {
			offset = offset - 0x0E + HPGP_MMTYPE_NW_INFO;
		} else if (offset >= 0x08) {
			offset = offset - 0x08 + HPGP_MMTYPE_BRG_INFO;
		}
		return (hpgp_mmtype_t)offset;
		break;
	case HPGP_ENTITY_VENDOR: /* fall through */
	case HPGP_ENTITY_PROXY: /* fall through */
	case HPGP_ENTITY_CCO_CCO: /* fall through */
	case HPGP_ENTITY_MANUFACTURE: /* fall through */
	default:
		break;
	}

	return HPGP_MMTYPE_MAX;
}

static size_t copy(void *dst, const void *src, size_t len)
{
	memcpy(dst, src, len);
	return len;
}

static size_t encode_empty(struct hpgp_mme *mme, const void *msg, size_t msglen)
{
	(void)mme;
	(void)msg;
	(void)msglen;
	return 0;
}

static size_t encode_generic(struct hpgp_mme *mme,
		const void *msg, size_t msglen)
{
	return copy(mme->data, msg, msglen);
}

static struct encoder encoders[] = {
	{ HPGP_VARIANT_REQ, HPGP_MMTYPE_SET_KEY,          encode_generic },
	{ HPGP_VARIANT_REQ, HPGP_MMTYPE_GET_KEY,          encode_generic },
	{ HPGP_VARIANT_REQ, HPGP_MMTYPE_SLAC_PARM,        encode_generic },
	{ HPGP_VARIANT_REQ, HPGP_MMTYPE_SLAC_MATCH,       encode_generic },
	{ HPGP_VARIANT_CNF, HPGP_MMTYPE_GET_KEY,          encode_generic },
	{ HPGP_VARIANT_CNF, HPGP_MMTYPE_SLAC_PARM,        encode_generic },
	{ HPGP_VARIANT_CNF, HPGP_MMTYPE_SLAC_MATCH,       encode_generic },
	{ HPGP_VARIANT_IND, HPGP_MMTYPE_ATTEN_CHAR,       encode_generic },
	{ HPGP_VARIANT_IND, HPGP_MMTYPE_MNBC_SOUND,       encode_generic },
	{ HPGP_VARIANT_IND, HPGP_MMTYPE_START_ATTEN_CHAR, encode_generic },
	{ HPGP_VARIANT_RSP, HPGP_MMTYPE_ATTEN_CHAR,       encode_generic },
};

static size_t encode(struct hpgp_frame *hpgp, hpgp_variant_t variant,
		hpgp_mmtype_t type, const void *msg, size_t msglen)
{
	struct hpgp_mme *mme = (struct hpgp_mme *)hpgp->body;
	const size_t hlen = sizeof(*hpgp) + sizeof(*mme);
	const uint16_t mmcode = mmtype_to_mmcode(type);

	set_header(hpgp, variant, get_entity(mmcode),
			mmcode >> MMTYPE_OFFSET_BIT);

	for (size_t i = 0; i < ARRAY_COUNT(encoders); i++) {
		struct encoder *p = &encoders[i];

		if (p->variant == variant && p->type == type) {
			return (*p->func)(mme, msg, msglen) + hlen;
		}
	}

	return encode_empty(mme, msg, msglen) + hlen;
}

size_t hpgp_encode_request(struct hpgp_frame *hpgp, hpgp_mmtype_t type,
		const void *msg, size_t msglen)
{
	return encode(hpgp, HPGP_VARIANT_REQ, type, msg, msglen);
}

size_t hpgp_encode_confirm(struct hpgp_frame *hpgp, hpgp_mmtype_t type,
		const void *msg, size_t msglen)
{
	return encode(hpgp, HPGP_VARIANT_CNF, type, msg, msglen);
}

size_t hpgp_encode_indication(struct hpgp_frame *hpgp, hpgp_mmtype_t type,
		const void *msg, size_t msglen)
{
	return encode(hpgp, HPGP_VARIANT_IND, type, msg, msglen);
}

size_t hpgp_encode_response(struct hpgp_frame *hpgp, hpgp_mmtype_t type,
		const void *msg, size_t msglen)
{
	return encode(hpgp, HPGP_VARIANT_RSP, type, msg, msglen);
}

hpgp_mmtype_t hpgp_mmtype(const struct hpgp_frame *hpgp)
{
	return mmcode_to_mmtype(hpgp->mmtype);
}

hpgp_mmtype_t hpgp_mmtype_raw(const struct hpgp_frame *hpgp)
{
	return get_mmtype(hpgp->mmtype);
}

hpgp_variant_t hpgp_variant(const struct hpgp_frame *hpgp)
{
	return get_variant(hpgp->mmtype);
}

hpgp_entity_t hpgp_entity(const struct hpgp_frame *hpgp)
{
	return get_entity(hpgp->mmtype);
}

int hpgp_set_header(struct hpgp_frame *hpgp, hpgp_variant_t variant,
		hpgp_entity_t entity, hpgp_mmtype_t type)
{
	set_header(hpgp, variant, entity, type);
	return 0;
}
