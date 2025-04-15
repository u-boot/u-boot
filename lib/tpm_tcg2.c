// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Limited
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
#include <bloblist.h>

int tcg2_get_pcr_info(struct udevice *dev, u32 *supported_bank, u32 *active_bank,
		      u32 *bank_num)
{
	struct tpml_pcr_selection pcrs;
	size_t i;
	u32 ret;

	*supported_bank = 0;
	*active_bank = 0;
	*bank_num = 0;

	ret = tpm2_get_pcr_info(dev, &pcrs);
	if (ret)
		return ret;

	for (i = 0; i < pcrs.count; i++) {
		struct tpms_pcr_selection *sel = &pcrs.selection[i];
		u32 hash_mask = tcg2_algorithm_to_mask(sel->hash);

		if (tpm2_algorithm_supported(sel->hash))
			*supported_bank |= hash_mask;
		else
			log_warning("%s: unknown algorithm %x\n", __func__,
				    sel->hash);

		if (tpm2_is_active_bank(sel))
			*active_bank |= hash_mask;
	}

	*bank_num = pcrs.count;

	return 0;
}

int tcg2_get_active_pcr_banks(struct udevice *dev, u32 *active_pcr_banks)
{
	u32 supported = 0;
	u32 pcr_banks = 0;
	u32 active = 0;
	int rc;

	rc = tcg2_get_pcr_info(dev, &supported, &active, &pcr_banks);
	if (rc)
		return rc;

	*active_pcr_banks = active;

	return 0;
}

u32 tcg2_event_get_size(struct tpml_digest_values *digest_list)
{
	u32 len;
	size_t i;

	len = offsetof(struct tcg_pcr_event2, digests);
	len += offsetof(struct tpml_digest_values, digests);
	for (i = 0; i < digest_list->count; ++i) {
		u16 l = tpm2_algorithm_to_len(digest_list->digests[i].hash_alg);

		if (!l)
			continue;

		len += l + offsetof(struct tpmt_ha, digest);
	}
	len += sizeof(u32);

	return len;
}

int tcg2_create_digest(struct udevice *dev, const u8 *input, u32 length,
		       struct tpml_digest_values *digest_list)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	u8 final[sizeof(union tpmu_ha)];
#if IS_ENABLED(CONFIG_SHA256)
	sha256_context ctx_256;
#endif
#if IS_ENABLED(CONFIG_SHA512)
	sha512_context ctx_512;
#endif
#if IS_ENABLED(CONFIG_SHA1)
	sha1_context ctx;
#endif
	size_t i;
	u32 len;

	digest_list->count = 0;
	for (i = 0; i < priv->active_bank_count; i++) {

		switch (priv->active_banks[i]) {
#if IS_ENABLED(CONFIG_SHA1)
		case TPM2_ALG_SHA1:
			sha1_starts(&ctx);
			sha1_update(&ctx, input, length);
			sha1_finish(&ctx, final);
			len = TPM2_SHA1_DIGEST_SIZE;
			break;
#endif
#if IS_ENABLED(CONFIG_SHA256)
		case TPM2_ALG_SHA256:
			sha256_starts(&ctx_256);
			sha256_update(&ctx_256, input, length);
			sha256_finish(&ctx_256, final);
			len = TPM2_SHA256_DIGEST_SIZE;
			break;
#endif
#if IS_ENABLED(CONFIG_SHA384)
		case TPM2_ALG_SHA384:
			sha384_starts(&ctx_512);
			sha384_update(&ctx_512, input, length);
			sha384_finish(&ctx_512, final);
			len = TPM2_SHA384_DIGEST_SIZE;
			break;
#endif
#if IS_ENABLED(CONFIG_SHA512)
		case TPM2_ALG_SHA512:
			sha512_starts(&ctx_512);
			sha512_update(&ctx_512, input, length);
			sha512_finish(&ctx_512, final);
			len = TPM2_SHA512_DIGEST_SIZE;
			break;
#endif
		default:
			printf("%s: unsupported algorithm %x\n", __func__,
			       priv->active_banks[i]);
			continue;
		}

		digest_list->digests[digest_list->count].hash_alg =
			priv->active_banks[i];
		memcpy(&digest_list->digests[digest_list->count].digest, final,
		       len);
		digest_list->count++;
	}

	return 0;
}

