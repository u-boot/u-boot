// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Limited
 * Copyright (c) 2018 Bootlin
 * Author: Miquel Raynal <miquel.raynal@bootlin.com>
 */

#include <dm.h>
#include <dm/of_access.h>
#include <tpm_api.h>
#include <tpm-common.h>
#include <tpm-v2.h>
#include <tpm_tcg2.h>
#include <u-boot/sha1.h>
#include <u-boot/sha256.h>
#include <u-boot/sha512.h>
#include <version_string.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/unaligned/be_byteshift.h>
#include <linux/unaligned/generic.h>
#include <linux/unaligned/le_byteshift.h>

#include "tpm-utils.h"

static int tpm2_update_active_banks(struct udevice *dev)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	struct tpml_pcr_selection pcrs;
	int ret, i;

	ret = tpm2_get_pcr_info(dev, &pcrs);
	if (ret)
		return ret;

	priv->active_bank_count = 0;
	for (i = 0; i < pcrs.count; i++) {
		if (!tpm2_is_active_bank(&pcrs.selection[i]))
			continue;
		priv->active_banks[priv->active_bank_count] = pcrs.selection[i].hash;
		priv->active_bank_count++;
	}

	return 0;
}

static void tpm2_print_selected_algorithm_name(u32 selected)
{
	size_t i;
	const char *str;

	for (i = 0; i < ARRAY_SIZE(hash_algo_list); i++) {
		const struct digest_info *algo = &hash_algo_list[i];

		if (!(selected & algo->hash_mask))
			continue;

		str = tpm2_algorithm_name(algo->hash_alg);
		if (str)
			log_info("%s\n", str);
	}
}

int tpm2_scan_masks(struct udevice *dev, u32 log_active, u32 *mask)
{
	struct tpml_pcr_selection pcrs;
	u32 active = 0;
	u32 supported = 0;
	int rc, i;

	*mask = 0;

	rc = tpm2_get_pcr_info(dev, &pcrs);
	if (rc)
		return rc;

	for (i = 0; i < pcrs.count; i++) {
		struct tpms_pcr_selection *sel = &pcrs.selection[i];
		size_t j;
		u32 hash_mask = 0;

		for (j = 0; j < ARRAY_SIZE(hash_algo_list); j++) {
			if (hash_algo_list[j].hash_alg == sel->hash)
				hash_mask = hash_algo_list[j].hash_mask;
		}

		if (tpm2_algorithm_supported(sel->hash))
			supported |= hash_mask;

		if (tpm2_is_active_bank(sel))
			active |= hash_mask;
	}

	/* All eventlog algorithm(s) must be supported */
	if (log_active & ~supported) {
		log_err("EventLog contains U-Boot unsupported algorithm(s)\n");
		tpm2_print_selected_algorithm_name(log_active & ~supported);
		rc = -1;
	}
	if (log_active && active & ~log_active) {
		log_warning("TPM active algorithm(s) not exist in eventlog\n");
		tpm2_print_selected_algorithm_name(active & ~log_active);
		*mask = log_active;
	}

	/* Any active algorithm(s) which are not supported must be removed */
	if (active & ~supported) {
		log_warning("TPM active algorithm(s) unsupported by u-boot\n");
		tpm2_print_selected_algorithm_name(active & ~supported);
		if (*mask)
			*mask = active & supported & *mask;
		else
			*mask = active & supported;
	}

	return rc;
}

static int tpm2_pcr_allocate(struct udevice *dev, u32 algo_mask)
{
	struct tpml_pcr_selection pcr = { 0 };
	u32 pcr_len = 0;
	int rc;

	rc = tpm2_get_pcr_info(dev, &pcr);
	if (rc)
		return rc;

	rc = tpm2_pcr_config_algo(dev, algo_mask, &pcr, &pcr_len);
	if (rc)
		return rc;

	/* Assume no password */
	rc = tpm2_send_pcr_allocate(dev, NULL, 0, &pcr, pcr_len);
	if (rc)
		return rc;

	/* Send TPM2_Shutdown, assume mode = TPM2_SU_CLEAR */
	return tpm2_startup(dev, false, TPM2_SU_CLEAR);
}

