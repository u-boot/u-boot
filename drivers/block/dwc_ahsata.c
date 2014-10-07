/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
 * Terry Lv <r65388@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <libata.h>
#include <ahci.h>
#include <fis.h>
#include <sata.h>

#include <common.h>
#include <malloc.h>
#include <linux/ctype.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include "dwc_ahsata.h"

struct sata_port_regs {
	u32 clb;
	u32 clbu;
	u32 fb;
	u32 fbu;
	u32 is;
	u32 ie;
	u32 cmd;
	u32 res1[1];
	u32 tfd;
	u32 sig;
	u32 ssts;
	u32 sctl;
	u32 serr;
	u32 sact;
	u32 ci;
	u32 sntf;
	u32 res2[1];
	u32 dmacr;
	u32 res3[1];
	u32 phycr;
	u32 physr;
};

struct sata_host_regs {
	u32 cap;
	u32 ghc;
	u32 is;
	u32 pi;
	u32 vs;
	u32 ccc_ctl;
	u32 ccc_ports;
	u32 res1[2];
	u32 cap2;
	u32 res2[30];
	u32 bistafr;
	u32 bistcr;
	u32 bistfctr;
	u32 bistsr;
	u32 bistdecr;
	u32 res3[2];
	u32 oobr;
	u32 res4[8];
	u32 timer1ms;
	u32 res5[1];
	u32 gparam1r;
	u32 gparam2r;
	u32 pparamr;
	u32 testr;
	u32 versionr;
	u32 idr;
};

#define MAX_DATA_BYTES_PER_SG  (4 * 1024 * 1024)
#define MAX_BYTES_PER_TRANS (AHCI_MAX_SG * MAX_DATA_BYTES_PER_SG)

#define writel_with_flush(a, b)	do { writel(a, b); readl(b); } while (0)

static int is_ready;

static inline u32 ahci_port_base(u32 base, u32 port)
{
	return base + 0x100 + (port * 0x80);
}

static int waiting_for_cmd_completed(u8 *offset,
					int timeout_msec,
					u32 sign)
{
	int i;
	u32 status;

	for (i = 0;
		((status = readl(offset)) & sign) && i < timeout_msec;
		++i)
		mdelay(1);

	return (i < timeout_msec) ? 0 : -1;
}

static int ahci_setup_oobr(struct ahci_probe_ent *probe_ent,
						int clk)
{
	struct sata_host_regs *host_mmio =
		(struct sata_host_regs *)probe_ent->mmio_base;

	writel(SATA_HOST_OOBR_WE, &(host_mmio->oobr));
	writel(0x02060b14, &(host_mmio->oobr));

	return 0;
}

