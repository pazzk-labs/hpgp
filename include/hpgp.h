/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef HPGP_H
#define HPGP_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "hpgp_mme.h"

#define HPGP_MMTYPE_MSB_BIT		13

typedef enum {
	HPGP_RC_UNKNOWN,
	HPGP_RC_READY,
	HPGP_RC_IN_PROGRESS,
	HPGP_RC_MATCHED,
	HPGP_RC_INVALID_INPUT,
	HPGP_RC_INCORRECT_INPUT,
} hpgp_rc_t;

enum hpgp_variant {
	HPGP_VARIANT_REQ, /*< Management message request */
	HPGP_VARIANT_CNF, /*< Management message confirm */
	HPGP_VARIANT_IND, /*< Management message indication */
	HPGP_VARIANT_RSP, /*< Management message response */
};

enum hpgp_entity {
	HPGP_ENTITY_STA_CCO, /*< messages exchanged between STA and CCo */
	HPGP_ENTITY_PROXY, /*< messages exchanged with the proxy coordinator */
	HPGP_ENTITY_CCO_CCO, /*< messages exchanged between neighboring CCos */
	HPGP_ENTITY_STA_STA, /*< messages exchanged between two stations */
	HPGP_ENTITY_MANUFACTURE,
	HPGP_ENTITY_VENDOR,
};

enum hpgp_mmtype {
	HPGP_MMTYPE_DISCOVER_LIST,
	HPGP_MMTYPE_ENCRYPTED,
	HPGP_MMTYPE_SET_KEY,
	HPGP_MMTYPE_GET_KEY,
	HPGP_MMTYPE_BRG_INFO,
	HPGP_MMTYPE_NW_INFO,
	HPGP_MMTYPE_HFID,
	HPGP_MMTYPE_NW_STATS,
	HPGP_MMTYPE_SLAC_PARM,
	HPGP_MMTYPE_START_ATTEN_CHAR,
	HPGP_MMTYPE_ATTEN_CHAR,
	HPGP_MMTYPE_PKCS_CERT,
	HPGP_MMTYPE_MNBC_SOUND,
	HPGP_MMTYPE_VALIDATE,
	HPGP_MMTYPE_SLAC_MATCH,
	HPGP_MMTYPE_SLAC_USER_DATA,
	HPGP_MMTYPE_ATTEN_PROFILE,
	HPGP_MMTYPE_MAX,
};

typedef uint16_t hpgp_variant_t;
typedef uint16_t hpgp_entity_t;
typedef uint16_t hpgp_mmtype_t;

struct hpgp_frame {
	uint8_t mmv;     /*< Management Message Version */
	uint16_t mmtype; /*< Management Message Type */
	uint8_t body[];
} __attribute__((packed));

struct hpgp_mme_req {
	union {
		struct hpgp_mme_setkey_req setkey;
		struct hpgp_mme_getkey_req getkey;
		struct hpgp_mme_slac_parm_req slac_parm;
	} msg;
};

struct hpgp_mme_cnf {
	union {
		struct hpgp_mme_slac_parm_cnf slac_parm;
		struct hpgp_mme_slac_match_cnf slac_match;
		struct hpgp_mme_getkey_cnf getkey;
	} msg;
};

struct hpgp_mme_ind {
	union {
		struct hpgp_mme_atten_char_ind atten_char;
	} msg;
};

size_t hpgp_encode_request(struct hpgp_frame *hpgp, hpgp_mmtype_t type,
		const void *msg, size_t msglen);
size_t hpgp_encode_confirm(struct hpgp_frame *hpgp, hpgp_mmtype_t type,
		const void *msg, size_t msglen);
size_t hpgp_encode_indication(struct hpgp_frame *hpgp, hpgp_mmtype_t type,
		const void *msg, size_t msglen);
size_t hpgp_encode_response(struct hpgp_frame *hpgp, hpgp_mmtype_t type,
		const void *msg, size_t msglen);

hpgp_mmtype_t hpgp_mmtype(const struct hpgp_frame *hpgp);
hpgp_variant_t hpgp_variant(const struct hpgp_frame *hpgp);
hpgp_entity_t hpgp_entity(const struct hpgp_frame *hpgp);
hpgp_mmtype_t hpgp_mmtype_raw(const struct hpgp_frame *hpgp);

int hpgp_set_header(struct hpgp_frame *hpgp, hpgp_variant_t variant,
		hpgp_entity_t entity, hpgp_mmtype_t type);

#if defined(__cplusplus)
}
#endif

#endif /* HPGP_H */
