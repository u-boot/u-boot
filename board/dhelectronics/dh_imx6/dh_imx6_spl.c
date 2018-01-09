/*
 * DHCOM DH-iMX6 PDK SPL support
 *
 * Copyright (C) 2017 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-ddr.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/io.h>
#include <errno.h>
#include <fuse.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <mmc.h>
#include <spl.h>

#define ENET_PAD_CTRL							\
	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	 PAD_CTL_HYS)

#define GPIO_PAD_CTRL							\
	(PAD_CTL_HYS | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm)

#define SPI_PAD_CTRL							\
	(PAD_CTL_HYS | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |		\
	PAD_CTL_SRE_FAST)

#define UART_PAD_CTRL							\
	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
	 PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL							\
	(PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |	\
	 PAD_CTL_SRE_FAST | PAD_CTL_HYS)

DECLARE_GLOBAL_DATA_PTR;

static const struct mx6dq_iomux_ddr_regs dhcom6dq_ddr_ioregs = {
	.dram_sdclk_0	= 0x00020030,
	.dram_sdclk_1	= 0x00020030,
	.dram_cas	= 0x00020030,
	.dram_ras	= 0x00020030,
	.dram_reset	= 0x00020030,
	.dram_sdcke0	= 0x00003000,
	.dram_sdcke1	= 0x00003000,
	.dram_sdba2	= 0x00000000,
	.dram_sdodt0	= 0x00003030,
	.dram_sdodt1	= 0x00003030,
	.dram_sdqs0	= 0x00000030,
	.dram_sdqs1	= 0x00000030,
	.dram_sdqs2	= 0x00000030,
	.dram_sdqs3	= 0x00000030,
	.dram_sdqs4	= 0x00000030,
	.dram_sdqs5	= 0x00000030,
	.dram_sdqs6	= 0x00000030,
	.dram_sdqs7	= 0x00000030,
	.dram_dqm0	= 0x00020030,
	.dram_dqm1	= 0x00020030,
	.dram_dqm2	= 0x00020030,
	.dram_dqm3	= 0x00020030,
	.dram_dqm4	= 0x00020030,
	.dram_dqm5	= 0x00020030,
	.dram_dqm6	= 0x00020030,
	.dram_dqm7	= 0x00020030,
};

static const struct mx6dq_iomux_grp_regs dhcom6dq_grp_ioregs = {
	.grp_ddr_type	= 0x000C0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke	= 0x00000000,
	.grp_addds	= 0x00000030,
	.grp_ctlds	= 0x00000030,
	.grp_ddrmode	= 0x00020000,
	.grp_b0ds	= 0x00000030,
	.grp_b1ds	= 0x00000030,
	.grp_b2ds	= 0x00000030,
	.grp_b3ds	= 0x00000030,
	.grp_b4ds	= 0x00000030,
	.grp_b5ds	= 0x00000030,
	.grp_b6ds	= 0x00000030,
	.grp_b7ds	= 0x00000030,
};

static const struct mx6sdl_iomux_ddr_regs dhcom6sdl_ddr_ioregs = {
	.dram_sdclk_0	= 0x00020030,
	.dram_sdclk_1	= 0x00020030,
	.dram_cas	= 0x00020030,
	.dram_ras	= 0x00020030,
	.dram_reset	= 0x00020030,
	.dram_sdcke0	= 0x00003000,
	.dram_sdcke1	= 0x00003000,
	.dram_sdba2	= 0x00000000,
	.dram_sdodt0	= 0x00003030,
	.dram_sdodt1	= 0x00003030,
	.dram_sdqs0	= 0x00000030,
	.dram_sdqs1	= 0x00000030,
	.dram_sdqs2	= 0x00000030,
	.dram_sdqs3	= 0x00000030,
	.dram_sdqs4	= 0x00000030,
	.dram_sdqs5	= 0x00000030,
	.dram_sdqs6	= 0x00000030,
	.dram_sdqs7	= 0x00000030,
	.dram_dqm0	= 0x00020030,
	.dram_dqm1	= 0x00020030,
	.dram_dqm2	= 0x00020030,
	.dram_dqm3	= 0x00020030,
	.dram_dqm4	= 0x00020030,
	.dram_dqm5	= 0x00020030,
	.dram_dqm6	= 0x00020030,
	.dram_dqm7	= 0x00020030,
};

static const struct mx6sdl_iomux_grp_regs dhcom6sdl_grp_ioregs = {
	.grp_ddr_type	= 0x000C0000,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_ddrpke	= 0x00000000,
	.grp_addds	= 0x00000030,
	.grp_ctlds	= 0x00000030,
	.grp_ddrmode	= 0x00020000,
	.grp_b0ds	= 0x00000030,
	.grp_b1ds	= 0x00000030,
	.grp_b2ds	= 0x00000030,
	.grp_b3ds	= 0x00000030,
	.grp_b4ds	= 0x00000030,
	.grp_b5ds	= 0x00000030,
	.grp_b6ds	= 0x00000030,
	.grp_b7ds	= 0x00000030,
};

static const struct mx6_mmdc_calibration dhcom_mmdc_calib = {
	.p0_mpwldectrl0	= 0x0011000E,
	.p0_mpwldectrl1	= 0x000E001B,
	.p1_mpwldectrl0	= 0x00190015,
	.p1_mpwldectrl1	= 0x00070018,
	.p0_mpdgctrl0	= 0x42720306,
	.p0_mpdgctrl1	= 0x026F0266,
	.p1_mpdgctrl0	= 0x4273030A,
	.p1_mpdgctrl1	= 0x02740240,
	.p0_mprddlctl	= 0x45393B3E,
	.p1_mprddlctl	= 0x403A3747,
	.p0_mpwrdlctl	= 0x40434541,
	.p1_mpwrdlctl	= 0x473E4A3B,
};

static const struct mx6_ddr3_cfg dhcom_mem_ddr = {
	.mem_speed	= 1600,
	.density	= 2,
	.width		= 64,
	.banks		= 8,
	.rowaddr	= 14,
	.coladdr	= 10,
	.pagesz		= 2,
	.trcd		= 1312,
	.trcmin		= 5863,
	.trasmin	= 3750,
};

static const struct mx6_ddr_sysinfo dhcom_ddr_info = {
	/* width of data bus:0=16,1=32,2=64 */
	.dsize		= 2,
	.cs_density	= 16,
	.ncs		= 1,	/* single chip select */
	.cs1_mirror	= 1,
	.rtt_wr		= 1,	/* DDR3_RTT_60_OHM, RTT_Wr = RZQ/4 */
	.rtt_nom	= 1,	/* DDR3_RTT_60_OHM, RTT_Nom = RZQ/4 */
	.walat		= 1,	/* Write additional latency */
	.ralat		= 5,	/* Read additional latency */
	.mif3_mode	= 3,	/* Command prediction working mode */
	.bi_on		= 1,	/* Bank interleaving enabled */
	.sde_to_rst	= 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke	= 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.refsel		= 1,	/* Refresh cycles at 32KHz */
	.refr		= 3,	/* 4 refresh commands per refresh cycle */
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

