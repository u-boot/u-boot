/*
 * (C) Copyright 2007-2009 DENX Software Engineering
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common.h>
#include <asm/bitops.h>
#include <command.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/mpc512x.h>
#include <fdt_support.h>
#ifdef CONFIG_MISC_INIT_R
#include <i2c.h>
#endif

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>

DECLARE_GLOBAL_DATA_PTR;

/* Clocks in use */
#define SCCR1_CLOCKS_EN	(CLOCK_SCCR1_CFG_EN |				\
			 CLOCK_SCCR1_DDR_EN |				\
			 CLOCK_SCCR1_FEC_EN |				\
			 CLOCK_SCCR1_LPC_EN |				\
			 CLOCK_SCCR1_NFC_EN |				\
			 CLOCK_SCCR1_PATA_EN |				\
			 CLOCK_SCCR1_PCI_EN |				\
			 CLOCK_SCCR1_PSC_EN(CONFIG_PSC_CONSOLE) |	\
			 CLOCK_SCCR1_PSCFIFO_EN |			\
			 CLOCK_SCCR1_TPR_EN)

#define SCCR2_CLOCKS_EN	(CLOCK_SCCR2_DIU_EN |		\
			 CLOCK_SCCR2_I2C_EN |		\
			 CLOCK_SCCR2_MEM_EN |		\
			 CLOCK_SCCR2_SPDIF_EN)

void __mpc5121_nfc_select_chip(struct mtd_info *mtd, int chip);

/* Active chip number set in board_nand_select_device() (mpc5121_nfc.c) */
extern int mpc5121_nfc_chip;

/* Control chips select signal on MPC5121ADS board */
void mpc5121_nfc_select_chip(struct mtd_info *mtd, int chip)
{
	unsigned char *csreg = (u8 *)CONFIG_SYS_CPLD_BASE + 0x09;
	u8 v;

	v = in_8(csreg);
	v |= 0x0F;

	if (chip >= 0) {
		__mpc5121_nfc_select_chip(mtd, 0);
		v &= ~(1 << mpc5121_nfc_chip);
	} else {
		__mpc5121_nfc_select_chip(mtd, -1);
	}

	out_8(csreg, v);
}

int board_early_init_f(void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 spridr;

	/*
	 * Initialize Local Window for the CPLD registers access (CS2 selects
	 * the CPLD chip)
	 */
	out_be32(&im->sysconf.lpcs2aw,
		CSAW_START(CONFIG_SYS_CPLD_BASE) |
		CSAW_STOP(CONFIG_SYS_CPLD_BASE, CONFIG_SYS_CPLD_SIZE)
	);
	out_be32(&im->lpc.cs_cfg[2], CONFIG_SYS_CS2_CFG);
	sync_law(&im->sysconf.lpcs2aw);

	/*
	 * Disable Boot NOR FLASH write protect - CPLD Reg 8 NOR FLASH Control
	 *
	 * Without this the flash identification routine fails, as it needs to issue
	 * write commands in order to establish the device ID.
	 */

#ifdef CONFIG_ADS5121_REV2
	out_8((u8 *)(CONFIG_SYS_CPLD_BASE + 0x08), 0xC1);
#else
	if (in_8((u8 *)(CONFIG_SYS_CPLD_BASE + 0x08)) & 0x04) {
		out_8((u8 *)(CONFIG_SYS_CPLD_BASE + 0x08), 0xC1);
	} else {
		/* running from Backup flash */
		out_8((u8 *)(CONFIG_SYS_CPLD_BASE + 0x08), 0x32);
	}
#endif
	/*
	 * Configure Flash Speed
	 */
	out_be32(&im->lpc.cs_cfg[0], CONFIG_SYS_CS0_CFG);

	spridr = in_be32(&im->sysconf.spridr);

	if (SVR_MJREV (spridr) >= 2)
		out_be32 (&im->lpc.altr, CONFIG_SYS_CS_ALETIMING);

	/*
	 * Enable clocks
	 */
	out_be32 (&im->clk.sccr[0], SCCR1_CLOCKS_EN);
	out_be32 (&im->clk.sccr[1], SCCR2_CLOCKS_EN);
#if defined(CONFIG_IIM) || defined(CONFIG_CMD_FUSE)
	setbits_be32 (&im->clk.sccr[1], CLOCK_SCCR2_IIM_EN);
#endif

	return 0;
}

