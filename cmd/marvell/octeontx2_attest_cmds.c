/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:    GPL-2.0
 * https://spdx.org/licenses
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <image.h>
#include <asm/arch/smc.h>
#include <asm/arch/board.h>
#include <asm/arch/smc-id.h>

static int get_fit_image_ids(const char *img_str,
			     sw_attestation_tlv_t **tlv_ptr, void *tlv_limit);

static void hexdump(const char *prefix, unsigned char *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			printf("%s%s%08lx: ", i ? "\n" : "",
			       prefix ? prefix : "",
			       (unsigned long)i /* i.e. offset */);
		printf("%02x ", buf[i]);
	}
	printf("\n");
}

static const char *tlv_type_name(enum sw_attestation_tlv_type type)
{
	static const char *tlv_types[ATT_TLV_TYPE_COUNT] = {
		[ATT_IMG_INIT_BIN] = "ATT_IMG_INIT_BIN",
		[ATT_IMG_ATF_BL1] = "ATT_IMG_ATF_BL1",
		[ATT_IMG_BOARD_DT] = "ATT_IMG_BOARD_DT",
		[ATT_IMG_LINUX_DT] = "ATT_IMG_LINUX_DT",
		[ATT_IMG_SCP_TBL1FW] = "ATT_IMG_SCP_TBL1FW",
		[ATT_IMG_MCP_TBL1FW] = "ATT_IMG_MCP_TBL1FW",
		[ATT_IMG_AP_TBL1FW] = "ATT_IMG_AP_TBL1FW",
		[ATT_IMG_ATF_BL2] = "ATT_IMG_ATF_BL2",
		[ATT_IMG_ATF_BL31] = "ATT_IMG_ATF_BL31",
		[ATT_IMG_ATF_BL33] = "ATT_IMG_ATF_BL33",
		[ATT_SIG_NONCE] = "ATT_SIG_NONCE",
		[ATT_IMG_FIT_KERNEL] = "ATT_IMG_FIT_KERNEL",
	};
	if (type < 0 || type >= ATT_TLV_TYPE_COUNT)
		return "Unknown";
	else
		return tlv_types[type];
}

static int do_attest(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	ssize_t len;
	uintptr_t attest_ret;
	const char *img_str;
	sw_attestation_info_hdr_t *att_info;
	sw_attestation_tlv_t *tlv;
	void *tlv_limit, *next_tlv, *cert, *sig;
	unsigned long nonce_len;

	if (argc < 2) {
		printf("Please specify nonce string & optional FIT address.\n");
		return CMD_RET_USAGE;
	}

	nonce_len = strlen(argv[1]);
	if (nonce_len > SW_ATT_INFO_NONCE_MAX_LEN) {
		printf("Input nonce too large (%lu vs %u)\n",
		       nonce_len, SW_ATT_INFO_NONCE_MAX_LEN);
		return CMD_RET_USAGE;
	}

	if (argc > 2)
		img_str = argv[2];
	else
		img_str = NULL;

	/* query for buffer address & len */
	attest_ret = smc_attest(OCTEONTX_ATTESTATION_QUERY_SUBCMD_BUFFER, 0);

	if ((ssize_t)attest_ret <= 0) {
		printf("Error: unable to obtain buffer address.\n");
		return CMD_RET_FAILURE;
	}

	att_info = (void *)attest_ret;

	if (ntohl(att_info->magic_be) != ATTESTATION_MAGIC_ID) {
		printf("Error: invalid buffer magic ID.\n");
		return CMD_RET_FAILURE;
	}

	tlv = att_info->tlv_list;
	tlv_limit = (void *)tlv + (long)(ntohs(att_info->tlv_len_be));

	/* Pass nonce data & optional FIT image data to service */
	len = nonce_len;
	tlv->type_be = htons(ATT_SIG_NONCE);
	tlv->length_be = htons(len);
	memcpy(tlv->value, argv[1], len);
	tlv = (sw_attestation_tlv_t *)&tlv->value[len];

	if (img_str) {
		if (get_fit_image_ids(img_str, &tlv, tlv_limit) != 0) {
			printf("Error parsing FIT image attestation IDs\n");
			return CMD_RET_FAILURE;
		}
	}

	len = (uintptr_t)tlv - (uintptr_t)att_info->tlv_list;

	attest_ret = smc_attest(OCTEONTX_ATTESTATION_QUERY_SUBCMD_INFO, len);

	if ((ssize_t)attest_ret != 0) {
		printf("Error %ld from attest command\n", attest_ret);
		return CMD_RET_FAILURE;
	}

	printf("Attestation decode:\n");
	printf("\tMagic: %08x\n", ntohl(att_info->magic_be));
	tlv = att_info->tlv_list;
	tlv_limit = (void *)tlv + (long)(ntohs(att_info->tlv_len_be));
	while ((uintptr_t)tlv < (uintptr_t)tlv_limit) {
		next_tlv = (void *)tlv + ntohs(tlv->length_be) +
			   sizeof(*tlv);
		if ((uintptr_t)next_tlv > (uintptr_t)tlv_limit) {
			printf("Error: invalid TLV construct\n");
			break;
		}
		printf("\t%s:\n", tlv_type_name(ntohs(tlv->type_be)));
		hexdump("\t   ", tlv->value, ntohs(tlv->length_be));
		tlv = (sw_attestation_tlv_t *)next_tlv;
	}

	/* command returned successfully, but with malformed data */
	if (tlv != tlv_limit)
		return CMD_RET_SUCCESS;

	/* Certificate is contiguous to TLV date */
	cert = tlv_limit;
	printf("\tCertificate:\n");
	hexdump("\t   ", (unsigned char *)cert,
		ntohs(att_info->certificate_len_be));

	len = ntohs(att_info->total_len_be);
	len -= ntohs(att_info->signature_len_be);
	printf("\tAttestation [Signed] Image:\n");
	hexdump("\t   ", (unsigned char *)att_info, len);

	/* Authentication signature is contiguous to certificate */
	sig = (void *)cert + ntohs(att_info->certificate_len_be);
	printf("\tAuthentication signature:\n");
	hexdump("\t   ", sig, htons(att_info->signature_len_be));

	return CMD_RET_SUCCESS;
}