static int ahci_host_init(struct ahci_probe_ent *probe_ent)
{
	u32 tmp, cap_save, num_ports;
	int i, j, timeout = 1000;
	struct sata_port_regs *port_mmio = NULL;
	struct sata_host_regs *host_mmio =
		(struct sata_host_regs *)probe_ent->mmio_base;
	int clk = mxc_get_clock(MXC_SATA_CLK);

	cap_save = readl(&(host_mmio->cap));
	cap_save |= SATA_HOST_CAP_SSS;

	/* global controller reset */
	tmp = readl(&(host_mmio->ghc));
	if ((tmp & SATA_HOST_GHC_HR) == 0)
		writel_with_flush(tmp | SATA_HOST_GHC_HR, &(host_mmio->ghc));

	while ((readl(&(host_mmio->ghc)) & SATA_HOST_GHC_HR)
		&& --timeout)
		;

	if (timeout <= 0) {
		debug("controller reset failed (0x%x)\n", tmp);
		return -1;
	}

	/* Set timer 1ms */
	writel(clk / 1000, &(host_mmio->timer1ms));

	ahci_setup_oobr(probe_ent, 0);

	writel_with_flush(SATA_HOST_GHC_AE, &(host_mmio->ghc));
	writel(cap_save, &(host_mmio->cap));
	num_ports = (cap_save & SATA_HOST_CAP_NP_MASK) + 1;
	writel_with_flush((1 << num_ports) - 1,
				&(host_mmio->pi));

	/*
	 * Determine which Ports are implemented by the DWC_ahsata,
	 * by reading the PI register. This bit map value aids the
	 * software to determine how many Ports are available and
	 * which Port registers need to be initialized.
	 */
	probe_ent->cap = readl(&(host_mmio->cap));
	probe_ent->port_map = readl(&(host_mmio->pi));

	/* Determine how many command slots the HBA supports */
	probe_ent->n_ports =
		(probe_ent->cap & SATA_HOST_CAP_NP_MASK) + 1;

	debug("cap 0x%x  port_map 0x%x  n_ports %d\n",
		probe_ent->cap, probe_ent->port_map, probe_ent->n_ports);

	for (i = 0; i < probe_ent->n_ports; i++) {
		probe_ent->port[i].port_mmio =
			ahci_port_base((u32)host_mmio, i);
		port_mmio =
			(struct sata_port_regs *)probe_ent->port[i].port_mmio;

		/* Ensure that the DWC_ahsata is in idle state */
		tmp = readl(&(port_mmio->cmd));

		/*
		 * When P#CMD.ST, P#CMD.CR, P#CMD.FRE and P#CMD.FR
		 * are all cleared, the Port is in an idle state.
		 */
		if (tmp & (SATA_PORT_CMD_CR | SATA_PORT_CMD_FR |
			SATA_PORT_CMD_FRE | SATA_PORT_CMD_ST)) {

			/*
			 * System software places a Port into the idle state by
			 * clearing P#CMD.ST and waiting for P#CMD.CR to return
			 * 0 when read.
			 */
			tmp &= ~SATA_PORT_CMD_ST;
			writel_with_flush(tmp, &(port_mmio->cmd));

			/*
			 * spec says 500 msecs for each bit, so
			 * this is slightly incorrect.
			 */
			mdelay(500);

			timeout = 1000;
			while ((readl(&(port_mmio->cmd)) & SATA_PORT_CMD_CR)
				&& --timeout)
				;

			if (timeout <= 0) {
				debug("port reset failed (0x%x)\n", tmp);
				return -1;
			}
		}

		/* Spin-up device */
		tmp = readl(&(port_mmio->cmd));
		writel((tmp | SATA_PORT_CMD_SUD), &(port_mmio->cmd));

		/* Wait for spin-up to finish */
		timeout = 1000;
		while (!(readl(&(port_mmio->cmd)) | SATA_PORT_CMD_SUD)
			&& --timeout)
			;
		if (timeout <= 0) {
			debug("Spin-Up can't finish!\n");
			return -1;
		}

		for (j = 0; j < 100; ++j) {
			mdelay(10);
			tmp = readl(&(port_mmio->ssts));
			if (((tmp & SATA_PORT_SSTS_DET_MASK) == 0x3) ||
				((tmp & SATA_PORT_SSTS_DET_MASK) == 0x1))
				break;
		}

		/* Wait for COMINIT bit 26 (DIAG_X) in SERR */
		timeout = 1000;
		while (!(readl(&(port_mmio->serr)) | SATA_PORT_SERR_DIAG_X)
			&& --timeout)
			;
		if (timeout <= 0) {
			debug("Can't find DIAG_X set!\n");
			return -1;
		}

		/*
		 * For each implemented Port, clear the P#SERR
		 * register, by writing ones to each implemented\
		 * bit location.
		 */
		tmp = readl(&(port_mmio->serr));
		debug("P#SERR 0x%x\n",
				tmp);
		writel(tmp, &(port_mmio->serr));

		/* Ack any pending irq events for this port */
		tmp = readl(&(host_mmio->is));
		debug("IS 0x%x\n", tmp);
		if (tmp)
			writel(tmp, &(host_mmio->is));

		writel(1 << i, &(host_mmio->is));

		/* set irq mask (enables interrupts) */
		writel(DEF_PORT_IRQ, &(port_mmio->ie));

		/* register linkup ports */
		tmp = readl(&(port_mmio->ssts));
		debug("Port %d status: 0x%x\n", i, tmp);
		if ((tmp & SATA_PORT_SSTS_DET_MASK) == 0x03)
			probe_ent->link_port_map |= (0x01 << i);
	}

	tmp = readl(&(host_mmio->ghc));
	debug("GHC 0x%x\n", tmp);
	writel(tmp | SATA_HOST_GHC_IE, &(host_mmio->ghc));
	tmp = readl(&(host_mmio->ghc));
	debug("GHC 0x%x\n", tmp);

	return 0;
}

