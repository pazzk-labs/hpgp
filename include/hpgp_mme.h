/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef HPGP_MME_H
#define HPGP_MME_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

struct hpgp_mme {
	uint8_t fmi;     /*< Fragmentation Message Sequence Number */
	uint8_t fmi_opt;
	uint8_t data[]; /*< Entry Data */
} __attribute__((packed));

struct hpgp_mme_setkey_req {
	uint8_t key_type;
	uint32_t my_nonce;
	uint32_t your_nonce;
	uint8_t pid;
	uint16_t prn;
	uint8_t pmn;
	uint8_t cco;
	uint8_t nid[7];
	uint8_t key_selection;
	uint8_t key[16];
} __attribute__((packed));

struct hpgp_mme_setkey_cnf {
	uint8_t result;
	uint32_t my_nonce;
	uint32_t your_nonce;
	uint8_t pid;
	uint16_t prn;
	uint8_t pmn;
	uint8_t cco;
} __attribute__((packed));

struct hpgp_mme_getkey_req {
	uint8_t req_type;
	uint8_t key_type;
	uint8_t nid[7];
	uint32_t my_nonce;
	uint8_t pid;
	uint16_t prn;
	uint8_t pmn;
	uint8_t key[16];
} __attribute__((packed));

struct hpgp_mme_getkey_cnf {
	uint8_t result;
	uint8_t key_type;
	uint32_t my_nonce;
	uint32_t your_nonce;
	uint8_t nid[7];
	uint8_t eks;
	uint8_t pid;
	uint16_t prn;
	uint8_t pmn;
	uint8_t key[16];
} __attribute__((packed));

struct hpgp_mme_slac_parm_req {
	uint8_t app_type;
	uint8_t sec_type;
	uint8_t run_id[8];
	uint8_t ciphersuite_size;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
	uint8_t ciphersuite[0];
#pragma GCC diagnostic pop
} __attribute__((packed));

struct hpgp_mme_slac_parm_cnf {
	uint8_t target_mac[6];
	uint8_t nr_sounds;
	uint8_t timeout_ms_hundredth;
	uint8_t forwarding_type;
	uint8_t forwarding_mac[6];
	uint8_t app_type;
	uint8_t sec_type;
	uint8_t run_id[8];
	uint16_t ciphersuite;
} __attribute__((packed));

struct hpgp_mme_start_atten_char_ind_body {
	uint8_t nr_sounds;
	uint8_t timeout_ms_hundredth;
	uint8_t resp_type;
	uint8_t forwarding_sta[6];
	uint8_t run_id[8];
} __attribute__((packed));

struct hpgp_mme_start_atten_char_ind {
	uint8_t app_type;
	uint8_t sec_type;
	struct hpgp_mme_start_atten_char_ind_body atten;
} __attribute__((packed));

struct hpgp_mme_mnbc_sound_ind_body {
	uint8_t sender_id[17];
	uint8_t count;
	uint8_t run_id[8];
	uint8_t reserved[8];
	uint8_t rnd[16];
} __attribute__((packed));

struct hpgp_mme_mnbc_sound_ind {
	uint8_t app_type;
	uint8_t sec_type;
	struct hpgp_mme_mnbc_sound_ind_body msound;
} __attribute__((packed));

struct hpgp_mme_atten_profile_ind {
	uint8_t pev_mac[6];
	uint8_t nr_groups;
	uint8_t reserved;
	uint8_t aag[];
} __attribute__((packed));

struct hpgp_mme_atten_char_ind_body {
	uint8_t mac_src[6];
	uint8_t run_id[8];
	uint8_t id_src[17];
	uint8_t id_rsp[17];
	uint8_t nr_sounds;
	uint8_t nr_groups;
	uint8_t aag[58];
} __attribute__((packed));

struct hpgp_mme_atten_char_ind {
	uint8_t app_type;
	uint8_t sec_type;
	struct hpgp_mme_atten_char_ind_body atten;
} __attribute__((packed));

struct hpgp_mme_atten_char_rsp_body {
	uint8_t mac_src[6];
	uint8_t run_id[8];
	uint8_t id_src[17];
	uint8_t id_rsp[17];
	uint8_t result;
} __attribute__((packed));

struct hpgp_mme_atten_char_rsp {
	uint8_t app_type;
	uint8_t sec_type;
	struct hpgp_mme_atten_char_rsp_body atten;
} __attribute__((packed));

struct hpgp_mme_validate_req {
	uint8_t signal_type;
	uint8_t vfield[];
} __attribute__((packed));

struct hpgp_mme_validate_cnf {
	uint8_t signal_type;
	uint8_t vfield[];
} __attribute__((packed));

struct hpgp_mme_slac_match_req_body {
	uint8_t id_pev[17];
	uint8_t mac_pev[6];
	uint8_t id_evse[17];
	uint8_t mac_evse[6];
	uint8_t run_id[8];
	uint8_t reserved[8];
} __attribute__((packed));

struct hpgp_mme_slac_match_req {
	uint8_t app_type;
	uint8_t sec_type;
	uint16_t len;
	struct hpgp_mme_slac_match_req_body mfield;
} __attribute__((packed));

struct hpgp_mme_slac_match_cnf_body {
	uint8_t id_pev[17];
	uint8_t mac_pev[6];
	uint8_t id_evse[17];
	uint8_t mac_evse[6];
	uint8_t run_id[8];
	uint8_t reserved[8];
	uint8_t nid[7];
	uint8_t reserved2;
	uint8_t nmk[16];
} __attribute__((packed));

struct hpgp_mme_slac_match_cnf {
	uint8_t app_type;
	uint8_t sec_type;
	uint16_t len;
	struct hpgp_mme_slac_match_cnf_body mfield;
} __attribute__((packed));

struct hpgp_mme_pkcs_cert_req {
	uint8_t target_mac[6];
	uint8_t ciphersuite_size;
	uint8_t ciphersuite[];
} __attribute__((packed));

struct hpgp_mme_pkcs_cert_cnf {
	uint8_t target_mac[6];
	uint8_t status;
	uint16_t ciphersuite;
	uint16_t cert_len;
	uint8_t cert[];
} __attribute__((packed));

struct hpgp_mme_pkcs_cert_ind {
	uint8_t target_mac[6];
	uint16_t ciphersuite;
	uint16_t cert_len;
	uint8_t cert[];
} __attribute__((packed));

struct hpgp_mme_pkcs_cert_rsp {
	uint8_t target_mac[6];
	uint8_t status;
	uint16_t ciphersuite_size;
	uint8_t ciphersuite[];
} __attribute__((packed));

#if defined(__cplusplus)
}
#endif

#endif /* HPGP_MME_H */
