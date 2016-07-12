/*
 * (C) Copyright 2009, 2010 Wolfgang Denk <wd@denx.de>
 *
 * (C) Copyright 2009-2010
 * Michael Weiß, ifm ecomatic gmbh, michael.weiss@ifm.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/bitops.h>
#include <command.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/mpc512x.h>
#include <fdt_support.h>
#include <flash.h>
#ifdef CONFIG_MISC_INIT_R
#include <i2c.h>
#endif
#include <serial.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[];
ulong flash_get_size (phys_addr_t base, int banknum);

sdram_conf_t mddrc_config[] = {
	{
		(512 << 20),	/* 512 MB RAM configuration */
		{
			CONFIG_SYS_MDDRC_SYS_CFG,
			CONFIG_SYS_MDDRC_TIME_CFG0,
			CONFIG_SYS_MDDRC_TIME_CFG1,
			CONFIG_SYS_MDDRC_TIME_CFG2
		}
	},
	{
		(128 << 20),	/* 128 MB RAM configuration */
		{
			CONFIG_SYS_MDDRC_SYS_CFG_ALT1,
			CONFIG_SYS_MDDRC_TIME_CFG0_ALT1,
			CONFIG_SYS_MDDRC_TIME_CFG1_ALT1,
			CONFIG_SYS_MDDRC_TIME_CFG2_ALT1
		}
	},
};

phys_size_t initdram (int board_type)
{
	int i;
	u32 msize = 0;
	u32 pdm360ng_init_seq[] = {
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_PCHG_ALL,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_RFSH,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_RFSH,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_MICRON_INIT_DEV_OP,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_EM2,
		CONFIG_SYS_DDRCMD_NOP,
		CONFIG_SYS_DDRCMD_PCHG_ALL,
		CONFIG_SYS_DDRCMD_EM2,
		CONFIG_SYS_DDRCMD_EM3,
		CONFIG_SYS_DDRCMD_EN_DLL,
		CONFIG_SYS_DDRCMD_RES_DLL,
		CONFIG_SYS_DDRCMD_PCHG_ALL,
		CONFIG_SYS_DDRCMD_RFSH,
		CONFIG_SYS_DDRCMD_RFSH,
		CONFIG_SYS_MICRON_INIT_DEV_OP,
		CONFIG_SYS_DDRCMD_OCD_DEFAULT,
		CONFIG_SYS_DDRCMD_OCD_EXIT,
		CONFIG_SYS_DDRCMD_PCHG_ALL,
		CONFIG_SYS_DDRCMD_NOP
	};

	for (i = 0; i < ARRAY_SIZE(mddrc_config); i++) {
		msize = fixed_sdram(&mddrc_config[i].cfg, pdm360ng_init_seq,
				    ARRAY_SIZE(pdm360ng_init_seq));
		if (msize == mddrc_config[i].size)
			break;
	}

	return msize;
}

static int set_lcd_brightness(char *);

int misc_init_r(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;

	/*
	 * Re-configure flash setup using auto-detected info
	 */
	if (flash_info[1].size > 0) {
		out_be32(&im->sysconf.lpcs1aw,
			CSAW_START(gd->bd->bi_flashstart + flash_info[1].size) |
			CSAW_STOP(gd->bd->bi_flashstart + flash_info[1].size,
				  flash_info[1].size));
		sync_law(&im->sysconf.lpcs1aw);
		/*
		 * Re-check to get correct base address
		 */
		flash_get_size (gd->bd->bi_flashstart + flash_info[1].size, 1);
	} else {
		/* Disable Bank 1 */
		out_be32(&im->sysconf.lpcs1aw, 0x01000100);
		sync_law(&im->sysconf.lpcs1aw);
	}

	out_be32(&im->sysconf.lpcs0aw,
		CSAW_START(gd->bd->bi_flashstart) |
		CSAW_STOP(gd->bd->bi_flashstart, flash_info[0].size));
	sync_law(&im->sysconf.lpcs0aw);

	/*
	 * Re-check to get correct base address
	 */
	flash_get_size (gd->bd->bi_flashstart, 0);

	/*
	 * Re-do flash protection upon new addresses
	 */
	flash_protect (FLAG_PROTECT_CLEAR,
		       gd->bd->bi_flashstart, 0xffffffff,
		       &flash_info[0]);

	/* Monitor protection ON by default */
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_SYS_MONITOR_BASE,
		       CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN - 1,
		       &flash_info[0]);

	/* Environment protection ON by default */
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_ENV_ADDR,
		       CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
		       &flash_info[0]);

