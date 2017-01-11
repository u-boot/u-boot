/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MVEBU_CPU_H
#define _MVEBU_CPU_H

#include <asm/system.h>

#ifndef __ASSEMBLY__

#define MVEBU_REG_PCIE_DEVID		(MVEBU_REG_PCIE_BASE + 0x00)
#define MVEBU_REG_PCIE_REVID		(MVEBU_REG_PCIE_BASE + 0x08)

enum memory_bank {
	BANK0,
	BANK1,
	BANK2,
	BANK3
};

enum cpu_winen {
	CPU_WIN_DISABLE,
	CPU_WIN_ENABLE
};

enum cpu_target {
	CPU_TARGET_DRAM = 0x0,
	CPU_TARGET_DEVICEBUS_BOOTROM_SPI = 0x1,
	CPU_TARGET_ETH23 = 0x3,
	CPU_TARGET_PCIE02 = 0x4,
	CPU_TARGET_ETH01 = 0x7,
	CPU_TARGET_PCIE13 = 0x8,
	CPU_TARGET_SASRAM = 0x9,
	CPU_TARGET_SATA01 = 0xa, /* A38X */
	CPU_TARGET_NAND = 0xd,
	CPU_TARGET_SATA23_DFX = 0xe, /* A38X */
};

enum cpu_attrib {
	CPU_ATTR_SASRAM = 0x01,
	CPU_ATTR_DRAM_CS0 = 0x0e,
	CPU_ATTR_DRAM_CS1 = 0x0d,
	CPU_ATTR_DRAM_CS2 = 0x0b,
	CPU_ATTR_DRAM_CS3 = 0x07,
	CPU_ATTR_NANDFLASH = 0x2f,
	CPU_ATTR_SPIFLASH = 0x1e,
	CPU_ATTR_SPI0_CS0 = 0x1e,
	CPU_ATTR_SPI0_CS1 = 0x5e,
	CPU_ATTR_SPI1_CS2 = 0x9a,
	CPU_ATTR_BOOTROM = 0x1d,
	CPU_ATTR_PCIE_IO = 0xe0,
	CPU_ATTR_PCIE_MEM = 0xe8,
	CPU_ATTR_DEV_CS0 = 0x3e,
	CPU_ATTR_DEV_CS1 = 0x3d,
	CPU_ATTR_DEV_CS2 = 0x3b,
	CPU_ATTR_DEV_CS3 = 0x37,
};

enum {
	MVEBU_SOC_AXP,
	MVEBU_SOC_A375,
	MVEBU_SOC_A38X,
	MVEBU_SOC_UNKNOWN,
};

/*
 * Default Device Address MAP BAR values
 */
#define MBUS_PCI_MEM_BASE	0xE8000000
#define MBUS_PCI_MEM_SIZE	(128 << 20)
#define MBUS_PCI_IO_BASE	0xF1100000
#define MBUS_PCI_IO_SIZE	(64 << 10)
#define MBUS_SPI_BASE		0xF4000000
#define MBUS_SPI_SIZE		(8 << 20)
#define MBUS_BOOTROM_BASE	0xF8000000
#define MBUS_BOOTROM_SIZE	(8 << 20)

struct mbus_win {
	u32 base;
	u32 size;
	u8 target;
	u8 attr;
};

/*
 * System registers
 * Ref: Datasheet sec:A.28
 */
struct mvebu_system_registers {
#if defined(CONFIG_ARMADA_375)
	u8 pad1[0x54];
#else
	u8 pad1[0x60];
#endif
	u32 rstoutn_mask; /* 0x60 */
	u32 sys_soft_rst; /* 0x64 */
};

/*
 * GPIO Registers
 * Ref: Datasheet sec:A.19
 */
struct kwgpio_registers {
	u32 dout;
	u32 oe;
	u32 blink_en;
	u32 din_pol;
	u32 din;
	u32 irq_cause;
	u32 irq_mask;
	u32 irq_level;
};

struct sar_freq_modes {
	u8 val;
	u8 ffc;		/* Fabric Frequency Configuration */
	u32 p_clk;
	u32 nb_clk;
	u32 d_clk;
};

/* Needed for dynamic (board-specific) mbus configuration */
extern struct mvebu_mbus_state mbus_state;

/*
 * functions
 */
unsigned int mvebu_sdram_bar(enum memory_bank bank);
unsigned int mvebu_sdram_bs(enum memory_bank bank);
void mvebu_sdram_size_adjust(enum memory_bank bank);
int mvebu_mbus_probe(struct mbus_win windows[], int count);
int mvebu_soc_family(void);
u32 mvebu_get_nand_clock(void);

void return_to_bootrom(void);

int mv_sdh_init(unsigned long regbase, u32 max_clk, u32 min_clk, u32 quirks);

void get_sar_freq(struct sar_freq_modes *sar_freq);

/*
 * Highspeed SERDES PHY config init, ported from bin_hdr
 * to mainline U-Boot
 */
int serdes_phy_config(void);

/*
 * DDR3 init / training code ported from Marvell bin_hdr. Now
 * available in mainline U-Boot in:
 * drivers/ddr/marvell
 */
int ddr3_init(void);

struct mvebu_lcd_info {
	u32 fb_base;
	int x_res;
	int y_res;
	int x_fp;		/* frontporch */
	int y_fp;
	int x_bp;		/* backporch */
	int y_bp;
};

int mvebu_lcd_register_init(struct mvebu_lcd_info *lcd_info);

/*
 * get_ref_clk
 *
 * return: reference clock in MHz (25 or 40)
 */
u32 get_ref_clk(void);

#endif /* __ASSEMBLY__ */
#endif /* _MVEBU_CPU_H */
