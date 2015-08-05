/*
 * Copyright (C) Freescale Semiconductor, Inc. 2006.
 * Author: Jason Jin<Jason.jin@freescale.com>
 *         Zhang Wei<wei.zhang@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * with the reference on libata and ahci drvier in kernel
 */
#include <common.h>

#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <malloc.h>
#include <scsi.h>
#include <libata.h>
#include <linux/ctype.h>
#include <ahci.h>

static int ata_io_flush(u8 port);

struct ahci_probe_ent *probe_ent = NULL;
u16 *ataid[AHCI_MAX_PORTS];

#define writel_with_flush(a,b)	do { writel(a,b); readl(b); } while (0)

/*
 * Some controllers limit number of blocks they can read/write at once.
 * Contemporary SSD devices work much faster if the read/write size is aligned
 * to a power of 2.  Let's set default to 128 and allowing to be overwritten if
 * needed.
 */
#ifndef MAX_SATA_BLOCKS_READ_WRITE
#define MAX_SATA_BLOCKS_READ_WRITE	0x80
#endif

/* Maximum timeouts for each event */
#define WAIT_MS_SPINUP	20000
#define WAIT_MS_DATAIO	10000
#define WAIT_MS_FLUSH	5000
#define WAIT_MS_LINKUP	200

static inline void __iomem *ahci_port_base(void __iomem *base, u32 port)
{
	return base + 0x100 + (port * 0x80);
}


static void ahci_setup_port(struct ahci_ioports *port, void __iomem *base,
			    unsigned int port_idx)
{
	base = ahci_port_base(base, port_idx);

	port->cmd_addr = base;
	port->scr_addr = base + PORT_SCR;
}


#define msleep(a) udelay(a * 1000)

static void ahci_dcache_flush_range(unsigned long begin, unsigned long len)
{
	const unsigned long start = begin;
	const unsigned long end = start + len;

	debug("%s: flush dcache: [%#lx, %#lx)\n", __func__, start, end);
	flush_dcache_range(start, end);
}

/*
 * SATA controller DMAs to physical RAM.  Ensure data from the
 * controller is invalidated from dcache; next access comes from
 * physical RAM.
 */
static void ahci_dcache_invalidate_range(unsigned long begin, unsigned long len)
{
	const unsigned long start = begin;
	const unsigned long end = start + len;

	debug("%s: invalidate dcache: [%#lx, %#lx)\n", __func__, start, end);
	invalidate_dcache_range(start, end);
}

/*
 * Ensure data for SATA controller is flushed out of dcache and
 * written to physical memory.
 */
static void ahci_dcache_flush_sata_cmd(struct ahci_ioports *pp)
{
	ahci_dcache_flush_range((unsigned long)pp->cmd_slot,
				AHCI_PORT_PRIV_DMA_SZ);
}

static int waiting_for_cmd_completed(void __iomem *offset,
				     int timeout_msec,
				     u32 sign)
{
	int i;
	u32 status;

	for (i = 0; ((status = readl(offset)) & sign) && i < timeout_msec; i++)
		msleep(1);

	return (i < timeout_msec) ? 0 : -1;
}

int __weak ahci_link_up(struct ahci_probe_ent *probe_ent, u8 port)
{
	u32 tmp;
	int j = 0;
	void __iomem *port_mmio = probe_ent->port[port].port_mmio;

	/*
	 * Bring up SATA link.
	 * SATA link bringup time is usually less than 1 ms; only very
	 * rarely has it taken between 1-2 ms. Never seen it above 2 ms.
	 */
	while (j < WAIT_MS_LINKUP) {
		tmp = readl(port_mmio + PORT_SCR_STAT);
		tmp &= PORT_SCR_STAT_DET_MASK;
		if (tmp == PORT_SCR_STAT_DET_PHYRDY)
			return 0;
		udelay(1000);
		j++;
	}
	return 1;
}

#ifdef CONFIG_SUNXI_AHCI
/* The sunxi AHCI controller requires this undocumented setup */
static void sunxi_dma_init(void __iomem *port_mmio)
{
	clrsetbits_le32(port_mmio + PORT_P0DMACR, 0x0000ff00, 0x00004400);
}
#endif

int ahci_reset(void __iomem *base)
{
	int i = 1000;
	u32 __iomem *host_ctl_reg = base + HOST_CTL;
	u32 tmp = readl(host_ctl_reg); /* global controller reset */

	if ((tmp & HOST_RESET) == 0)
		writel_with_flush(tmp | HOST_RESET, host_ctl_reg);

	/*
	 * reset must complete within 1 second, or
	 * the hardware should be considered fried.
	 */
	do {
		udelay(1000);
		tmp = readl(host_ctl_reg);
		i--;
	} while ((i > 0) && (tmp & HOST_RESET));

	if (i == 0) {
		printf("controller reset failed (0x%x)\n", tmp);
		return -1;
	}

	return 0;
}