void tcg2_log_append(u32 pcr_index, u32 event_type,
		     struct tpml_digest_values *digest_list, u32 size,
		     const u8 *event, u8 *log)
{
	size_t len;
	size_t pos;
	u32 i;

	pos = offsetof(struct tcg_pcr_event2, pcr_index);
	put_unaligned_le32(pcr_index, log);
	pos = offsetof(struct tcg_pcr_event2, event_type);
	put_unaligned_le32(event_type, log + pos);
	pos = offsetof(struct tcg_pcr_event2, digests) +
		offsetof(struct tpml_digest_values, count);
	put_unaligned_le32(digest_list->count, log + pos);

	pos = offsetof(struct tcg_pcr_event2, digests) +
		offsetof(struct tpml_digest_values, digests);
	for (i = 0; i < digest_list->count; ++i) {
		u16 hash_alg = digest_list->digests[i].hash_alg;

		len = tpm2_algorithm_to_len(hash_alg);
		if (!len)
			continue;

		pos += offsetof(struct tpmt_ha, hash_alg);
		put_unaligned_le16(hash_alg, log + pos);
		pos += offsetof(struct tpmt_ha, digest);
		memcpy(log + pos, (u8 *)&digest_list->digests[i].digest, len);
		pos += len;
	}

	put_unaligned_le32(size, log + pos);
	pos += sizeof(u32);
	memcpy(log + pos, event, size);
}

static int tcg2_log_append_check(struct tcg2_event_log *elog, u32 pcr_index,
				 u32 event_type,
				 struct tpml_digest_values *digest_list,
				 u32 size, const u8 *event)
{
	u32 event_size;
	u8 *log;

	event_size = size + tcg2_event_get_size(digest_list);
	if (elog->log_position + event_size > elog->log_size) {
		printf("%s: log too large: %u + %u > %u\n", __func__,
		       elog->log_position, event_size, elog->log_size);
		return -ENOBUFS;
	}

	log = elog->log + elog->log_position;
	elog->log_position += event_size;

	tcg2_log_append(pcr_index, event_type, digest_list, size, event, log);

	return 0;
}

static int tcg2_log_init(struct udevice *dev, struct tcg2_event_log *elog)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	struct tcg_efi_spec_id_event *ev;
	struct tcg_pcr_event *log;
	u32 event_size;
	u32 count = 0;
	u32 log_size;
	size_t i;
	u16 len;

	count = priv->active_bank_count;
	event_size = offsetof(struct tcg_efi_spec_id_event, digest_sizes);
	event_size += 1 +
		(sizeof(struct tcg_efi_spec_id_event_algorithm_size) * count);
	log_size = offsetof(struct tcg_pcr_event, event) + event_size;

	if (log_size > elog->log_size) {
		printf("%s: log too large: %u > %u\n", __func__, log_size,
		       elog->log_size);
		return -ENOBUFS;
	}

	log = (struct tcg_pcr_event *)elog->log;
	put_unaligned_le32(0, &log->pcr_index);
	put_unaligned_le32(EV_NO_ACTION, &log->event_type);
	memset(&log->digest, 0, sizeof(log->digest));
	put_unaligned_le32(event_size, &log->event_size);

	ev = (struct tcg_efi_spec_id_event *)log->event;
	strlcpy((char *)ev->signature, TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03,
		sizeof(ev->signature));
	put_unaligned_le32(0, &ev->platform_class);
	ev->spec_version_minor = TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MINOR_TPM2;
	ev->spec_version_major = TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MAJOR_TPM2;
	ev->spec_errata = TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_ERRATA_TPM2;
	ev->uintn_size = sizeof(size_t) / sizeof(u32);
	put_unaligned_le32(count, &ev->number_of_algorithms);

	for (i = 0; i < count; ++i) {
		len = tpm2_algorithm_to_len(priv->active_banks[i]);
		put_unaligned_le16(priv->active_banks[i],
				   &ev->digest_sizes[i].algorithm_id);
		put_unaligned_le16(len, &ev->digest_sizes[i].digest_size);
	}

	*((u8 *)ev + (event_size - 1)) = 0;
	elog->log_position = log_size;

	return 0;
}