int tpm2_activate_banks(struct udevice *dev, u32 log_active)
{
	u32 algo_mask = 0;
	int rc;

	rc = tpm2_scan_masks(dev, log_active, &algo_mask);
	if (rc)
		return rc;

	if (algo_mask) {
		if (!IS_ENABLED(CONFIG_TPM_PCR_ALLOCATE))
			return -1;

		rc = tpm2_pcr_allocate(dev, algo_mask);
		if (rc)
			return rc;

		log_info("PCR allocate done, shutdown TPM and reboot\n");
		do_reset(NULL, 0, 0, NULL);
		log_err("reset does not work!\n");
		return -1;
	}

	return 0;
}

u32 tpm2_startup(struct udevice *dev, bool bon, enum tpm2_startup_types mode)
{
	int op = bon ? TPM2_CC_STARTUP : TPM2_CC_SHUTDOWN;
	const u8 command_v2[12] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(12),
		tpm_u32(op),
		tpm_u16(mode),
	};
	int ret;

	/*
	 * Note TPM2_Startup command will return RC_SUCCESS the first time,
	 * but will return RC_INITIALIZE otherwise.
	 */
	ret = tpm_sendrecv_command(dev, command_v2, NULL, NULL);
	if ((ret && ret != TPM2_RC_INITIALIZE) || !bon)
		return ret;

	return tpm2_update_active_banks(dev);
}

u32 tpm2_self_test(struct udevice *dev, enum tpm2_yes_no full_test)
{
	const u8 command_v2[12] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(11),
		tpm_u32(TPM2_CC_SELF_TEST),
		full_test,
	};

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_auto_start(struct udevice *dev)
{
	u32 rc;

	rc = tpm2_self_test(dev, TPMI_YES);

	if (rc == TPM2_RC_INITIALIZE) {
		rc = tpm2_startup(dev, true, TPM2_SU_CLEAR);
		if (rc)
			return rc;

		rc = tpm2_self_test(dev, TPMI_YES);
	}
	if (rc)
		return rc;

	return tpm2_update_active_banks(dev);
}

u32 tpm2_clear(struct udevice *dev, u32 handle, const char *pw,
	       const ssize_t pw_sz)
{
	/* Length of the message header, up to start of password */
	uint offset = 27;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(offset + pw_sz),	/* Length */
		tpm_u32(TPM2_CC_CLEAR),		/* Command code */

		/* HANDLE */
		tpm_u32(handle),		/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */
	};
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "s",
			       offset, pw, pw_sz);
	offset += pw_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_nv_define_space(struct udevice *dev, u32 space_index,
			 size_t space_size, u32 nv_attributes,
			 const u8 *nv_policy, size_t nv_policy_size)
{
	/*
	 * Calculate the offset of the nv_policy piece by adding each of the
	 * chunks below.
	 */
	const int platform_len = sizeof(u32);
	const int session_hdr_len = 13;
	const int message_len = 14;
	uint offset = TPM2_HDR_LEN + platform_len + session_hdr_len +
		message_len;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(offset + nv_policy_size + 2),/* Length */
		tpm_u32(TPM2_CC_NV_DEFINE_SPACE),/* Command code */

		/* handles 4 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Primary platform seed */

		/* session header 13 bytes */
		tpm_u32(9),			/* Header size */
		tpm_u32(TPM2_RS_PW),		/* Password authorisation */
		tpm_u16(0),			/* nonce_size */
		0,				/* session_attrs */
		tpm_u16(0),			/* auth_size */

		/* message 14 bytes + policy */
		tpm_u16(message_len + nv_policy_size),	/* size */
		tpm_u32(space_index),
		tpm_u16(TPM2_ALG_SHA256),
		tpm_u32(nv_attributes),
		tpm_u16(nv_policy_size),
		/*
		 * nv_policy
		 * space_size
		 */
	};
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sw",
			       offset, nv_policy, nv_policy_size,
			       offset + nv_policy_size, space_size);
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_extend(struct udevice *dev, u32 index, u32 algorithm,
		    const u8 *digest, u32 digest_len)
{
	/* Length of the message header, up to start of digest */
	uint offset = 33;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(offset + digest_len),	/* Length */
		tpm_u32(TPM2_CC_PCR_EXTEND),	/* Command code */

		/* HANDLE */
		tpm_u32(index),			/* Handle (PCR Index) */

		/* AUTH_SESSION */
		tpm_u32(9),			/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(0),			/* Size of <hmac/password> */
						/* <hmac/password> (if any) */

		/* hashes */
		tpm_u32(1),			/* Count (number of hashes) */
		tpm_u16(algorithm),	/* Algorithm of the hash */
		/* STRING(digest)		   Digest */
	};
	int ret;

	if (!digest)
		return -EINVAL;

	if (!tpm2_check_active_banks(dev)) {
		log_err("Cannot extend PCRs if all the TPM enabled algorithms are not supported\n");

		ret = tpm2_pcr_allocate(dev, 0);
		if (ret)
			return -EINVAL;
	}
	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the digest
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "s",
			       offset, digest, digest_len);
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_nv_read_value(struct udevice *dev, u32 index, void *data, u32 count)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(10 + 8 + 4 + 9 + 4),	/* Length */
		tpm_u32(TPM2_CC_NV_READ),	/* Command code */

		/* handles 8 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Primary platform seed */
		tpm_u32(HR_NV_INDEX + index),	/* Password authorisation */

		/* AUTH_SESSION */
		tpm_u32(9),			/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(0),			/* Size of <hmac/password> */
						/* <hmac/password> (if any) */

		tpm_u16(count),			/* Number of bytes */
		tpm_u16(0),			/* Offset */
	};
	size_t response_len = COMMAND_BUFFER_SIZE;
	u8 response[COMMAND_BUFFER_SIZE];
	int ret;
	u16 tag;
	u32 size, code;

	ret = tpm_sendrecv_command(dev, command_v2, response, &response_len);
	if (ret)
		return log_msg_ret("read", ret);
	if (unpack_byte_string(response, response_len, "wdds",
			       0, &tag, 2, &size, 6, &code,
			       16, data, count))
		return TPM_LIB_ERROR;

	return 0;
}