phys_size_t initdram(int board_type)
{
	u32 msize = 0;

	msize = fixed_sdram();

	return msize;
}

int misc_init_r(void)
{
	u8 tmp_val;

	/* Using this for DIU init before the driver in linux takes over
	 *  Enable the TFP410 Encoder (I2C address 0x38)
	 */

	i2c_set_bus_num(2);
	tmp_val = 0xBF;
	i2c_write(0x38, 0x08, 1, &tmp_val, sizeof(tmp_val));
	/* Verify if enabled */
	tmp_val = 0;
	i2c_read(0x38, 0x08, 1, &tmp_val, sizeof(tmp_val));
	debug("DVI Encoder Read: 0x%02lx\n", tmp_val);

	tmp_val = 0x10;
	i2c_write(0x38, 0x0A, 1, &tmp_val, sizeof(tmp_val));
	/* Verify if enabled */
	tmp_val = 0;
	i2c_read(0x38, 0x0A, 1, &tmp_val, sizeof(tmp_val));
	debug("DVI Encoder Read: 0x%02lx\n", tmp_val);

#ifdef CONFIG_FSL_DIU_FB
# if	!(defined(CONFIG_VIDEO) || defined(CONFIG_CFB_CONSOLE))
	mpc5121_diu_init();
# endif
#endif
	return 0;
}

static  iopin_t ioregs_init[] = {
	/* FUNC1=FEC_RX_DV Sets Next 3 to FEC pads */
	{
		offsetof(struct ioctrl512x, io_control_spdif_txclk), 3, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* Set highest Slew on 9 PATA pins */
	{
		offsetof(struct ioctrl512x, io_control_pata_ce1), 9, 1,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=FEC_COL Sets Next 15 to FEC pads */
	{
		offsetof(struct ioctrl512x, io_control_psc0_0), 15, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=SPDIF_TXCLK */
	{
		offsetof(struct ioctrl512x, io_control_lpc_cs1), 1, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=SPDIF_TX and sets Next pin to SPDIF_RX */
	{
		offsetof(struct ioctrl512x, io_control_i2c1_scl), 2, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=DIU CLK */
	{
		offsetof(struct ioctrl512x, io_control_psc6_0), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=DIU_HSYNC */
	{
		offsetof(struct ioctrl512x, io_control_psc6_1), 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC2=DIUVSYNC Sets Next 26 to DIU Pads */
	{
		offsetof(struct ioctrl512x, io_control_psc6_4), 26, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	}
};

static  iopin_t rev2_silicon_pci_ioregs_init[] = {
	/* FUNC0=PCI Sets next 54 to PCI pads */
	{
		offsetof(struct ioctrl512x, io_control_pci_ad31), 54, 0,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_DS(0)
	}
};

int checkboard (void)
{
	ushort brd_rev = *(vu_short *) (CONFIG_SYS_CPLD_BASE + 0x00);
	uchar cpld_rev = *(vu_char *) (CONFIG_SYS_CPLD_BASE + 0x02);
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 spridr = in_be32(&im->sysconf.spridr);

	printf ("Board: ADS5121 rev. 0x%04x (CPLD rev. 0x%02x)\n",
		brd_rev, cpld_rev);

	/* initialize function mux & slew rate IO inter alia on IO Pins  */
	iopin_initialize(ioregs_init, ARRAY_SIZE(ioregs_init));

	if (SVR_MJREV (spridr) >= 2)
		iopin_initialize(rev2_silicon_pci_ioregs_init, 1);

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