static int tcg2_replay_eventlog(struct tcg2_event_log *elog,
				struct udevice *dev,
				struct tpml_digest_values *digest_list,
				u32 log_position)
{
	const u32 offset = offsetof(struct tcg_pcr_event2, digests) +
		offsetof(struct tpml_digest_values, digests);
	u32 event_size;
	u32 count;
	u16 algo;
	u32 pcr;
	u32 pos;
	u16 len;
	u8 *log;
	int rc;
	u32 i;

	while (log_position + offset < elog->log_size) {
		log = elog->log + log_position;

		pos = offsetof(struct tcg_pcr_event2, pcr_index);
		pcr = get_unaligned_le32(log + pos);
		pos = offsetof(struct tcg_pcr_event2, event_type);
		if (!get_unaligned_le32(log + pos))
			return 0;

		pos = offsetof(struct tcg_pcr_event2, digests) +
			offsetof(struct tpml_digest_values, count);
		count = get_unaligned_le32(log + pos);
		if (count > ARRAY_SIZE(hash_algo_list) ||
		    (digest_list->count && digest_list->count != count))
			return 0;

		pos = offsetof(struct tcg_pcr_event2, digests) +
			offsetof(struct tpml_digest_values, digests);
		for (i = 0; i < count; ++i) {
			pos += offsetof(struct tpmt_ha, hash_alg);
			if (log_position + pos + sizeof(u16) >= elog->log_size)
				return 0;

			algo = get_unaligned_le16(log + pos);
			pos += offsetof(struct tpmt_ha, digest);
			switch (algo) {
			case TPM2_ALG_SHA1:
			case TPM2_ALG_SHA256:
			case TPM2_ALG_SHA384:
			case TPM2_ALG_SHA512:
				len = tpm2_algorithm_to_len(algo);
				break;
			default:
				return 0;
			}

			if (digest_list->count) {
				if (algo != digest_list->digests[i].hash_alg ||
				    log_position + pos + len >= elog->log_size)
					return 0;

				memcpy(digest_list->digests[i].digest.sha512,
				       log + pos, len);
			}

			pos += len;
		}

		if (log_position + pos + sizeof(u32) >= elog->log_size)
			return 0;

		event_size = get_unaligned_le32(log + pos);
		pos += event_size + sizeof(u32);
		if (log_position + pos > elog->log_size)
			return 0;

		if (digest_list->count) {
			rc = tcg2_pcr_extend(dev, pcr, digest_list);
			if (rc)
				return rc;
		}

		log_position += pos;
	}

	elog->log_position = log_position;
	elog->found = true;
	return 0;
}

static int tcg2_log_parse(struct udevice *dev, struct tcg2_event_log *elog,
			  u32 *log_active)
{
	struct tpml_digest_values digest_list;
	struct tcg_efi_spec_id_event *event;
	struct tcg_pcr_event *log;
	u32 calc_size;
	u32 active;
	u32 count;
	u32 evsz;
	u32 mask;
	u16 algo;
	u16 len;
	int rc;
	u32 i;

	*log_active = 0;

	if (elog->log_size <= offsetof(struct tcg_pcr_event, event))
		return 0;

	log = (struct tcg_pcr_event *)elog->log;
	if (get_unaligned_le32(&log->pcr_index) != 0 ||
	    get_unaligned_le32(&log->event_type) != EV_NO_ACTION)
		return 0;

	for (i = 0; i < sizeof(log->digest); i++) {
		if (log->digest[i])
			return 0;
	}

	evsz = get_unaligned_le32(&log->event_size);
	if (evsz < offsetof(struct tcg_efi_spec_id_event, digest_sizes) ||
	    evsz + offsetof(struct tcg_pcr_event, event) > elog->log_size)
		return 0;

	event = (struct tcg_efi_spec_id_event *)log->event;
	if (memcmp(event->signature, TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03,
		   sizeof(TCG_EFI_SPEC_ID_EVENT_SIGNATURE_03)))
		return 0;

	if (event->spec_version_minor != TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MINOR_TPM2 ||
	    event->spec_version_major != TCG_EFI_SPEC_ID_EVENT_SPEC_VERSION_MAJOR_TPM2)
		return 0;

	count = get_unaligned_le32(&event->number_of_algorithms);
	if (count > ARRAY_SIZE(hash_algo_list))
		return 0;

	calc_size = offsetof(struct tcg_efi_spec_id_event, digest_sizes) +
		(sizeof(struct tcg_efi_spec_id_event_algorithm_size) * count) +
		1;
	if (evsz != calc_size)
		return 0;

	/*
	 * Go through the algorithms the EventLog contains.  If the EventLog
	 * algorithms don't match the active TPM ones exit and report the
	 * erroneous banks.
	 * We've already checked that U-Boot supports all the enabled TPM
	 * algorithms, so just check the EvenLog against the TPM active ones.
	 */
	digest_list.count = 0;
	for (i = 0; i < count; ++i) {
		algo = get_unaligned_le16(&event->digest_sizes[i].algorithm_id);
		mask = tcg2_algorithm_to_mask(algo);

		switch (algo) {
		case TPM2_ALG_SHA1:
		case TPM2_ALG_SHA256:
		case TPM2_ALG_SHA384:
		case TPM2_ALG_SHA512:
			len = get_unaligned_le16(&event->digest_sizes[i].digest_size);
			if (tpm2_algorithm_to_len(algo) != len) {
				log_err("EventLog invalid algorithm length\n");
				return -1;
			}
			digest_list.digests[digest_list.count++].hash_alg = algo;
			break;
		default:
			/*
			 * We can ignore this if the TPM PCRs is not extended
			 * by the previous bootloader. But for now just exit
			 */
			log_err("EventLog has unsupported algorithm 0x%x\n",
				algo);
			return -1;
		}
		*log_active |= mask;
	}

	rc = tcg2_get_active_pcr_banks(dev, &active);
	if (rc)
		return rc;
	/* If the EventLog and active algorithms don't match exit */
	if (*log_active != active)
		return -ERESTARTSYS;

	/* Read PCR0 to check if previous firmware extended the PCRs or not. */
	rc = tcg2_pcr_read(dev, 0, &digest_list);
	if (rc)
		return rc;

	for (i = 0; i < digest_list.count; ++i) {
		u8 hash_buf[TPM2_SHA512_DIGEST_SIZE] = { 0 };
		u16 hash_alg = digest_list.digests[i].hash_alg;

		if (memcmp((u8 *)&digest_list.digests[i].digest, hash_buf,
			   tpm2_algorithm_to_len(hash_alg)))
			digest_list.count = 0;

	}

	return tcg2_replay_eventlog(elog, dev, &digest_list,
				    offsetof(struct tcg_pcr_event, event) +
				    evsz);
}