static int ahci_host_init(struct ahci_probe_ent *probe_ent)
{
#ifndef CONFIG_SCSI_AHCI_PLAT
	pci_dev_t pdev = probe_ent->dev;
	u16 tmp16;
	unsigned short vendor;
#endif
	void __iomem *mmio = probe_ent->mmio_base;
	u32 tmp, cap_save, cmd;
	int i, j, ret;
	void __iomem *port_mmio;
	u32 port_map;

	debug("ahci_host_init: start\n");

	cap_save = readl(mmio + HOST_CAP);
	cap_save &= ((1 << 28) | (1 << 17));
	cap_save |= (1 << 27);  /* Staggered Spin-up. Not needed. */

	ret = ahci_reset(probe_ent->mmio_base);
	if (ret)
		return ret;

	writel_with_flush(HOST_AHCI_EN, mmio + HOST_CTL);
	writel(cap_save, mmio + HOST_CAP);
	writel_with_flush(0xf, mmio + HOST_PORTS_IMPL);

#ifndef CONFIG_SCSI_AHCI_PLAT
	pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor);

	if (vendor == PCI_VENDOR_ID_INTEL) {
		u16 tmp16;
		pci_read_config_word(pdev, 0x92, &tmp16);
		tmp16 |= 0xf;
		pci_write_config_word(pdev, 0x92, tmp16);
	}
#endif
	probe_ent->cap = readl(mmio + HOST_CAP);
	probe_ent->port_map = readl(mmio + HOST_PORTS_IMPL);
	port_map = probe_ent->port_map;
	probe_ent->n_ports = (probe_ent->cap & 0x1f) + 1;

	debug("cap 0x%x  port_map 0x%x  n_ports %d\n",
	      probe_ent->cap, probe_ent->port_map, probe_ent->n_ports);

	if (probe_ent->n_ports > CONFIG_SYS_SCSI_MAX_SCSI_ID)
		probe_ent->n_ports = CONFIG_SYS_SCSI_MAX_SCSI_ID;

	for (i = 0; i < probe_ent->n_ports; i++) {
		if (!(port_map & (1 << i)))
			continue;
		probe_ent->port[i].port_mmio = ahci_port_base(mmio, i);
		port_mmio = (u8 *) probe_ent->port[i].port_mmio;
		ahci_setup_port(&probe_ent->port[i], mmio, i);

		/* make sure port is not active */
		tmp = readl(port_mmio + PORT_CMD);
		if (tmp & (PORT_CMD_LIST_ON | PORT_CMD_FIS_ON |
			   PORT_CMD_FIS_RX | PORT_CMD_START)) {
			debug("Port %d is active. Deactivating.\n", i);
			tmp &= ~(PORT_CMD_LIST_ON | PORT_CMD_FIS_ON |
				 PORT_CMD_FIS_RX | PORT_CMD_START);
			writel_with_flush(tmp, port_mmio + PORT_CMD);

			/* spec says 500 msecs for each bit, so
			 * this is slightly incorrect.
			 */
			msleep(500);
		}

#ifdef CONFIG_SUNXI_AHCI
		sunxi_dma_init(port_mmio);
#endif

		/* Add the spinup command to whatever mode bits may
		 * already be on in the command register.
		 */
		cmd = readl(port_mmio + PORT_CMD);
		cmd |= PORT_CMD_SPIN_UP;
		writel_with_flush(cmd, port_mmio + PORT_CMD);

		/* Bring up SATA link. */
		ret = ahci_link_up(probe_ent, i);
		if (ret) {
			printf("SATA link %d timeout.\n", i);
			continue;
		} else {
			debug("SATA link ok.\n");
		}

		/* Clear error status */
		tmp = readl(port_mmio + PORT_SCR_ERR);
		if (tmp)
			writel(tmp, port_mmio + PORT_SCR_ERR);

		debug("Spinning up device on SATA port %d... ", i);

		j = 0;
		while (j < WAIT_MS_SPINUP) {
			tmp = readl(port_mmio + PORT_TFDATA);
			if (!(tmp & (ATA_BUSY | ATA_DRQ)))
				break;
			udelay(1000);
			tmp = readl(port_mmio + PORT_SCR_STAT);
			tmp &= PORT_SCR_STAT_DET_MASK;
			if (tmp == PORT_SCR_STAT_DET_PHYRDY)
				break;
			j++;
		}

		tmp = readl(port_mmio + PORT_SCR_STAT) & PORT_SCR_STAT_DET_MASK;
		if (tmp == PORT_SCR_STAT_DET_COMINIT) {
			debug("SATA link %d down (COMINIT received), retrying...\n", i);
			i--;
			continue;
		}

		printf("Target spinup took %d ms.\n", j);
		if (j == WAIT_MS_SPINUP)
			debug("timeout.\n");
		else
			debug("ok.\n");

		tmp = readl(port_mmio + PORT_SCR_ERR);
		debug("PORT_SCR_ERR 0x%x\n", tmp);
		writel(tmp, port_mmio + PORT_SCR_ERR);

		/* ack any pending irq events for this port */
		tmp = readl(port_mmio + PORT_IRQ_STAT);
		debug("PORT_IRQ_STAT 0x%x\n", tmp);
		if (tmp)
			writel(tmp, port_mmio + PORT_IRQ_STAT);

		writel(1 << i, mmio + HOST_IRQ_STAT);

		/* register linkup ports */
		tmp = readl(port_mmio + PORT_SCR_STAT);
		debug("SATA port %d status: 0x%x\n", i, tmp);
		if ((tmp & PORT_SCR_STAT_DET_MASK) == PORT_SCR_STAT_DET_PHYRDY)
			probe_ent->link_port_map |= (0x01 << i);
	}

	tmp = readl(mmio + HOST_CTL);
	debug("HOST_CTL 0x%x\n", tmp);
	writel(tmp | HOST_IRQ_EN, mmio + HOST_CTL);
	tmp = readl(mmio + HOST_CTL);
	debug("HOST_CTL 0x%x\n", tmp);
