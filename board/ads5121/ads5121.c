/*
 * (C) Copyright 2007 DENX Software Engineering
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
#include <mpc512x.h>
#include <asm/bitops.h>
#include <command.h>
#include <asm/processor.h>
#include <fdt_support.h>
#ifdef CONFIG_MISC_INIT_R
#include <i2c.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* Clocks in use */
#define SCCR1_CLOCKS_EN	(CLOCK_SCCR1_CFG_EN |				\
			 CLOCK_SCCR1_LPC_EN |				\
			 CLOCK_SCCR1_PSC_EN(CONFIG_PSC_CONSOLE) |	\
			 CLOCK_SCCR1_PSCFIFO_EN |			\
			 CLOCK_SCCR1_DDR_EN |				\
			 CLOCK_SCCR1_FEC_EN |				\
			 CLOCK_SCCR1_PATA_EN |				\
			 CLOCK_SCCR1_PCI_EN |				\
			 CLOCK_SCCR1_TPR_EN)

#define SCCR2_CLOCKS_EN	(CLOCK_SCCR2_MEM_EN |		\
			 CLOCK_SCCR2_SPDIF_EN |		\
			 CLOCK_SCCR2_DIU_EN |		\
			 CLOCK_SCCR2_I2C_EN)

#define CSAW_START(start)	((start) & 0xFFFF0000)
#define CSAW_STOP(start, size)	(((start) + (size) - 1) >> 16)

long int fixed_sdram(void);

int board_early_init_f (void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 lpcaw;

	/*
	 * Initialize Local Window for the CPLD registers access (CS2 selects
	 * the CPLD chip)
	 */
	im->sysconf.lpcs2aw = CSAW_START(CONFIG_SYS_CPLD_BASE) |
			      CSAW_STOP(CONFIG_SYS_CPLD_BASE, CONFIG_SYS_CPLD_SIZE);
	im->lpc.cs_cfg[2] = CONFIG_SYS_CS2_CFG;

	/*
	 * According to MPC5121e RM, configuring local access windows should
	 * be followed by a dummy read of the config register that was
	 * modified last and an isync
	 */
	lpcaw = im->sysconf.lpcs2aw;
	__asm__ __volatile__ ("isync");

	/*
	 * Disable Boot NOR FLASH write protect - CPLD Reg 8 NOR FLASH Control
	 *
	 * Without this the flash identification routine fails, as it needs to issue
	 * write commands in order to establish the device ID.
	 */

#ifdef CONFIG_ADS5121_REV2
	*((volatile u8 *)(CONFIG_SYS_CPLD_BASE + 0x08)) = 0xC1;
#else
	if (*((u8 *)(CONFIG_SYS_CPLD_BASE + 0x08)) & 0x04) {
		*((volatile u8 *)(CONFIG_SYS_CPLD_BASE + 0x08)) = 0xC1;
	} else {
		/* running from Backup flash */
		*((volatile u8 *)(CONFIG_SYS_CPLD_BASE + 0x08)) = 0x32;
	}
#endif
	/*
	 * Configure Flash Speed
	 */
	*((volatile u32 *)(CONFIG_SYS_IMMR + LPC_OFFSET + CS0_CONFIG)) = CONFIG_SYS_CS0_CFG;
	if (SVR_MJREV (im->sysconf.spridr) >= 2) {
		*((volatile u32 *)(CONFIG_SYS_IMMR + LPC_OFFSET + CS_ALE_TIMING_CONFIG)) = CONFIG_SYS_CS_ALETIMING;
	}
	/*
	 * Enable clocks
	 */
	im->clk.sccr[0] = SCCR1_CLOCKS_EN;
	im->clk.sccr[1] = SCCR2_CLOCKS_EN;
#if defined(CONFIG_IIM) || defined(CONFIG_CMD_FUSE)
	im->clk.sccr[1] |= CLOCK_SCCR2_IIM_EN;
#endif

	return 0;
}

phys_size_t initdram (int board_type)
{
	u32 msize = 0;

	msize = fixed_sdram ();

	return msize;
}

/*
 * fixed sdram init -- the board doesn't use memory modules that have serial presence
 * detect or similar mechanism for discovery of the DRAM settings
 */