u32 tpm2_nv_write_value(struct udevice *dev, u32 index, const void *data,
			u32 count)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	uint offset = 10 + 8 + 4 + 9 + 2;
	uint len = offset + count + 2;
	/* Use empty password auth if platform hierarchy is disabled */
	u32 auth = priv->plat_hier_disabled ? HR_NV_INDEX + index :
		TPM2_RH_PLATFORM;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(len),			/* Length */
		tpm_u32(TPM2_CC_NV_WRITE),	/* Command code */

		/* handles 8 bytes */
		tpm_u32(auth),			/* Primary platform seed */
		tpm_u32(HR_NV_INDEX + index),	/* Password authorisation */

		/* AUTH_SESSION */
		tpm_u32(9),			/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(0),			/* Size of <hmac/password> */
						/* <hmac/password> (if any) */

		tpm_u16(count),
	};
	size_t response_len = COMMAND_BUFFER_SIZE;
	u8 response[COMMAND_BUFFER_SIZE];
	int ret;

	ret = pack_byte_string(command_v2, sizeof(command_v2), "sw",
			       offset, data, count,
			       offset + count, 0);
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, response, &response_len);
}

u32 tpm2_pcr_read(struct udevice *dev, u32 idx, unsigned int idx_min_sz,
		  u16 algorithm, void *data, u32 digest_len,
		  unsigned int *updates)
{
	u8 idx_array_sz = max(idx_min_sz, DIV_ROUND_UP(idx, 8));
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),	/* TAG */
		tpm_u32(17 + idx_array_sz),	/* Length */
		tpm_u32(TPM2_CC_PCR_READ),	/* Command code */

		/* TPML_PCR_SELECTION */
		tpm_u32(1),			/* Number of selections */
		tpm_u16(algorithm),		/* Algorithm of the hash */
		idx_array_sz,			/* Array size for selection */
		/* bitmap(idx)			   Selected PCR bitmap */
	};
	size_t response_len = COMMAND_BUFFER_SIZE;
	u8 response[COMMAND_BUFFER_SIZE];
	unsigned int pcr_sel_idx = idx / 8;
	u8 pcr_sel_bit = BIT(idx % 8);
	unsigned int counter = 0;
	int ret;

	if (pack_byte_string(command_v2, COMMAND_BUFFER_SIZE, "b",
			     17 + pcr_sel_idx, pcr_sel_bit))
		return TPM_LIB_ERROR;

	ret = tpm_sendrecv_command(dev, command_v2, response, &response_len);
	if (ret)
		return ret;

	if (digest_len > response_len)
		return TPM_LIB_ERROR;

	if (unpack_byte_string(response, response_len, "ds",
			       10, &counter,
			       response_len - digest_len, data,
			       digest_len))
		return TPM_LIB_ERROR;

	if (updates)
		*updates = counter;

	return 0;
}

