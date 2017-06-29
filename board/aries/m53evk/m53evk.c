/*
 * Aries M53 module
 *
 * Copyright (C) 2012-2013 Marek Vasut <marex@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx53.h>
#include <asm/mach-imx/mx5_video.h>
#include <asm/spl.h>
#include <linux/errno.h>
#include <netdev.h>
#include <i2c.h>
#include <mmc.h>
#include <spl.h>
#include <fsl_esdhc.h>
#include <asm/gpio.h>
#include <usb/ehci-ci.h>
#include <linux/fb.h>
#include <ipu_pixfmt.h>

/* Special MXCFB sync flags are here. */
#include "../drivers/video/mxcfb.h"

DECLARE_GLOBAL_DATA_PTR;

static uint32_t mx53_dram_size[2];

phys_size_t get_effective_memsize(void)
{
	/*
	 * WARNING: We must override get_effective_memsize() function here
	 * to report only the size of the first DRAM bank. This is to make
	 * U-Boot relocator place U-Boot into valid memory, that is, at the
	 * end of the first DRAM bank. If we did not override this function
	 * like so, U-Boot would be placed at the address of the first DRAM
	 * bank + total DRAM size - sizeof(uboot), which in the setup where
	 * each DRAM bank contains 512MiB of DRAM would result in placing
	 * U-Boot into invalid memory area close to the end of the first
	 * DRAM bank.
	 */
	return mx53_dram_size[0];
}

int dram_init(void)
{
	mx53_dram_size[0] = get_ram_size((void *)PHYS_SDRAM_1, 1 << 30);
	mx53_dram_size[1] = get_ram_size((void *)PHYS_SDRAM_2, 1 << 30);

	gd->ram_size = mx53_dram_size[0] + mx53_dram_size[1];

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = mx53_dram_size[0];

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = mx53_dram_size[1];

	return 0;
}

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		MX53_PAD_PATA_BUFFER_EN__UART2_RXD_MUX,
		MX53_PAD_PATA_DMARQ__UART2_TXD_MUX,
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

#ifdef CONFIG_USB_EHCI_MX5
int board_ehci_hcd_init(int port)
{
	if (port == 0) {
		/* USB OTG PWRON */
		imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX53_PAD_GPIO_4__GPIO1_4,
					PAD_CTL_PKE | PAD_CTL_DSE_HIGH));
		gpio_direction_output(IMX_GPIO_NR(1, 4), 0);

		/* USB OTG Over Current */
		imx_iomux_v3_setup_pad(MX53_PAD_GPIO_18__GPIO7_13);
	} else if (port == 1) {
		/* USB Host PWRON */
		imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX53_PAD_GPIO_2__GPIO1_2,
					PAD_CTL_PKE | PAD_CTL_DSE_HIGH));
		gpio_direction_output(IMX_GPIO_NR(1, 2), 0);

		/* USB Host Over Current */
		imx_iomux_v3_setup_pad(MX53_PAD_GPIO_3__USBOH3_USBH1_OC);
	}

	return 0;
}
#endif

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		/* MDIO pads */
		NEW_PAD_CTRL(MX53_PAD_FEC_MDIO__FEC_MDIO, PAD_CTL_HYS |
			PAD_CTL_DSE_HIGH | PAD_CTL_PUS_22K_UP | PAD_CTL_ODE),
		NEW_PAD_CTRL(MX53_PAD_FEC_MDC__FEC_MDC, PAD_CTL_DSE_HIGH),

		/* FEC 0 pads */
		NEW_PAD_CTRL(MX53_PAD_FEC_CRS_DV__FEC_RX_DV,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_REF_CLK__FEC_TX_CLK,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RX_ER__FEC_RX_ER,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_TX_EN__FEC_TX_EN, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD0__FEC_RDATA_0,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD1__FEC_RDATA_1,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD0__FEC_TDATA_0, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD1__FEC_TDATA_1, PAD_CTL_DSE_HIGH),

		/* FEC 1 pads */
		NEW_PAD_CTRL(MX53_PAD_KEY_COL0__FEC_RDATA_3,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_ROW0__FEC_TX_ER,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL1__FEC_RX_CLK,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_ROW1__FEC_COL,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL2__FEC_RDATA_2,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_KEY_ROW2__FEC_TDATA_2, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL3__FEC_CRS,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_GPIO_19__FEC_TDATA_3, PAD_CTL_DSE_HIGH),
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg = {
	MMC_SDHC1_BASE_ADDR,
};