#ifndef CONFIG_SCSI_AHCI_PLAT
	pci_read_config_word(pdev, PCI_COMMAND, &tmp16);
	tmp |= PCI_COMMAND_MASTER;
	pci_write_config_word(pdev, PCI_COMMAND, tmp16);
#endif
	return 0;
}


static void ahci_print_info(struct ahci_probe_ent *probe_ent)
{
#ifndef CONFIG_SCSI_AHCI_PLAT
	pci_dev_t pdev = probe_ent->dev;
	u16 cc;
#endif
	void __iomem *mmio = probe_ent->mmio_base;
	u32 vers, cap, cap2, impl, speed;
	const char *speed_s;
	const char *scc_s;

	vers = readl(mmio + HOST_VERSION);
	cap = probe_ent->cap;
	cap2 = readl(mmio + HOST_CAP2);
	impl = probe_ent->port_map;

	speed = (cap >> 20) & 0xf;
	if (speed == 1)
		speed_s = "1.5";
	else if (speed == 2)
		speed_s = "3";
	else if (speed == 3)
		speed_s = "6";
	else
		speed_s = "?";

#ifdef CONFIG_SCSI_AHCI_PLAT
	scc_s = "SATA";
#else
	pci_read_config_word(pdev, 0x0a, &cc);
	if (cc == 0x0101)
		scc_s = "IDE";
	else if (cc == 0x0106)
		scc_s = "SATA";
	else if (cc == 0x0104)
		scc_s = "RAID";
	else
		scc_s = "unknown";
#endif
	printf("AHCI %02x%02x.%02x%02x "
	       "%u slots %u ports %s Gbps 0x%x impl %s mode\n",
	       (vers >> 24) & 0xff,
	       (vers >> 16) & 0xff,
	       (vers >> 8) & 0xff,
	       vers & 0xff,
	       ((cap >> 8) & 0x1f) + 1, (cap & 0x1f) + 1, speed_s, impl, scc_s);

	printf("flags: "
	       "%s%s%s%s%s%s%s"
	       "%s%s%s%s%s%s%s"
	       "%s%s%s%s%s%s\n",
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
	       cap & (1 << 16) ? "fbss " : "",
	       cap & (1 << 15) ? "pio " : "",
	       cap & (1 << 14) ? "slum " : "",
	       cap & (1 << 13) ? "part " : "",
	       cap & (1 << 7) ? "ccc " : "",
	       cap & (1 << 6) ? "ems " : "",
	       cap & (1 << 5) ? "sxs " : "",
	       cap2 & (1 << 2) ? "apst " : "",
	       cap2 & (1 << 1) ? "nvmp " : "",
	       cap2 & (1 << 0) ? "boh " : "");
}

