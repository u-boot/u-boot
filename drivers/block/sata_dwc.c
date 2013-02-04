/*
 * sata_dwc.c
 *
 * Synopsys DesignWare Cores (DWC) SATA host driver
 *
 * Author: Mark Miesfeld <mmiesfeld@amcc.com>
 *
 * Ported from 2.6.19.2 to 2.6.25/26 by Stefan Roese <sr@denx.de>
 * Copyright 2008 DENX Software Engineering
 *
 * Based on versions provided by AMCC and Synopsys which are:
 *          Copyright 2006 Applied Micro Circuits Corporation
 *          COPYRIGHT (C) 2005  SYNOPSYS, INC.  ALL RIGHTS RESERVED
 *
 * This program is free software; you can redistribute
 * it and/or modify it under the terms of the GNU
 * General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License,
 * or (at your option) any later version.
 *
 */
/*
 * SATA support based on the chip canyonlands.
 *
 * 04-17-2009
 *		The local version of this driver for the canyonlands board
 *		does not use interrupts but polls the chip instead.
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <malloc.h>
#include <ata.h>
#include <sata.h>
#include <linux/ctype.h>

#include "sata_dwc.h"

#define DMA_NUM_CHANS			1
#define DMA_NUM_CHAN_REGS		8

#define AHB_DMA_BRST_DFLT		16

struct dmareg {
	u32 low;
	u32 high;
};

struct dma_chan_regs {
	struct dmareg sar;
	struct dmareg dar;
	struct dmareg llp;
	struct dmareg ctl;
	struct dmareg sstat;
	struct dmareg dstat;
	struct dmareg sstatar;
	struct dmareg dstatar;
	struct dmareg cfg;
	struct dmareg sgr;
	struct dmareg dsr;
};

struct dma_interrupt_regs {
	struct dmareg tfr;
	struct dmareg block;
	struct dmareg srctran;
	struct dmareg dsttran;
	struct dmareg error;
};

struct ahb_dma_regs {
	struct dma_chan_regs	chan_regs[DMA_NUM_CHAN_REGS];
	struct dma_interrupt_regs	interrupt_raw;
	struct dma_interrupt_regs	interrupt_status;
	struct dma_interrupt_regs	interrupt_mask;
	struct dma_interrupt_regs	interrupt_clear;
	struct dmareg			statusInt;
	struct dmareg			rq_srcreg;
	struct dmareg			rq_dstreg;
	struct dmareg			rq_sgl_srcreg;
	struct dmareg			rq_sgl_dstreg;
	struct dmareg			rq_lst_srcreg;
	struct dmareg			rq_lst_dstreg;
	struct dmareg			dma_cfg;
	struct dmareg			dma_chan_en;
	struct dmareg			dma_id;
	struct dmareg			dma_test;
	struct dmareg			res1;
	struct dmareg			res2;
	/* DMA Comp Params
	 * Param 6 = dma_param[0], Param 5 = dma_param[1],
	 * Param 4 = dma_param[2] ...
	 */
	struct dmareg			dma_params[6];
};

#define DMA_EN			0x00000001
#define DMA_DI			0x00000000
#define DMA_CHANNEL(ch)		(0x00000001 << (ch))
#define DMA_ENABLE_CHAN(ch)	((0x00000001 << (ch)) |	\
				((0x000000001 << (ch)) << 8))
#define DMA_DISABLE_CHAN(ch)	(0x00000000 | 	\
				((0x000000001 << (ch)) << 8))

#define SATA_DWC_MAX_PORTS	1
#define SATA_DWC_SCR_OFFSET	0x24
#define SATA_DWC_REG_OFFSET	0x64

struct sata_dwc_regs {
	u32 fptagr;
	u32 fpbor;
	u32 fptcr;
	u32 dmacr;
	u32 dbtsr;
	u32 intpr;
	u32 intmr;
	u32 errmr;
	u32 llcr;
	u32 phycr;
	u32 physr;
	u32 rxbistpd;
	u32 rxbistpd1;
	u32 rxbistpd2;
	u32 txbistpd;
	u32 txbistpd1;
	u32 txbistpd2;
	u32 bistcr;
	u32 bistfctr;
	u32 bistsr;
	u32 bistdecr;
	u32 res[15];
	u32 testr;
	u32 versionr;
	u32 idr;
	u32 unimpl[192];
	u32 dmadr[256];
};

#define SATA_DWC_TXFIFO_DEPTH		0x01FF
#define SATA_DWC_RXFIFO_DEPTH		0x01FF

#define SATA_DWC_DBTSR_MWR(size)	((size / 4) & SATA_DWC_TXFIFO_DEPTH)
#define SATA_DWC_DBTSR_MRD(size)	(((size / 4) &	\
					SATA_DWC_RXFIFO_DEPTH) << 16)
#define SATA_DWC_INTPR_DMAT		0x00000001
#define SATA_DWC_INTPR_NEWFP		0x00000002
#define SATA_DWC_INTPR_PMABRT		0x00000004
#define SATA_DWC_INTPR_ERR		0x00000008
#define SATA_DWC_INTPR_NEWBIST		0x00000010
#define SATA_DWC_INTPR_IPF		0x10000000
#define SATA_DWC_INTMR_DMATM		0x00000001
#define SATA_DWC_INTMR_NEWFPM		0x00000002
#define SATA_DWC_INTMR_PMABRTM		0x00000004
#define SATA_DWC_INTMR_ERRM		0x00000008
#define SATA_DWC_INTMR_NEWBISTM		0x00000010

#define SATA_DWC_DMACR_TMOD_TXCHEN	0x00000004
#define SATA_DWC_DMACR_TXRXCH_CLEAR	SATA_DWC_DMACR_TMOD_TXCHEN

#define SATA_DWC_QCMD_MAX	32

#define SATA_DWC_SERROR_ERR_BITS	0x0FFF0F03

#define HSDEVP_FROM_AP(ap)	(struct sata_dwc_device_port*)	\
				(ap)->private_data

struct sata_dwc_device {
	struct device		*dev;
	struct ata_probe_ent	*pe;
	struct ata_host		*host;
	u8			*reg_base;
	struct sata_dwc_regs	*sata_dwc_regs;
	int			irq_dma;
};

struct sata_dwc_device_port {
	struct sata_dwc_device	*hsdev;
	int			cmd_issued[SATA_DWC_QCMD_MAX];
	u32			dma_chan[SATA_DWC_QCMD_MAX];
	int			dma_pending[SATA_DWC_QCMD_MAX];
};

enum {
	SATA_DWC_CMD_ISSUED_NOT		= 0,
	SATA_DWC_CMD_ISSUED_PEND	= 1,
	SATA_DWC_CMD_ISSUED_EXEC	= 2,
	SATA_DWC_CMD_ISSUED_NODATA	= 3,

	SATA_DWC_DMA_PENDING_NONE	= 0,
	SATA_DWC_DMA_PENDING_TX		= 1,
	SATA_DWC_DMA_PENDING_RX		= 2,
};

#define msleep(a)	udelay(a * 1000)
#define ssleep(a)	msleep(a * 1000)

static int ata_probe_timeout = (ATA_TMOUT_INTERNAL / 100);

enum sata_dev_state {
	SATA_INIT = 0,
	SATA_READY = 1,
	SATA_NODEVICE = 2,
	SATA_ERROR = 3,
};
enum sata_dev_state dev_state = SATA_INIT;

static struct ahb_dma_regs		*sata_dma_regs = 0;
static struct ata_host			*phost;
static struct ata_port			ap;
static struct ata_port			*pap = &ap;
static struct ata_device		ata_device;
static struct sata_dwc_device_port	dwc_devp;

static void	*scr_addr_sstatus;
static u32	temp_n_block = 0;

static unsigned ata_exec_internal(struct ata_device *dev,
			struct ata_taskfile *tf, const u8 *cdb,
			int dma_dir, unsigned int buflen,
			unsigned long timeout);
static unsigned int ata_dev_set_feature(struct ata_device *dev,
			u8 enable,u8 feature);
static unsigned int ata_dev_init_params(struct ata_device *dev,
			u16 heads, u16 sectors);
static u8 ata_irq_on(struct ata_port *ap);
static struct ata_queued_cmd *__ata_qc_from_tag(struct ata_port *ap,
			unsigned int tag);