#ifdef CONFIG_ENV_ADDR_REDUND
	/* Redundant environment protection ON by default */
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_ENV_ADDR_REDUND,
		       CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
		       &flash_info[0]);
#endif

#ifdef CONFIG_FSL_DIU_FB
	set_lcd_brightness(0);
	/* Switch LCD-Backlight and LVDS-Interface on */
	setbits_be32(&im->gpio.gpdir, 0x01040000);
	clrsetbits_be32(&im->gpio.gpdat, 0x01000000, 0x00040000);
#endif

#if defined(CONFIG_HARD_I2C)
	if (!getenv("ethaddr")) {
		uchar buf[6];
		uchar ifm_oui[3] = { 0, 2, 1, };
		int ret;

		/* I2C-0 for on-board eeprom */
		i2c_set_bus_num(CONFIG_SYS_I2C_EEPROM_BUS_NUM);

		/* Read ethaddr from EEPROM */
		ret = i2c_read(CONFIG_SYS_I2C_EEPROM_ADDR,
			       CONFIG_SYS_I2C_EEPROM_MAC_OFFSET, 1, buf, 6);
		if (ret != 0) {
			printf("Error: Unable to read MAC from I2C"
				" EEPROM at address %02X:%02X\n",
				CONFIG_SYS_I2C_EEPROM_ADDR,
				CONFIG_SYS_I2C_EEPROM_MAC_OFFSET);
			return 1;
		}

		/* Owned by IFM ? */
		if (memcmp(buf, ifm_oui, sizeof(ifm_oui))) {
			printf("Illegal MAC address in EEPROM: %pM\n", buf);
			return 1;
		}

		eth_setenv_enetaddr("ethaddr", buf);
	}
#endif /* defined(CONFIG_HARD_I2C) */

	return 0;
}