#ifndef CONFIG_SCSI_AHCI_PLAT
static int ahci_init_one(pci_dev_t pdev)
{
	u16 vendor;
	int rc;

	probe_ent = malloc(sizeof(struct ahci_probe_ent));
	if (!probe_ent) {
		printf("%s: No memory for probe_ent\n", __func__);
		return -ENOMEM;
	}

	memset(probe_ent, 0, sizeof(struct ahci_probe_ent));
	probe_ent->dev = pdev;

	probe_ent->host_flags = ATA_FLAG_SATA
				| ATA_FLAG_NO_LEGACY
				| ATA_FLAG_MMIO
				| ATA_FLAG_PIO_DMA
				| ATA_FLAG_NO_ATAPI;
	probe_ent->pio_mask = 0x1f;
	probe_ent->udma_mask = 0x7f;	/*Fixme,assume to support UDMA6 */

	probe_ent->mmio_base = pci_map_bar(pdev, PCI_BASE_ADDRESS_5,
					   PCI_REGION_MEM);
	debug("ahci mmio_base=0x%p\n", probe_ent->mmio_base);

	/* Take from kernel:
	 * JMicron-specific fixup:
	 * make sure we're in AHCI mode
	 */
	pci_read_config_word(pdev, PCI_VENDOR_ID, &vendor);
	if (vendor == 0x197b)
		pci_write_config_byte(pdev, 0x41, 0xa1);

	/* initialize adapter */
	rc = ahci_host_init(probe_ent);
	if (rc)
		goto err_out;

	ahci_print_info(probe_ent);

	return 0;

      err_out:
	return rc;
}
#endif

#define MAX_DATA_BYTE_COUNT  (4*1024*1024)

static int ahci_fill_sg(u8 port, unsigned char *buf, int buf_len)
{
	struct ahci_ioports *pp = &(probe_ent->port[port]);
	struct ahci_sg *ahci_sg = pp->cmd_tbl_sg;
	u32 sg_count;
	int i;

	sg_count = ((buf_len - 1) / MAX_DATA_BYTE_COUNT) + 1;
	if (sg_count > AHCI_MAX_SG) {
		printf("Error:Too much sg!\n");
		return -1;
	}

	for (i = 0; i < sg_count; i++) {
		ahci_sg->addr =
		    cpu_to_le32((unsigned long) buf + i * MAX_DATA_BYTE_COUNT);
		ahci_sg->addr_hi = 0;
		ahci_sg->flags_size = cpu_to_le32(0x3fffff &
					  (buf_len < MAX_DATA_BYTE_COUNT
					   ? (buf_len - 1)
					   : (MAX_DATA_BYTE_COUNT - 1)));
		ahci_sg++;
		buf_len -= MAX_DATA_BYTE_COUNT;
	}

	return sg_count;
}


static void ahci_fill_cmd_slot(struct ahci_ioports *pp, u32 opts)
{
	pp->cmd_slot->opts = cpu_to_le32(opts);
	pp->cmd_slot->status = 0;
	pp->cmd_slot->tbl_addr = cpu_to_le32((u32)pp->cmd_tbl & 0xffffffff);
#ifdef CONFIG_PHYS_64BIT
	pp->cmd_slot->tbl_addr_hi =
	    cpu_to_le32((u32)(((pp->cmd_tbl) >> 16) >> 16));
#endif
}


#ifdef CONFIG_AHCI_SETFEATURES_XFER
static void ahci_set_feature(u8 port)
{
	struct ahci_ioports *pp = &(probe_ent->port[port]);
	void __iomem *port_mmio = pp->port_mmio;
	u32 cmd_fis_len = 5;	/* five dwords */
	u8 fis[20];

	/* set feature */
	memset(fis, 0, sizeof(fis));
	fis[0] = 0x27;
	fis[1] = 1 << 7;
	fis[2] = ATA_CMD_SET_FEATURES;
	fis[3] = SETFEATURES_XFER;
	fis[12] = __ilog2(probe_ent->udma_mask + 1) + 0x40 - 0x01;

	memcpy((unsigned char *)pp->cmd_tbl, fis, sizeof(fis));
	ahci_fill_cmd_slot(pp, cmd_fis_len);
	ahci_dcache_flush_sata_cmd(pp);
	writel(1, port_mmio + PORT_CMD_ISSUE);
	readl(port_mmio + PORT_CMD_ISSUE);

	if (waiting_for_cmd_completed(port_mmio + PORT_CMD_ISSUE,
				WAIT_MS_DATAIO, 0x1)) {
		printf("set feature error on port %d!\n", port);
	}
}
#endif