static int ata_hsm_move(struct ata_port *ap, struct ata_queued_cmd *qc,
			u8 status, int in_wq);
static void ata_tf_to_host(struct ata_port *ap,
			const struct ata_taskfile *tf);
static void ata_exec_command(struct ata_port *ap,
			const struct ata_taskfile *tf);
static unsigned int ata_qc_issue_prot(struct ata_queued_cmd *qc);
static u8 ata_check_altstatus(struct ata_port *ap);
static u8 ata_check_status(struct ata_port *ap);
static void ata_dev_select(struct ata_port *ap, unsigned int device,
			unsigned int wait, unsigned int can_sleep);
static void ata_qc_issue(struct ata_queued_cmd *qc);
static void ata_tf_load(struct ata_port *ap,
			const struct ata_taskfile *tf);
static int ata_dev_read_sectors(unsigned char* pdata,
			unsigned long datalen, u32 block, u32 n_block);
static int ata_dev_write_sectors(unsigned char* pdata,
			unsigned long datalen , u32 block, u32 n_block);
static void ata_std_dev_select(struct ata_port *ap, unsigned int device);
static void ata_qc_complete(struct ata_queued_cmd *qc);
static void __ata_qc_complete(struct ata_queued_cmd *qc);
static void fill_result_tf(struct ata_queued_cmd *qc);
static void ata_tf_read(struct ata_port *ap, struct ata_taskfile *tf);
static void ata_mmio_data_xfer(struct ata_device *dev,
			unsigned char *buf,
			unsigned int buflen,int do_write);
static void ata_pio_task(struct ata_port *arg_ap);
static void __ata_port_freeze(struct ata_port *ap);
static int ata_port_freeze(struct ata_port *ap);
static void ata_qc_free(struct ata_queued_cmd *qc);
static void ata_pio_sectors(struct ata_queued_cmd *qc);
static void ata_pio_sector(struct ata_queued_cmd *qc);
static void ata_pio_queue_task(struct ata_port *ap,
			void *data,unsigned long delay);
static void ata_hsm_qc_complete(struct ata_queued_cmd *qc, int in_wq);
static int sata_dwc_softreset(struct ata_port *ap);
static int ata_dev_read_id(struct ata_device *dev, unsigned int *p_class,
		unsigned int flags, u16 *id);
static int check_sata_dev_state(void);

static const struct ata_port_info sata_dwc_port_info[] = {
	{
		.flags		= ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY |
				ATA_FLAG_MMIO | ATA_FLAG_PIO_POLLING |
				ATA_FLAG_SRST | ATA_FLAG_NCQ,
		.pio_mask	= 0x1f,
		.mwdma_mask	= 0x07,
		.udma_mask	= 0x7f,
	},
};

int init_sata(int dev)
{
	struct sata_dwc_device hsdev;
	struct ata_host host;
	struct ata_port_info pi = sata_dwc_port_info[0];
	struct ata_link *link;
	struct sata_dwc_device_port hsdevp = dwc_devp;
	u8 *base = 0;
	u8 *sata_dma_regs_addr = 0;
	u8 status;
	unsigned long base_addr = 0;
	int chan = 0;
	int rc;
	int i;

	phost = &host;

	base = (u8*)SATA_BASE_ADDR;

	hsdev.sata_dwc_regs = (void *__iomem)(base + SATA_DWC_REG_OFFSET);

	host.n_ports = SATA_DWC_MAX_PORTS;

	for (i = 0; i < SATA_DWC_MAX_PORTS; i++) {
		ap.pflags |= ATA_PFLAG_INITIALIZING;
		ap.flags = ATA_FLAG_DISABLED;
		ap.print_id = -1;
		ap.ctl = ATA_DEVCTL_OBS;
		ap.host = &host;
		ap.last_ctl = 0xFF;

		link = &ap.link;
		link->ap = &ap;
		link->pmp = 0;
		link->active_tag = ATA_TAG_POISON;
		link->hw_sata_spd_limit = 0;

		ap.port_no = i;
		host.ports[i] = &ap;
	}

	ap.pio_mask = pi.pio_mask;
	ap.mwdma_mask = pi.mwdma_mask;
	ap.udma_mask = pi.udma_mask;
	ap.flags |= pi.flags;
	ap.link.flags |= pi.link_flags;

	host.ports[0]->ioaddr.cmd_addr = base;
	host.ports[0]->ioaddr.scr_addr = base + SATA_DWC_SCR_OFFSET;
	scr_addr_sstatus = base + SATA_DWC_SCR_OFFSET;

	base_addr = (unsigned long)base;

	host.ports[0]->ioaddr.cmd_addr = (void *)base_addr + 0x00;
	host.ports[0]->ioaddr.data_addr = (void *)base_addr + 0x00;

	host.ports[0]->ioaddr.error_addr = (void *)base_addr + 0x04;
	host.ports[0]->ioaddr.feature_addr = (void *)base_addr + 0x04;

	host.ports[0]->ioaddr.nsect_addr = (void *)base_addr + 0x08;

	host.ports[0]->ioaddr.lbal_addr = (void *)base_addr + 0x0c;
	host.ports[0]->ioaddr.lbam_addr = (void *)base_addr + 0x10;
	host.ports[0]->ioaddr.lbah_addr = (void *)base_addr + 0x14;

	host.ports[0]->ioaddr.device_addr = (void *)base_addr + 0x18;
	host.ports[0]->ioaddr.command_addr = (void *)base_addr + 0x1c;
	host.ports[0]->ioaddr.status_addr = (void *)base_addr + 0x1c;

	host.ports[0]->ioaddr.altstatus_addr = (void *)base_addr + 0x20;
	host.ports[0]->ioaddr.ctl_addr = (void *)base_addr + 0x20;

	sata_dma_regs_addr = (u8*)SATA_DMA_REG_ADDR;
	sata_dma_regs = (void *__iomem)sata_dma_regs_addr;

	status = ata_check_altstatus(&ap);

	if (status == 0x7f) {
		printf("Hard Disk not found.\n");
		dev_state = SATA_NODEVICE;
		rc = FALSE;
		return rc;
	}

	printf("Waiting for device...");
	i = 0;
	while (1) {
		udelay(10000);

		status = ata_check_altstatus(&ap);

		if ((status & ATA_BUSY) == 0) {
			printf("\n");
			break;
		}

		i++;
		if (i > (ATA_RESET_TIME * 100)) {
			printf("** TimeOUT **\n");

			dev_state = SATA_NODEVICE;
			rc = FALSE;
			return rc;
		}
		if ((i >= 100) && ((i % 100) == 0))
			printf(".");
	}

	rc = sata_dwc_softreset(&ap);

	if (rc) {
		printf("sata_dwc : error. soft reset failed\n");
		return rc;
	}

	for (chan = 0; chan < DMA_NUM_CHANS; chan++) {
		out_le32(&(sata_dma_regs->interrupt_mask.error.low),
				DMA_DISABLE_CHAN(chan));

		out_le32(&(sata_dma_regs->interrupt_mask.tfr.low),
				DMA_DISABLE_CHAN(chan));
	}

	out_le32(&(sata_dma_regs->dma_cfg.low), DMA_DI);

	out_le32(&hsdev.sata_dwc_regs->intmr,
		SATA_DWC_INTMR_ERRM |
		SATA_DWC_INTMR_PMABRTM);

	/* Unmask the error bits that should trigger
	 * an error interrupt by setting the error mask register.
	 */
	out_le32(&hsdev.sata_dwc_regs->errmr, SATA_DWC_SERROR_ERR_BITS);

	hsdev.host = ap.host;
	memset(&hsdevp, 0, sizeof(hsdevp));
	hsdevp.hsdev = &hsdev;

	for (i = 0; i < SATA_DWC_QCMD_MAX; i++)
		hsdevp.cmd_issued[i] = SATA_DWC_CMD_ISSUED_NOT;

	out_le32((void __iomem *)scr_addr_sstatus + 4,
		in_le32((void __iomem *)scr_addr_sstatus + 4));

	rc = 0;
	return rc;
}

static u8 ata_check_altstatus(struct ata_port *ap)
{
	u8 val = 0;
	val = readb(ap->ioaddr.altstatus_addr);
	return val;
}