static void ahci_print_info(struct ahci_probe_ent *probe_ent)
{
	struct sata_host_regs *host_mmio =
		(struct sata_host_regs *)probe_ent->mmio_base;
	u32 vers, cap, impl, speed;
	const char *speed_s;
	const char *scc_s;

	vers = readl(&(host_mmio->vs));
	cap = probe_ent->cap;
	impl = probe_ent->port_map;

	speed = (cap & SATA_HOST_CAP_ISS_MASK)
		>> SATA_HOST_CAP_ISS_OFFSET;
	if (speed == 1)
		speed_s = "1.5";
	else if (speed == 2)
		speed_s = "3";
	else
		speed_s = "?";

	scc_s = "SATA";

	printf("AHCI %02x%02x.%02x%02x "
		"%u slots %u ports %s Gbps 0x%x impl %s mode\n",
		(vers >> 24) & 0xff,
		(vers >> 16) & 0xff,
		(vers >> 8) & 0xff,
		vers & 0xff,
		((cap >> 8) & 0x1f) + 1,
		(cap & 0x1f) + 1,
		speed_s,
		impl,
		scc_s);

	printf("flags: "
		"%s%s%s%s%s%s"
		"%s%s%s%s%s%s%s\n",
		cap & (1 << 31) ? "64bit " : "",
		cap & (1 << 30) ? "ncq " : "",
		cap & (1 << 28) ? "ilck " : "",
		cap & (1 << 27) ? "stag " : "",
		cap & (1 << 26) ? "pm " : "",
		cap & (1 << 25) ? "led " : "",
		cap & (1 << 24) ? "clo " : "",
		cap & (1 << 19) ? "nz " : "",
		cap & (1 << 18) ? "only " : "",
		cap & (1 << 17) ? "pmp " : "",
		cap & (1 << 15) ? "pio " : "",
		cap & (1 << 14) ? "slum " : "",
		cap & (1 << 13) ? "part " : "");
}

static int ahci_init_one(int pdev)
{
	int rc;
	struct ahci_probe_ent *probe_ent = NULL;

	probe_ent = malloc(sizeof(struct ahci_probe_ent));
	memset(probe_ent, 0, sizeof(struct ahci_probe_ent));
	probe_ent->dev = pdev;

	probe_ent->host_flags = ATA_FLAG_SATA
				| ATA_FLAG_NO_LEGACY
				| ATA_FLAG_MMIO
				| ATA_FLAG_PIO_DMA
				| ATA_FLAG_NO_ATAPI;

	probe_ent->mmio_base = CONFIG_DWC_AHSATA_BASE_ADDR;

	/* initialize adapter */
	rc = ahci_host_init(probe_ent);
	if (rc)
		goto err_out;

	ahci_print_info(probe_ent);

	/* Save the private struct to block device struct */
	sata_dev_desc[pdev].priv = (void *)probe_ent;

	return 0;

err_out:
	return rc;
}

static int ahci_fill_sg(struct ahci_probe_ent *probe_ent,
			u8 port, unsigned char *buf, int buf_len)
{
	struct ahci_ioports *pp = &(probe_ent->port[port]);
	struct ahci_sg *ahci_sg = pp->cmd_tbl_sg;
	u32 sg_count, max_bytes;
	int i;

	max_bytes = MAX_DATA_BYTES_PER_SG;
	sg_count = ((buf_len - 1) / max_bytes) + 1;
	if (sg_count > AHCI_MAX_SG) {
		printf("Error:Too much sg!\n");
		return -1;
	}

	for (i = 0; i < sg_count; i++) {
		ahci_sg->addr =
			cpu_to_le32((u32)buf + i * max_bytes);
		ahci_sg->addr_hi = 0;
		ahci_sg->flags_size = cpu_to_le32(0x3fffff &
					(buf_len < max_bytes
					? (buf_len - 1)
					: (max_bytes - 1)));
		ahci_sg++;
		buf_len -= max_bytes;
	}

	return sg_count;
}

