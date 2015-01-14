/*
 * Copyright (C) 2014 Gateworks Corporation
 * Author: Tim Harvey <tharvey@gateworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
#include <spl.h>

#include "ventana_eeprom.h"

DECLARE_GLOBAL_DATA_PTR;

#define RTT_NOM_120OHM /* use 120ohm Rtt_nom vs 60ohm (lower power) */
#define I2C_GSC			0
#define GSC_EEPROM_ADDR		0x51
#define GSC_EEPROM_DDR_SIZE	0x2B	/* enum (512,1024,2048) MB */
#define GSC_EEPROM_DDR_WIDTH	0x2D	/* enum (32,64) bit */
#define I2C_PAD_CTRL  (PAD_CTL_PUS_100K_UP |                    \
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |   \
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
#define CONFIG_SYS_I2C_SPEED	100000

/* I2C1: GSC */
static struct i2c_pads_info mx6q_i2c_pad_info0 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_EIM_D21__I2C1_SCL | PC,
		.gpio_mode = MX6Q_PAD_EIM_D21__GPIO3_IO21 | PC,
		.gp = IMX_GPIO_NR(3, 21)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_EIM_D28__I2C1_SDA | PC,
		.gpio_mode = MX6Q_PAD_EIM_D28__GPIO3_IO28 | PC,
		.gp = IMX_GPIO_NR(3, 28)
	}
};
static struct i2c_pads_info mx6dl_i2c_pad_info0 = {
	.scl = {
		.i2c_mode = MX6DL_PAD_EIM_D21__I2C1_SCL | PC,
		.gpio_mode = MX6DL_PAD_EIM_D21__GPIO3_IO21 | PC,
		.gp = IMX_GPIO_NR(3, 21)
	},
	.sda = {
		.i2c_mode = MX6DL_PAD_EIM_D28__I2C1_SDA | PC,
		.gpio_mode = MX6DL_PAD_EIM_D28__GPIO3_IO28 | PC,
		.gp = IMX_GPIO_NR(3, 28)
	}
};

static void i2c_setup_iomux(void)
{
	if (is_cpu_type(MXC_CPU_MX6Q))
		setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info0);
	else
		setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6dl_i2c_pad_info0);
}

/* configure MX6Q/DUAL mmdc DDR io registers */
struct mx6dq_iomux_ddr_regs mx6dq_ddr_ioregs = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0 = 0x00020030,
	.dram_sdclk_1 = 0x00020030,
	.dram_cas = 0x00020030,
	.dram_ras = 0x00020030,
	.dram_reset = 0x00020030,
	/* SDCKE[0:1]: 100k pull-up */
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	/* SDBA2: pull-up disabled */
	.dram_sdba2 = 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0 = 0x00003030,
	.dram_sdodt1 = 0x00003030,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0 = 0x00000030,
	.dram_sdqs1 = 0x00000030,
	.dram_sdqs2 = 0x00000030,
	.dram_sdqs3 = 0x00000030,
	.dram_sdqs4 = 0x00000030,
	.dram_sdqs5 = 0x00000030,
	.dram_sdqs6 = 0x00000030,
	.dram_sdqs7 = 0x00000030,

	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0 = 0x00020030,
	.dram_dqm1 = 0x00020030,
	.dram_dqm2 = 0x00020030,
	.dram_dqm3 = 0x00020030,
	.dram_dqm4 = 0x00020030,
	.dram_dqm5 = 0x00020030,
	.dram_dqm6 = 0x00020030,
	.dram_dqm7 = 0x00020030,
};

/* configure MX6Q/DUAL mmdc GRP io registers */
struct mx6dq_iomux_grp_regs mx6dq_grp_ioregs = {
	/* DDR3 */
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	/* disable DDR pullups */
	.grp_ddrpke = 0x00000000,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds = 0x00000030,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds = 0x00000030,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000030,
	.grp_b1ds = 0x00000030,
	.grp_b2ds = 0x00000030,
	.grp_b3ds = 0x00000030,
	.grp_b4ds = 0x00000030,
	.grp_b5ds = 0x00000030,
	.grp_b6ds = 0x00000030,
	.grp_b7ds = 0x00000030,
};