/* Board ID */
static iomux_v3_cfg_t const hwcode_pads[] = {
	IOMUX_PADS(PAD_EIM_A19__GPIO2_IO19	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A23__GPIO6_IO06	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_A22__GPIO2_IO16	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
};

static void setup_iomux_boardid(void)
{
	/* HW code pins: Setup alternate function and configure pads */
	SETUP_IOMUX_PADS(hwcode_pads);
}

/* GPIO */
static iomux_v3_cfg_t const gpio_pads[] = {
	IOMUX_PADS(PAD_GPIO_2__GPIO1_IO02	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_4__GPIO1_IO04	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_5__GPIO1_IO05	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT17__GPIO6_IO03	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_19__GPIO4_IO05	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_DI0_PIN4__GPIO4_IO20	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D27__GPIO3_IO27	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_ROW0__GPIO4_IO07	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_COL1__GPIO4_IO08	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS1__GPIO6_IO14	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS2__GPIO6_IO15	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_KEY_ROW1__GPIO4_IO09	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__GPIO7_IO00	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__GPIO7_IO01	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_VSYNC__GPIO5_IO21	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_18__GPIO7_IO13	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_CMD__GPIO1_IO18	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT1__GPIO1_IO17	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_PIXCLK__GPIO5_IO18	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_MCLK__GPIO5_IO19	| MUX_PAD_CTRL(GPIO_PAD_CTRL)),
};

static void setup_iomux_gpio(void)
{
	SETUP_IOMUX_PADS(gpio_pads);
}

/* Ethernet */
static iomux_v3_cfg_t const enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TX_EN__ENET_TX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TXD0__ENET_TX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_TXD1__ENET_TX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_GPIO_16__ENET_REF_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RX_ER__ENET_RX_ER	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RXD0__ENET_RX_DATA0	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RXD1__ENET_RX_DATA1	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_CRS_DV__ENET_RX_EN	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	/* SMSC PHY Reset */
	IOMUX_PADS(PAD_EIM_WAIT__GPIO5_IO00	| MUX_PAD_CTRL(NO_PAD_CTRL)),
	/* ENET_VIO_GPIO */
	IOMUX_PADS(PAD_GPIO_7__GPIO1_IO07	| MUX_PAD_CTRL(NO_PAD_CTRL)),
	/* ENET_Interrupt - (not used) */
	IOMUX_PADS(PAD_RGMII_RD0__GPIO6_IO25	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void setup_iomux_enet(void)
{
	SETUP_IOMUX_PADS(enet_pads);
}

/* SD interface */
static iomux_v3_cfg_t const usdhc2_pads[] = {
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_NANDF_CS3__GPIO6_IO16	| MUX_PAD_CTRL(NO_PAD_CTRL)), /* CD */
};

/* onboard microSD */
static iomux_v3_cfg_t const usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_RST__GPIO7_IO08	| MUX_PAD_CTRL(NO_PAD_CTRL)), /* CD */
};