static  iopin_t ioregs_init[] = {
	/* FUNC1=LPC_CS4 */
	{
		offsetof(struct ioctrl512x, io_control_pata_ce1), 1, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(1) |
		IO_PIN_PUE(1) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=GPIO10 */
	{
		offsetof(struct ioctrl512x, io_control_pata_ce2), 1, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC1=CAN3_TX */
	{
		offsetof(struct ioctrl512x, io_control_pata_isolate), 1, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC3=GPIO14 */
	{
		offsetof(struct ioctrl512x, io_control_pata_iochrdy), 1, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC2=DIU_LD22 Sets Next 2 to DIU_LD pads */
	/* DIU_LD22-DIU_LD23 */
	{
		offsetof(struct ioctrl512x, io_control_pci_ad31), 2, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(1)
	},
	/* FUNC2=USB1_DATA7 Sets Next 12 to USB1 pads */
	/* USB1_DATA7-USB1_DATA0, USB1_STOP, USB1_NEXT, USB1_CLK, USB1_DIR */
	{
		offsetof(struct ioctrl512x, io_control_pci_ad29), 12, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(1)
	},
	/* FUNC1=VIU_DATA0 Sets Next 3 to VIU_DATA pads */
	/* VIU_DATA0-VIU_DATA2 */
	{
		offsetof(struct ioctrl512x, io_control_pci_ad17), 3, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(1)
	},
	/* FUNC2=FEC_TXD_0 */
	{
		offsetof(struct ioctrl512x, io_control_pci_ad14), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(1)
	},
	/* FUNC1=VIU_DATA3 Sets Next 2 to VIU_DATA pads */
	/* VIU_DATA3, VIU_DATA4 */
	{
		offsetof(struct ioctrl512x, io_control_pci_ad13), 2, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(1)
	},
	/* FUNC2=FEC_RXD_1 Sets Next 12 to FEC pads */
	/* FEC_RXD_1, FEC_RXD_0, FEC_RX_CLK, FEC_TX_CLK, FEC_RX_ER, FEC_RX_DV */
	/* FEC_TX_EN, FEC_TX_ER, FEC_CRS, FEC_MDC, FEC_MDIO, FEC_COL */
	{
		offsetof(struct ioctrl512x, io_control_pci_ad11), 12, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(1)
	},
	/* FUNC2=DIU_LD03 Sets Next 25 to DIU pads */
	/* DIU_LD00-DIU_LD21 */
	{
		offsetof(struct ioctrl512x, io_control_pci_cbe0), 22, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(1)
	},
	/* FUNC2=DIU_CLK Sets Next 3 to DIU pads */
	/* DIU_CLK, DIU_VSYNC, DIU_HSYNC */
	{
		offsetof(struct ioctrl512x, io_control_spdif_txclk), 3, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC2=CAN3_RX */
	{
		offsetof(struct ioctrl512x, io_control_irq1), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* Sets lowest slew on 2 CAN_TX Pins*/
	{
		offsetof(struct ioctrl512x, io_control_can1_tx), 2, 0,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC3=CAN4_TX Sets Next 2 to CAN4 pads */
	/* CAN4_TX, CAN4_RX */
	{
		offsetof(struct ioctrl512x, io_control_j1850_tx), 2, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC3=GPIO8 Sets Next 2 to GPIO pads */
	/* GPIO8, GPIO9 */
	{
		offsetof(struct ioctrl512x, io_control_psc0_0), 2, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC1=FEC_TXD_1 Sets Next 3 to FEC pads */
	/* FEC_TXD_1, FEC_TXD_2, FEC_TXD_3 */
	{
		offsetof(struct ioctrl512x, io_control_psc0_4), 3, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=FEC_RXD_3 Sets Next 2 to FEC pads */
	/* FEC_RXD_3, FEC_RXD_2 */
	{
		offsetof(struct ioctrl512x, io_control_psc1_4), 2, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=GPIO17 */
	{
		offsetof(struct ioctrl512x, io_control_psc2_1), 1, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC3=GPIO2/GPT2 Sets Next 3 to GPIO pads */
	/* GPIO2, GPIO20, GPIO21 */
	{
		offsetof(struct ioctrl512x, io_control_psc2_4), 3, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC2=VIU_PIX_CLK */
	{
		offsetof(struct ioctrl512x, io_control_psc3_4), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=GPIO24 Sets Next 2 to GPIO pads */
	/* GPIO24, GPIO25 */
	{
		offsetof(struct ioctrl512x, io_control_psc4_0), 2, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC1=NFC_CE2 */
	{
		offsetof(struct ioctrl512x, io_control_psc4_4), 1, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(1) |
		IO_PIN_PUE(1) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC2=VIU_DATA5 Sets Next 5 to VIU_DATA pads */
	/* VIU_DATA5-VIU_DATA9 */
	{
		offsetof(struct ioctrl512x, io_control_psc5_0), 5, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=LPC_TSIZ1 Sets Next 2 to LPC_TSIZ pads */
	/* LPC_TSIZ1-LPC_TSIZ2 */
	{
		offsetof(struct ioctrl512x, io_control_psc6_0), 2, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=LPC_TS */
	{
		offsetof(struct ioctrl512x, io_control_psc6_4), 1, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC3=GPIO16 */
	{
		offsetof(struct ioctrl512x, io_control_psc7_0), 1, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC3=GPIO18 Sets Next 3 to GPIO pads */
	/* GPIO18-GPIO19, GPT7/GPIO7 */
	{
		offsetof(struct ioctrl512x, io_control_psc7_2), 3, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC3=GPIO0/GPT0 */
	{
		offsetof(struct ioctrl512x, io_control_psc8_4), 1, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC3=GPIO11 Sets Next 4 to GPIO pads */
	/* GPIO11, GPIO2, GPIO12, GPIO13 */
	{
		offsetof(struct ioctrl512x, io_control_psc10_3), 4, 0,
		IO_PIN_FMUX(3) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(0)
	},
	/* FUNC2=DIU_DE */
	{
		offsetof(struct ioctrl512x, io_control_psc11_4), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	}
};

int checkboard (void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;

	puts("Board: PDM360NG\n");

	/* initialize function mux & slew rate IO inter alia on IO Pins  */

	iopin_initialize(ioregs_init, ARRAY_SIZE(ioregs_init));

	/* initialize IO_CONTROL_GP (GPIO/GPT-mux-register) */
	setbits_be32(&im->io_ctrl.io_control_gp,
		     (1 << 0) |   /* GP_MUX7->GPIO7 */
		     (1 << 5));	  /* GP_MUX2->GPIO2 */

	/* configure GPIO24 (VIU_CE), output/high */
	setbits_be32(&im->gpio.gpdir, 0x80);
	setbits_be32(&im->gpio.gpdat, 0x80);

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
struct node_info nodes[] = {
	{ "fsl,mpc5121-nfc",	MTD_DEV_TYPE_NAND, },
	{ "cfi-flash",		MTD_DEV_TYPE_NOR,  },
};
#endif

#if defined(CONFIG_VIDEO)
/*
 * EDID block has been generated using Phoenix EDID Designer 1.3.
 * This tool creates a text file containing:
 *
 * EDID BYTES:
 * 0x   00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
 *     ------------------------------------------------
 * 00 | 00 FF FF FF FF FF FF 00 42 C9 34 12 01 00 00 00
 * 10 | 0A 0C 01 03 80 98 5B 78 CA 7E 50 A0 58 4E 96 25
 * 20 | 1E 50 54 00 00 00 01 01 01 01 01 01 01 01 01 01
 * 30 | 01 01 01 01 01 01 80 0C 20 00 31 E0 2D 10 2A 80
 * 40 | 12 08 30 E4 10 00 00 18 00 00 00 FD 00 38 3C 1F
 * 50 | 3C 04 0A 20 20 20 20 20 20 20 00 00 00 FF 00 50
 * 60 | 4D 30 37 30 57 4C 33 0A 0A 0A 0A 0A 00 00 00 FF
 * 70 | 00 41 30 30 30 30 30 30 30 30 30 30 30 31 00 D4
 *
 * Then this data has been manually converted to the char
 * array below.
 */
static unsigned char edid_buf[128] = {
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
	0x42, 0xC9, 0x34, 0x12, 0x01, 0x00, 0x00, 0x00,
	0x0A, 0x0C, 0x01, 0x03, 0x80, 0x98, 0x5B, 0x78,
	0xCA, 0x7E, 0x50, 0xA0, 0x58, 0x4E, 0x96, 0x25,
	0x1E, 0x50, 0x54, 0x00, 0x00, 0x00, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x80, 0x0C,
	0x20, 0x00, 0x31, 0xE0, 0x2D, 0x10, 0x2A, 0x80,
	0x12, 0x08, 0x30, 0xE4, 0x10, 0x00, 0x00, 0x18,
	0x00, 0x00, 0x00, 0xFD, 0x00, 0x38, 0x3C, 0x1F,
	0x3C, 0x04, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x50,
	0x4D, 0x30, 0x37, 0x30, 0x57, 0x4C, 0x33, 0x0A,
	0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0xFF,
	0x00, 0x41, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
	0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x00, 0xD4,
};
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	u32 val[8];
	int rc, i = 0;

	ft_cpu_setup(blob, bd);
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif
#if defined(CONFIG_VIDEO)
	fdt_add_edid(blob, "fsl,mpc5121-diu", edid_buf);
#endif

	/* Fixup NOR FLASH mapping */
	val[i++] = 0;				/* chip select number */
	val[i++] = 0;				/* always 0 */
	val[i++] = gd->bd->bi_flashstart;
	val[i++] = gd->bd->bi_flashsize;

	/* Fixup MRAM mapping */
	val[i++] = 2;				/* chip select number */
	val[i++] = 0;				/* always 0 */
	val[i++] = CONFIG_SYS_MRAM_BASE;
	val[i++] = CONFIG_SYS_MRAM_SIZE;

	rc = fdt_find_and_setprop(blob, "/localbus", "ranges",
				  val, i * sizeof(u32), 1);
	if (rc)
		printf("Unable to update localbus ranges, err=%s\n",
		       fdt_strerror(rc));

	/* Fixup reg property in NOR Flash node */
	i = 0;
	val[i++] = 0;			/* always 0 */
	val[i++] = 0;			/* start at offset 0 */
	val[i++] = flash_info[0].size;	/* size of Bank 0 */

	/* Second Bank available? */
	if (flash_info[1].size > 0) {
		val[i++] = 0;			/* always 0 */
		val[i++] = flash_info[0].size;	/* offset of Bank 1 */
		val[i++] = flash_info[1].size;	/* size of Bank 1 */
	}

	rc = fdt_find_and_setprop(blob, "/localbus/flash", "reg",
				  val, i * sizeof(u32), 1);
	if (rc)
		printf("Unable to update flash reg property, err=%s\n",
		       fdt_strerror(rc));

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */

/*
 * If argument is NULL, set the LCD brightness to the
 * value from "brightness" environment variable. Set
 * the LCD brightness to the value specified by the
 * argument otherwise. Default brightness is zero.
 */
#define MAX_BRIGHTNESS	99
static int set_lcd_brightness(char *brightness)
{
	struct stdio_dev *cop_port;
	char *env;
	char cmd_buf[20];
	int val = 0;
	int cs = 0;
	int len, i;

	if (brightness) {
		val = simple_strtol(brightness, NULL, 10);
	} else {
		env = getenv("brightness");
		if (env)
			val = simple_strtol(env, NULL, 10);
	}

	if (val < 0)
		val = 0;

	if (val > MAX_BRIGHTNESS)
		val = MAX_BRIGHTNESS;

	sprintf(cmd_buf, "$SB;%04d;", val);

	len = strlen(cmd_buf);
	for (i = 1; i <= len; i++)
		cs += cmd_buf[i];

	cs = (~cs + 1) & 0xff;
	sprintf(cmd_buf + len, "%02X\n", cs);

	/* IO Coprocessor communication */
	cop_port = open_port(4, CONFIG_SYS_PDM360NG_COPROC_BAUDRATE);
	if (!cop_port) {
		printf("Error: Can't open IO Coprocessor port.\n");
		return -1;
	}

	debug("%s: cmd: %s", __func__, cmd_buf);
	write_port(cop_port, cmd_buf);
	/*
	 * Wait for transmission and maybe response data
	 * before closing the port.
	 */
	udelay(CONFIG_SYS_PDM360NG_COPROC_READ_DELAY);
	memset(cmd_buf, 0, sizeof(cmd_buf));
	len = read_port(cop_port, cmd_buf, sizeof(cmd_buf));
	if (len)
		printf("Error: %s\n", cmd_buf);

	close_port(4);

	return 0;
}

static int cmd_lcd_brightness(cmd_tbl_t *cmdtp, int flag,
			      int argc, char * const argv[])
{
	if (argc < 2)
		return cmd_usage(cmdtp);

	return set_lcd_brightness(argv[1]);
}

U_BOOT_CMD(lcdbr, 2, 1, cmd_lcd_brightness,
	"set LCD brightness",
	"<brightness> - set LCD backlight level to <brightness>.\n"
);
