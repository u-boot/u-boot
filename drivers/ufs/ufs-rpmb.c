// SPDX-License-Identifier: GPL-2.0+
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <scsi.h>
#include <ufs.h>
#include <asm/cache.h>
#include <asm/unaligned.h>
#include <linux/errno.h>
#include <linux/string.h>
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