long int fixed_sdram (void)
{
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msize = CONFIG_SYS_DDR_SIZE * 1024 * 1024;
	u32 msize_log2 = __ilog2 (msize);
	u32 i;

	/* Initialize IO Control */
	im->io_ctrl.regs[IOCTL_MEM/4] = IOCTRL_MUX_DDR;

	/* Initialize DDR Local Window */
	im->sysconf.ddrlaw.bar = CONFIG_SYS_DDR_BASE & 0xFFFFF000;
	im->sysconf.ddrlaw.ar = msize_log2 - 1;

	/*
	 * According to MPC5121e RM, configuring local access windows should
	 * be followed by a dummy read of the config register that was
	 * modified last and an isync
	 */
	i = im->sysconf.ddrlaw.ar;
	__asm__ __volatile__ ("isync");

	/* Enable DDR */
	im->mddrc.ddr_sys_config = CONFIG_SYS_MDDRC_SYS_CFG_EN;

	/* Initialize DDR Priority Manager */
	im->mddrc.prioman_config1 = CONFIG_SYS_MDDRCGRP_PM_CFG1;
	im->mddrc.prioman_config2 = CONFIG_SYS_MDDRCGRP_PM_CFG2;
	im->mddrc.hiprio_config = CONFIG_SYS_MDDRCGRP_HIPRIO_CFG;
	im->mddrc.lut_table0_main_upper = CONFIG_SYS_MDDRCGRP_LUT0_MU;
	im->mddrc.lut_table0_main_lower = CONFIG_SYS_MDDRCGRP_LUT0_ML;
	im->mddrc.lut_table1_main_upper = CONFIG_SYS_MDDRCGRP_LUT1_MU;
	im->mddrc.lut_table1_main_lower = CONFIG_SYS_MDDRCGRP_LUT1_ML;
	im->mddrc.lut_table2_main_upper = CONFIG_SYS_MDDRCGRP_LUT2_MU;
	im->mddrc.lut_table2_main_lower = CONFIG_SYS_MDDRCGRP_LUT2_ML;
	im->mddrc.lut_table3_main_upper = CONFIG_SYS_MDDRCGRP_LUT3_MU;
	im->mddrc.lut_table3_main_lower = CONFIG_SYS_MDDRCGRP_LUT3_ML;
	im->mddrc.lut_table4_main_upper = CONFIG_SYS_MDDRCGRP_LUT4_MU;
	im->mddrc.lut_table4_main_lower = CONFIG_SYS_MDDRCGRP_LUT4_ML;
	im->mddrc.lut_table0_alternate_upper = CONFIG_SYS_MDDRCGRP_LUT0_AU;
	im->mddrc.lut_table0_alternate_lower = CONFIG_SYS_MDDRCGRP_LUT0_AL;
	im->mddrc.lut_table1_alternate_upper = CONFIG_SYS_MDDRCGRP_LUT1_AU;
	im->mddrc.lut_table1_alternate_lower = CONFIG_SYS_MDDRCGRP_LUT1_AL;
	im->mddrc.lut_table2_alternate_upper = CONFIG_SYS_MDDRCGRP_LUT2_AU;
	im->mddrc.lut_table2_alternate_lower = CONFIG_SYS_MDDRCGRP_LUT2_AL;
	im->mddrc.lut_table3_alternate_upper = CONFIG_SYS_MDDRCGRP_LUT3_AU;
	im->mddrc.lut_table3_alternate_lower = CONFIG_SYS_MDDRCGRP_LUT3_AL;
	im->mddrc.lut_table4_alternate_upper = CONFIG_SYS_MDDRCGRP_LUT4_AU;
	im->mddrc.lut_table4_alternate_lower = CONFIG_SYS_MDDRCGRP_LUT4_AL;

	/* Initialize MDDRC */
	im->mddrc.ddr_sys_config = CONFIG_SYS_MDDRC_SYS_CFG;
	im->mddrc.ddr_time_config0 = CONFIG_SYS_MDDRC_TIME_CFG0;
	im->mddrc.ddr_time_config1 = CONFIG_SYS_MDDRC_TIME_CFG1;
	im->mddrc.ddr_time_config2 = CONFIG_SYS_MDDRC_TIME_CFG2;

	/* Initialize DDR */
	for (i = 0; i < 10; i++)
		im->mddrc.ddr_command = CONFIG_SYS_MICRON_NOP;

	im->mddrc.ddr_command = CONFIG_SYS_MICRON_PCHG_ALL;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_NOP;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_RFSH;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_NOP;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_RFSH;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_NOP;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_INIT_DEV_OP;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_NOP;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_EM2;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_NOP;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_PCHG_ALL;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_EM2;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_EM3;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_EN_DLL;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_INIT_DEV_OP;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_PCHG_ALL;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_RFSH;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_INIT_DEV_OP;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_OCD_DEFAULT;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_PCHG_ALL;
	im->mddrc.ddr_command = CONFIG_SYS_MICRON_NOP;

	/* Start MDDRC */
	im->mddrc.ddr_time_config0 = CONFIG_SYS_MDDRC_TIME_CFG0_RUN;
	im->mddrc.ddr_sys_config = CONFIG_SYS_MDDRC_SYS_CFG_RUN;

	return msize;
}