u32 tpm2_get_capability(struct udevice *dev, u32 capability, u32 property,
			void *buf, size_t prop_count)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),		/* TAG */
		tpm_u32(22),				/* Length */
		tpm_u32(TPM2_CC_GET_CAPABILITY),	/* Command code */

		tpm_u32(capability),			/* Capability */
		tpm_u32(property),			/* Property */
		tpm_u32(prop_count),			/* Property count */
	};
	u8 response[COMMAND_BUFFER_SIZE];
	size_t response_len = COMMAND_BUFFER_SIZE;
	unsigned int properties_off;
	int ret;

	ret = tpm_sendrecv_command(dev, command_v2, response, &response_len);
	if (ret)
		return ret;

	/*
	 * In the response buffer, the properties are located after the:
	 * tag (u16), response size (u32), response code (u32),
	 * YES/NO flag (u8), TPM_CAP (u32).
	 */
	properties_off = sizeof(u16) + sizeof(u32) + sizeof(u32) +
			 sizeof(u8) + sizeof(u32);
	memcpy(buf, &response[properties_off], response_len - properties_off);

	return 0;
}

u32 tpm2_pcr_config_algo(struct udevice *dev, u32 algo_mask,
			 struct tpml_pcr_selection *pcr, u32 *pcr_len)
{
	int i;

	if (pcr->count > TPM2_NUM_PCR_BANKS)
		return TPM_LIB_ERROR;

	*pcr_len = sizeof(pcr->count);

	for (i = 0; i < pcr->count; i++) {
		struct tpms_pcr_selection *sel = &pcr->selection[i];
		u8 pad = 0;
		int j;

		if (sel->size_of_select > TPM2_PCR_SELECT_MAX)
			return TPM_LIB_ERROR;

		/*
		 * Found the algorithm (bank) that matches, and enable all PCR
		 * bits.
		 * TODO: only select the bits needed
		 */
		for (j = 0; j < ARRAY_SIZE(hash_algo_list); j++) {
			if (hash_algo_list[j].hash_alg != sel->hash)
				continue;

			if (algo_mask & hash_algo_list[j].hash_mask)
				pad = 0xff;
		}

		for (j = 0; j < sel->size_of_select; j++)
			sel->pcr_select[j] = pad;

		log_info("set bank[%d] %s %s\n", i,
			 tpm2_algorithm_name(sel->hash),
			 tpm2_is_active_bank(sel) ? "on" : "off");

		*pcr_len += sizeof(sel->hash) + sizeof(sel->size_of_select) +
			    sel->size_of_select;
	}

	return 0;
}

u32 tpm2_send_pcr_allocate(struct udevice *dev, const char *pw,
			   const ssize_t pw_sz, struct tpml_pcr_selection *pcr,
			   u32 pcr_len)
{
	/* Length of the message header, up to start of password */
	uint offset = 27;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),   /* TAG */
		tpm_u32(offset + pw_sz + pcr_len), /* Length */
		tpm_u32(TPM2_CC_PCR_ALLOCATE),  /* Command code */

		/* handles 4 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Primary platform seed */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* TPML_PCR_SELECTION */
	};
	u8 response[COMMAND_BUFFER_SIZE];
	size_t response_len = COMMAND_BUFFER_SIZE;
	u32 i;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 * the password (if any)
	 */
	if (pack_byte_string(command_v2, sizeof(command_v2), "s", offset, pw,
			     pw_sz))
		return TPM_LIB_ERROR;

	offset += pw_sz;

	/* Pack the count field */
	if (pack_byte_string(command_v2, sizeof(command_v2), "d", offset, pcr->count))
		return TPM_LIB_ERROR;

	offset += sizeof(pcr->count);

	/* Pack each tpms_pcr_selection */
	for (i = 0; i < pcr->count; i++) {
		struct tpms_pcr_selection *sel = &pcr->selection[i];

		/* Pack hash (16-bit) */
		if (pack_byte_string(command_v2, sizeof(command_v2), "w", offset,
				     sel->hash))
			return TPM_LIB_ERROR;

		offset += sizeof(sel->hash);

		/* Pack size_of_select (8-bit) */
		if (pack_byte_string(command_v2, sizeof(command_v2), "b", offset,
				     sel->size_of_select))
			return TPM_LIB_ERROR;

		offset += sizeof(sel->size_of_select);

		/* Pack pcr_select array */
		if (pack_byte_string(command_v2, sizeof(command_v2), "s", offset,
				     sel->pcr_select, sel->size_of_select))
			return TPM_LIB_ERROR;

		offset += sel->size_of_select;
	}

	ret = tpm_sendrecv_command(dev, command_v2, response, &response_len);
	if (!ret)
		tpm_init(dev);

	return ret;
}