static int sata_dwc_softreset(struct ata_port *ap)
{
	u8 nsect,lbal = 0;
	u8 tmp = 0;
	struct ata_ioports *ioaddr = &ap->ioaddr;

	in_le32((void *)ap->ioaddr.scr_addr + (SCR_ERROR * 4));

	writeb(0x55, ioaddr->nsect_addr);
	writeb(0xaa, ioaddr->lbal_addr);
	writeb(0xaa, ioaddr->nsect_addr);
	writeb(0x55, ioaddr->lbal_addr);
	writeb(0x55, ioaddr->nsect_addr);
	writeb(0xaa, ioaddr->lbal_addr);

	nsect = readb(ioaddr->nsect_addr);
	lbal = readb(ioaddr->lbal_addr);

	if ((nsect == 0x55) && (lbal == 0xaa)) {
		printf("Device found\n");
	} else {
		printf("No device found\n");
		dev_state = SATA_NODEVICE;
		return FALSE;
	}

	tmp = ATA_DEVICE_OBS;
	writeb(tmp, ioaddr->device_addr);
	writeb(ap->ctl, ioaddr->ctl_addr);

	udelay(200);

	writeb(ap->ctl | ATA_SRST, ioaddr->ctl_addr);

	udelay(200);
	writeb(ap->ctl, ioaddr->ctl_addr);

	msleep(150);
	ata_check_status(ap);

	msleep(50);
	ata_check_status(ap);

	while (1) {
		u8 status = ata_check_status(ap);

		if (!(status & ATA_BUSY))
			break;

		printf("Hard Disk status is BUSY.\n");
		msleep(50);
	}

	tmp = ATA_DEVICE_OBS;
	writeb(tmp, ioaddr->device_addr);

	nsect = readb(ioaddr->nsect_addr);
	lbal = readb(ioaddr->lbal_addr);

	return 0;
}

static u8 ata_check_status(struct ata_port *ap)
{
	u8 val = 0;
	val = readb(ap->ioaddr.status_addr);
	return val;
}

static int ata_id_has_hipm(const u16 *id)
{
	u16 val = id[76];

	if (val == 0 || val == 0xffff)
		return -1;

	return val & (1 << 9);
}

static int ata_id_has_dipm(const u16 *id)
{
	u16 val = id[78];

	if (val == 0 || val == 0xffff)
		return -1;

	return val & (1 << 3);
}

int scan_sata(int dev)
{
	int i;
	int rc;
	u8 status;
	const u16 *id;
	struct ata_device *ata_dev = &ata_device;
	unsigned long pio_mask, mwdma_mask;
	char revbuf[7];
	u16 iobuf[ATA_SECTOR_WORDS];

	memset(iobuf, 0, sizeof(iobuf));

	if (dev_state == SATA_NODEVICE)
		return 1;

	printf("Waiting for device...");
	i = 0;
	while (1) {
		udelay(10000);

		status = ata_check_altstatus(&ap);

		if ((status & ATA_BUSY) == 0) {
			printf("\n");
			break;
		}

		i++;
		if (i > (ATA_RESET_TIME * 100)) {
			printf("** TimeOUT **\n");

			dev_state = SATA_NODEVICE;
			return 1;
		}
		if ((i >= 100) && ((i % 100) == 0))
			printf(".");
	}

	udelay(1000);

	rc = ata_dev_read_id(ata_dev, &ata_dev->class,
			ATA_READID_POSTRESET,ata_dev->id);
	if (rc) {
		printf("sata_dwc : error. failed sata scan\n");
		return 1;
	}

	/* SATA drives indicate we have a bridge. We don't know which
	 * end of the link the bridge is which is a problem
	 */
	if (ata_id_is_sata(ata_dev->id))
		ap.cbl = ATA_CBL_SATA;

	id = ata_dev->id;

	ata_dev->flags &= ~ATA_DFLAG_CFG_MASK;
	ata_dev->max_sectors = 0;
	ata_dev->cdb_len = 0;
	ata_dev->n_sectors = 0;
	ata_dev->cylinders = 0;
	ata_dev->heads = 0;
	ata_dev->sectors = 0;

	if (id[ATA_ID_FIELD_VALID] & (1 << 1)) {
		pio_mask = id[ATA_ID_PIO_MODES] & 0x03;
		pio_mask <<= 3;
		pio_mask |= 0x7;
	} else {
		/* If word 64 isn't valid then Word 51 high byte holds
		 * the PIO timing number for the maximum. Turn it into
		 * a mask.
		 */
		u8 mode = (id[ATA_ID_OLD_PIO_MODES] >> 8) & 0xFF;
		if (mode < 5) {
			pio_mask = (2 << mode) - 1;
		} else {
			pio_mask = 1;
		}
	}

	mwdma_mask = id[ATA_ID_MWDMA_MODES] & 0x07;

	if (ata_id_is_cfa(id)) {
		int pio = id[163] & 0x7;
		int dma = (id[163] >> 3) & 7;

		if (pio)
			pio_mask |= (1 << 5);
		if (pio > 1)
			pio_mask |= (1 << 6);
		if (dma)
			mwdma_mask |= (1 << 3);
		if (dma > 1)
			mwdma_mask |= (1 << 4);
	}

	if (ata_dev->class == ATA_DEV_ATA) {
		if (ata_id_is_cfa(id)) {
			if (id[162] & 1)
				printf("supports DRM functions and may "
					"not be fully accessable.\n");
			sprintf(revbuf, "%s", "CFA");
		} else {
			if (ata_id_has_tpm(id))
				printf("supports DRM functions and may "
						"not be fully accessable.\n");
		}

		ata_dev->n_sectors = ata_id_n_sectors((u16*)id);

		if (ata_dev->id[59] & 0x100)
			ata_dev->multi_count = ata_dev->id[59] & 0xff;

		if (ata_id_has_lba(id)) {
			char ncq_desc[20];

			ata_dev->flags |= ATA_DFLAG_LBA;
			if (ata_id_has_lba48(id)) {
				ata_dev->flags |= ATA_DFLAG_LBA48;

				if (ata_dev->n_sectors >= (1UL << 28) &&
					ata_id_has_flush_ext(id))
					ata_dev->flags |= ATA_DFLAG_FLUSH_EXT;
			}
			if (!ata_id_has_ncq(ata_dev->id))
				ncq_desc[0] = '\0';

			if (ata_dev->horkage & ATA_HORKAGE_NONCQ)
				sprintf(ncq_desc, "%s", "NCQ (not used)");

			if (ap.flags & ATA_FLAG_NCQ)
				ata_dev->flags |= ATA_DFLAG_NCQ;
		}
		ata_dev->cdb_len = 16;
	}
	ata_dev->max_sectors = ATA_MAX_SECTORS;
	if (ata_dev->flags & ATA_DFLAG_LBA48)
		ata_dev->max_sectors = ATA_MAX_SECTORS_LBA48;

	if (!(ata_dev->horkage & ATA_HORKAGE_IPM)) {
		if (ata_id_has_hipm(ata_dev->id))
			ata_dev->flags |= ATA_DFLAG_HIPM;
		if (ata_id_has_dipm(ata_dev->id))
			ata_dev->flags |= ATA_DFLAG_DIPM;
	}

	if ((ap.cbl == ATA_CBL_SATA) && (!ata_id_is_sata(ata_dev->id))) {
		ata_dev->udma_mask &= ATA_UDMA5;
		ata_dev->max_sectors = ATA_MAX_SECTORS;
	}

	if (ata_dev->horkage & ATA_HORKAGE_DIAGNOSTIC) {
		printf("Drive reports diagnostics failure."
				"This may indicate a drive\n");
		printf("fault or invalid emulation."
				"Contact drive vendor for information.\n");
	}

	rc = check_sata_dev_state();

	ata_id_c_string(ata_dev->id,
			(unsigned char *)sata_dev_desc[dev].revision,
			 ATA_ID_FW_REV, sizeof(sata_dev_desc[dev].revision));
	ata_id_c_string(ata_dev->id,
			(unsigned char *)sata_dev_desc[dev].vendor,
			 ATA_ID_PROD, sizeof(sata_dev_desc[dev].vendor));
	ata_id_c_string(ata_dev->id,
			(unsigned char *)sata_dev_desc[dev].product,
			 ATA_ID_SERNO, sizeof(sata_dev_desc[dev].product));

	sata_dev_desc[dev].lba = (u32) ata_dev->n_sectors;

#ifdef CONFIG_LBA48
	if (ata_dev->id[83] & (1 << 10)) {
		sata_dev_desc[dev].lba48 = 1;
	} else {
		sata_dev_desc[dev].lba48 = 0;
	}
#endif

	return 0;
}

