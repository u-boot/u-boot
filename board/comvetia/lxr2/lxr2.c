// SPDX-License-Identifier: GPL-2.0+
//
// Copyright (C) 2017 Stefano Babic <sbabic@denx.de>
// Copyright (C) 2024 Fabio Estevam <festevam@denx.de>

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>

#include <asm/mach-imx/spi.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <nand.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/arch/sys_proto.h>
#include <asm/sections.h>
#include <linux/delay.h>

#include <image.h>
#include <init.h>
#include <serial.h>
#include <spl.h>
#include <linux/sizes.h>
#include <mmc.h>
#include <fsl_esdhc_imx.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | \
		      PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define NAND_PAD_CTRL (PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
	       PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

static const iomux_v3_cfg_t uart4_pads[] = {
	MX6_PAD_KEY_COL0__UART4_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_KEY_ROW0__UART4_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	/* gate ENFC_CLK_ROOT clock first,before clk source switch */
	clrbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	/* config gpmi and bch clock to 100 MHz */
	clrsetbits_le32(&mxc_ccm->cs2cdr,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED_MASK |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL_MASK,
			MXC_CCM_CS2CDR_ENFC_CLK_PODF(0) |
			MXC_CCM_CS2CDR_ENFC_CLK_PRED(3) |
			MXC_CCM_CS2CDR_ENFC_CLK_SEL(3));

	/* enable ENFC_CLK_ROOT clock */
	setbits_le32(&mxc_ccm->CCGR2, MXC_CCM_CCGR2_IOMUX_IPT_CLK_IO_MASK);

	/* enable gpmi and bch clock gating */
	setbits_le32(&mxc_ccm->CCGR4,
		     MXC_CCM_CCGR4_RAWNAND_U_BCH_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_BCH_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_BCH_INPUT_GPMI_IO_MASK |
		     MXC_CCM_CCGR4_RAWNAND_U_GPMI_INPUT_APB_MASK |
		     MXC_CCM_CCGR4_PL301_MX6QPER1_BCH_OFFSET);

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}

int board_spi_cs_gpio(unsigned int bus, unsigned int cs)
{
	return IMX_GPIO_NR(4, 24);
}

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	setup_gpmi_nand();

	return 0;
}

/*
 * BOOT_CFG1, BOOT_CFG2, BOOT_CFG3, BOOT_CFG4
 * see Table 8-11 and Table 5-9
 *  BOOT_CFG1[7] = 1 (boot from NAND)
 *  BOOT_CFG1[5] = 0 - raw NAND
 *  BOOT_CFG1[4] = 0 - default pad settings
 *  BOOT_CFG1[3:2] = 00 - devices = 1
 *  BOOT_CFG1[1:0] = 00 - Row Address Cycles = 3
 *  BOOT_CFG2[4:3] = 00 - Boot Search Count = 2
 *  BOOT_CFG2[2:1] = 01 - Pages In Block = 64
 *  BOOT_CFG2[0] = 0 - Reset time 12ms
 */
static const struct boot_mode board_boot_modes[] = {
	/* NAND: 64pages per block, 3 row addr cycles, 2 copies of FCB/DBBT */
	{"nand", MAKE_CFGVAL(0x80, 0x02, 0x00, 0x00)},
	{"mmc0",  MAKE_CFGVAL(0x40, 0x20, 0x00, 0x00)},
	{NULL, 0},
};

int board_late_init(void)
{
	add_board_boot_modes(board_boot_modes);

	return 0;
}

#ifdef CONFIG_XPL_BUILD
#include <spl.h>

#define MX6_PHYFLEX_ERR006282	IMX_GPIO_NR(2, 11)
static void phyflex_err006282_workaround(void)
{
	/*
	 * Boards beginning with 1362.2 have the SD4_DAT3 pin connected
	 * to the CMIC. If this pin isn't toggled within 10s the boards
	 * reset. The pin is unconnected on older boards, so we do not
	 * need a check for older boards before applying this fixup.
	 */

	gpio_request(MX6_PHYFLEX_ERR006282, "errata_gpio");
	gpio_direction_output(MX6_PHYFLEX_ERR006282, 0);
	mdelay(2);
	gpio_direction_output(MX6_PHYFLEX_ERR006282, 1);
	mdelay(2);
	gpio_set_value(MX6_PHYFLEX_ERR006282, 0);

	imx_iomux_v3_setup_pad(MX6_PAD_SD4_DAT3__GPIO2_IO11);

	gpio_direction_input(MX6_PHYFLEX_ERR006282);
}