static void ahci_fill_cmd_slot(struct ahci_ioports *pp, u32 cmd_slot, u32 opts)
{
	struct ahci_cmd_hdr *cmd_hdr = (struct ahci_cmd_hdr *)(pp->cmd_slot +
					AHCI_CMD_SLOT_SZ * cmd_slot);

	memset(cmd_hdr, 0, AHCI_CMD_SLOT_SZ);
	cmd_hdr->opts = cpu_to_le32(opts);
	cmd_hdr->status = 0;
	cmd_hdr->tbl_addr = cpu_to_le32(pp->cmd_tbl & 0xffffffff);
	cmd_hdr->tbl_addr_hi = 0;
}

#define AHCI_GET_CMD_SLOT(c) ((c) ? ffs(c) : 0)

static int ahci_exec_ata_cmd(struct ahci_probe_ent *probe_ent,
		u8 port, struct sata_fis_h2d *cfis,
		u8 *buf, u32 buf_len, s32 is_write)
{
	struct ahci_ioports *pp = &(probe_ent->port[port]);
	struct sata_port_regs *port_mmio =
			(struct sata_port_regs *)pp->port_mmio;
	u32 opts;
	int sg_count = 0, cmd_slot = 0;

	cmd_slot = AHCI_GET_CMD_SLOT(readl(&(port_mmio->ci)));
	if (32 == cmd_slot) {
		printf("Can't find empty command slot!\n");
		return 0;
	}

	/* Check xfer length */
	if (buf_len > MAX_BYTES_PER_TRANS) {
		printf("Max transfer length is %dB\n\r",
			MAX_BYTES_PER_TRANS);
		return 0;
	}

	memcpy((u8 *)(pp->cmd_tbl), cfis, sizeof(struct sata_fis_h2d));
	if (buf && buf_len)
		sg_count = ahci_fill_sg(probe_ent, port, buf, buf_len);
	opts = (sizeof(struct sata_fis_h2d) >> 2) | (sg_count << 16);
	if (is_write) {
		opts |= 0x40;
		flush_cache((ulong)buf, buf_len);
	}
	ahci_fill_cmd_slot(pp, cmd_slot, opts);

	flush_cache((int)(pp->cmd_slot), AHCI_PORT_PRIV_DMA_SZ);
	writel_with_flush(1 << cmd_slot, &(port_mmio->ci));

	if (waiting_for_cmd_completed((u8 *)&(port_mmio->ci),
				10000, 0x1 << cmd_slot)) {
		printf("timeout exit!\n");
		return -1;
	}
	invalidate_dcache_range((int)(pp->cmd_slot),
				(int)(pp->cmd_slot)+AHCI_PORT_PRIV_DMA_SZ);
	debug("ahci_exec_ata_cmd: %d byte transferred.\n",
	      pp->cmd_slot->status);
	if (!is_write)
		invalidate_dcache_range((ulong)buf, (ulong)buf+buf_len);

	return buf_len;
}

static void ahci_set_feature(u8 dev, u8 port)
{
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));
	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 1 << 7;
	cfis->command = ATA_CMD_SET_FEATURES;
	cfis->features = SETFEATURES_XFER;
	cfis->sector_count = ffs(probe_ent->udma_mask + 1) + 0x3e;

	ahci_exec_ata_cmd(probe_ent, port, cfis, NULL, 0, READ_CMD);
}