static u8 ata_busy_wait(struct ata_port *ap,
		unsigned int bits,unsigned int max)
{
	u8 status;

	do {
		udelay(10);
		status = ata_check_status(ap);
		max--;
	} while (status != 0xff && (status & bits) && (max > 0));

	return status;
}

static int ata_dev_read_id(struct ata_device *dev, unsigned int *p_class,
		unsigned int flags, u16 *id)
{
	struct ata_port *ap = pap;
	unsigned int class = *p_class;
	struct ata_taskfile tf;
	unsigned int err_mask = 0;
	const char *reason;
	int may_fallback = 1, tried_spinup = 0;
	u8 status;
	int rc;

	status = ata_busy_wait(ap, ATA_BUSY, 30000);
	if (status & ATA_BUSY) {
		printf("BSY = 0 check. timeout.\n");
		rc = FALSE;
		return rc;
	}

	ata_dev_select(ap, dev->devno, 1, 1);

retry:
	memset(&tf, 0, sizeof(tf));
	ap->print_id = 1;
	ap->flags &= ~ATA_FLAG_DISABLED;
	tf.ctl = ap->ctl;
	tf.device = ATA_DEVICE_OBS;
	tf.command = ATA_CMD_ID_ATA;
	tf.protocol = ATA_PROT_PIO;

	/* Some devices choke if TF registers contain garbage.  Make
	 * sure those are properly initialized.
	 */
	tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;

	/* Device presence detection is unreliable on some
	 * controllers.  Always poll IDENTIFY if available.
	 */
	tf.flags |= ATA_TFLAG_POLLING;

	temp_n_block = 1;

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_FROM_DEVICE,
					sizeof(id[0]) * ATA_ID_WORDS, 0);

	if (err_mask) {
		if (err_mask & AC_ERR_NODEV_HINT) {
			printf("NODEV after polling detection\n");
			return -ENOENT;
		}

		if ((err_mask == AC_ERR_DEV) && (tf.feature & ATA_ABORTED)) {
			/* Device or controller might have reported
			 * the wrong device class.  Give a shot at the
			 * other IDENTIFY if the current one is
			 * aborted by the device.
			 */
			if (may_fallback) {
				may_fallback = 0;

				if (class == ATA_DEV_ATA) {
					class = ATA_DEV_ATAPI;
				} else {
					class = ATA_DEV_ATA;
				}
				goto retry;
			}
			/* Control reaches here iff the device aborted
			 * both flavors of IDENTIFYs which happens
			 * sometimes with phantom devices.
			 */
			printf("both IDENTIFYs aborted, assuming NODEV\n");
			return -ENOENT;
		}
		rc = -EIO;
		reason = "I/O error";
		goto err_out;
	}

	/* Falling back doesn't make sense if ID data was read
	 * successfully at least once.
	 */
	may_fallback = 0;

	unsigned int id_cnt;

	for (id_cnt = 0; id_cnt < ATA_ID_WORDS; id_cnt++)
		id[id_cnt] = le16_to_cpu(id[id_cnt]);


	rc = -EINVAL;
	reason = "device reports invalid type";

	if (class == ATA_DEV_ATA) {
		if (!ata_id_is_ata(id) && !ata_id_is_cfa(id))
			goto err_out;
	} else {
		if (ata_id_is_ata(id))
			goto err_out;
	}
	if (!tried_spinup && (id[2] == 0x37c8 || id[2] == 0x738c)) {
		tried_spinup = 1;
		/*
		 * Drive powered-up in standby mode, and requires a specific
		 * SET_FEATURES spin-up subcommand before it will accept
		 * anything other than the original IDENTIFY command.
		 */
		err_mask = ata_dev_set_feature(dev, SETFEATURES_SPINUP, 0);
		if (err_mask && id[2] != 0x738c) {
			rc = -EIO;
			reason = "SPINUP failed";
			goto err_out;
		}
		/*
		 * If the drive initially returned incomplete IDENTIFY info,
		 * we now must reissue the IDENTIFY command.
		 */
		if (id[2] == 0x37c8)
			goto retry;
	}

	if ((flags & ATA_READID_POSTRESET) && class == ATA_DEV_ATA) {
		/*
		 * The exact sequence expected by certain pre-ATA4 drives is:
		 * SRST RESET
		 * IDENTIFY (optional in early ATA)
		 * INITIALIZE DEVICE PARAMETERS (later IDE and ATA)
		 * anything else..
		 * Some drives were very specific about that exact sequence.
		 *
		 * Note that ATA4 says lba is mandatory so the second check
		 * shoud never trigger.
		 */
		if (ata_id_major_version(id) < 4 || !ata_id_has_lba(id)) {
			err_mask = ata_dev_init_params(dev, id[3], id[6]);
			if (err_mask) {
				rc = -EIO;
				reason = "INIT_DEV_PARAMS failed";
				goto err_out;
			}

			/* current CHS translation info (id[53-58]) might be
			 * changed. reread the identify device info.
			 */
			flags &= ~ATA_READID_POSTRESET;
			goto retry;
		}
	}

	*p_class = class;
	return 0;

err_out:
	printf("failed to READ ID (%s, err_mask=0x%x)\n", reason, err_mask);
	return rc;
}

static u8 ata_wait_idle(struct ata_port *ap)
{
	u8 status = ata_busy_wait(ap, ATA_BUSY | ATA_DRQ, 1000);
	return status;
}

static void ata_dev_select(struct ata_port *ap, unsigned int device,
		unsigned int wait, unsigned int can_sleep)
{
	if (wait)
		ata_wait_idle(ap);

	ata_std_dev_select(ap, device);

	if (wait)
		ata_wait_idle(ap);
}

static void ata_std_dev_select(struct ata_port *ap, unsigned int device)
{
	u8 tmp;

	if (device == 0) {
		tmp = ATA_DEVICE_OBS;
	} else {
		tmp = ATA_DEVICE_OBS | ATA_DEV1;
	}

	writeb(tmp, ap->ioaddr.device_addr);

	readb(ap->ioaddr.altstatus_addr);

	udelay(1);
}

static int waiting_for_reg_state(volatile u8 *offset,
				int timeout_msec,
				u32 sign)
{
	int i;
	u32 status;

	for (i = 0; i < timeout_msec; i++) {
		status = readl(offset);
		if ((status & sign) != 0)
			break;
		msleep(1);
	}

	return (i < timeout_msec) ? 0 : -1;
}

static void ata_qc_reinit(struct ata_queued_cmd *qc)
{
	qc->dma_dir = DMA_NONE;
	qc->flags = 0;
	qc->nbytes = qc->extrabytes = qc->curbytes = 0;
	qc->n_elem = 0;
	qc->err_mask = 0;
	qc->sect_size = ATA_SECT_SIZE;
	qc->nbytes = ATA_SECT_SIZE * temp_n_block;

	memset(&qc->tf, 0, sizeof(qc->tf));
	qc->tf.ctl = 0;
	qc->tf.device = ATA_DEVICE_OBS;

	qc->result_tf.command = ATA_DRDY;
	qc->result_tf.feature = 0;
}

struct ata_queued_cmd *__ata_qc_from_tag(struct ata_port *ap,
					unsigned int tag)
{
	if (tag < ATA_MAX_QUEUE)
		return &ap->qcmd[tag];
	return NULL;
}

static void __ata_port_freeze(struct ata_port *ap)
{
	printf("set port freeze.\n");
	ap->pflags |= ATA_PFLAG_FROZEN;
}

static int ata_port_freeze(struct ata_port *ap)
{
	__ata_port_freeze(ap);
	return 0;
}

