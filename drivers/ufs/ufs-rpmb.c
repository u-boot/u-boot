// SPDX-License-Identifier: GPL-2.0+
#include <dm.h>
#include <hexdump.h>
#include <log.h>
#include <malloc.h>
#include <scsi.h>
#include <ufs.h>
#include <vsprintf.h>
#include <asm/cache.h>
#include <asm/unaligned.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <u-boot/blake2.h>
#include "ufs.h"

#define RPMB_REQ_KEY		1
#define RPMB_REQ_WCOUNTER	2
#define RPMB_REQ_WRITE_DATA	3
#define RPMB_REQ_READ_DATA	4
#define RPMB_REQ_STATUS		5

#define RPMB_FRAME_SIZE		512
#define RPMB_FRAME_REQ_OFFSET	510

#define UFS_RPMB_UA_RETRIES	3

/* ASC 29h: POWER ON, RESET, OR BUS DEVICE RESET OCCURRED (any ASCQ) */
#define UFS_RPMB_UA_ASC_POR	0x29

#define SEC_PROTOCOL_UFS		0xEC
#define UFS_RPMB_SEC_PROTOCOL_ID	0x01

#define GEOMETRY_DESC_RPMB_RW_SIZE	0x17

#define RPMB_UNIT_DESC_LOGICAL_BLK_SIZE		0x0A
#define RPMB_UNIT_DESC_LOGICAL_BLK_COUNT	0x0B
#define RPMB_UNIT_DESC_REGION0_SIZE		0x13
#define RPMB_UNIT_DESC_REGION1_SIZE		0x14
#define RPMB_UNIT_DESC_REGION2_SIZE		0x15
#define RPMB_UNIT_DESC_REGION3_SIZE		0x16
#define UFS_RPMB_LEGACY_SPEC_VER		0x0220
#define UFS_RPMB_REGION_UNIT_SHIFT		17

static u8 ufs_rpmb_sense_key(const struct scsi_cmd *pccb)
{
	const u8 *sense = pccb->sense_buf;

	if (!pccb->sensedatalen)
		return SENSE_NO_SENSE;

	/* fixed (0x70/0x71) vs descriptor (0x72/0x73) sense format */
	if ((sense[0] & 0x7f) == 0x72 || (sense[0] & 0x7f) == 0x73)
		return sense[1] & 0xf;

	return sense[2] & 0xf;
}

static u8 ufs_rpmb_sense_asc(const struct scsi_cmd *pccb)
{
	const u8 *sense = pccb->sense_buf;

	if (!pccb->sensedatalen)
		return 0;

	/* fixed (0x70/0x71) vs descriptor (0x72/0x73) sense format */
	if ((sense[0] & 0x7f) == 0x72 || (sense[0] & 0x7f) == 0x73)
		return pccb->sensedatalen > 2 ? sense[2] : 0;

	return pccb->sensedatalen > 12 ? sense[12] : 0;
}

static int ufs_rpmb_secprot(struct udevice *scsi_dev, unsigned int region,
			    u8 opcode, void *buf, unsigned int nframes,
			    enum dma_data_direction dir)
{
	u16 spsp = (region << 8) | UFS_RPMB_SEC_PROTOCOL_ID;
	u32 len = nframes * RPMB_FRAME_SIZE;
	struct scsi_cmd pccb;
	void *dma_buf = buf;
	void *bounce = NULL;
	int retries;
	int ret = 0;

	if (!IS_ALIGNED((uintptr_t)buf, ARCH_DMA_MINALIGN)) {
		bounce = memalign(ARCH_DMA_MINALIGN,
				  ALIGN(len, ARCH_DMA_MINALIGN));
		if (!bounce)
			return -ENOMEM;
		dma_buf = bounce;
		if (dir == DMA_TO_DEVICE)
			memcpy(bounce, buf, len);
	}

	memset(&pccb, 0, sizeof(pccb));
	pccb.lun = UFS_UPIU_RPMB_WLUN;
	pccb.cmd[0] = opcode;
	pccb.cmd[1] = SEC_PROTOCOL_UFS;
	put_unaligned_be16(spsp, &pccb.cmd[2]);
	pccb.cmd[4] = 0;
	pccb.cmd[5] = 0;
	put_unaligned_be32(len, &pccb.cmd[6]);
	pccb.cmd[10] = 0;
	pccb.cmd[11] = 0;
	pccb.cmdlen = 12;
	pccb.pdata = dma_buf;
	pccb.datalen = len;
	pccb.dma_dir = dir;

