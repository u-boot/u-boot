/*
 * sata_dwc.h
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
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*
 * SATA support based on the chip canyonlands.
 *
 * 04-17-2009
 *		The local version of this driver for the canyonlands board
 *		does not use interrupts but polls the chip instead.
 */


#ifndef _SATA_DWC_H_
#define _SATA_DWC_H_

#define __U_BOOT__

#define HZ 100
#define READ 0
#define WRITE 1

enum {
	ATA_READID_POSTRESET	= (1 << 0),

	ATA_DNXFER_PIO		= 0,
	ATA_DNXFER_DMA		= 1,
	ATA_DNXFER_40C		= 2,
	ATA_DNXFER_FORCE_PIO	= 3,
	ATA_DNXFER_FORCE_PIO0	= 4,

	ATA_DNXFER_QUIET	= (1 << 31),
};

enum hsm_task_states {
	HSM_ST_IDLE,
	HSM_ST_FIRST,
	HSM_ST,
	HSM_ST_LAST,
	HSM_ST_ERR,
};

#define	ATA_SHORT_PAUSE		((HZ >> 6) + 1)

struct ata_queued_cmd {
	struct ata_port		*ap;
	struct ata_device	*dev;

	struct ata_taskfile	tf;
	u8			cdb[ATAPI_CDB_LEN];
	unsigned long		flags;
	unsigned int		tag;
	unsigned int		n_elem;

	int			dma_dir;
	unsigned int		sect_size;

	unsigned int		nbytes;
	unsigned int		extrabytes;
	unsigned int		curbytes;

	unsigned int		err_mask;
	struct ata_taskfile	result_tf;

	void			*private_data;
#ifndef __U_BOOT__
	void			*lldd_task;
#endif
	unsigned char		*pdata;
};

typedef void (*ata_qc_cb_t) (struct ata_queued_cmd *qc);

#define ATA_TAG_POISON	0xfafbfcfdU

enum {
	LIBATA_MAX_PRD		= ATA_MAX_PRD / 2,
	LIBATA_DUMB_MAX_PRD	= ATA_MAX_PRD / 4,
	ATA_MAX_PORTS		= 8,
	ATA_DEF_QUEUE		= 1,
	ATA_MAX_QUEUE		= 32,
	ATA_TAG_INTERNAL	= ATA_MAX_QUEUE - 1,
	ATA_MAX_BUS		= 2,
	ATA_DEF_BUSY_WAIT	= 10000,

	ATAPI_MAX_DRAIN		= 16 << 10,

	ATA_SHT_EMULATED	= 1,
	ATA_SHT_CMD_PER_LUN	= 1,
	ATA_SHT_THIS_ID		= -1,
	ATA_SHT_USE_CLUSTERING	= 1,

	ATA_DFLAG_LBA		= (1 << 0),
	ATA_DFLAG_LBA48		= (1 << 1),
	ATA_DFLAG_CDB_INTR	= (1 << 2),
	ATA_DFLAG_NCQ		= (1 << 3),
	ATA_DFLAG_FLUSH_EXT	= (1 << 4),
	ATA_DFLAG_ACPI_PENDING 	= (1 << 5),
	ATA_DFLAG_ACPI_FAILED	= (1 << 6),
	ATA_DFLAG_AN		= (1 << 7),
	ATA_DFLAG_HIPM		= (1 << 8),
	ATA_DFLAG_DIPM		= (1 << 9),
	ATA_DFLAG_DMADIR	= (1 << 10),
	ATA_DFLAG_CFG_MASK	= (1 << 12) - 1,

	ATA_DFLAG_PIO		= (1 << 12),
	ATA_DFLAG_NCQ_OFF	= (1 << 13),
	ATA_DFLAG_SPUNDOWN	= (1 << 14),
	ATA_DFLAG_SLEEPING	= (1 << 15),
	ATA_DFLAG_DUBIOUS_XFER	= (1 << 16),
	ATA_DFLAG_INIT_MASK	= (1 << 24) - 1,

	ATA_DFLAG_DETACH	= (1 << 24),
	ATA_DFLAG_DETACHED	= (1 << 25),