unsigned ata_exec_internal(struct ata_device *dev,
			struct ata_taskfile *tf, const u8 *cdb,
			int dma_dir, unsigned int buflen,
			unsigned long timeout)
{
	struct ata_link *link = dev->link;
	struct ata_port *ap = pap;
	struct ata_queued_cmd *qc;
	unsigned int tag, preempted_tag;
	u32 preempted_sactive, preempted_qc_active;
	int preempted_nr_active_links;
	unsigned int err_mask;
	int rc = 0;
	u8 status;

	status = ata_busy_wait(ap, ATA_BUSY, 300000);
	if (status & ATA_BUSY) {
		printf("BSY = 0 check. timeout.\n");
		rc = FALSE;
		return rc;
	}

	if (ap->pflags & ATA_PFLAG_FROZEN)
		return AC_ERR_SYSTEM;

	tag = ATA_TAG_INTERNAL;

	if (test_and_set_bit(tag, &ap->qc_allocated)) {
		rc = FALSE;
		return rc;
	}

	qc = __ata_qc_from_tag(ap, tag);
	qc->tag = tag;
	qc->ap = ap;
	qc->dev = dev;

	ata_qc_reinit(qc);

	preempted_tag = link->active_tag;
	preempted_sactive = link->sactive;
	preempted_qc_active = ap->qc_active;
	preempted_nr_active_links = ap->nr_active_links;
	link->active_tag = ATA_TAG_POISON;
	link->sactive = 0;
	ap->qc_active = 0;
	ap->nr_active_links = 0;

	qc->tf = *tf;
	if (cdb)
		memcpy(qc->cdb, cdb, ATAPI_CDB_LEN);
	qc->flags |= ATA_QCFLAG_RESULT_TF;
	qc->dma_dir = dma_dir;
	qc->private_data = 0;

	ata_qc_issue(qc);

	if (!timeout)
		timeout = ata_probe_timeout * 1000 / HZ;

	status = ata_busy_wait(ap, ATA_BUSY, 30000);
	if (status & ATA_BUSY) {
		printf("BSY = 0 check. timeout.\n");
		printf("altstatus = 0x%x.\n", status);
		qc->err_mask |= AC_ERR_OTHER;
		return qc->err_mask;
	}

	if (waiting_for_reg_state(ap->ioaddr.altstatus_addr, 1000, 0x8)) {
		u8 status = 0;
		u8 errorStatus = 0;

		status = readb(ap->ioaddr.altstatus_addr);
		if ((status & 0x01) != 0) {
			errorStatus = readb(ap->ioaddr.feature_addr);
			if (errorStatus == 0x04 &&
				qc->tf.command == ATA_CMD_PIO_READ_EXT){
				printf("Hard Disk doesn't support LBA48\n");
				dev_state = SATA_ERROR;
				qc->err_mask |= AC_ERR_OTHER;
				return qc->err_mask;
			}
		}
		qc->err_mask |= AC_ERR_OTHER;
		return qc->err_mask;
	}

	status = ata_busy_wait(ap, ATA_BUSY, 10);
	if (status & ATA_BUSY) {
		printf("BSY = 0 check. timeout.\n");
		qc->err_mask |= AC_ERR_OTHER;
		return qc->err_mask;
	}

	ata_pio_task(ap);

	if (!rc) {
		if (qc->flags & ATA_QCFLAG_ACTIVE) {
			qc->err_mask |= AC_ERR_TIMEOUT;
			ata_port_freeze(ap);
		}
	}

	if (qc->flags & ATA_QCFLAG_FAILED) {
		if (qc->result_tf.command & (ATA_ERR | ATA_DF))
			qc->err_mask |= AC_ERR_DEV;

		if (!qc->err_mask)
			qc->err_mask |= AC_ERR_OTHER;

		if (qc->err_mask & ~AC_ERR_OTHER)
			qc->err_mask &= ~AC_ERR_OTHER;
	}

	*tf = qc->result_tf;
	err_mask = qc->err_mask;
	ata_qc_free(qc);
	link->active_tag = preempted_tag;
	link->sactive = preempted_sactive;
	ap->qc_active = preempted_qc_active;
	ap->nr_active_links = preempted_nr_active_links;

	if (ap->flags & ATA_FLAG_DISABLED) {
		err_mask |= AC_ERR_SYSTEM;
		ap->flags &= ~ATA_FLAG_DISABLED;
	}

	return err_mask;
}

static void ata_qc_issue(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct ata_link *link = qc->dev->link;
	u8 prot = qc->tf.protocol;

	if (ata_is_ncq(prot)) {
		if (!link->sactive)
			ap->nr_active_links++;
		link->sactive |= 1 << qc->tag;
	} else {
		ap->nr_active_links++;
		link->active_tag = qc->tag;
	}

	qc->flags |= ATA_QCFLAG_ACTIVE;
	ap->qc_active |= 1 << qc->tag;

	if (qc->dev->flags & ATA_DFLAG_SLEEPING) {
		msleep(1);
		return;
	}

	qc->err_mask |= ata_qc_issue_prot(qc);
	if (qc->err_mask)
		goto err;

	return;
err:
	ata_qc_complete(qc);
}

static unsigned int ata_qc_issue_prot(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;

	if (ap->flags & ATA_FLAG_PIO_POLLING) {
		switch (qc->tf.protocol) {
		case ATA_PROT_PIO:
		case ATA_PROT_NODATA:
		case ATAPI_PROT_PIO:
		case ATAPI_PROT_NODATA:
			qc->tf.flags |= ATA_TFLAG_POLLING;
			break;
		default:
			break;
		}
	}

	ata_dev_select(ap, qc->dev->devno, 1, 0);

	switch (qc->tf.protocol) {
	case ATA_PROT_PIO:
		if (qc->tf.flags & ATA_TFLAG_POLLING)
			qc->tf.ctl |= ATA_NIEN;

		ata_tf_to_host(ap, &qc->tf);

		ap->hsm_task_state = HSM_ST;

		if (qc->tf.flags & ATA_TFLAG_POLLING)
			ata_pio_queue_task(ap, qc, 0);

		break;

	default:
		return AC_ERR_SYSTEM;
	}

	return 0;
}

static void ata_tf_to_host(struct ata_port *ap,
			const struct ata_taskfile *tf)
{
	ata_tf_load(ap, tf);
	ata_exec_command(ap, tf);
}

static void ata_tf_load(struct ata_port *ap,
			const struct ata_taskfile *tf)
{
	struct ata_ioports *ioaddr = &ap->ioaddr;
	unsigned int is_addr = tf->flags & ATA_TFLAG_ISADDR;

	if (tf->ctl != ap->last_ctl) {
		if (ioaddr->ctl_addr)
			writeb(tf->ctl, ioaddr->ctl_addr);
		ap->last_ctl = tf->ctl;
		ata_wait_idle(ap);
	}

	if (is_addr && (tf->flags & ATA_TFLAG_LBA48)) {
		writeb(tf->hob_feature, ioaddr->feature_addr);
		writeb(tf->hob_nsect, ioaddr->nsect_addr);
		writeb(tf->hob_lbal, ioaddr->lbal_addr);
		writeb(tf->hob_lbam, ioaddr->lbam_addr);
		writeb(tf->hob_lbah, ioaddr->lbah_addr);
	}

	if (is_addr) {
		writeb(tf->feature, ioaddr->feature_addr);
		writeb(tf->nsect, ioaddr->nsect_addr);
		writeb(tf->lbal, ioaddr->lbal_addr);
		writeb(tf->lbam, ioaddr->lbam_addr);
		writeb(tf->lbah, ioaddr->lbah_addr);
	}

	if (tf->flags & ATA_TFLAG_DEVICE)
		writeb(tf->device, ioaddr->device_addr);

	ata_wait_idle(ap);
}

static void ata_exec_command(struct ata_port *ap,
			const struct ata_taskfile *tf)
{
	writeb(tf->command, ap->ioaddr.command_addr);

	readb(ap->ioaddr.altstatus_addr);

	udelay(1);
}

static void ata_pio_queue_task(struct ata_port *ap,
			void *data,unsigned long delay)
{
	ap->port_task_data = data;
}

static unsigned int ac_err_mask(u8 status)
{
	if (status & (ATA_BUSY | ATA_DRQ))
		return AC_ERR_HSM;
	if (status & (ATA_ERR | ATA_DF))
		return AC_ERR_DEV;
	return 0;
}

static unsigned int __ac_err_mask(u8 status)
{
	unsigned int mask = ac_err_mask(status);
	if (mask == 0)
		return AC_ERR_OTHER;
	return mask;
}