int board_mmc_getcd(struct mmc *mmc)
{
	imx_iomux_v3_setup_pad(MX53_PAD_GPIO_1__GPIO1_1);
	gpio_direction_input(IMX_GPIO_NR(1, 1));

	return !gpio_get_value(IMX_GPIO_NR(1, 1));
}

#define SD_CMD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
				 PAD_CTL_PUS_100K_UP)
#define SD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | \
				 PAD_CTL_DSE_HIGH)

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_SD1_CMD__ESDHC1_CMD, SD_CMD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_CLK__ESDHC1_CLK, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA0__ESDHC1_DAT0, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA1__ESDHC1_DAT1, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA2__ESDHC1_DAT2, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA3__ESDHC1_DAT3, SD_PAD_CTRL),
		MX53_PAD_EIM_DA13__GPIO3_13,

		MX53_PAD_EIM_EB3__GPIO2_31, /* SD power */
	};

	esdhc_cfg.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	imx_iomux_v3_setup_multiple_pads(sd1_pads, ARRAY_SIZE(sd1_pads));

	/* GPIO 2_31 is SD power */
	gpio_direction_output(IMX_GPIO_NR(2, 31), 0);

	return fsl_esdhc_initialize(bis, &esdhc_cfg);
}
#endif

#ifdef CONFIG_VIDEO
static struct fb_videomode const ampire_wvga = {
	.name		= "Ampire",
	.refresh	= 60,
	.xres		= 800,
	.yres		= 480,
	.pixclock	= 29851, /* picosecond (33.5 MHz) */
	.left_margin	= 89,
	.right_margin	= 164,
	.upper_margin	= 23,
	.lower_margin	= 10,
	.hsync_len	= 10,
	.vsync_len	= 10,
	.sync		= FB_SYNC_CLK_LAT_FALL,
};

int board_video_skip(void)
{
	int ret;
	ret = ipuv3_fb_init(&ampire_wvga, 1, IPU_PIX_FMT_RGB666);
	if (ret)
		printf("Ampire LCD cannot be configured: %d\n", ret);
	return ret;
}
#endif

#define I2C_PAD_CTRL	(PAD_CTL_SRE_FAST | PAD_CTL_DSE_HIGH | \
			 PAD_CTL_PUS_100K_UP | PAD_CTL_ODE)

static void setup_iomux_i2c(void)
{
	static const iomux_v3_cfg_t i2c_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_EIM_D16__I2C2_SDA, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_EIM_EB2__I2C2_SCL, I2C_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(i2c_pads, ARRAY_SIZE(i2c_pads));
}

static void setup_iomux_video(void)
{
	static const iomux_v3_cfg_t lcd_pads[] = {
		MX53_PAD_EIM_DA9__IPU_DISP1_DAT_0,
		MX53_PAD_EIM_DA8__IPU_DISP1_DAT_1,
		MX53_PAD_EIM_DA7__IPU_DISP1_DAT_2,
		MX53_PAD_EIM_DA6__IPU_DISP1_DAT_3,
		MX53_PAD_EIM_DA5__IPU_DISP1_DAT_4,
		MX53_PAD_EIM_DA4__IPU_DISP1_DAT_5,
		MX53_PAD_EIM_DA3__IPU_DISP1_DAT_6,
		MX53_PAD_EIM_DA2__IPU_DISP1_DAT_7,
		MX53_PAD_EIM_DA1__IPU_DISP1_DAT_8,
		MX53_PAD_EIM_DA0__IPU_DISP1_DAT_9,
		MX53_PAD_EIM_EB1__IPU_DISP1_DAT_10,
		MX53_PAD_EIM_EB0__IPU_DISP1_DAT_11,
		MX53_PAD_EIM_A17__IPU_DISP1_DAT_12,
		MX53_PAD_EIM_A18__IPU_DISP1_DAT_13,
		MX53_PAD_EIM_A19__IPU_DISP1_DAT_14,
		MX53_PAD_EIM_A20__IPU_DISP1_DAT_15,
		MX53_PAD_EIM_A21__IPU_DISP1_DAT_16,
		MX53_PAD_EIM_A22__IPU_DISP1_DAT_17,
		MX53_PAD_EIM_A23__IPU_DISP1_DAT_18,
		MX53_PAD_EIM_A24__IPU_DISP1_DAT_19,
		MX53_PAD_EIM_D31__IPU_DISP1_DAT_20,
		MX53_PAD_EIM_D30__IPU_DISP1_DAT_21,
		MX53_PAD_EIM_D26__IPU_DISP1_DAT_22,
		MX53_PAD_EIM_D27__IPU_DISP1_DAT_23,
		MX53_PAD_EIM_A16__IPU_DI1_DISP_CLK,
		MX53_PAD_EIM_DA13__IPU_DI1_D0_CS,
		MX53_PAD_EIM_DA14__IPU_DI1_D1_CS,
		MX53_PAD_EIM_DA15__IPU_DI1_PIN1,
		MX53_PAD_EIM_DA11__IPU_DI1_PIN2,
		MX53_PAD_EIM_DA12__IPU_DI1_PIN3,
		MX53_PAD_EIM_A25__IPU_DI1_PIN12,
		MX53_PAD_EIM_DA10__IPU_DI1_PIN15,
	};

	imx_iomux_v3_setup_multiple_pads(lcd_pads, ARRAY_SIZE(lcd_pads));
}