static int wait_spinup(void __iomem *port_mmio)
{
	ulong start;
	u32 tf_data;

	start = get_timer(0);
	do {
		tf_data = readl(port_mmio + PORT_TFDATA);
		if (!(tf_data & ATA_BUSY))
			return 0;
	} while (get_timer(start) < WAIT_MS_SPINUP);

	return -ETIMEDOUT;
}

static int ahci_port_start(u8 port)
{
	struct ahci_ioports *pp = &(probe_ent->port[port]);
	void __iomem *port_mmio = pp->port_mmio;
	u32 port_status;
	void __iomem *mem;

	debug("Enter start port: %d\n", port);
	port_status = readl(port_mmio + PORT_SCR_STAT);
	debug("Port %d status: %x\n", port, port_status);
	if ((port_status & 0xf) != 0x03) {
		printf("No Link on this port!\n");
		return -1;
	}

	mem = malloc(AHCI_PORT_PRIV_DMA_SZ + 2048);
	if (!mem) {
		free(pp);
		printf("%s: No mem for table!\n", __func__);
		return -ENOMEM;
	}

	/* Aligned to 2048-bytes */
	mem = memalign(2048, AHCI_PORT_PRIV_DMA_SZ);
	memset(mem, 0, AHCI_PORT_PRIV_DMA_SZ);

	/*
	 * First item in chunk of DMA memory: 32-slot command table,
	 * 32 bytes each in size
	 */
	pp->cmd_slot =
		(struct ahci_cmd_hdr *)(uintptr_t)virt_to_phys((void *)mem);
	debug("cmd_slot = %p\n", pp->cmd_slot);
	mem += (AHCI_CMD_SLOT_SZ + 224);

	/*
	 * Second item: Received-FIS area
	 */
	pp->rx_fis = virt_to_phys((void *)mem);
	mem += AHCI_RX_FIS_SZ;

	/*
	 * Third item: data area for storing a single command
	 * and its scatter-gather table
	 */
	pp->cmd_tbl = virt_to_phys((void *)mem);
	debug("cmd_tbl_dma = %lx\n", pp->cmd_tbl);

	mem += AHCI_CMD_TBL_HDR;
	pp->cmd_tbl_sg =
			(struct ahci_sg *)(uintptr_t)virt_to_phys((void *)mem);

	writel_with_flush((unsigned long)pp->cmd_slot,
			  port_mmio + PORT_LST_ADDR);

	writel_with_flush(pp->rx_fis, port_mmio + PORT_FIS_ADDR);

#ifdef CONFIG_SUNXI_AHCI
	sunxi_dma_init(port_mmio);
#endif

	writel_with_flush(PORT_CMD_ICC_ACTIVE | PORT_CMD_FIS_RX |
			  PORT_CMD_POWER_ON | PORT_CMD_SPIN_UP |
			  PORT_CMD_START, port_mmio + PORT_CMD);

	debug("Exit start port %d\n", port);

	/*
	 * Make sure interface is not busy based on error and status
	 * information from task file data register before proceeding
	 */
	return wait_spinup(port_mmio);
}


static int ahci_device_data_io(u8 port, u8 *fis, int fis_len, u8 *buf,
				int buf_len, u8 is_write)
{

	struct ahci_ioports *pp = &(probe_ent->port[port]);
	void __iomem *port_mmio = pp->port_mmio;
	u32 opts;
	u32 port_status;
	int sg_count;

	debug("Enter %s: for port %d\n", __func__, port);

	if (port > probe_ent->n_ports) {
		printf("Invalid port number %d\n", port);
		return -1;
	}

	port_status = readl(port_mmio + PORT_SCR_STAT);
	if ((port_status & 0xf) != 0x03) {
		debug("No Link on port %d!\n", port);
		return -1;
	}

	memcpy((unsigned char *)pp->cmd_tbl, fis, fis_len);

	sg_count = ahci_fill_sg(port, buf, buf_len);
	opts = (fis_len >> 2) | (sg_count << 16) | (is_write << 6);
	ahci_fill_cmd_slot(pp, opts);

	ahci_dcache_flush_sata_cmd(pp);
	ahci_dcache_flush_range((unsigned long)buf, (unsigned long)buf_len);

	writel_with_flush(1, port_mmio + PORT_CMD_ISSUE);

	if (waiting_for_cmd_completed(port_mmio + PORT_CMD_ISSUE,
				WAIT_MS_DATAIO, 0x1)) {
		printf("timeout exit!\n");
		return -1;
	}

	ahci_dcache_invalidate_range((unsigned long)buf,
				     (unsigned long)buf_len);
	debug("%s: %d byte transferred.\n", __func__, pp->cmd_slot->status);

	return 0;
}