int tcg2_pcr_extend(struct udevice *dev, u32 pcr_index,
		    struct tpml_digest_values *digest_list)
{
	u32 rc;
	u32 i;

	for (i = 0; i < digest_list->count; i++) {
		u32 alg = digest_list->digests[i].hash_alg;

		rc = tpm2_pcr_extend(dev, pcr_index, alg,
				     (u8 *)&digest_list->digests[i].digest,
				     tpm2_algorithm_to_len(alg));
		if (rc) {
			printf("%s: error pcr:%u alg:%08x\n", __func__,
			       pcr_index, alg);
			return rc;
		}
	}

	return 0;
}

int tcg2_pcr_read(struct udevice *dev, u32 pcr_index,
		  struct tpml_digest_values *digest_list)
{
	struct tpm_chip_priv *priv;
	u32 rc;
	u32 i;

	priv = dev_get_uclass_priv(dev);
	if (!priv)
		return -ENODEV;

	for (i = 0; i < digest_list->count; i++) {
		u32 alg = digest_list->digests[i].hash_alg;
		u8 *digest = (u8 *)&digest_list->digests[i].digest;

		rc = tpm2_pcr_read(dev, pcr_index, priv->pcr_select_min, alg,
				   digest, tpm2_algorithm_to_len(alg), NULL);
		if (rc) {
			printf("%s: error pcr:%u alg:%08x\n", __func__,
			       pcr_index, alg);
			return rc;
		}
	}

	return 0;
}

int tcg2_measure_data(struct udevice *dev, struct tcg2_event_log *elog,
		      u32 pcr_index, u32 size, const u8 *data, u32 event_type,
		      u32 event_size, const u8 *event)
{
	struct tpml_digest_values digest_list;
	int rc;

	if (data)
		rc = tcg2_create_digest(dev, data, size, &digest_list);
	else
		rc = tcg2_create_digest(dev, event, event_size, &digest_list);
	if (rc)
		return rc;

	rc = tcg2_pcr_extend(dev, pcr_index, &digest_list);
	if (rc)
		return rc;

	return tcg2_log_append_check(elog, pcr_index, event_type, &digest_list,
				     event_size, event);
}

int tcg2_log_prepare_buffer(struct udevice *dev, struct tcg2_event_log *elog,
			    bool ignore_existing_log)
{
	struct tcg2_event_log log;
	int rc;
	u32 log_active = 0;

	elog->log_position = 0;
	elog->found = false;

	rc = tcg2_platform_get_log(dev, (void **)&log.log, &log.log_size);
	if (!rc) {
		log.log_position = 0;
		log.found = false;

		if (!ignore_existing_log) {
			rc = tcg2_log_parse(dev, &log, &log_active);
			if (rc == -ERESTARTSYS && log_active)
				goto pcr_allocate;
			if (rc)
				return rc;
		}

		if (elog->log_size) {
			if (log.found) {
				if (elog->log_size < log.log_position)
					return -ENOBUFS;

				/*
				 * Copy the discovered log into the user buffer
				 * if there's enough space.
				 */
				memcpy(elog->log, log.log, log.log_position);
			}

			unmap_physmem(log.log, MAP_NOCACHE);
		} else {
			elog->log = log.log;
			elog->log_size = log.log_size;
		}

		elog->log_position = log.log_position;
		elog->found = log.found;
	}

pcr_allocate:
	rc = tpm2_activate_banks(dev, log_active);
	if (rc)
		return rc;

	/*
	 * Initialize the log buffer if no log was discovered and the buffer is
	 * valid. User's can pass in their own buffer as a fallback if no
	 * memory region is found.
	 */
	if (!elog->found && elog->log_size)
		rc = tcg2_log_init(dev, elog);

	return rc;
}

