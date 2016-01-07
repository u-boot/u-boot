/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FW_CFG__
#define __FW_CFG__

#define FW_CONTROL_PORT	0x510
#define FW_DATA_PORT		0x511
#define FW_DMA_PORT_LOW	0x514
#define FW_DMA_PORT_HIGH	0x518

enum qemu_fwcfg_items {
	FW_CFG_SIGNATURE	= 0x00,
	FW_CFG_ID		= 0x01,
	FW_CFG_UUID		= 0x02,
	FW_CFG_RAM_SIZE		= 0x03,
	FW_CFG_NOGRAPHIC	= 0x04,
	FW_CFG_NB_CPUS		= 0x05,
	FW_CFG_MACHINE_ID	= 0x06,
	FW_CFG_KERNEL_ADDR	= 0x07,
	FW_CFG_KERNEL_SIZE	= 0x08,
	FW_CFG_KERNEL_CMDLINE   = 0x09,
	FW_CFG_INITRD_ADDR	= 0x0a,
	FW_CFG_INITRD_SIZE	= 0x0b,
	FW_CFG_BOOT_DEVICE	= 0x0c,
	FW_CFG_NUMA		= 0x0d,
	FW_CFG_BOOT_MENU	= 0x0e,
	FW_CFG_MAX_CPUS		= 0x0f,
	FW_CFG_KERNEL_ENTRY	= 0x10,
	FW_CFG_KERNEL_DATA	= 0x11,
	FW_CFG_INITRD_DATA	= 0x12,
	FW_CFG_CMDLINE_ADDR	= 0x13,
	FW_CFG_CMDLINE_SIZE	= 0x14,
	FW_CFG_CMDLINE_DATA	= 0x15,
	FW_CFG_SETUP_ADDR	= 0x16,
	FW_CFG_SETUP_SIZE	= 0x17,
	FW_CFG_SETUP_DATA	= 0x18,
	FW_CFG_FILE_DIR		= 0x19,
	FW_CFG_FILE_FIRST	= 0x20,
	FW_CFG_WRITE_CHANNEL	= 0x4000,
	FW_CFG_ARCH_LOCAL	= 0x8000,
	FW_CFG_INVALID		= 0xffff,
};

#define FW_CFG_FILE_SLOTS	0x10
#define FW_CFG_MAX_ENTRY	(FW_CFG_FILE_FIRST + FW_CFG_FILE_SLOTS)
#define FW_CFG_ENTRY_MASK	 ~(FW_CFG_WRITE_CHANNEL | FW_CFG_ARCH_LOCAL)

#define FW_CFG_MAX_FILE_PATH	56

#define QEMU_FW_CFG_SIGNATURE	(('Q' << 24) | ('E' << 16) | ('M' << 8) | 'U')

#define FW_CFG_DMA_ERROR	(1 << 0)
#define FW_CFG_DMA_READ	(1 << 1)
#define FW_CFG_DMA_SKIP	(1 << 2)
#define FW_CFG_DMA_SELECT	(1 << 3)

#define FW_CFG_DMA_ENABLED	(1 << 1)

struct fw_cfg_file {
	__be32 size;
	__be16 select;
	__be16 reserved;
	char name[FW_CFG_MAX_FILE_PATH];
};

struct fw_cfg_files {
	__be32 count;
	struct fw_cfg_file files[];
};

struct fw_cfg_dma_access {
	__be32 control;
	__be32 length;
	__be64 address;
};

/**
 * Initialize QEMU fw_cfg interface
 */
void qemu_fwcfg_init(void);

/**
 * Get system cpu number
 *
 * @return:   cpu number in system
 */
int qemu_fwcfg_online_cpus(void);

#endif