static void setup_iomux_nand(void)
{
	static const iomux_v3_cfg_t nand_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_NANDF_WE_B__EMI_NANDF_WE_B,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_RE_B__EMI_NANDF_RE_B,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_CLE__EMI_NANDF_CLE,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_ALE__EMI_NANDF_ALE,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_WP_B__EMI_NANDF_WP_B,
				PAD_CTL_PUS_100K_UP),
		NEW_PAD_CTRL(MX53_PAD_NANDF_RB0__EMI_NANDF_RB_0,
				PAD_CTL_PUS_100K_UP),
		NEW_PAD_CTRL(MX53_PAD_NANDF_CS0__EMI_NANDF_CS_0,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA0__EMI_NANDF_D_0,
				PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA1__EMI_NANDF_D_1,
				PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA2__EMI_NANDF_D_2,
				PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA3__EMI_NANDF_D_3,
				PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA4__EMI_NANDF_D_4,
				PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA5__EMI_NANDF_D_5,
				PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA6__EMI_NANDF_D_6,
				PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA7__EMI_NANDF_D_7,
				PAD_CTL_DSE_HIGH | PAD_CTL_PKE),
	};

	imx_iomux_v3_setup_multiple_pads(nand_pads, ARRAY_SIZE(nand_pads));
}

static void m53_set_clock(void)
{
	int ret;
	const uint32_t ref_clk = MXC_HCLK;
	const uint32_t dramclk = 400;
	uint32_t cpuclk;

	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX53_PAD_GPIO_10__GPIO4_0,
					    PAD_CTL_DSE_HIGH | PAD_CTL_PKE));
	gpio_direction_input(IMX_GPIO_NR(4, 0));

	/* GPIO10 selects modules' CPU speed, 1 = 1200MHz ; 0 = 800MHz */
	cpuclk = gpio_get_value(IMX_GPIO_NR(4, 0)) ? 1200 : 800;

	ret = mxc_set_clock(ref_clk, cpuclk, MXC_ARM_CLK);
	if (ret)
		printf("CPU:   Switch CPU clock to %dMHz failed\n", cpuclk);

	ret = mxc_set_clock(ref_clk, dramclk, MXC_PERIPH_CLK);
	if (ret) {
		printf("CPU:   Switch peripheral clock to %dMHz failed\n",
			dramclk);
	}

	ret = mxc_set_clock(ref_clk, dramclk, MXC_DDR_CLK);
	if (ret)
		printf("CPU:   Switch DDR clock to %dMHz failed\n", dramclk);
}

static void m53_set_nand(void)
{
	u32 i;

	/* NAND flash is muxed on ATA pins */
	setbits_le32(M4IF_BASE_ADDR + 0xc, M4IF_GENP_WEIM_MM_MASK);

	/* Wait for Grant/Ack sequence (see EIM_CSnGCR2:MUX16_BYP_GRANT) */
	for (i = 0x4; i < 0x94; i += 0x18) {
		clrbits_le32(WEIM_BASE_ADDR + i,
			     WEIM_GCR2_MUX16_BYP_GRANT_MASK);
	}

	mxc_set_clock(0, 33, MXC_NFC_CLK);
	enable_nfc_clk(1);
}

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_fec();
	setup_iomux_i2c();
	setup_iomux_nand();
	setup_iomux_video();

	m53_set_clock();

	mxc_set_sata_internal_clock();

	/* NAND clock @ 33MHz */
	m53_set_nand();

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: Aries M53EVK\n");

	return 0;
}

/*
 * NAND SPL
 */
#ifdef CONFIG_SPL_BUILD
void spl_board_init(void)
{
	setup_iomux_nand();
	m53_set_clock();
	m53_set_nand();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NAND;
}
#endif