	for (retries = UFS_RPMB_UA_RETRIES; retries > 0; retries--) {
		ret = scsi_exec(scsi_dev, &pccb);
		if (!ret)
			break;

		/*
		 * A power-on or reset UNIT ATTENTION (ASC 29h, any ASCQ) is a
		 * transient condition the device clears once it has been
		 * reported, so retry it. Any other failure - including other
		 * UNIT ATTENTION conditions such as 2Ah MODE PARAMETERS CHANGED
		 * - is final: stop and report it.
		 */
		if (ufs_rpmb_sense_key(&pccb) != SENSE_UNIT_ATTENTION ||
		    ufs_rpmb_sense_asc(&pccb) != UFS_RPMB_UA_ASC_POR)
			break;
	}

	if (bounce) {
		if (!ret && dir == DMA_FROM_DEVICE)
			memcpy(buf, bounce, len);

		free(bounce);
	}

	return ret;
}

static int ufs_rpmb_send(struct udevice *scsi_dev, unsigned int region,
			 void *frames, unsigned int nframes)
{
	return ufs_rpmb_secprot(scsi_dev, region, SCSI_SECURITY_PROTOCOL_OUT,
				frames, nframes, DMA_TO_DEVICE);
}

static int ufs_rpmb_recv(struct udevice *scsi_dev, unsigned int region,
			 void *frames, unsigned int nframes)
{
	return ufs_rpmb_secprot(scsi_dev, region, SCSI_SECURITY_PROTOCOL_IN,
				frames, nframes, DMA_FROM_DEVICE);
}

int ufs_rpmb_route_frames(struct udevice *scsi_dev, unsigned int region,
			  void *req, unsigned long reqlen, void *rsp,
			  unsigned long rsplen)
{
	unsigned int n_req = reqlen / RPMB_FRAME_SIZE;
	unsigned int n_rsp = rsplen / RPMB_FRAME_SIZE;
	u16 request;
	int ret;

	if (!scsi_dev || reqlen % RPMB_FRAME_SIZE ||
	    rsplen % RPMB_FRAME_SIZE || !n_req)
		return -EINVAL;

	request = get_unaligned_be16((u8 *)req + RPMB_FRAME_REQ_OFFSET);

	switch (request) {
	case RPMB_REQ_KEY:
	case RPMB_REQ_WRITE_DATA: {
		u8 status_frame[RPMB_FRAME_SIZE];

		ret = ufs_rpmb_send(scsi_dev, region, req, n_req);
		if (ret)
			return ret;

		memset(status_frame, 0, sizeof(status_frame));
		put_unaligned_be16(RPMB_REQ_STATUS,
				   &status_frame[RPMB_FRAME_REQ_OFFSET]);
		ret = ufs_rpmb_send(scsi_dev, region, status_frame, 1);
		if (ret)
			return ret;

		if (n_rsp < 1)
			return -EINVAL;

		return ufs_rpmb_recv(scsi_dev, region, rsp, 1);
	}
	case RPMB_REQ_WCOUNTER:
	case RPMB_REQ_READ_DATA:
		ret = ufs_rpmb_send(scsi_dev, region, req, 1);
		if (ret)
			return ret;

		if (n_rsp < 1)
			return -EINVAL;

		return ufs_rpmb_recv(scsi_dev, region, rsp, n_rsp);
	default:
		debug("ufs-rpmb: unsupported request 0x%x\n", request);
		return -EINVAL;
	}
}

static int ufs_rpmb_read_geometry(struct udevice *scsi_dev, u8 *rpmb_rw_size)
{
	struct ufs_hba *hba = dev_get_uclass_priv(scsi_dev->parent);
	u8 desc[QUERY_DESC_GEOMETRY_DEF_SIZE];
	int ret;

	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_GEOMETRY, 0, 0,
				     desc, sizeof(desc));
	if (ret)
		return ret;

	*rpmb_rw_size = desc[GEOMETRY_DESC_RPMB_RW_SIZE];

	return 0;
}

static int ufs_rpmb_read_region_sizes(struct ufs_hba *hba, u16 spec_ver,
				      u8 sizes[UFS_RPMB_NUM_REGIONS])
{
	u8 unit[QUERY_DESC_UNIT_DEF_SIZE] = { };
	int ret;

	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_UNIT,
				     UFS_UPIU_RPMB_WLUN, 0, unit, sizeof(unit));
	if (ret)
		return ret;

	memset(sizes, 0, UFS_RPMB_NUM_REGIONS);

	if (spec_ver > UFS_RPMB_LEGACY_SPEC_VER) {
		sizes[0] = unit[RPMB_UNIT_DESC_REGION0_SIZE];
		sizes[1] = unit[RPMB_UNIT_DESC_REGION1_SIZE];
		sizes[2] = unit[RPMB_UNIT_DESC_REGION2_SIZE];
		sizes[3] = unit[RPMB_UNIT_DESC_REGION3_SIZE];
	} else {
		u8 blk_shift = unit[RPMB_UNIT_DESC_LOGICAL_BLK_SIZE];
		u64 region;

		/*
		 * bLogicalBlockSize is a device-supplied log2 block size;
		 * reject values that would make the shift below undefined.
		 */
		if (blk_shift >= 64)
			return -EINVAL;

		region = get_unaligned_be64(unit +
					    RPMB_UNIT_DESC_LOGICAL_BLK_COUNT);
		region <<= blk_shift;
		region >>= UFS_RPMB_REGION_UNIT_SHIFT;

		sizes[0] = region > 0xff ? 0xff : region;
	}

	return 0;
}