static char *ata_id_strcpy(u16 *target, u16 *src, int len)
{
	int i;
	for (i = 0; i < len / 2; i++)
		target[i] = swab16(src[i]);
	return (char *)target;
}

/*
 * SCSI INQUIRY command operation.
 */
static int ata_scsiop_inquiry(ccb *pccb)
{
	static const u8 hdr[] = {
		0,
		0,
		0x5,		/* claim SPC-3 version compatibility */
		2,
		95 - 4,
	};
	u8 fis[20];
	u16 *idbuf;
	ALLOC_CACHE_ALIGN_BUFFER(u16, tmpid, ATA_ID_WORDS);
	u8 port;

	/* Clean ccb data buffer */
	memset(pccb->pdata, 0, pccb->datalen);

	memcpy(pccb->pdata, hdr, sizeof(hdr));

	if (pccb->datalen <= 35)
		return 0;

	memset(fis, 0, sizeof(fis));
	/* Construct the FIS */
	fis[0] = 0x27;		/* Host to device FIS. */
	fis[1] = 1 << 7;	/* Command FIS. */
	fis[2] = ATA_CMD_ID_ATA; /* Command byte. */

	/* Read id from sata */
	port = pccb->target;

	if (ahci_device_data_io(port, (u8 *) &fis, sizeof(fis), (u8 *)tmpid,
				ATA_ID_WORDS * 2, 0)) {
		debug("scsi_ahci: SCSI inquiry command failure.\n");
		return -EIO;
	}

	if (!ataid[port]) {
		ataid[port] = malloc(ATA_ID_WORDS * 2);
		if (!ataid[port]) {
			printf("%s: No memory for ataid[port]\n", __func__);
			return -ENOMEM;
		}
	}

	idbuf = ataid[port];

	memcpy(idbuf, tmpid, ATA_ID_WORDS * 2);
	ata_swap_buf_le16(idbuf, ATA_ID_WORDS);

	memcpy(&pccb->pdata[8], "ATA     ", 8);
	ata_id_strcpy((u16 *)&pccb->pdata[16], &idbuf[ATA_ID_PROD], 16);
	ata_id_strcpy((u16 *)&pccb->pdata[32], &idbuf[ATA_ID_FW_REV], 4);

#ifdef DEBUG
	ata_dump_id(idbuf);
#endif
	return 0;
}


/*
 * SCSI READ10/WRITE10 command operation.
 */