static int ahci_port_start(struct ahci_probe_ent *probe_ent,
					u8 port)
{
	struct ahci_ioports *pp = &(probe_ent->port[port]);
	struct sata_port_regs *port_mmio =
		(struct sata_port_regs *)pp->port_mmio;
	u32 port_status;
	u32 mem;
	int timeout = 10000000;

	debug("Enter start port: %d\n", port);
	port_status = readl(&(port_mmio->ssts));
	debug("Port %d status: %x\n", port, port_status);
	if ((port_status & 0xf) != 0x03) {
		printf("No Link on this port!\n");
		return -1;
	}

	mem = (u32)malloc(AHCI_PORT_PRIV_DMA_SZ + 1024);
	if (!mem) {
		free(pp);
		printf("No mem for table!\n");
		return -ENOMEM;
	}

	mem = (mem + 0x400) & (~0x3ff);	/* Aligned to 1024-bytes */
	memset((u8 *)mem, 0, AHCI_PORT_PRIV_DMA_SZ);

	/*
	 * First item in chunk of DMA memory: 32-slot command table,
	 * 32 bytes each in size
	 */
	pp->cmd_slot = (struct ahci_cmd_hdr *)mem;
	debug("cmd_slot = 0x%x\n", (unsigned int) pp->cmd_slot);
	mem += (AHCI_CMD_SLOT_SZ * DWC_AHSATA_MAX_CMD_SLOTS);

	/*
	 * Second item: Received-FIS area, 256-Byte aligned
	 */
	pp->rx_fis = mem;
	mem += AHCI_RX_FIS_SZ;

	/*
	 * Third item: data area for storing a single command
	 * and its scatter-gather table
	 */
	pp->cmd_tbl = mem;
	debug("cmd_tbl_dma = 0x%x\n", pp->cmd_tbl);

	mem += AHCI_CMD_TBL_HDR;

	writel_with_flush(0x00004444, &(port_mmio->dmacr));
	pp->cmd_tbl_sg = (struct ahci_sg *)mem;
	writel_with_flush((u32)pp->cmd_slot, &(port_mmio->clb));
	writel_with_flush(pp->rx_fis, &(port_mmio->fb));

	/* Enable FRE */
	writel_with_flush((SATA_PORT_CMD_FRE | readl(&(port_mmio->cmd))),
			&(port_mmio->cmd));

	/* Wait device ready */
	while ((readl(&(port_mmio->tfd)) & (SATA_PORT_TFD_STS_ERR |
		SATA_PORT_TFD_STS_DRQ | SATA_PORT_TFD_STS_BSY))
		&& --timeout)
		;
	if (timeout <= 0) {
		debug("Device not ready for BSY, DRQ and"
			"ERR in TFD!\n");
		return -1;
	}

	writel_with_flush(PORT_CMD_ICC_ACTIVE | PORT_CMD_FIS_RX |
			  PORT_CMD_POWER_ON | PORT_CMD_SPIN_UP |
			  PORT_CMD_START, &(port_mmio->cmd));

	debug("Exit start port %d\n", port);

	return 0;
}