/* configure MX6SOLO/DUALLITE mmdc DDR io registers */
struct mx6sdl_iomux_ddr_regs mx6sdl_ddr_ioregs = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0 = 0x00020030,
	.dram_sdclk_1 = 0x00020030,
	.dram_cas = 0x00020030,
	.dram_ras = 0x00020030,
	.dram_reset = 0x00020030,
	/* SDCKE[0:1]: 100k pull-up */
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	/* SDBA2: pull-up disabled */
	.dram_sdba2 = 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0 = 0x00003030,
	.dram_sdodt1 = 0x00003030,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0 = 0x00000030,
	.dram_sdqs1 = 0x00000030,
	.dram_sdqs2 = 0x00000030,
	.dram_sdqs3 = 0x00000030,
	.dram_sdqs4 = 0x00000030,
	.dram_sdqs5 = 0x00000030,
	.dram_sdqs6 = 0x00000030,
	.dram_sdqs7 = 0x00000030,

	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0 = 0x00020030,
	.dram_dqm1 = 0x00020030,
	.dram_dqm2 = 0x00020030,
	.dram_dqm3 = 0x00020030,
	.dram_dqm4 = 0x00020030,
	.dram_dqm5 = 0x00020030,
	.dram_dqm6 = 0x00020030,
	.dram_dqm7 = 0x00020030,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs = {
	/* DDR3 */
	.grp_ddr_type = 0x000c0000,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.grp_ddrmode_ctl = 0x00020000,
	/* disable DDR pullups */
	.grp_ddrpke = 0x00000000,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds = 0x00000030,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds = 0x00000030,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000030,
	.grp_b1ds = 0x00000030,
	.grp_b2ds = 0x00000030,
	.grp_b3ds = 0x00000030,
	.grp_b4ds = 0x00000030,
	.grp_b5ds = 0x00000030,
	.grp_b6ds = 0x00000030,
	.grp_b7ds = 0x00000030,
};

/* MT41K128M16JT-125 */
static struct mx6_ddr3_cfg mt41k128m16jt_125 = {
	.mem_speed = 1600,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/* MT41K256M16HA-125 */
static struct mx6_ddr3_cfg mt41k256m16ha_125 = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/*
 * calibration - these are the various CPU/DDR3 combinations we support
 */

static struct mx6_mmdc_calibration mx6dq_128x32_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00190017,
	.p0_mpwldectrl1 = 0x00140026,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43380347,
	.p0_mpdgctrl1 = 0x433C034D,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x3C313539,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x36393C39,
};

static struct mx6_mmdc_calibration mx6sdl_128x32_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x003C003C,
	.p0_mpwldectrl1 = 0x001F002A,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x42410244,
	.p0_mpdgctrl1 = 0x4234023A,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x484A4C4B,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x33342B32,
};

static struct mx6_mmdc_calibration mx6dq_128x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00190017,
	.p0_mpwldectrl1 = 0x00140026,
	.p1_mpwldectrl0 = 0x0021001C,
	.p1_mpwldectrl1 = 0x0011001D,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43380347,
	.p0_mpdgctrl1 = 0x433C034D,
	.p1_mpdgctrl0 = 0x032C0324,
	.p1_mpdgctrl1 = 0x03310232,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x3C313539,
	.p1_mprddlctl = 0x37343141,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x36393C39,
	.p1_mpwrdlctl = 0x42344438,
};

static struct mx6_mmdc_calibration mx6sdl_128x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x003C003C,
	.p0_mpwldectrl1 = 0x001F002A,
	.p1_mpwldectrl0 = 0x00330038,
	.p1_mpwldectrl1 = 0x0022003F,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x42410244,
	.p0_mpdgctrl1 = 0x4234023A,
	.p1_mpdgctrl0 = 0x022D022D,
	.p1_mpdgctrl1 = 0x021C0228,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x484A4C4B,
	.p1_mprddlctl = 0x4B4D4E4B,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x33342B32,
	.p1_mpwrdlctl = 0x3933332B,
};

static struct mx6_mmdc_calibration mx6dq_256x32_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x001E001A,
	.p0_mpwldectrl1 = 0x0026001F,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43370349,
	.p0_mpdgctrl1 = 0x032D0327,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x3D303639,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x32363934,
};

static struct mx6_mmdc_calibration mx6dq_256x64_mmdc_calib = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0X00220021,
	.p0_mpwldectrl1 = 0X00200030,
	.p1_mpwldectrl0 = 0X002D0027,
	.p1_mpwldectrl1 = 0X00150026,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x43330342,
	.p0_mpdgctrl1 = 0x0339034A,
	.p1_mpdgctrl0 = 0x032F0325,
	.p1_mpdgctrl1 = 0x032F022E,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0X3A2E3437,
	.p1_mprddlctl = 0X35312F3F,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0X33363B37,
	.p1_mpwrdlctl = 0X40304239,
};