static int tpm2_get_num_pcr(struct udevice *dev, u32 *num_pcr)
{
	u8 response[(sizeof(struct tpms_capability_data) -
		offsetof(struct tpms_capability_data, data))];
	u32 properties_offset =
		offsetof(struct tpml_tagged_tpm_property, tpm_property) +
		offsetof(struct tpms_tagged_property, value);
	u32 ret;

	memset(response, 0, sizeof(response));
	ret = tpm2_get_capability(dev, TPM2_CAP_TPM_PROPERTIES,
				  TPM2_PT_PCR_COUNT, response, 1);
	if (ret)
		return ret;

	*num_pcr = get_unaligned_be32(response + properties_offset);
	if (*num_pcr > TPM2_MAX_PCRS) {
		printf("%s: too many pcrs: %u\n", __func__, *num_pcr);
		return -E2BIG;
	}

	return 0;
}

int tpm2_get_pcr_info(struct udevice *dev, struct tpml_pcr_selection *pcrs)
{
	u8 response[(sizeof(struct tpms_capability_data) -
		offsetof(struct tpms_capability_data, data))];
	u32 num_pcr;
	size_t i;
	u32 ret;

	ret = tpm2_get_capability(dev, TPM2_CAP_PCRS, 0, response, 1);
	if (ret)
		return ret;

	pcrs->count = get_unaligned_be32(response);
	/*
	 * We only support 4 algorithms for now so check against that
	 * instead of TPM2_NUM_PCR_BANKS
	 */
	if (pcrs->count > 4 || pcrs->count < 1) {
		printf("%s: too many pcrs: %u\n", __func__, pcrs->count);
		return -EMSGSIZE;
	}

	ret = tpm2_get_num_pcr(dev, &num_pcr);
	if (ret)
		return ret;

	for (i = 0; i < pcrs->count; i++) {
		/*
		 * Definition of TPMS_PCR_SELECTION Structure
		 * hash: u16
		 * size_of_select: u8
		 * pcr_select: u8 array
		 *
		 * The offsets depend on the number of the device PCRs
		 * so we have to calculate them based on that
		 */
		u32 hash_offset = offsetof(struct tpml_pcr_selection, selection) +
			i * offsetof(struct tpms_pcr_selection, pcr_select) +
			i * ((num_pcr + 7) / 8);
		u32 size_select_offset =
			hash_offset + offsetof(struct tpms_pcr_selection,
					       size_of_select);
		u32 pcr_select_offset =
			hash_offset + offsetof(struct tpms_pcr_selection,
					       pcr_select);

		pcrs->selection[i].hash =
			get_unaligned_be16(response + hash_offset);
		pcrs->selection[i].size_of_select =
			__get_unaligned_be(response + size_select_offset);
		if (pcrs->selection[i].size_of_select > TPM2_PCR_SELECT_MAX) {
			printf("%s: pcrs selection too large: %u\n", __func__,
			       pcrs->selection[i].size_of_select);
			return -ENOBUFS;
		}
		/* copy the array of pcr_select */
		memcpy(pcrs->selection[i].pcr_select, response + pcr_select_offset,
		       pcrs->selection[i].size_of_select);
	}

	return 0;
}