static void ata_pio_task(struct ata_port *arg_ap)
{
	struct ata_port *ap = arg_ap;
	struct ata_queued_cmd *qc = ap->port_task_data;
	u8 status;
	int poll_next;

fsm_start:
	/*
	 * This is purely heuristic.  This is a fast path.
	 * Sometimes when we enter, BSY will be cleared in
	 * a chk-status or two.  If not, the drive is probably seeking
	 * or something.  Snooze for a couple msecs, then
	 * chk-status again.  If still busy, queue delayed work.
	 */
	status = ata_busy_wait(ap, ATA_BUSY, 5);
	if (status & ATA_BUSY) {
		msleep(2);
		status = ata_busy_wait(ap, ATA_BUSY, 10);
		if (status & ATA_BUSY) {
			ata_pio_queue_task(ap, qc, ATA_SHORT_PAUSE);
			return;
		}
	}

	poll_next = ata_hsm_move(ap, qc, status, 1);

	/* another command or interrupt handler
	 * may be running at this point.
	 */
	if (poll_next)
		goto fsm_start;
}

static int ata_hsm_move(struct ata_port *ap, struct ata_queued_cmd *qc,
			u8 status, int in_wq)
{
	int poll_next;

fsm_start:
	switch (ap->hsm_task_state) {
	case HSM_ST_FIRST:
		poll_next = (qc->tf.flags & ATA_TFLAG_POLLING);

		if ((status & ATA_DRQ) == 0) {
			if (status & (ATA_ERR | ATA_DF)) {
				qc->err_mask |= AC_ERR_DEV;
			} else {
				qc->err_mask |= AC_ERR_HSM;
			}
			ap->hsm_task_state = HSM_ST_ERR;
			goto fsm_start;
		}

		/* Device should not ask for data transfer (DRQ=1)
		 * when it finds something wrong.
		 * We ignore DRQ here and stop the HSM by
		 * changing hsm_task_state to HSM_ST_ERR and
		 * let the EH abort the command or reset the device.
		 */
		if (status & (ATA_ERR | ATA_DF)) {
			if (!(qc->dev->horkage & ATA_HORKAGE_STUCK_ERR)) {
				printf("DRQ=1 with device error, "
					"dev_stat 0x%X\n", status);
				qc->err_mask |= AC_ERR_HSM;
				ap->hsm_task_state = HSM_ST_ERR;
				goto fsm_start;
			}
		}

		if (qc->tf.protocol == ATA_PROT_PIO) {
			/* PIO data out protocol.
			 * send first data block.
			 */
			/* ata_pio_sectors() might change the state
			 * to HSM_ST_LAST. so, the state is changed here
			 * before ata_pio_sectors().
			 */
			ap->hsm_task_state = HSM_ST;
			ata_pio_sectors(qc);
		} else {
			printf("protocol is not ATA_PROT_PIO \n");
		}
		break;

	case HSM_ST:
		if ((status & ATA_DRQ) == 0) {
			if (status & (ATA_ERR | ATA_DF)) {
				qc->err_mask |= AC_ERR_DEV;
			} else {
				/* HSM violation. Let EH handle this.
				 * Phantom devices also trigger this
				 * condition.  Mark hint.
				 */
				qc->err_mask |= AC_ERR_HSM | AC_ERR_NODEV_HINT;
			}

			ap->hsm_task_state = HSM_ST_ERR;
			goto fsm_start;
		}
		/* For PIO reads, some devices may ask for
		 * data transfer (DRQ=1) alone with ERR=1.
		 * We respect DRQ here and transfer one
		 * block of junk data before changing the
		 * hsm_task_state to HSM_ST_ERR.
		 *
		 * For PIO writes, ERR=1 DRQ=1 doesn't make
		 * sense since the data block has been
		 * transferred to the device.
		 */
		if (status & (ATA_ERR | ATA_DF)) {
			qc->err_mask |= AC_ERR_DEV;

			if (!(qc->tf.flags & ATA_TFLAG_WRITE)) {
				ata_pio_sectors(qc);
				status = ata_wait_idle(ap);
			}

			if (status & (ATA_BUSY | ATA_DRQ))
				qc->err_mask |= AC_ERR_HSM;

			/* ata_pio_sectors() might change the
			 * state to HSM_ST_LAST. so, the state
			 * is changed after ata_pio_sectors().
			 */
			ap->hsm_task_state = HSM_ST_ERR;
			goto fsm_start;
		}

		ata_pio_sectors(qc);
		if (ap->hsm_task_state == HSM_ST_LAST &&
			(!(qc->tf.flags & ATA_TFLAG_WRITE))) {
			status = ata_wait_idle(ap);
			goto fsm_start;
		}

		poll_next = 1;
		break;

	case HSM_ST_LAST:
		if (!ata_ok(status)) {
			qc->err_mask |= __ac_err_mask(status);
			ap->hsm_task_state = HSM_ST_ERR;
			goto fsm_start;
		}

		ap->hsm_task_state = HSM_ST_IDLE;

		ata_hsm_qc_complete(qc, in_wq);

		poll_next = 0;
		break;

	case HSM_ST_ERR:
		/* make sure qc->err_mask is available to
		 * know what's wrong and recover
		 */
		ap->hsm_task_state = HSM_ST_IDLE;

		ata_hsm_qc_complete(qc, in_wq);

		poll_next = 0;
		break;
	default:
		poll_next = 0;
	}

	return poll_next;
}

static void ata_pio_sectors(struct ata_queued_cmd *qc)
{
	struct ata_port *ap;
	ap = pap;
	qc->pdata = ap->pdata;

	ata_pio_sector(qc);

	readb(qc->ap->ioaddr.altstatus_addr);
	udelay(1);
}

static void ata_pio_sector(struct ata_queued_cmd *qc)
{
	int do_write = (qc->tf.flags & ATA_TFLAG_WRITE);
	struct ata_port *ap = qc->ap;
	unsigned int offset;
	unsigned char *buf;
	char temp_data_buf[512];

	if (qc->curbytes == qc->nbytes - qc->sect_size)
		ap->hsm_task_state = HSM_ST_LAST;

	offset = qc->curbytes;

	switch (qc->tf.command) {
	case ATA_CMD_ID_ATA:
		buf = (unsigned char *)&ata_device.id[0];
		break;
	case ATA_CMD_PIO_READ_EXT:
	case ATA_CMD_PIO_READ:
	case ATA_CMD_PIO_WRITE_EXT:
	case ATA_CMD_PIO_WRITE:
		buf = qc->pdata + offset;
		break;
	default:
		buf = (unsigned char *)&temp_data_buf[0];
	}

	ata_mmio_data_xfer(qc->dev, buf, qc->sect_size, do_write);

	qc->curbytes += qc->sect_size;

}

static void ata_mmio_data_xfer(struct ata_device *dev, unsigned char *buf,
				unsigned int buflen, int do_write)
{
	struct ata_port *ap = pap;
	void __iomem *data_addr = ap->ioaddr.data_addr;
	unsigned int words = buflen >> 1;
	u16 *buf16 = (u16 *)buf;
	unsigned int i = 0;

	udelay(100);
	if (do_write) {
		for (i = 0; i < words; i++)
			writew(le16_to_cpu(buf16[i]), data_addr);
	} else {
		for (i = 0; i < words; i++)
			buf16[i] = cpu_to_le16(readw(data_addr));
	}

	if (buflen & 0x01) {
		__le16 align_buf[1] = { 0 };
		unsigned char *trailing_buf = buf + buflen - 1;

		if (do_write) {
			memcpy(align_buf, trailing_buf, 1);
			writew(le16_to_cpu(align_buf[0]), data_addr);
		} else {
			align_buf[0] = cpu_to_le16(readw(data_addr));
			memcpy(trailing_buf, align_buf, 1);
		}
	}
}

static void ata_hsm_qc_complete(struct ata_queued_cmd *qc, int in_wq)
{
	struct ata_port *ap = qc->ap;

	if (in_wq) {
		/* EH might have kicked in while host lock is
		 * released.
		 */
		qc = &ap->qcmd[qc->tag];
		if (qc) {
			if (!(qc->err_mask & AC_ERR_HSM)) {
				ata_irq_on(ap);
				ata_qc_complete(qc);
			} else {
				ata_port_freeze(ap);
			}
		}
	} else {
		if (!(qc->err_mask & AC_ERR_HSM)) {
			ata_qc_complete(qc);
		} else {
			ata_port_freeze(ap);
		}
	}
}