static int ata_scsiop_read_write(ccb *pccb, u8 is_write)
{
	lbaint_t lba = 0;
	u16 blocks = 0;
	u8 fis[20];
	u8 *user_buffer = pccb->pdata;
	u32 user_buffer_size = pccb->datalen;

	/* Retrieve the base LBA number from the ccb structure. */
	if (pccb->cmd[0] == SCSI_READ16) {
		memcpy(&lba, pccb->cmd + 2, 8);
		lba = be64_to_cpu(lba);
	} else {
		u32 temp;
		memcpy(&temp, pccb->cmd + 2, 4);
		lba = be32_to_cpu(temp);
	}

	/*
	 * Retrieve the base LBA number and the block count from
	 * the ccb structure.
	 *
	 * For 10-byte and 16-byte SCSI R/W commands, transfer
	 * length 0 means transfer 0 block of data.
	 * However, for ATA R/W commands, sector count 0 means
	 * 256 or 65536 sectors, not 0 sectors as in SCSI.
	 *
	 * WARNING: one or two older ATA drives treat 0 as 0...
	 */
	if (pccb->cmd[0] == SCSI_READ16)
		blocks = (((u16)pccb->cmd[13]) << 8) | ((u16) pccb->cmd[14]);
	else
		blocks = (((u16)pccb->cmd[7]) << 8) | ((u16) pccb->cmd[8]);

	debug("scsi_ahci: %s %u blocks starting from lba 0x" LBAFU "\n",
	      is_write ?  "write" : "read", blocks, lba);

	/* Preset the FIS */
	memset(fis, 0, sizeof(fis));
	fis[0] = 0x27;		 /* Host to device FIS. */
	fis[1] = 1 << 7;	 /* Command FIS. */
	/* Command byte (read/write). */
	fis[2] = is_write ? ATA_CMD_WRITE_EXT : ATA_CMD_READ_EXT;

	while (blocks) {
		u16 now_blocks; /* number of blocks per iteration */
		u32 transfer_size; /* number of bytes per iteration */

		now_blocks = min((u16)MAX_SATA_BLOCKS_READ_WRITE, blocks);

		transfer_size = ATA_SECT_SIZE * now_blocks;
		if (transfer_size > user_buffer_size) {
			printf("scsi_ahci: Error: buffer too small.\n");
			return -EIO;
		}

		/*
		 * LBA48 SATA command but only use 32bit address range within
		 * that (unless we've enabled 64bit LBA support). The next
		 * smaller command range (28bit) is too small.
		 */
		fis[4] = (lba >> 0) & 0xff;
		fis[5] = (lba >> 8) & 0xff;
		fis[6] = (lba >> 16) & 0xff;
		fis[7] = 1 << 6; /* device reg: set LBA mode */
		fis[8] = ((lba >> 24) & 0xff);
#ifdef CONFIG_SYS_64BIT_LBA
		if (pccb->cmd[0] == SCSI_READ16) {
			fis[9] = ((lba >> 32) & 0xff);
			fis[10] = ((lba >> 40) & 0xff);
		}
#endif

		fis[3] = 0xe0; /* features */

		/* Block (sector) count */
		fis[12] = (now_blocks >> 0) & 0xff;
		fis[13] = (now_blocks >> 8) & 0xff;

		/* Read/Write from ahci */
		if (ahci_device_data_io(pccb->target, (u8 *) &fis, sizeof(fis),
					user_buffer, transfer_size,
					is_write)) {
			debug("scsi_ahci: SCSI %s10 command failure.\n",
			      is_write ? "WRITE" : "READ");
			return -EIO;
		}

		/* If this transaction is a write, do a following flush.
		 * Writes in u-boot are so rare, and the logic to know when is
		 * the last write and do a flush only there is sufficiently
		 * difficult. Just do a flush after every write. This incurs,
		 * usually, one extra flush when the rare writes do happen.
		 */
		if (is_write) {
			if (-EIO == ata_io_flush(pccb->target))
				return -EIO;
		}
		user_buffer += transfer_size;
		user_buffer_size -= transfer_size;
		blocks -= now_blocks;
		lba += now_blocks;
	}

	return 0;
}


/*
 * SCSI READ CAPACITY10 command operation.
 */
static int ata_scsiop_read_capacity10(ccb *pccb)
{
	u32 cap;
	u64 cap64;
	u32 block_size;

	if (!ataid[pccb->target]) {
		printf("scsi_ahci: SCSI READ CAPACITY10 command failure. "
		       "\tNo ATA info!\n"
		       "\tPlease run SCSI commmand INQUIRY firstly!\n");
		return -EPERM;
	}

	cap64 = ata_id_n_sectors(ataid[pccb->target]);
	if (cap64 > 0x100000000ULL)
		cap64 = 0xffffffff;

	cap = cpu_to_be32(cap64);
	memcpy(pccb->pdata, &cap, sizeof(cap));

	block_size = cpu_to_be32((u32)512);
	memcpy(&pccb->pdata[4], &block_size, 4);

	return 0;
}


/*
 * SCSI READ CAPACITY16 command operation.
 */
static int ata_scsiop_read_capacity16(ccb *pccb)
{
	u64 cap;
	u64 block_size;

	if (!ataid[pccb->target]) {
		printf("scsi_ahci: SCSI READ CAPACITY16 command failure. "
		       "\tNo ATA info!\n"
		       "\tPlease run SCSI commmand INQUIRY firstly!\n");
		return -EPERM;
	}

	cap = ata_id_n_sectors(ataid[pccb->target]);
	cap = cpu_to_be64(cap);
	memcpy(pccb->pdata, &cap, sizeof(cap));

	block_size = cpu_to_be64((u64)512);
	memcpy(&pccb->pdata[8], &block_size, 8);

	return 0;
}


/*
 * SCSI TEST UNIT READY command operation.
 */
static int ata_scsiop_test_unit_ready(ccb *pccb)
{
	return (ataid[pccb->target]) ? 0 : -EPERM;
}