u32 tpm2_dam_reset(struct udevice *dev, const char *pw, const ssize_t pw_sz)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(27 + pw_sz),		/* Length */
		tpm_u32(TPM2_CC_DAM_RESET),	/* Command code */

		/* HANDLE */
		tpm_u32(TPM2_RH_LOCKOUT),	/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "s",
			       offset, pw, pw_sz);
	offset += pw_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_dam_parameters(struct udevice *dev, const char *pw,
			const ssize_t pw_sz, unsigned int max_tries,
			unsigned int recovery_time,
			unsigned int lockout_recovery)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(27 + pw_sz + 12),	/* Length */
		tpm_u32(TPM2_CC_DAM_PARAMETERS), /* Command code */

		/* HANDLE */
		tpm_u32(TPM2_RH_LOCKOUT),	/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* LOCKOUT PARAMETERS */
		/* tpm_u32(max_tries)		   Max tries (0, always lock) */
		/* tpm_u32(recovery_time)	   Recovery time (0, no lock) */
		/* tpm_u32(lockout_recovery)	   Lockout recovery */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 *     - max tries
	 *     - recovery time
	 *     - lockout recovery
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sddd",
			       offset, pw, pw_sz,
			       offset + pw_sz, max_tries,
			       offset + pw_sz + 4, recovery_time,
			       offset + pw_sz + 8, lockout_recovery);
	offset += pw_sz + 12;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

int tpm2_change_auth(struct udevice *dev, u32 handle, const char *newpw,
		     const ssize_t newpw_sz, const char *oldpw,
		     const ssize_t oldpw_sz)
{
	unsigned int offset = 27;
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(offset + oldpw_sz + 2 + newpw_sz), /* Length */
		tpm_u32(TPM2_CC_HIERCHANGEAUTH), /* Command code */

		/* HANDLE */
		tpm_u32(handle),		/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + oldpw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* Session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(oldpw_sz)		/* Size of <hmac/password> */
		/* STRING(oldpw)		   <hmac/password> (if any) */

		/* TPM2B_AUTH (TPM2B_DIGEST) */
		/* tpm_u16(newpw_sz)		   Digest size, new pw length */
		/* STRING(newpw)		   Digest buffer, new pw */
	};
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the old password (if any)
	 *     - size of the new password
	 *     - new password
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sws",
			       offset, oldpw, oldpw_sz,
			       offset + oldpw_sz, newpw_sz,
			       offset + oldpw_sz + 2, newpw, newpw_sz);
	offset += oldpw_sz + 2 + newpw_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_setauthpolicy(struct udevice *dev, const char *pw,
			   const ssize_t pw_sz, u32 index, const char *key)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(35 + pw_sz + TPM2_DIGEST_LEN), /* Length */
		tpm_u32(TPM2_CC_PCR_SETAUTHPOL), /* Command code */

		/* HANDLE */
		tpm_u32(TPM2_RH_PLATFORM),	/* TPM resource handle */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz)			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* TPM2B_AUTH (TPM2B_DIGEST) */
		/* tpm_u16(TPM2_DIGEST_LEN)	   Digest size length */
		/* STRING(key)			   Digest buffer (PCR key) */

		/* TPMI_ALG_HASH */
		/* tpm_u16(TPM2_ALG_SHA256)   Algorithm of the hash */

		/* TPMI_DH_PCR */
		/* tpm_u32(index),		   PCR Index */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 *     - the PCR key length
	 *     - the PCR key
	 *     - the hash algorithm
	 *     - the PCR index
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "swswd",
			       offset, pw, pw_sz,
			       offset + pw_sz, TPM2_DIGEST_LEN,
			       offset + pw_sz + 2, key, TPM2_DIGEST_LEN,
			       offset + pw_sz + 2 + TPM2_DIGEST_LEN,
			       TPM2_ALG_SHA256,
			       offset + pw_sz + 4 + TPM2_DIGEST_LEN, index);
	offset += pw_sz + 2 + TPM2_DIGEST_LEN + 2 + 4;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_pcr_setauthvalue(struct udevice *dev, const char *pw,
			  const ssize_t pw_sz, u32 index, const char *key,
			  const ssize_t key_sz)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(33 + pw_sz + TPM2_DIGEST_LEN), /* Length */
		tpm_u32(TPM2_CC_PCR_SETAUTHVAL), /* Command code */

		/* HANDLE */
		tpm_u32(index),			/* Handle (PCR Index) */

		/* AUTH_SESSION */
		tpm_u32(9 + pw_sz),		/* Authorization size */
		tpm_u32(TPM2_RS_PW),		/* session handle */
		tpm_u16(0),			/* Size of <nonce> */
						/* <nonce> (if any) */
		0,				/* Attributes: Cont/Excl/Rst */
		tpm_u16(pw_sz),			/* Size of <hmac/password> */
		/* STRING(pw)			   <hmac/password> (if any) */

		/* TPM2B_DIGEST */
		/* tpm_u16(key_sz)		   Key length */
		/* STRING(key)			   Key */
	};
	unsigned int offset = 27;
	int ret;

	/*
	 * Fill the command structure starting from the first buffer:
	 *     - the password (if any)
	 *     - the number of digests, 1 in our case
	 *     - the algorithm, sha256 in our case
	 *     - the digest (64 bytes)
	 */
	ret = pack_byte_string(command_v2, sizeof(command_v2), "sws",
			       offset, pw, pw_sz,
			       offset + pw_sz, key_sz,
			       offset + pw_sz + 2, key, key_sz);
	offset += pw_sz + 2 + key_sz;
	if (ret)
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_get_random(struct udevice *dev, void *data, u32 count)
{
	const u8 command_v2[10] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(12),
		tpm_u32(TPM2_CC_GET_RANDOM),
	};
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];

	const size_t data_size_offset = 10;
	const size_t data_offset = 12;
	size_t response_length = sizeof(response);
	u32 data_size;
	u8 *out = data;

	while (count > 0) {
		u32 this_bytes = min((size_t)count,
				     sizeof(response) - data_offset);
		u32 err;

		if (pack_byte_string(buf, sizeof(buf), "sw",
				     0, command_v2, sizeof(command_v2),
				     sizeof(command_v2), this_bytes))
			return TPM_LIB_ERROR;
		err = tpm_sendrecv_command(dev, buf, response,
					   &response_length);
		if (err)
			return err;
		if (unpack_byte_string(response, response_length, "w",
				       data_size_offset, &data_size))
			return TPM_LIB_ERROR;
		if (data_size > this_bytes)
			return TPM_LIB_ERROR;
		if (unpack_byte_string(response, response_length, "s",
				       data_offset, out, data_size))
			return TPM_LIB_ERROR;

		count -= data_size;
		out += data_size;
	}

	return 0;
}