int misc_init_r(void)
{
	u8 tmp_val;
	extern int ads5121_diu_init(void);

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
#if	!(defined(CONFIG_VIDEO) || defined(CONFIG_CFB_CONSOLE))
	ads5121_diu_init();
#endif
#endif

	return 0;
}
static  iopin_t ioregs_init[] = {
	/* FUNC1=FEC_RX_DV Sets Next 3 to FEC pads */
	{
		IOCTL_SPDIF_TXCLK, 3, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* Set highest Slew on 9 PATA pins */
	{
		IOCTL_PATA_CE1, 9, 1,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=FEC_COL Sets Next 15 to FEC pads */
	{
		IOCTL_PSC0_0, 15, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC1=SPDIF_TXCLK */
	{
		IOCTL_LPC_CS1, 1, 0,
		IO_PIN_FMUX(1) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=SPDIF_TX and sets Next pin to SPDIF_RX */
	{
		IOCTL_I2C1_SCL, 2, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=DIU CLK */
	{
		IOCTL_PSC6_0, 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(1) | IO_PIN_DS(3)
	},
	/* FUNC2=DIU_HSYNC */
	{
		IOCTL_PSC6_1, 1, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	},
	/* FUNC2=DIUVSYNC Sets Next 26 to DIU Pads */
	{
		IOCTL_PSC6_4, 26, 0,
		IO_PIN_FMUX(2) | IO_PIN_HOLD(0) | IO_PIN_PUD(0) |
		IO_PIN_PUE(0) | IO_PIN_ST(0) | IO_PIN_DS(3)
	}
};

static  iopin_t rev2_silicon_pci_ioregs_init[] = {
	/* FUNC0=PCI Sets next 54 to PCI pads */
	{
		IOCTL_PCI_AD31, 54, 0,
		IO_PIN_FMUX(0) | IO_PIN_HOLD(0) | IO_PIN_DS(0)
	}
};

int checkboard (void)
{
	ushort brd_rev = *(vu_short *) (CONFIG_SYS_CPLD_BASE + 0x00);
	uchar cpld_rev = *(vu_char *) (CONFIG_SYS_CPLD_BASE + 0x02);
	volatile immap_t *im = (immap_t *) CONFIG_SYS_IMMR;

	printf ("Board: ADS5121 rev. 0x%04x (CPLD rev. 0x%02x)\n",
		brd_rev, cpld_rev);
	/* initialize function mux & slew rate IO inter alia on IO Pins  */

	iopin_initialize(ioregs_init, sizeof(ioregs_init) / sizeof(ioregs_init[0]));
	if (SVR_MJREV (im->sysconf.spridr) >= 2) {
		iopin_initialize(rev2_silicon_pci_ioregs_init, 1);
	}

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_RESET)

void init_ide_reset (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	debug ("init_ide_reset\n");

	/*
	 * Clear the reset bit to reset the interface
	 * cf. RefMan MPC5121EE: 28.4.1 Resetting the ATA Bus
	 */
	immr->pata.pata_ata_control = 0;
	udelay(100);
	/* Assert the reset bit to enable the interface */
	immr->pata.pata_ata_control = FSL_ATA_CTRL_ATA_RST_B;
	udelay(100);

}

void ide_set_reset (int idereset)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	debug ("ide_set_reset(%d)\n", idereset);

	if (idereset) {
		immr->pata.pata_ata_control = 0;
		udelay(100);
	} else {
		immr->pata.pata_ata_control = FSL_ATA_CTRL_ATA_RST_B;
		udelay(100);
	}
}

#define CALC_TIMING(t) (t + period - 1) / period

int ide_preinit (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	long t;
	const struct {
		short t0;
		short t1;
		short t2_8;
		short t2_16;
		short t2i;
		short t4;
		short t9;
		short tA;
	} pio_specs = {
		.t0    = 600,
		.t1    =  70,
		.t2_8  = 290,
		.t2_16 = 165,
		.t2i   =   0,
		.t4    =  30,
		.t9    =  20,
		.tA    =  50,
	};
	union {
		u32 config;
		struct {
			u8 field1;
			u8 field2;
			u8 field3;
			u8 field4;
		}bytes;
	}cfg;

	debug ("IDE preinit using PATA peripheral at IMMR-ADDR %08x\n",
		(u32)&immr->pata);

	/* Set the reset bit to 1 to enable the interface */
	immr->pata.pata_ata_control = FSL_ATA_CTRL_ATA_RST_B;

	/* Init timings : we use PIO mode 0 timings */
	t = 1000000000 / gd->ips_clk;	/* period in ns */
	cfg.bytes.field1 = 3;
	cfg.bytes.field2 = 3;
	cfg.bytes.field3 = (pio_specs.t1 + t) / t;
	cfg.bytes.field4 = (pio_specs.t2_8 + t) / t;

	immr->pata.pata_time1 = cfg.config;

	cfg.bytes.field1 = (pio_specs.t2_8 + t) / t;
	cfg.bytes.field2 = (pio_specs.tA + t) / t + 2;
	cfg.bytes.field3 = 1;
	cfg.bytes.field4 = (pio_specs.t4 + t) / t;

	immr->pata.pata_time2 = cfg.config;

	cfg.config = immr->pata.pata_time3;
	cfg.bytes.field1 = (pio_specs.t9 + t) / t;

	immr->pata.pata_time3 = cfg.config;
	debug ("PATA preinit complete.\n");

	return 0;
}

#endif /* defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_RESET) */