static const iomux_v3_cfg_t gpios_pads[] = {
	MX6_PAD_SD4_DAT3__GPIO2_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_DAT4__GPIO2_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_DAT5__GPIO2_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_DAT6__GPIO2_IO14 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_SD4_DAT7__GPIO2_IO15 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_EIM_EB3__GPIO2_IO31 | MUX_PAD_CTRL(NO_PAD_CTRL),
	MX6_PAD_ENET_TXD0__GPIO1_IO30 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_gpios(void)
{
	imx_iomux_v3_setup_multiple_pads(gpios_pads, ARRAY_SIZE(gpios_pads));
}

static const struct mx6dq_iomux_ddr_regs mx6_ddr_ioregs = {
	.dram_sdclk_0 = 0x00000030,
	.dram_sdclk_1 = 0x00000030,
	.dram_cas = 0x00000030,
	.dram_ras = 0x00000030,
	.dram_reset = 0x00000030,
	.dram_sdcke0 = 0x00003000,
	.dram_sdcke1 = 0x00003000,
	.dram_sdba2 = 0x00000030,
	.dram_sdodt0 = 0x00000030,
	.dram_sdodt1 = 0x00000030,

	.dram_sdqs0 = 0x00000028,
	.dram_sdqs1 = 0x00000028,
	.dram_sdqs2 = 0x00000028,
	.dram_sdqs3 = 0x00000028,
	.dram_sdqs4 = 0x00000028,
	.dram_sdqs5 = 0x00000028,
	.dram_sdqs6 = 0x00000028,
	.dram_sdqs7 = 0x00000028,
	.dram_dqm0 = 0x00000028,
	.dram_dqm1 = 0x00000028,
	.dram_dqm2 = 0x00000028,
	.dram_dqm3 = 0x00000028,
	.dram_dqm4 = 0x00000028,
	.dram_dqm5 = 0x00000028,
	.dram_dqm6 = 0x00000028,
	.dram_dqm7 = 0x00000028,
};

static const struct mx6dq_iomux_grp_regs mx6_grp_ioregs = {
	.grp_ddr_type =  0x000C0000,
	.grp_ddrmode_ctl =  0x00020000,
	.grp_ddrpke =  0x00000000,
	.grp_addds = 0x30,
	.grp_ctlds = 0x30,
	.grp_ddrmode =  0x00020000,
	.grp_b0ds = 0x00000028,
	.grp_b1ds = 0x00000028,
	.grp_b2ds = 0x00000028,
	.grp_b3ds = 0x00000028,
	.grp_b4ds = 0x00000028,
	.grp_b5ds = 0x00000028,
	.grp_b6ds = 0x00000028,
	.grp_b7ds = 0x00000028,
};

static const struct mx6_mmdc_calibration mx6_mmcd_calib = {
	.p0_mpwldectrl0 =  0x00170018,
	.p0_mpwldectrl1 =  0x003B0039,
	.p1_mpwldectrl0 =  0x00350048,
	.p1_mpwldectrl1 =  0x00410052,
	.p0_mpdgctrl0 =  0x03600374,
	.p0_mpdgctrl1 =  0x03680360,
	.p1_mpdgctrl0 =  0x0370037C,
	.p1_mpdgctrl1 =  0x03700350,
	.p0_mprddlctl =  0x3A363234,
	.p1_mprddlctl =  0x3634363C,
	.p0_mpwrdlctl =  0x38383E3C,
	.p1_mpwrdlctl =  0x422A483C,
};

/* MT41K64M16JT-125 (1Gb density) */
static struct mx6_ddr3_cfg mem_ddr = {
	.mem_speed = 1600,
	.density = 1,
	.width = 16,
	.banks = 8,
	.rowaddr = 13,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
	.SRT       = 1,
};

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FC03, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static void spl_dram_init(void)
{
	struct mx6_ddr_sysinfo sysinfo = {
		.dsize = 2,
		.cs_density = 6,
		.ncs = 2,
		.cs1_mirror = 1,
		.rtt_wr = 1,
		.rtt_nom = 1,
		.walat = 1,
		.ralat = 5,
		.mif3_mode = 3,
		.bi_on = 1,
		.sde_to_rst = 0x10,
		.rst_to_cke = 0x23,
		.ddr_type = DDR_TYPE_DDR3,
		.refsel = 1,
		.refr = 7,
	};

	mx6dq_dram_iocfg(64, &mx6_ddr_ioregs, &mx6_grp_ioregs);
	mx6_dram_cfg(&sysinfo, &mx6_mmcd_calib, &mem_ddr);
}

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

struct fsl_esdhc_cfg usdhc_cfg[1] = {
	{USDHC3_BASE_ADDR},
};

static const iomux_v3_cfg_t usdhc3_pads[] = {
	MX6_PAD_SD3_CLK__SD3_CLK	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_CMD__SD3_CMD	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT0__SD3_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT1__SD3_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT2__SD3_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD3_DAT3__SD3_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

int board_mmc_init(struct bd_info *bis)
{
	imx_iomux_v3_setup_multiple_pads(usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
	usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
	usdhc_cfg[0].max_bus_width = 4;
	gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;

	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();

	switch (spl_boot_list[0]) {
	case BOOT_DEVICE_SPI:
		spl_boot_list[1] = BOOT_DEVICE_UART;
		break;
	case BOOT_DEVICE_MMC1:
		spl_boot_list[1] = BOOT_DEVICE_SPI;
		spl_boot_list[2] = BOOT_DEVICE_UART;
		break;
	default:
		printf("Boot device %x\n", spl_boot_list[0]);
	}
}

static const iomux_v3_cfg_t ecspi3_pads[] = {
	MX6_PAD_DISP0_DAT0__ECSPI3_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT1__ECSPI3_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT2__ECSPI3_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_DISP0_DAT3__GPIO4_IO24 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static void setup_spi(void)
{
	imx_iomux_v3_setup_multiple_pads(ecspi3_pads, ARRAY_SIZE(ecspi3_pads));

	enable_spi_clk(true, 2);
}

void board_init_f(ulong dummy)
{
	/* setup clock gating */
	ccgr_init();

	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	/* setup AXI */
	gpr_init();

	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	setup_spi();

	setup_gpios();

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	phyflex_err006282_workaround();

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
#endif