u32 tpm2_write_lock(struct udevice *dev, u32 index)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(10 + 8 + 13), /* Length */
		tpm_u32(TPM2_CC_NV_WRITELOCK), /* Command code */

		/* handles 8 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Primary platform seed */
		tpm_u32(HR_NV_INDEX + index),	/* Password authorisation */

		/* session header 9 bytes */
		tpm_u32(9),			/* Header size */
		tpm_u32(TPM2_RS_PW),		/* Password authorisation */
		tpm_u16(0),			/* nonce_size */
		0,				/* session_attrs */
		tpm_u16(0),			/* auth_size */
	};

	return tpm_sendrecv_command(dev, command_v2, NULL, NULL);
}

u32 tpm2_disable_platform_hierarchy(struct udevice *dev)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_SESSIONS),	/* TAG */
		tpm_u32(10 + 4 + 13 + 5),	/* Length */
		tpm_u32(TPM2_CC_HIER_CONTROL),	/* Command code */

		/* 4 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Primary platform seed */

		/* session header 9 bytes */
		tpm_u32(9),			/* Header size */
		tpm_u32(TPM2_RS_PW),		/* Password authorisation */
		tpm_u16(0),			/* nonce_size */
		0,				/* session_attrs */
		tpm_u16(0),			/* auth_size */

		/* payload 5 bytes */
		tpm_u32(TPM2_RH_PLATFORM),	/* Hierarchy to disable */
		0,				/* 0=disable */
	};
	int ret;

	ret = tpm_sendrecv_command(dev, command_v2, NULL, NULL);
	log_info("ret=%s, %x\n", dev->name, ret);
	if (ret)
		return ret;

	priv->plat_hier_disabled = true;

	return 0;
}