	ATA_LFLAG_HRST_TO_RESUME	= (1 << 0),
	ATA_LFLAG_SKIP_D2H_BSY		= (1 << 1),
	ATA_LFLAG_NO_SRST		= (1 << 2),
	ATA_LFLAG_ASSUME_ATA		= (1 << 3),
	ATA_LFLAG_ASSUME_SEMB		= (1 << 4),
	ATA_LFLAG_ASSUME_CLASS = ATA_LFLAG_ASSUME_ATA | ATA_LFLAG_ASSUME_SEMB,
	ATA_LFLAG_NO_RETRY		= (1 << 5),
	ATA_LFLAG_DISABLED		= (1 << 6),

	ATA_FLAG_SLAVE_POSS	= (1 << 0),
	ATA_FLAG_SATA		= (1 << 1),
	ATA_FLAG_NO_LEGACY	= (1 << 2),
	ATA_FLAG_MMIO		= (1 << 3),
	ATA_FLAG_SRST		= (1 << 4),
	ATA_FLAG_SATA_RESET	= (1 << 5),
	ATA_FLAG_NO_ATAPI	= (1 << 6),
	ATA_FLAG_PIO_DMA	= (1 << 7),
	ATA_FLAG_PIO_LBA48	= (1 << 8),
	ATA_FLAG_PIO_POLLING	= (1 << 9),
	ATA_FLAG_NCQ		= (1 << 10),
	ATA_FLAG_DEBUGMSG	= (1 << 13),
	ATA_FLAG_IGN_SIMPLEX	= (1 << 15),
	ATA_FLAG_NO_IORDY	= (1 << 16),
	ATA_FLAG_ACPI_SATA	= (1 << 17),
	ATA_FLAG_AN		= (1 << 18),
	ATA_FLAG_PMP		= (1 << 19),
	ATA_FLAG_IPM		= (1 << 20),

	ATA_FLAG_DISABLED	= (1 << 23),

	ATA_PFLAG_EH_PENDING		= (1 << 0),
	ATA_PFLAG_EH_IN_PROGRESS	= (1 << 1),
	ATA_PFLAG_FROZEN		= (1 << 2),
	ATA_PFLAG_RECOVERED		= (1 << 3),
	ATA_PFLAG_LOADING		= (1 << 4),
	ATA_PFLAG_UNLOADING		= (1 << 5),
	ATA_PFLAG_SCSI_HOTPLUG		= (1 << 6),
	ATA_PFLAG_INITIALIZING		= (1 << 7),
	ATA_PFLAG_RESETTING		= (1 << 8),
	ATA_PFLAG_SUSPENDED		= (1 << 17),
	ATA_PFLAG_PM_PENDING		= (1 << 18),

	ATA_QCFLAG_ACTIVE	= (1 << 0),
	ATA_QCFLAG_DMAMAP	= (1 << 1),
	ATA_QCFLAG_IO		= (1 << 3),
	ATA_QCFLAG_RESULT_TF	= (1 << 4),
	ATA_QCFLAG_CLEAR_EXCL	= (1 << 5),
	ATA_QCFLAG_QUIET	= (1 << 6),

	ATA_QCFLAG_FAILED	= (1 << 16),
	ATA_QCFLAG_SENSE_VALID	= (1 << 17),
	ATA_QCFLAG_EH_SCHEDULED	= (1 << 18),

	ATA_HOST_SIMPLEX	= (1 << 0),
	ATA_HOST_STARTED	= (1 << 1),

	ATA_TMOUT_BOOT			= 30 * 100,
	ATA_TMOUT_BOOT_QUICK		= 7 * 100,
	ATA_TMOUT_INTERNAL		= 30 * 100,
	ATA_TMOUT_INTERNAL_QUICK	= 5 * 100,

	/* FIXME: GoVault needs 2s but we can't afford that without
	 * parallel probing.  800ms is enough for iVDR disk
	 * HHD424020F7SV00.  Increase to 2secs when parallel probing
	 * is in place.
	 */
	ATA_TMOUT_FF_WAIT	= 4 * 100 / 5,

	BUS_UNKNOWN		= 0,
	BUS_DMA			= 1,
	BUS_IDLE		= 2,
	BUS_NOINTR		= 3,
	BUS_NODATA		= 4,
	BUS_TIMER		= 5,
	BUS_PIO			= 6,
	BUS_EDD			= 7,
	BUS_IDENTIFY		= 8,
	BUS_PACKET		= 9,

	PORT_UNKNOWN		= 0,
	PORT_ENABLED		= 1,
	PORT_DISABLED		= 2,

	/* encoding various smaller bitmaps into a single
	 * unsigned long bitmap
	 */
	ATA_NR_PIO_MODES	= 7,
	ATA_NR_MWDMA_MODES	= 5,
	ATA_NR_UDMA_MODES	= 8,