/*
 * get_fit_image_ids()
 *
 * This extracts the kernel image hash from within a FIT image and
 * adds it as a TLV entry to be submitted to ATF as part of the
 * Software Attestation information.
 *
 * on entry,
 *   img_str:   FIT image specifier; this supports syntax <addr>:kernel@x.
 *              If not kernel is specified, the first one will be used.
 *   tlv_ptr:   address of TLV entries to be filled-in
 *   tlv_limit: limit of TLV data
 *
 * returns,
 *   0 upon success
 *   -1 upon error
 */
static int get_fit_image_ids(const char *img_str,
			     sw_attestation_tlv_t **tlv_ptr, void *tlv_limit)
{
	int images_noffset, ndepth, count, ret, len, fit_value_len;
	int subnoffset, noffset;
	u8 img_type;
	u8 *fit_value;
	void *fit_hdr;
	const char *name, *img_name;
	uintptr_t img_addr;
	sw_attestation_tlv_t *tlv;

	if (!tlv_ptr)
		return -1;

	tlv = *tlv_ptr;

	/* default to error */
	ret = -1;

	img_addr = simple_strtoul(img_str, NULL, 16);
	fit_parse_subimage(img_str, img_addr, &img_addr, &img_name);
	fit_hdr = (void *)img_addr;

	if ((genimg_get_format(fit_hdr) != IMAGE_FORMAT_FIT) ||
	    !fit_check_format(fit_hdr))
		return ret;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit_hdr, FIT_IMAGES_PATH);
	if (images_noffset < 0)
		return ret;

	for (ndepth = 0, count = 0,
	     noffset = fdt_next_node(fit_hdr, images_noffset, &ndepth);
			(noffset >= 0) && (ndepth > 0);
			noffset = img_name ? -1 :
				  fdt_next_node(fit_hdr, noffset, &ndepth)) {
		/* if image was specified, use its offset */
		if (img_name) {
			noffset = fdt_subnode_offset(fit_hdr, images_noffset,
						     img_name);
			if (noffset < 0) {
				printf("Unable to locate '%s'...\n", img_name);
				continue;
			}
			ndepth = 1;
		}

		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			count++;

			fit_image_get_type(fit_hdr, noffset, &img_type);
			if (img_type != IH_TYPE_KERNEL) {
				printf("img type not KERNEL (%d)\n", img_type);
				continue;
			}

			/* locate the HASH for this image */
			fdt_for_each_subnode(subnoffset, fit_hdr, noffset) {
				name = fit_get_name(fit_hdr, subnoffset, NULL);
				if (strncmp(name, FIT_HASH_NODENAME,
					    strlen(FIT_HASH_NODENAME)))
					continue;

				if (fit_image_hash_get_value(fit_hdr,
							     subnoffset,
							     &fit_value,
							     &fit_value_len))
					continue;

				len = fit_value_len;
				tlv->type_be = htons(ATT_IMG_FIT_KERNEL);
				tlv->length_be = htons(len);
				if (((uintptr_t)tlv + len) > (uintptr_t)
				    tlv_limit)
					break;

				memcpy(tlv->value, fit_value, len);
				tlv = (sw_attestation_tlv_t *)&tlv->value[len];
				ret = 0;
				break;
			}
			break;
		}
	}

	*tlv_ptr = tlv;

	return ret;
}

U_BOOT_CMD(attest, 3, 0, do_attest,
	   "Retrieve attestation information",
	   "<nonce_string> [<FIT_addr>]\n"
	   "    - <nonce_string> consists of ASCII characters.\n"
	   "    - <FIT_addr> specifies FIT image containing Linux kernel.\n"
	   "      This supports the FIT syntax <address:kernel@x>.\n"
	   "\nExample: attest ABCDEF0123\n"
);