int scsi_exec(ccb *pccb)
{
	int ret;

	switch (pccb->cmd[0]) {
	case SCSI_READ16:
	case SCSI_READ10:
		ret = ata_scsiop_read_write(pccb, 0);
		break;
	case SCSI_WRITE10:
		ret = ata_scsiop_read_write(pccb, 1);
		break;
	case SCSI_RD_CAPAC10:
		ret = ata_scsiop_read_capacity10(pccb);
		break;
	case SCSI_RD_CAPAC16:
		ret = ata_scsiop_read_capacity16(pccb);
		break;
	case SCSI_TST_U_RDY:
		ret = ata_scsiop_test_unit_ready(pccb);
		break;
	case SCSI_INQUIRY:
		ret = ata_scsiop_inquiry(pccb);
		break;
	default:
		printf("Unsupport SCSI command 0x%02x\n", pccb->cmd[0]);
		return false;
	}

	if (ret) {
		debug("SCSI command 0x%02x ret errno %d\n", pccb->cmd[0], ret);
		return false;
	}
	return true;

}


void scsi_low_level_init(int busdevfunc)
{
	int i;
	u32 linkmap;

#ifndef CONFIG_SCSI_AHCI_PLAT
	ahci_init_one(busdevfunc);
#endif

	linkmap = probe_ent->link_port_map;

	for (i = 0; i < CONFIG_SYS_SCSI_MAX_SCSI_ID; i++) {
		if (((linkmap >> i) & 0x01)) {
			if (ahci_port_start((u8) i)) {
				printf("Can not start port %d\n", i);
				continue;
			}
#ifdef CONFIG_AHCI_SETFEATURES_XFER
			ahci_set_feature((u8) i);
#endif
		}
	}
}

#ifdef CONFIG_SCSI_AHCI_PLAT
int ahci_init(void __iomem *base)
{
	int i, rc = 0;
	u32 linkmap;

	probe_ent = malloc(sizeof(struct ahci_probe_ent));
	if (!probe_ent) {
		printf("%s: No memory for probe_ent\n", __func__);
		return -ENOMEM;
	}

	memset(probe_ent, 0, sizeof(struct ahci_probe_ent));

	probe_ent->host_flags = ATA_FLAG_SATA
				| ATA_FLAG_NO_LEGACY
				| ATA_FLAG_MMIO
				| ATA_FLAG_PIO_DMA
				| ATA_FLAG_NO_ATAPI;
	probe_ent->pio_mask = 0x1f;
	probe_ent->udma_mask = 0x7f;	/*Fixme,assume to support UDMA6 */

	probe_ent->mmio_base = base;

	/* initialize adapter */
	rc = ahci_host_init(probe_ent);
	if (rc)
		goto err_out;

	ahci_print_info(probe_ent);

	linkmap = probe_ent->link_port_map;

	for (i = 0; i < CONFIG_SYS_SCSI_MAX_SCSI_ID; i++) {
		if (((linkmap >> i) & 0x01)) {
			if (ahci_port_start((u8) i)) {
				printf("Can not start port %d\n", i);
				continue;
			}
#ifdef CONFIG_AHCI_SETFEATURES_XFER
			ahci_set_feature((u8) i);
#endif
		}
	}
err_out:
	return rc;
}

void __weak scsi_init(void)
{
}

#endif

/*
 * In the general case of generic rotating media it makes sense to have a
 * flush capability. It probably even makes sense in the case of SSDs because
 * one cannot always know for sure what kind of internal cache/flush mechanism
 * is embodied therein. At first it was planned to invoke this after the last
 * write to disk and before rebooting. In practice, knowing, a priori, which
 * is the last write is difficult. Because writing to the disk in u-boot is
 * very rare, this flush command will be invoked after every block write.
 */
static int ata_io_flush(u8 port)
{
	u8 fis[20];
	struct ahci_ioports *pp = &(probe_ent->port[port]);
	void __iomem *port_mmio = pp->port_mmio;
	u32 cmd_fis_len = 5;	/* five dwords */

	/* Preset the FIS */
	memset(fis, 0, 20);
	fis[0] = 0x27;		 /* Host to device FIS. */
	fis[1] = 1 << 7;	 /* Command FIS. */
	fis[2] = ATA_CMD_FLUSH_EXT;

	memcpy((unsigned char *)pp->cmd_tbl, fis, 20);
	ahci_fill_cmd_slot(pp, cmd_fis_len);
	writel_with_flush(1, port_mmio + PORT_CMD_ISSUE);

	if (waiting_for_cmd_completed(port_mmio + PORT_CMD_ISSUE,
			WAIT_MS_FLUSH, 0x1)) {
		debug("scsi_ahci: flush command timeout on port %d.\n", port);
		return -EIO;
	}

	return 0;
}


__weak void scsi_bus_reset(void)
{
	/*Not implement*/
}

void scsi_print_error(ccb * pccb)
{
	/*The ahci error info can be read in the ahci driver*/
}