u32 tpm2_submit_command(struct udevice *dev, const u8 *sendbuf,
			u8 *recvbuf, size_t *recv_size)
{
	return tpm_sendrecv_command(dev, sendbuf, recvbuf, recv_size);
}

u32 tpm2_report_state(struct udevice *dev, uint vendor_cmd, uint vendor_subcmd,
		      u8 *recvbuf, size_t *recv_size)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_NO_SESSIONS),		/* TAG */
		tpm_u32(10 + 2),			/* Length */
		tpm_u32(vendor_cmd),	/* Command code */

		tpm_u16(vendor_subcmd),
	};
	int ret;

	ret = tpm_sendrecv_command(dev, command_v2, recvbuf, recv_size);
	log_debug("ret=%s, %x\n", dev->name, ret);
	if (ret)
		return ret;
	if (*recv_size < 12)
		return -ENODATA;
	*recv_size -= 12;
	memmove(recvbuf, recvbuf + 12, *recv_size);

	return 0;
}

u32 tpm2_enable_nvcommits(struct udevice *dev, uint vendor_cmd,
			  uint vendor_subcmd)
{
	u8 command_v2[COMMAND_BUFFER_SIZE] = {
		/* header 10 bytes */
		tpm_u16(TPM2_ST_NO_SESSIONS),		/* TAG */
		tpm_u32(10 + 2),			/* Length */
		tpm_u32(vendor_cmd),	/* Command code */

		tpm_u16(vendor_subcmd),
	};
	int ret;

	ret = tpm_sendrecv_command(dev, command_v2, NULL, NULL);
	log_debug("ret=%s, %x\n", dev->name, ret);
	if (ret)
		return ret;

	return 0;
}

bool tpm2_is_active_bank(struct tpms_pcr_selection *selection)
{
	int i;

	for (i = 0; i < selection->size_of_select; i++) {
		if (selection->pcr_select[i])
			return true;
	}

	return false;
}

enum tpm2_algorithms tpm2_name_to_algorithm(const char *name)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(hash_algo_list); ++i) {
		if (!strcasecmp(name, hash_algo_list[i].hash_name))
			return hash_algo_list[i].hash_alg;
	}
	printf("%s: unsupported algorithm %s\n", __func__, name);

	return TPM2_ALG_INVAL;
}

const char *tpm2_algorithm_name(enum tpm2_algorithms algo)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(hash_algo_list); ++i) {
		if (hash_algo_list[i].hash_alg == algo)
			return hash_algo_list[i].hash_name;
	}

	return "";
}

bool tpm2_algorithm_supported(enum tpm2_algorithms algo)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(hash_algo_list); ++i) {
		if (hash_algo_list[i].hash_alg == algo)
			return hash_algo_list[i].supported;
	}

	return false;
}

u16 tpm2_algorithm_to_len(enum tpm2_algorithms algo)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(hash_algo_list); ++i) {
		if (hash_algo_list[i].hash_alg == algo)
			return hash_algo_list[i].hash_len;
	}

	return 0;
}

bool tpm2_check_active_banks(struct udevice *dev)
{
	struct tpml_pcr_selection pcrs;
	size_t i;
	int rc;

	rc = tpm2_get_pcr_info(dev, &pcrs);
	if (rc)
		return false;

	for (i = 0; i < pcrs.count; i++) {
		if (tpm2_is_active_bank(&pcrs.selection[i]) &&
		    !tpm2_algorithm_supported(pcrs.selection[i].hash))
			return false;
	}

	return true;
}

void tpm2_print_active_banks(struct udevice *dev)
{
	struct tpml_pcr_selection pcrs;
	size_t i;
	int rc;

	rc = tpm2_get_pcr_info(dev, &pcrs);
	if (rc) {
		log_err("Can't retrieve active PCRs\n");
		return;
	}

	for (i = 0; i < pcrs.count; i++) {
		if (tpm2_is_active_bank(&pcrs.selection[i])) {
			const char *str;

			str = tpm2_algorithm_name(pcrs.selection[i].hash);
			if (str)
				log_info("%s\n", str);
		}
	}
}