static void ufs_rpmb_string_to_ascii(const u8 *raw, char *out, size_t outsz)
{
	int nchars = ((int)raw[QUERY_DESC_LENGTH_OFFSET] - QUERY_DESC_HDR_SIZE);
	int i, n = 0;

	nchars = nchars > 0 ? nchars / 2 : 0;
	for (i = 0; i < nchars && n < (int)outsz - 1; i++) {
		u16 c = get_unaligned_be16(raw + QUERY_DESC_HDR_SIZE + i * 2);

		out[n++] = (c >= 0x20 && c <= 0x7e) ? (char)c : ' ';
	}
	out[n] = '\0';
}

static int ufs_rpmb_build_cid(struct ufs_hba *hba, const u8 *dev_desc,
			      unsigned int region, u8 *cid)
{
	char serial_hex[QUERY_DESC_MAX_SIZE * 2 + 1];
	u16 manf_id, spec_ver, dev_ver, manf_date;
	u8 serial[QUERY_DESC_MAX_SIZE] = { };
	char idstr[QUERY_DESC_MAX_SIZE * 3];
	char model[MAX_MODEL_LEN * 8];
	u8 raw[QUERY_DESC_MAX_SIZE];
	u8 blen;
	int ret;

	manf_date = get_unaligned_be16(dev_desc + DEVICE_DESC_PARAM_MANF_DATE);
	spec_ver = get_unaligned_be16(dev_desc + DEVICE_DESC_PARAM_SPEC_VER);
	manf_id = get_unaligned_be16(dev_desc + DEVICE_DESC_PARAM_MANF_ID);
	dev_ver = get_unaligned_be16(dev_desc + DEVICE_DESC_PARAM_DEV_VER);

	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_STRING,
				     dev_desc[DEVICE_DESC_PARAM_PRDCT_NAME], 0,
				     raw, sizeof(raw));
	if (ret)
		return ret;

	ufs_rpmb_string_to_ascii(raw, model, sizeof(model));

	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_STRING,
				     dev_desc[DEVICE_DESC_PARAM_SN], 0,
				     raw, sizeof(raw));
	if (ret)
		return ret;

	blen = raw[QUERY_DESC_LENGTH_OFFSET];
	if (blen < QUERY_DESC_HDR_SIZE)
		return -EINVAL;

	memcpy(serial, raw + QUERY_DESC_HDR_SIZE, blen - QUERY_DESC_HDR_SIZE);
	/*
	 * The identifier format is an ABI shared with the secure world. The
	 * kernel's ufshcd_create_device_id() hex-encodes the full descriptor
	 * length (header included), so the serial always carries two trailing
	 * zero bytes; reproduce that here so the derived CID matches.
	 */
	bin2hex(serial_hex, serial, blen);
	serial_hex[blen * 2] = '\0';

	snprintf(idstr, sizeof(idstr), "%04X-%04X-%s-%s-%04X-%04X-R%u",
		 manf_id, spec_ver, model, serial_hex, dev_ver, manf_date,
		 region);

	if (blake2b(cid, UFS_RPMB_CID_SIZE, idstr, strlen(idstr), NULL, 0))
		return -EIO;

	return 0;
}

int ufs_rpmb_get_region_info(struct udevice *scsi_dev, unsigned int region,
			     u8 *size_mult, u8 *rel_wr, u8 *cid)
{
	struct ufs_hba *hba = dev_get_uclass_priv(scsi_dev->parent);
	u8 dev_desc[QUERY_DESC_DEVICE_DEF_SIZE] = { };
	u8 sizes[UFS_RPMB_NUM_REGIONS];
	u16 spec_ver;
	int ret;

	if (region >= UFS_RPMB_NUM_REGIONS)
		return 0;

	ret = ufshcd_read_desc_param(hba, QUERY_DESC_IDN_DEVICE, 0, 0,
				     dev_desc, sizeof(dev_desc));
	if (ret)
		return ret;

	spec_ver = get_unaligned_be16(dev_desc + DEVICE_DESC_PARAM_SPEC_VER);

	ret = ufs_rpmb_read_region_sizes(hba, spec_ver, sizes);
	if (ret)
		return ret;

	if (!sizes[region])
		return 0;

	ret = ufs_rpmb_read_geometry(scsi_dev, rel_wr);
	if (ret)
		return ret;

	if (!*rel_wr)
		*rel_wr = 1;

	ret = ufs_rpmb_build_cid(hba, dev_desc, region, cid);
	if (ret)
		return ret;

	*size_mult = sizes[region];

	return 1;
}