static void spl_dram_init(int width, int size_mb, int board_model)
{
	struct mx6_ddr3_cfg *mem = NULL;
	struct mx6_mmdc_calibration *calib = NULL;
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize = width/32,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32, /* 32Gb per CS */
		/* single chip select */
		.ncs = 1,
		.cs1_mirror = 0,
		.rtt_wr = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Wr = RZQ/4 */
#ifdef RTT_NOM_120OHM
		.rtt_nom = 2 /*DDR3_RTT_120_OHM*/,	/* RTT_Nom = RZQ/2 */
#else
		.rtt_nom = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Nom = RZQ/4 */
#endif
		.walat = 1,	/* Write additional latency */
		.ralat = 5,	/* Read additional latency */
		.mif3_mode = 3,	/* Command prediction working mode */
		.bi_on = 1,	/* Bank interleaving enabled */
		.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	};

	/*
	 * MMDC Calibration requires the following data:
	 *   mx6_mmdc_calibration - board-specific calibration (routing delays)
	 *      these calibration values depend on board routing, SoC, and DDR
	 *   mx6_ddr_sysinfo - board-specific memory architecture (width/cs/etc)
	 *   mx6_ddr_cfg - chip specific timing/layout details
	 */
	if (width == 32 && size_mb == 512) {
		mem = &mt41k128m16jt_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_128x32_mmdc_calib;
		else
			calib = &mx6sdl_128x32_mmdc_calib;
		debug("2gB density\n");
	} else if (width == 64 && size_mb == 1024) {
		mem = &mt41k128m16jt_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_128x64_mmdc_calib;
		else
			calib = &mx6sdl_128x64_mmdc_calib;
		debug("2gB density\n");
	} else if (width == 32 && size_mb == 1024) {
		mem = &mt41k256m16ha_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_256x32_mmdc_calib;
		debug("4gB density\n");
	} else if (width == 64 && size_mb == 2048) {
		mem = &mt41k256m16ha_125;
		if (is_cpu_type(MXC_CPU_MX6Q))
			calib = &mx6dq_256x64_mmdc_calib;
		debug("4gB density\n");
	}

	if (!mem) {
		puts("Error: Invalid Memory Configuration\n");
		hang();
	}
	if (!calib) {
		puts("Error: Invalid Board Calibration Configuration\n");
		hang();
	}

	if (is_cpu_type(MXC_CPU_MX6Q))
		mx6dq_dram_iocfg(width, &mx6dq_ddr_ioregs,
				 &mx6dq_grp_ioregs);
	else
		mx6sdl_dram_iocfg(width, &mx6sdl_ddr_ioregs,
				  &mx6sdl_grp_ioregs);
	mx6_dram_cfg(&sysinfo, calib, mem);
}

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0xFFFFF300, &ccm->CCGR4);	/* enable NAND/GPMI/BCH clks */
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static void gpr_init(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* enable AXI cache for VDOA/VPU/IPU */
	writel(0xF00000CF, &iomux->gpr[4]);
	/* set IPU AXI-id0 Qos=0xf(bypass) AXI-id1 Qos=0x7 */
	writel(0x007F007F, &iomux->gpr[6]);
	writel(0x007F007F, &iomux->gpr[7]);
}

/*
 * called from C runtime startup code (arch/arm/lib/crt0.S:_main)
 * - we have a stack and a place to store GD, both in SRAM
 * - no variable global data is available
 */
void board_init_f(ulong dummy)
{
	struct ventana_board_info ventana_info;
	int board_model;

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux and setup of i2c */
	board_early_init_f();
	i2c_setup_iomux();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* read/validate EEPROM info to determine board model and SDRAM cfg */
	board_model = read_eeprom(I2C_GSC, &ventana_info);

	/* provide some some default: 32bit 128MB */
	if (GW_UNKNOWN == board_model) {
		ventana_info.sdram_width = 2;
		ventana_info.sdram_size = 3;
	}

	/* configure MMDC for SDRAM width/size and per-model calibration */
	spl_dram_init(8 << ventana_info.sdram_width,
		      16 << ventana_info.sdram_size,
		      board_model);

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

void reset_cpu(ulong addr)
{
}