static u8 ata_irq_on(struct ata_port *ap)
{
	struct ata_ioports *ioaddr = &ap->ioaddr;
	u8 tmp;

	ap->ctl &= ~ATA_NIEN;
	ap->last_ctl = ap->ctl;

	if (ioaddr->ctl_addr)
		writeb(ap->ctl, ioaddr->ctl_addr);

	tmp = ata_wait_idle(ap);

	return tmp;
}

static unsigned int ata_tag_internal(unsigned int tag)
{
	return tag == ATA_MAX_QUEUE - 1;
}

static void ata_qc_complete(struct ata_queued_cmd *qc)
{
	struct ata_device *dev = qc->dev;
	if (qc->err_mask)
		qc->flags |= ATA_QCFLAG_FAILED;

	if (qc->flags & ATA_QCFLAG_FAILED) {
		if (!ata_tag_internal(qc->tag)) {
			fill_result_tf(qc);
			return;
		}
	}
	if (qc->flags & ATA_QCFLAG_RESULT_TF)
		fill_result_tf(qc);

	/* Some commands need post-processing after successful
	 * completion.
	 */
	switch (qc->tf.command) {
	case ATA_CMD_SET_FEATURES:
		if (qc->tf.feature != SETFEATURES_WC_ON &&
				qc->tf.feature != SETFEATURES_WC_OFF)
			break;
	case ATA_CMD_INIT_DEV_PARAMS:
	case ATA_CMD_SET_MULTI:
		break;

	case ATA_CMD_SLEEP:
		dev->flags |= ATA_DFLAG_SLEEPING;
		break;
	}

	__ata_qc_complete(qc);
}

static void fill_result_tf(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;

	qc->result_tf.flags = qc->tf.flags;
	ata_tf_read(ap, &qc->result_tf);
}

static void ata_tf_read(struct ata_port *ap, struct ata_taskfile *tf)
{
	struct ata_ioports *ioaddr = &ap->ioaddr;

	tf->command = ata_check_status(ap);
	tf->feature = readb(ioaddr->error_addr);
	tf->nsect = readb(ioaddr->nsect_addr);
	tf->lbal = readb(ioaddr->lbal_addr);
	tf->lbam = readb(ioaddr->lbam_addr);
	tf->lbah = readb(ioaddr->lbah_addr);
	tf->device = readb(ioaddr->device_addr);

	if (tf->flags & ATA_TFLAG_LBA48) {
		if (ioaddr->ctl_addr) {
			writeb(tf->ctl | ATA_HOB, ioaddr->ctl_addr);

			tf->hob_feature = readb(ioaddr->error_addr);
			tf->hob_nsect = readb(ioaddr->nsect_addr);
			tf->hob_lbal = readb(ioaddr->lbal_addr);
			tf->hob_lbam = readb(ioaddr->lbam_addr);
			tf->hob_lbah = readb(ioaddr->lbah_addr);

			writeb(tf->ctl, ioaddr->ctl_addr);
			ap->last_ctl = tf->ctl;
		} else {
			printf("sata_dwc warnning register read.\n");
		}
	}
}

static void __ata_qc_complete(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct ata_link *link = qc->dev->link;

	link->active_tag = ATA_TAG_POISON;
	ap->nr_active_links--;

	if (qc->flags & ATA_QCFLAG_CLEAR_EXCL && ap->excl_link == link)
		ap->excl_link = NULL;

	qc->flags &= ~ATA_QCFLAG_ACTIVE;
	ap->qc_active &= ~(1 << qc->tag);
}

static void ata_qc_free(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	unsigned int tag;
	qc->flags = 0;
	tag = qc->tag;
	if (tag < ATA_MAX_QUEUE) {
		qc->tag = ATA_TAG_POISON;
		clear_bit(tag, &ap->qc_allocated);
	}
}

static int check_sata_dev_state(void)
{
	unsigned long datalen;
	unsigned char *pdata;
	int ret = 0;
	int i = 0;
	char temp_data_buf[512];

	while (1) {
		udelay(10000);

		pdata = (unsigned char*)&temp_data_buf[0];
		datalen = 512;

		ret = ata_dev_read_sectors(pdata, datalen, 0, 1);

		if (ret == TRUE)
			break;

		i++;
		if (i > (ATA_RESET_TIME * 100)) {
			printf("** TimeOUT **\n");
			dev_state = SATA_NODEVICE;
			return FALSE;
		}

		if ((i >= 100) && ((i % 100) == 0))
			printf(".");
	}

	dev_state = SATA_READY;

	return TRUE;
}

static unsigned int ata_dev_set_feature(struct ata_device *dev,
				u8 enable, u8 feature)
{
	struct ata_taskfile tf;
	struct ata_port *ap;
	ap = pap;
	unsigned int err_mask;

	memset(&tf, 0, sizeof(tf));
	tf.ctl = ap->ctl;

	tf.device = ATA_DEVICE_OBS;
	tf.command = ATA_CMD_SET_FEATURES;
	tf.feature = enable;
	tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
	tf.protocol = ATA_PROT_NODATA;
	tf.nsect = feature;

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_NONE, 0, 0);

	return err_mask;
}

static unsigned int ata_dev_init_params(struct ata_device *dev,
				u16 heads, u16 sectors)
{
	struct ata_taskfile tf;
	struct ata_port *ap;
	ap = pap;
	unsigned int err_mask;

	if (sectors < 1 || sectors > 255 || heads < 1 || heads > 16)
		return AC_ERR_INVALID;

	memset(&tf, 0, sizeof(tf));
	tf.ctl = ap->ctl;
	tf.device = ATA_DEVICE_OBS;
	tf.command = ATA_CMD_INIT_DEV_PARAMS;
	tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
	tf.protocol = ATA_PROT_NODATA;
	tf.nsect = sectors;
	tf.device |= (heads - 1) & 0x0f;

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_NONE, 0, 0);

	if (err_mask == AC_ERR_DEV && (tf.feature & ATA_ABORTED))
		err_mask = 0;

	return err_mask;
}

#if defined(CONFIG_SATA_DWC) && !defined(CONFIG_LBA48)
#define SATA_MAX_READ_BLK 0xFF
#else
#define SATA_MAX_READ_BLK 0xFFFF
#endif

ulong sata_read(int device, ulong blknr, lbaint_t blkcnt, void *buffer)
{
	ulong start,blks, buf_addr;
	unsigned short smallblks;
	unsigned long datalen;
	unsigned char *pdata;
	device &= 0xff;

	u32 block = 0;
	u32 n_block = 0;

	if (dev_state != SATA_READY)
		return 0;

	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	do {
		pdata = (unsigned char *)buf_addr;
		if (blks > SATA_MAX_READ_BLK) {
			datalen = sata_dev_desc[device].blksz * SATA_MAX_READ_BLK;
			smallblks = SATA_MAX_READ_BLK;

			block = (u32)start;
			n_block = (u32)smallblks;

			start += SATA_MAX_READ_BLK;
			blks -= SATA_MAX_READ_BLK;
		} else {
			datalen = sata_dev_desc[device].blksz * SATA_MAX_READ_BLK;
			datalen = sata_dev_desc[device].blksz * blks;
			smallblks = (unsigned short)blks;

			block = (u32)start;
			n_block = (u32)smallblks;

			start += blks;
			blks = 0;
		}

		if (ata_dev_read_sectors(pdata, datalen, block, n_block) != TRUE) {
			printf("sata_dwc : Hard disk read error.\n");
			blkcnt -= blks;
			break;
		}
		buf_addr += datalen;
	} while (blks != 0);

	return (blkcnt);
}