int init_sata(int dev)
{
	int i;
	u32 linkmap;
	struct ahci_probe_ent *probe_ent = NULL;

#if defined(CONFIG_MX6)
	if (!is_cpu_type(MXC_CPU_MX6Q) && !is_cpu_type(MXC_CPU_MX6D))
		return 1;
#endif
	if (dev < 0 || dev > (CONFIG_SYS_SATA_MAX_DEVICE - 1)) {
		printf("The sata index %d is out of ranges\n\r", dev);
		return -1;
	}

	ahci_init_one(dev);

	probe_ent = (struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	linkmap = probe_ent->link_port_map;

	if (0 == linkmap) {
		printf("No port device detected!\n");
		return 1;
	}

	for (i = 0; i < probe_ent->n_ports; i++) {
		if ((linkmap >> i) && ((linkmap >> i) & 0x01)) {
			if (ahci_port_start(probe_ent, (u8)i)) {
				printf("Can not start port %d\n", i);
				return 1;
			}
			probe_ent->hard_port_no = i;
			break;
		}
	}

	return 0;
}

static void dwc_ahsata_print_info(int dev)
{
	block_dev_desc_t *pdev = &(sata_dev_desc[dev]);

	printf("SATA Device Info:\n\r");
#ifdef CONFIG_SYS_64BIT_LBA
	printf("S/N: %s\n\rProduct model number: %s\n\r"
		"Firmware version: %s\n\rCapacity: %lld sectors\n\r",
		pdev->product, pdev->vendor, pdev->revision, pdev->lba);
#else
	printf("S/N: %s\n\rProduct model number: %s\n\r"
		"Firmware version: %s\n\rCapacity: %ld sectors\n\r",
		pdev->product, pdev->vendor, pdev->revision, pdev->lba);
#endif
}

static void dwc_ahsata_identify(int dev, u16 *id)
{
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = probe_ent->hard_port_no;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_ID_ATA;

	ahci_exec_ata_cmd(probe_ent, port, cfis,
			(u8 *)id, ATA_ID_WORDS * 2, READ_CMD);
	ata_swap_buf_le16(id, ATA_ID_WORDS);
}

static void dwc_ahsata_xfer_mode(int dev, u16 *id)
{
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;

	probe_ent->pio_mask = id[ATA_ID_PIO_MODES];
	probe_ent->udma_mask = id[ATA_ID_UDMA_MODES];
	debug("pio %04x, udma %04x\n\r",
		probe_ent->pio_mask, probe_ent->udma_mask);
}

static u32 dwc_ahsata_rw_cmd(int dev, u32 start, u32 blkcnt,
				u8 *buffer, int is_write)
{
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = probe_ent->hard_port_no;
	u32 block;

	block = start;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = (is_write) ? ATA_CMD_WRITE : ATA_CMD_READ;
	cfis->device = ATA_LBA;

	cfis->device |= (block >> 24) & 0xf;
	cfis->lba_high = (block >> 16) & 0xff;
	cfis->lba_mid = (block >> 8) & 0xff;
	cfis->lba_low = block & 0xff;
	cfis->sector_count = (u8)(blkcnt & 0xff);

	if (ahci_exec_ata_cmd(probe_ent, port, cfis,
			buffer, ATA_SECT_SIZE * blkcnt, is_write) > 0)
		return blkcnt;
	else
		return 0;
}

void dwc_ahsata_flush_cache(int dev)
{
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = probe_ent->hard_port_no;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_FLUSH;

	ahci_exec_ata_cmd(probe_ent, port, cfis, NULL, 0, 0);
}

static u32 dwc_ahsata_rw_cmd_ext(int dev, u32 start, lbaint_t blkcnt,
				u8 *buffer, int is_write)
{
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = probe_ent->hard_port_no;
	u64 block;

	block = (u64)start;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */

	cfis->command = (is_write) ? ATA_CMD_WRITE_EXT
				 : ATA_CMD_READ_EXT;

	cfis->lba_high_exp = (block >> 40) & 0xff;
	cfis->lba_mid_exp = (block >> 32) & 0xff;
	cfis->lba_low_exp = (block >> 24) & 0xff;
	cfis->lba_high = (block >> 16) & 0xff;
	cfis->lba_mid = (block >> 8) & 0xff;
	cfis->lba_low = block & 0xff;
	cfis->device = ATA_LBA;
	cfis->sector_count_exp = (blkcnt >> 8) & 0xff;
	cfis->sector_count = blkcnt & 0xff;

	if (ahci_exec_ata_cmd(probe_ent, port, cfis, buffer,
			ATA_SECT_SIZE * blkcnt, is_write) > 0)
		return blkcnt;
	else
		return 0;
}

u32 dwc_ahsata_rw_ncq_cmd(int dev, u32 start, lbaint_t blkcnt,
				u8 *buffer, int is_write)
{
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = probe_ent->hard_port_no;
	u64 block;

	if (sata_dev_desc[dev].lba48 != 1) {
		printf("execute FPDMA command on non-LBA48 hard disk\n\r");
		return -1;
	}

	block = (u64)start;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */

	cfis->command = (is_write) ? ATA_CMD_FPDMA_WRITE
				 : ATA_CMD_FPDMA_READ;

	cfis->lba_high_exp = (block >> 40) & 0xff;
	cfis->lba_mid_exp = (block >> 32) & 0xff;
	cfis->lba_low_exp = (block >> 24) & 0xff;
	cfis->lba_high = (block >> 16) & 0xff;
	cfis->lba_mid = (block >> 8) & 0xff;
	cfis->lba_low = block & 0xff;

	cfis->device = ATA_LBA;
	cfis->features_exp = (blkcnt >> 8) & 0xff;
	cfis->features = blkcnt & 0xff;

	/* Use the latest queue */
	ahci_exec_ata_cmd(probe_ent, port, cfis,
			buffer, ATA_SECT_SIZE * blkcnt, is_write);

	return blkcnt;
}

void dwc_ahsata_flush_cache_ext(int dev)
{
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	struct sata_fis_h2d h2d __aligned(ARCH_DMA_MINALIGN);
	struct sata_fis_h2d *cfis = &h2d;
	u8 port = probe_ent->hard_port_no;

	memset(cfis, 0, sizeof(struct sata_fis_h2d));

	cfis->fis_type = SATA_FIS_TYPE_REGISTER_H2D;
	cfis->pm_port_c = 0x80; /* is command */
	cfis->command = ATA_CMD_FLUSH_EXT;

	ahci_exec_ata_cmd(probe_ent, port, cfis, NULL, 0, 0);
}

static void dwc_ahsata_init_wcache(int dev, u16 *id)
{
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;

	if (ata_id_has_wcache(id) && ata_id_wcache_enabled(id))
		probe_ent->flags |= SATA_FLAG_WCACHE;
	if (ata_id_has_flush(id))
		probe_ent->flags |= SATA_FLAG_FLUSH;
	if (ata_id_has_flush_ext(id))
		probe_ent->flags |= SATA_FLAG_FLUSH_EXT;
}

u32 ata_low_level_rw_lba48(int dev, u32 blknr, lbaint_t blkcnt,
				const void *buffer, int is_write)
{
	u32 start, blks;
	u8 *addr;
	int max_blks;

	start = blknr;
	blks = blkcnt;
	addr = (u8 *)buffer;

	max_blks = ATA_MAX_SECTORS_LBA48;

	do {
		if (blks > max_blks) {
			if (max_blks != dwc_ahsata_rw_cmd_ext(dev, start,
						max_blks, addr, is_write))
				return 0;
			start += max_blks;
			blks -= max_blks;
			addr += ATA_SECT_SIZE * max_blks;
		} else {
			if (blks != dwc_ahsata_rw_cmd_ext(dev, start,
						blks, addr, is_write))
				return 0;
			start += blks;
			blks = 0;
			addr += ATA_SECT_SIZE * blks;
		}
	} while (blks != 0);

	return blkcnt;
}

u32 ata_low_level_rw_lba28(int dev, u32 blknr, lbaint_t blkcnt,
				const void *buffer, int is_write)
{
	u32 start, blks;
	u8 *addr;
	int max_blks;

	start = blknr;
	blks = blkcnt;
	addr = (u8 *)buffer;

	max_blks = ATA_MAX_SECTORS;
	do {
		if (blks > max_blks) {
			if (max_blks != dwc_ahsata_rw_cmd(dev, start,
						max_blks, addr, is_write))
				return 0;
			start += max_blks;
			blks -= max_blks;
			addr += ATA_SECT_SIZE * max_blks;
		} else {
			if (blks != dwc_ahsata_rw_cmd(dev, start,
						blks, addr, is_write))
				return 0;
			start += blks;
			blks = 0;
			addr += ATA_SECT_SIZE * blks;
		}
	} while (blks != 0);

	return blkcnt;
}

int sata_port_status(int dev, int port)
{
	struct sata_port_regs *port_mmio;
	struct ahci_probe_ent *probe_ent = NULL;

	if (dev < 0 || dev > (CONFIG_SYS_SATA_MAX_DEVICE - 1))
		return -EINVAL;

	if (sata_dev_desc[dev].priv == NULL)
		return -ENODEV;

	probe_ent = (struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	port_mmio = (struct sata_port_regs *)probe_ent->port[port].port_mmio;

	return readl(&(port_mmio->ssts)) && SATA_PORT_SSTS_DET_MASK;
}

/*
 * SATA interface between low level driver and command layer
 */
ulong sata_read(int dev, ulong blknr, lbaint_t blkcnt, void *buffer)
{
	u32 rc;

	if (sata_dev_desc[dev].lba48)
		rc = ata_low_level_rw_lba48(dev, blknr, blkcnt,
						buffer, READ_CMD);
	else
		rc = ata_low_level_rw_lba28(dev, blknr, blkcnt,
						buffer, READ_CMD);
	return rc;
}

ulong sata_write(int dev, ulong blknr, lbaint_t blkcnt, const void *buffer)
{
	u32 rc;
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	u32 flags = probe_ent->flags;

	if (sata_dev_desc[dev].lba48) {
		rc = ata_low_level_rw_lba48(dev, blknr, blkcnt,
						buffer, WRITE_CMD);
		if ((flags & SATA_FLAG_WCACHE) &&
			(flags & SATA_FLAG_FLUSH_EXT))
			dwc_ahsata_flush_cache_ext(dev);
	} else {
		rc = ata_low_level_rw_lba28(dev, blknr, blkcnt,
						buffer, WRITE_CMD);
		if ((flags & SATA_FLAG_WCACHE) &&
			(flags & SATA_FLAG_FLUSH))
			dwc_ahsata_flush_cache(dev);
	}
	return rc;
}

int scan_sata(int dev)
{
	u8 serial[ATA_ID_SERNO_LEN + 1] = { 0 };
	u8 firmware[ATA_ID_FW_REV_LEN + 1] = { 0 };
	u8 product[ATA_ID_PROD_LEN + 1] = { 0 };
	u16 *id;
	u64 n_sectors;
	struct ahci_probe_ent *probe_ent =
		(struct ahci_probe_ent *)sata_dev_desc[dev].priv;
	u8 port = probe_ent->hard_port_no;
	block_dev_desc_t *pdev = &(sata_dev_desc[dev]);

	id = (u16 *)memalign(ARCH_DMA_MINALIGN,
				roundup(ARCH_DMA_MINALIGN,
					(ATA_ID_WORDS * 2)));
	if (!id) {
		printf("id malloc failed\n\r");
		return -1;
	}

	/* Identify device to get information */
	dwc_ahsata_identify(dev, id);

	/* Serial number */
	ata_id_c_string(id, serial, ATA_ID_SERNO, sizeof(serial));
	memcpy(pdev->product, serial, sizeof(serial));

	/* Firmware version */
	ata_id_c_string(id, firmware, ATA_ID_FW_REV, sizeof(firmware));
	memcpy(pdev->revision, firmware, sizeof(firmware));

	/* Product model */
	ata_id_c_string(id, product, ATA_ID_PROD, sizeof(product));
	memcpy(pdev->vendor, product, sizeof(product));

	/* Totoal sectors */
	n_sectors = ata_id_n_sectors(id);
	pdev->lba = (u32)n_sectors;

	pdev->type = DEV_TYPE_HARDDISK;
	pdev->blksz = ATA_SECT_SIZE;
	pdev->lun = 0 ;

	/* Check if support LBA48 */
	if (ata_id_has_lba48(id)) {
		pdev->lba48 = 1;
		debug("Device support LBA48\n\r");
	}

	/* Get the NCQ queue depth from device */
	probe_ent->flags &= (~SATA_FLAG_Q_DEP_MASK);
	probe_ent->flags |= ata_id_queue_depth(id);

	/* Get the xfer mode from device */
	dwc_ahsata_xfer_mode(dev, id);

	/* Get the write cache status from device */
	dwc_ahsata_init_wcache(dev, id);

	/* Set the xfer mode to highest speed */
	ahci_set_feature(dev, port);

	free((void *)id);

	dwc_ahsata_print_info(dev);

	is_ready = 1;

	return 0;
}