	ATA_SHIFT_PIO		= 0,
	ATA_SHIFT_MWDMA		= ATA_SHIFT_PIO + ATA_NR_PIO_MODES,
	ATA_SHIFT_UDMA		= ATA_SHIFT_MWDMA + ATA_NR_MWDMA_MODES,

	ATA_DMA_PAD_SZ		= 4,

	ATA_ERING_SIZE		= 32,

	ATA_DEFER_LINK		= 1,
	ATA_DEFER_PORT		= 2,

	ATA_EH_DESC_LEN		= 80,

	ATA_EH_REVALIDATE	= (1 << 0),
	ATA_EH_SOFTRESET	= (1 << 1),
	ATA_EH_HARDRESET	= (1 << 2),
	ATA_EH_ENABLE_LINK	= (1 << 3),
	ATA_EH_LPM		= (1 << 4),

	ATA_EH_RESET_MASK	= ATA_EH_SOFTRESET | ATA_EH_HARDRESET,
	ATA_EH_PERDEV_MASK	= ATA_EH_REVALIDATE,

	ATA_EHI_HOTPLUGGED	= (1 << 0),
	ATA_EHI_RESUME_LINK	= (1 << 1),
	ATA_EHI_NO_AUTOPSY	= (1 << 2),
	ATA_EHI_QUIET		= (1 << 3),

	ATA_EHI_DID_SOFTRESET	= (1 << 16),
	ATA_EHI_DID_HARDRESET	= (1 << 17),
	ATA_EHI_PRINTINFO	= (1 << 18),
	ATA_EHI_SETMODE		= (1 << 19),
	ATA_EHI_POST_SETMODE	= (1 << 20),

	ATA_EHI_DID_RESET = ATA_EHI_DID_SOFTRESET | ATA_EHI_DID_HARDRESET,
	ATA_EHI_RESET_MODIFIER_MASK = ATA_EHI_RESUME_LINK,

	ATA_EH_MAX_TRIES	= 5,

	ATA_PROBE_MAX_TRIES	= 3,
	ATA_EH_DEV_TRIES	= 3,
	ATA_EH_PMP_TRIES	= 5,
	ATA_EH_PMP_LINK_TRIES	= 3,

	SATA_PMP_SCR_TIMEOUT	= 250,

	/* Horkage types. May be set by libata or controller on drives
	(some horkage may be drive/controller pair dependant */

	ATA_HORKAGE_DIAGNOSTIC	= (1 << 0),
	ATA_HORKAGE_NODMA	= (1 << 1),
	ATA_HORKAGE_NONCQ	= (1 << 2),
	ATA_HORKAGE_MAX_SEC_128	= (1 << 3),
	ATA_HORKAGE_BROKEN_HPA	= (1 << 4),
	ATA_HORKAGE_SKIP_PM	= (1 << 5),
	ATA_HORKAGE_HPA_SIZE	= (1 << 6),
	ATA_HORKAGE_IPM		= (1 << 7),
	ATA_HORKAGE_IVB		= (1 << 8),
	ATA_HORKAGE_STUCK_ERR	= (1 << 9),

	ATA_DMA_MASK_ATA	= (1 << 0),
	ATA_DMA_MASK_ATAPI	= (1 << 1),
	ATA_DMA_MASK_CFA	= (1 << 2),

	ATAPI_READ		= 0,
	ATAPI_WRITE		= 1,
	ATAPI_READ_CD		= 2,
	ATAPI_PASS_THRU		= 3,
	ATAPI_MISC		= 4,
};

enum ata_completion_errors {
	AC_ERR_DEV		= (1 << 0),
	AC_ERR_HSM		= (1 << 1),
	AC_ERR_TIMEOUT		= (1 << 2),
	AC_ERR_MEDIA		= (1 << 3),
	AC_ERR_ATA_BUS		= (1 << 4),
	AC_ERR_HOST_BUS		= (1 << 5),
	AC_ERR_SYSTEM		= (1 << 6),
	AC_ERR_INVALID		= (1 << 7),
	AC_ERR_OTHER		= (1 << 8),
	AC_ERR_NODEV_HINT	= (1 << 9),
	AC_ERR_NCQ		= (1 << 10),
};