static int ata_dev_read_sectors(unsigned char *pdata, unsigned long datalen,
						u32 block, u32 n_block)
{
	struct ata_port *ap = pap;
	struct ata_device *dev = &ata_device;
	struct ata_taskfile tf;
	unsigned int class = ATA_DEV_ATA;
	unsigned int err_mask = 0;
	const char *reason;
	int may_fallback = 1;

	if (dev_state == SATA_ERROR)
		return FALSE;

	ata_dev_select(ap, dev->devno, 1, 1);

retry:
	memset(&tf, 0, sizeof(tf));
	tf.ctl = ap->ctl;
	ap->print_id = 1;
	ap->flags &= ~ATA_FLAG_DISABLED;

	ap->pdata = pdata;

	tf.device = ATA_DEVICE_OBS;

	temp_n_block = n_block;

#ifdef CONFIG_LBA48
	tf.command = ATA_CMD_PIO_READ_EXT;
	tf.flags |= ATA_TFLAG_LBA | ATA_TFLAG_LBA48;

	tf.hob_feature = 31;
	tf.feature = 31;
	tf.hob_nsect = (n_block >> 8) & 0xff;
	tf.nsect = n_block & 0xff;

	tf.hob_lbah = 0x0;
	tf.hob_lbam = 0x0;
	tf.hob_lbal = (block >> 24) & 0xff;
	tf.lbah = (block >> 16) & 0xff;
	tf.lbam = (block >> 8) & 0xff;
	tf.lbal = block & 0xff;

	tf.device = 1 << 6;
	if (tf.flags & ATA_TFLAG_FUA)
		tf.device |= 1 << 7;
#else
	tf.command = ATA_CMD_PIO_READ;
	tf.flags |= ATA_TFLAG_LBA ;

	tf.feature = 31;
	tf.nsect = n_block & 0xff;

	tf.lbah = (block >> 16) & 0xff;
	tf.lbam = (block >> 8) & 0xff;
	tf.lbal = block & 0xff;

	tf.device = (block >> 24) & 0xf;

	tf.device |= 1 << 6;
	if (tf.flags & ATA_TFLAG_FUA)
		tf.device |= 1 << 7;

#endif

	tf.protocol = ATA_PROT_PIO;

	/* Some devices choke if TF registers contain garbage.  Make
	 * sure those are properly initialized.
	 */
	tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
	tf.flags |= ATA_TFLAG_POLLING;

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_FROM_DEVICE, 0, 0);

	if (err_mask) {
		if (err_mask & AC_ERR_NODEV_HINT) {
			printf("READ_SECTORS NODEV after polling detection\n");
			return -ENOENT;
		}

		if ((err_mask == AC_ERR_DEV) && (tf.feature & ATA_ABORTED)) {
			/* Device or controller might have reported
			 * the wrong device class.  Give a shot at the
			 * other IDENTIFY if the current one is
			 * aborted by the device.
			 */
			if (may_fallback) {
				may_fallback = 0;

				if (class == ATA_DEV_ATA) {
					class = ATA_DEV_ATAPI;
				} else {
					class = ATA_DEV_ATA;
				}
				goto retry;
			}
			/* Control reaches here iff the device aborted
			 * both flavors of IDENTIFYs which happens
			 * sometimes with phantom devices.
			 */
			printf("both IDENTIFYs aborted, assuming NODEV\n");
			return -ENOENT;
		}

		reason = "I/O error";
		goto err_out;
	}

	return TRUE;

err_out:
	printf("failed to READ SECTORS (%s, err_mask=0x%x)\n", reason, err_mask);
	return FALSE;
}

#if defined(CONFIG_SATA_DWC) && !defined(CONFIG_LBA48)
#define SATA_MAX_WRITE_BLK 0xFF
#else
#define SATA_MAX_WRITE_BLK 0xFFFF
#endif

ulong sata_write(int device, ulong blknr, lbaint_t blkcnt, const void *buffer)
{
	ulong start,blks, buf_addr;
	unsigned short smallblks;
	unsigned long datalen;
	unsigned char *pdata;
	device &= 0xff;


	u32 block = 0;
	u32 n_block = 0;

	if (dev_state != SATA_READY)
		return 0;

	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	do {
		pdata = (unsigned char *)buf_addr;
		if (blks > SATA_MAX_WRITE_BLK) {
			datalen = sata_dev_desc[device].blksz * SATA_MAX_WRITE_BLK;
			smallblks = SATA_MAX_WRITE_BLK;

			block = (u32)start;
			n_block = (u32)smallblks;

			start += SATA_MAX_WRITE_BLK;
			blks -= SATA_MAX_WRITE_BLK;
		} else {
			datalen = sata_dev_desc[device].blksz * blks;
			smallblks = (unsigned short)blks;

			block = (u32)start;
			n_block = (u32)smallblks;

			start += blks;
			blks = 0;
		}

		if (ata_dev_write_sectors(pdata, datalen, block, n_block) != TRUE) {
			printf("sata_dwc : Hard disk read error.\n");
			blkcnt -= blks;
			break;
		}
		buf_addr += datalen;
	} while (blks != 0);

	return (blkcnt);
}

static int ata_dev_write_sectors(unsigned char* pdata, unsigned long datalen,
						u32 block, u32 n_block)
{
	struct ata_port *ap = pap;
	struct ata_device *dev = &ata_device;
	struct ata_taskfile tf;
	unsigned int class = ATA_DEV_ATA;
	unsigned int err_mask = 0;
	const char *reason;
	int may_fallback = 1;

	if (dev_state == SATA_ERROR)
		return FALSE;

	ata_dev_select(ap, dev->devno, 1, 1);

retry:
	memset(&tf, 0, sizeof(tf));
	tf.ctl = ap->ctl;
	ap->print_id = 1;
	ap->flags &= ~ATA_FLAG_DISABLED;

	ap->pdata = pdata;

	tf.device = ATA_DEVICE_OBS;

	temp_n_block = n_block;


#ifdef CONFIG_LBA48
	tf.command = ATA_CMD_PIO_WRITE_EXT;
	tf.flags |= ATA_TFLAG_LBA | ATA_TFLAG_LBA48 | ATA_TFLAG_WRITE;

	tf.hob_feature = 31;
	tf.feature = 31;
	tf.hob_nsect = (n_block >> 8) & 0xff;
	tf.nsect = n_block & 0xff;

	tf.hob_lbah = 0x0;
	tf.hob_lbam = 0x0;
	tf.hob_lbal = (block >> 24) & 0xff;
	tf.lbah = (block >> 16) & 0xff;
	tf.lbam = (block >> 8) & 0xff;
	tf.lbal = block & 0xff;

	tf.device = 1 << 6;
	if (tf.flags & ATA_TFLAG_FUA)
		tf.device |= 1 << 7;
#else
	tf.command = ATA_CMD_PIO_WRITE;
	tf.flags |= ATA_TFLAG_LBA | ATA_TFLAG_WRITE;

	tf.feature = 31;
	tf.nsect = n_block & 0xff;

	tf.lbah = (block >> 16) & 0xff;
	tf.lbam = (block >> 8) & 0xff;
	tf.lbal = block & 0xff;

	tf.device = (block >> 24) & 0xf;

	tf.device |= 1 << 6;
	if (tf.flags & ATA_TFLAG_FUA)
		tf.device |= 1 << 7;

#endif

	tf.protocol = ATA_PROT_PIO;

	/* Some devices choke if TF registers contain garbage.  Make
	 * sure those are properly initialized.
	 */
	tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
	tf.flags |= ATA_TFLAG_POLLING;

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_FROM_DEVICE, 0, 0);

	if (err_mask) {
		if (err_mask & AC_ERR_NODEV_HINT) {
			printf("READ_SECTORS NODEV after polling detection\n");
			return -ENOENT;
		}

		if ((err_mask == AC_ERR_DEV) && (tf.feature & ATA_ABORTED)) {
			/* Device or controller might have reported
			 * the wrong device class.  Give a shot at the
			 * other IDENTIFY if the current one is
			 * aborted by the device.
			 */
			if (may_fallback) {
				may_fallback = 0;

				if (class == ATA_DEV_ATA) {
					class = ATA_DEV_ATAPI;
				} else {
					class = ATA_DEV_ATA;
				}
				goto retry;
			}
			/* Control reaches here iff the device aborted
			 * both flavors of IDENTIFYs which happens
			 * sometimes with phantom devices.
			 */
			printf("both IDENTIFYs aborted, assuming NODEV\n");
			return -ENOENT;
		}

		reason = "I/O error";
		goto err_out;
	}

	return TRUE;

err_out:
	printf("failed to WRITE SECTORS (%s, err_mask=0x%x)\n", reason, err_mask);
	return FALSE;
}