/* eMMC */
static iomux_v3_cfg_t const usdhc4_pads[] = {
	IOMUX_PADS(PAD_SD4_DAT0__SD4_DATA0	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT1__SD4_DATA1	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT2__SD4_DATA2	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT3__SD4_DATA3	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT4__SD4_DATA4	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT5__SD4_DATA5	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT6__SD4_DATA6	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT7__SD4_DATA7	| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CLK__SD4_CLK		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CMD__SD4_CMD		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};

/* SD */
static void setup_iomux_sd(void)
{
	SETUP_IOMUX_PADS(usdhc2_pads);
	SETUP_IOMUX_PADS(usdhc3_pads);
	SETUP_IOMUX_PADS(usdhc4_pads);
}

/* SPI */
static iomux_v3_cfg_t const ecspi1_pads[] = {
	/* SS0 */
	IOMUX_PADS(PAD_EIM_EB2__GPIO2_IO30	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D17__ECSPI1_MISO	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D18__ECSPI1_MOSI	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D16__ECSPI1_SCLK	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
};

static void setup_iomux_spi(void)
{
	SETUP_IOMUX_PADS(ecspi1_pads);
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if (bus == 0 && cs == 0)
		return IMX_GPIO_NR(2, 30);
	else
		return -1;
}

/* UART */
static iomux_v3_cfg_t const uart1_pads[] = {
	IOMUX_PADS(PAD_SD3_DAT7__UART1_TX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__UART1_RX_DATA	| MUX_PAD_CTRL(UART_PAD_CTRL)),
};

static void setup_iomux_uart(void)
{
	SETUP_IOMUX_PADS(uart1_pads);
}

/* USB */
static iomux_v3_cfg_t const usb_pads[] = {
	IOMUX_PADS(PAD_GPIO_1__USB_OTG_ID	| MUX_PAD_CTRL(NO_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D31__GPIO3_IO31	| MUX_PAD_CTRL(NO_PAD_CTRL)),
};

static void setup_iomux_usb(void)
{
	SETUP_IOMUX_PADS(usb_pads);
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* setup GP timer */
	timer_init();

	setup_iomux_boardid();
	setup_iomux_gpio();
	setup_iomux_enet();
	setup_iomux_sd();
	setup_iomux_spi();
	setup_iomux_uart();
	setup_iomux_usb();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	/* Start the DDR DRAM */
	if (is_mx6dq())
		mx6dq_dram_iocfg(dhcom_mem_ddr.width, &dhcom6dq_ddr_ioregs,
				 &dhcom6dq_grp_ioregs);
	else
		mx6sdl_dram_iocfg(dhcom_mem_ddr.width, &dhcom6sdl_ddr_ioregs,
				  &dhcom6sdl_grp_ioregs);
	mx6_dram_cfg(&dhcom_ddr_info, &dhcom_mmdc_calib, &dhcom_mem_ddr);

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}