int tcg2_measurement_init(struct udevice **dev, struct tcg2_event_log *elog,
			  bool ignore_existing_log)
{
	int rc;

	rc = tcg2_platform_get_tpm2(dev);
	if (rc)
		return rc;

	rc = tpm_auto_start(*dev);
	if (rc)
		return rc;

	rc = tcg2_log_prepare_buffer(*dev, elog, ignore_existing_log);
	if (rc) {
		tcg2_measurement_term(*dev, elog, true);
		return rc;
	}

	rc = tcg2_measure_event(*dev, elog, 0, EV_S_CRTM_VERSION,
				strlen(version_string) + 1,
				(u8 *)version_string);
	if (rc) {
		tcg2_measurement_term(*dev, elog, true);
		return rc;
	}

	return 0;
}

void tcg2_measurement_term(struct udevice *dev, struct tcg2_event_log *elog,
			   bool error)
{
	u32 event = error ? 0x1 : 0xffffffff;
	int i;

	for (i = 0; i < 8; ++i)
		tcg2_measure_event(dev, elog, i, EV_SEPARATOR, sizeof(event),
				   (const u8 *)&event);

	if (elog->log)
		unmap_physmem(elog->log, MAP_NOCACHE);
}

__weak int tcg2_platform_get_log(struct udevice *dev, void **addr, u32 *size)
{
	const __be32 *addr_prop = NULL;
	const __be32 *size_prop = NULL;
	int asize;
	int ssize;
	struct ofnode_phandle_args args;
	phys_addr_t a;
	fdt_size_t s;

	*addr = NULL;
	*size = 0;

	*addr = bloblist_get_blob(BLOBLISTT_TPM_EVLOG, size);
	if (*addr && *size) {
		*addr = map_physmem((uintptr_t)(*addr), *size, MAP_NOCACHE);
		return 0;
	}

	/*
	 * TODO:
	 * Replace BLOBLIST with a new kconfig for handoff all components
	 * (fdt, tpm event log, etc...) from previous boot stage via bloblist
	 * mandatorily following Firmware Handoff spec.
	 */
	if (!CONFIG_IS_ENABLED(BLOBLIST)) {
		addr_prop = dev_read_prop(dev, "tpm_event_log_addr", &asize);
		size_prop = dev_read_prop(dev, "tpm_event_log_size", &ssize);
	}

	/*
	 * If no eventlog was observed, a sml buffer is required for the kernel
	 * to discover the eventlog.
	 */
	if (!addr_prop || !size_prop) {
		addr_prop = dev_read_prop(dev, "linux,sml-base", &asize);
		size_prop = dev_read_prop(dev, "linux,sml-size", &ssize);
	}

	if (addr_prop && size_prop) {
		u64 a = of_read_number(addr_prop, asize / sizeof(__be32));
		u64 s = of_read_number(size_prop, ssize / sizeof(__be32));

		*addr = map_physmem(a, s, MAP_NOCACHE);
		*size = (u32)s;

		return 0;
	}

	if (dev_read_phandle_with_args(dev, "memory-region", NULL, 0, 0, &args))
		return -ENODEV;

	a = ofnode_get_addr_size(args.node, "reg", &s);
	if (a == FDT_ADDR_T_NONE)
		return -ENOMEM;

	*addr = map_physmem(a, s, MAP_NOCACHE);
	*size = (u32)s;

	return 0;
}

__weak int tcg2_platform_get_tpm2(struct udevice **dev)
{
	for_each_tpm_device(*dev) {
		if (tpm_get_version(*dev) == TPM_V2)
			return 0;
	}

	return -ENODEV;
}

u32 tcg2_algorithm_to_mask(enum tpm2_algorithms algo)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(hash_algo_list); i++) {
		if (hash_algo_list[i].hash_alg == algo)
			return hash_algo_list[i].hash_mask;
	}

	return 0;
}

__weak void tcg2_platform_startup_error(struct udevice *dev, int rc) {}