enum ata_xfer_mask {
	ATA_MASK_PIO	= ((1LU << ATA_NR_PIO_MODES) - 1) << ATA_SHIFT_PIO,
	ATA_MASK_MWDMA	= ((1LU << ATA_NR_MWDMA_MODES) - 1) << ATA_SHIFT_MWDMA,
	ATA_MASK_UDMA	= ((1LU << ATA_NR_UDMA_MODES) - 1) << ATA_SHIFT_UDMA,
};

struct ata_port_info {
#ifndef __U_BOOT__
	struct scsi_host_template	*sht;
#endif
	unsigned long			flags;
	unsigned long			link_flags;
	unsigned long			pio_mask;
	unsigned long			mwdma_mask;
	unsigned long			udma_mask;
#ifndef __U_BOOT__
	const struct ata_port_operations *port_ops;
	void				*private_data;
#endif
};

struct ata_ioports {
	void __iomem		*cmd_addr;
	void __iomem		*data_addr;
	void __iomem		*error_addr;
	void __iomem		*feature_addr;
	void __iomem		*nsect_addr;
	void __iomem		*lbal_addr;
	void __iomem		*lbam_addr;
	void __iomem		*lbah_addr;
	void __iomem		*device_addr;
	void __iomem		*status_addr;
	void __iomem		*command_addr;
	void __iomem		*altstatus_addr;
	void __iomem		*ctl_addr;
#ifndef __U_BOOT__
	void __iomem		*bmdma_addr;
#endif
	void __iomem		*scr_addr;
};

struct ata_host {
#ifndef __U_BOOT__
	void __iomem * const	*iomap;
	void			*private_data;
	const struct ata_port_operations *ops;
	unsigned long		flags;
	struct ata_port		*simplex_claimed;
#endif
	unsigned int		n_ports;
	struct ata_port		*ports[0];
};

#ifndef __U_BOOT__
struct ata_port_stats {
	unsigned long		unhandled_irq;
	unsigned long		idle_irq;
	unsigned long		rw_reqbuf;
};
#endif

struct ata_device {
	struct ata_link		*link;
	unsigned int		devno;
	unsigned long		flags;
	unsigned int		horkage;
#ifndef __U_BOOT__
	struct scsi_device	*sdev;
#ifdef CONFIG_ATA_ACPI
	acpi_handle		acpi_handle;
	union acpi_object	*gtf_cache;
#endif
#endif
	u64			n_sectors;
	unsigned int		class;

	union {
		u16		id[ATA_ID_WORDS];
		u32		gscr[SATA_PMP_GSCR_DWORDS];
	};
#ifndef __U_BOOT__
	u8			pio_mode;
	u8			dma_mode;
	u8			xfer_mode;
	unsigned int		xfer_shift;
#endif
	unsigned int		multi_count;
	unsigned int		max_sectors;
	unsigned int		cdb_len;
#ifndef __U_BOOT__
	unsigned long		pio_mask;
	unsigned long		mwdma_mask;
#endif
	unsigned long		udma_mask;
	u16			cylinders;
	u16			heads;
	u16			sectors;
#ifndef __U_BOOT__
	int			spdn_cnt;
#endif
};

struct ata_link {
	struct ata_port		*ap;
	int			pmp;
	unsigned int		active_tag;
	u32			sactive;
	unsigned int		flags;
	unsigned int		hw_sata_spd_limit;
#ifndef __U_BOOT__
	unsigned int		sata_spd_limit;
	unsigned int		sata_spd;
	struct ata_device	device[2];
#endif
};

struct ata_port {
	unsigned long		flags;
	unsigned int		pflags;
	unsigned int		print_id;
	unsigned int		port_no;

	struct ata_ioports	ioaddr;

	u8			ctl;
	u8			last_ctl;
	unsigned int		pio_mask;
	unsigned int		mwdma_mask;
	unsigned int		udma_mask;
	unsigned int		cbl;

	struct ata_queued_cmd	qcmd[ATA_MAX_QUEUE];
	unsigned long		qc_allocated;
	unsigned int		qc_active;
	int			nr_active_links;

	struct ata_link		link;
#ifndef __U_BOOT__
	int			nr_pmp_links;
	struct ata_link		*pmp_link;
#endif
	struct ata_link		*excl_link;
	int			nr_pmp_links;
#ifndef __U_BOOT__
	struct ata_port_stats	stats;
	struct device		*dev;
	u32			msg_enable;
#endif
	struct ata_host		*host;
	void			*port_task_data;

	unsigned int		hsm_task_state;
	void			*private_data;
	unsigned char		*pdata;
};

#endif
