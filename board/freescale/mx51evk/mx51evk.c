/*
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
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
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx51_pins.h>
#include <asm/arch/iomux.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include "mx51evk.h"

DECLARE_GLOBAL_DATA_PTR;

static u32 system_rev;
struct io_board_ctrl *mx51_io_board;

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR, 1, 1},
	{MMC_SDHC2_BASE_ADDR, 1, 1},
};
#endif

u32 get_board_rev(void)
{
	return system_rev;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = get_ram_size((long *)PHYS_SDRAM_1,
			PHYS_SDRAM_1_SIZE);
	return 0;
}

static void setup_iomux_uart(void)
{
	unsigned int pad = PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE |
			PAD_CTL_PUE_PULL | PAD_CTL_DRV_HIGH;

	mxc_request_iomux(MX51_PIN_UART1_RXD, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_UART1_RXD, pad | PAD_CTL_SRE_FAST);
	mxc_request_iomux(MX51_PIN_UART1_TXD, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_UART1_TXD, pad | PAD_CTL_SRE_FAST);
	mxc_request_iomux(MX51_PIN_UART1_RTS, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_UART1_RTS, pad);
	mxc_request_iomux(MX51_PIN_UART1_CTS, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_UART1_CTS, pad);
}

static void setup_expio(void)
{
	u32 reg;
	struct weim *pweim = (struct weim *)WEIM_BASE_ADDR;
	struct clkctl *pclkctl = (struct clkctl *)CCM_BASE_ADDR;

	/* CS5 setup */
	mxc_request_iomux(MX51_PIN_EIM_CS5, IOMUX_CONFIG_ALT0);
	writel(0x00410089, &pweim[5].csgcr1);
	writel(0x00000002, &pweim[5].csgcr2);

	/* RWSC=50, RADVA=2, RADVN=6, OEA=0, OEN=0, RCSA=0, RCSN=0 */
	writel(0x32260000, &pweim[5].csrcr1);

	/* APR = 0 */
	writel(0x00000000, &pweim[5].csrcr2);

	/*
	 * WAL=0, WBED=1, WWSC=50, WADVA=2, WADVN=6, WEA=0, WEN=0,
	 * WCSA=0, WCSN=0
	 */
	writel(0x72080F00, &pweim[5].cswcr1);

	mx51_io_board = (struct io_board_ctrl *)(CS5_BASE_ADDR +
						IO_BOARD_OFFSET);
	if ((readw(&mx51_io_board->id1) == 0xAAAA) &&
		(readw(&mx51_io_board->id2) == 0x5555)) {
		if (is_soc_rev(CHIP_REV_2_0) < 0) {
			reg = readl(&pclkctl->cbcdr);
			reg = (reg & (~0x70000)) | 0x30000;
			writel(reg, &pclkctl->cbcdr);
			/* make sure divider effective */
			while (readl(&pclkctl->cdhipr) != 0)
				;
			writel(0x0, &pclkctl->ccdr);
		}
	} else {
		/* CS1 */
		writel(0x00410089, &pweim[1].csgcr1);
		writel(0x00000002, &pweim[1].csgcr2);
		/*  RWSC=50, RADVA=2, RADVN=6, OEA=0, OEN=0, RCSA=0, RCSN=0 */
		writel(0x32260000, &pweim[1].csrcr1);
		/* APR=0 */
		writel(0x00000000, &pweim[1].csrcr2);
		/*
		 * WAL=0, WBED=1, WWSC=50, WADVA=2, WADVN=6, WEA=0,
		 * WEN=0, WCSA=0, WCSN=0
		 */
		writel(0x72080F00, &pweim[1].cswcr1);
		mx51_io_board = (struct io_board_ctrl *)(CS1_BASE_ADDR +
						IO_BOARD_OFFSET);
	}

	/* Reset interrupt status reg */
	writew(0x1F, &(mx51_io_board->int_rest));
	writew(0x00, &(mx51_io_board->int_rest));
	writew(0xFFFF, &(mx51_io_board->int_mask));

	/* Reset the XUART and Ethernet controllers */
	reg = readw(&(mx51_io_board->sw_reset));
	reg |= 0x9;
	writew(reg, &(mx51_io_board->sw_reset));
	reg &= ~0x9;
	writew(reg, &(mx51_io_board->sw_reset));
}

static void setup_iomux_fec(void)
{
	/*FEC_MDIO*/
	mxc_request_iomux(MX51_PIN_EIM_EB2 , IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_EB2 , 0x1FD);

	/*FEC_MDC*/
	mxc_request_iomux(MX51_PIN_NANDF_CS3, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS3, 0x2004);

	/* FEC RDATA[3] */
	mxc_request_iomux(MX51_PIN_EIM_CS3, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_CS3, 0x180);

	/* FEC RDATA[2] */
	mxc_request_iomux(MX51_PIN_EIM_CS2, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_CS2, 0x180);

	/* FEC RDATA[1] */
	mxc_request_iomux(MX51_PIN_EIM_EB3, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_EB3, 0x180);

	/* FEC RDATA[0] */
	mxc_request_iomux(MX51_PIN_NANDF_D9, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_D9, 0x2180);

	/* FEC TDATA[3] */
	mxc_request_iomux(MX51_PIN_NANDF_CS6, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS6, 0x2004);

	/* FEC TDATA[2] */
	mxc_request_iomux(MX51_PIN_NANDF_CS5, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS5, 0x2004);

	/* FEC TDATA[1] */
	mxc_request_iomux(MX51_PIN_NANDF_CS4, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS4, 0x2004);

	/* FEC TDATA[0] */
	mxc_request_iomux(MX51_PIN_NANDF_D8, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_D8, 0x2004);

	/* FEC TX_EN */
	mxc_request_iomux(MX51_PIN_NANDF_CS7, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS7, 0x2004);

	/* FEC TX_ER */
	mxc_request_iomux(MX51_PIN_NANDF_CS2, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_CS2, 0x2004);

	/* FEC TX_CLK */
	mxc_request_iomux(MX51_PIN_NANDF_RDY_INT, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_NANDF_RDY_INT, 0x2180);

	/* FEC TX_COL */
	mxc_request_iomux(MX51_PIN_NANDF_RB2, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_NANDF_RB2, 0x2180);

	/* FEC RX_CLK */
	mxc_request_iomux(MX51_PIN_NANDF_RB3, IOMUX_CONFIG_ALT1);
	mxc_iomux_set_pad(MX51_PIN_NANDF_RB3, 0x2180);

	/* FEC RX_CRS */
	mxc_request_iomux(MX51_PIN_EIM_CS5, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_CS5, 0x180);

	/* FEC RX_ER */
	mxc_request_iomux(MX51_PIN_EIM_CS4, IOMUX_CONFIG_ALT3);
	mxc_iomux_set_pad(MX51_PIN_EIM_CS4, 0x180);

	/* FEC RX_DV */
	mxc_request_iomux(MX51_PIN_NANDF_D11, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_NANDF_D11, 0x2180);
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_getcd(u8 *cd, struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		*cd = readl(GPIO1_BASE_ADDR) & 0x01;
	else
		*cd = readl(GPIO1_BASE_ADDR) & 0x40;

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	u32 index;
	s32 status = 0;

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM;
			index++) {
		switch (index) {
		case 0:
			mxc_request_iomux(MX51_PIN_SD1_CMD,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_CLK,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_DATA0,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_DATA1,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_DATA2,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD1_DATA3,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_SD1_CMD,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_CLK,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_NONE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_DATA0,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_DATA1,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_DATA2,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_47K_PU |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD1_DATA3,
				PAD_CTL_DRV_MAX | PAD_CTL_DRV_VOT_HIGH |
				PAD_CTL_HYS_ENABLE | PAD_CTL_100K_PD |
				PAD_CTL_PUE_PULL |
				PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST);
			mxc_request_iomux(MX51_PIN_GPIO1_0,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_GPIO1_0,
				PAD_CTL_HYS_ENABLE);
			mxc_request_iomux(MX51_PIN_GPIO1_1,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_GPIO1_1,
				PAD_CTL_HYS_ENABLE);
			break;
		case 1:
			mxc_request_iomux(MX51_PIN_SD2_CMD,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD2_CLK,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_SD2_DATA0,
				IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX51_PIN_SD2_DATA1,
				IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX51_PIN_SD2_DATA2,
				IOMUX_CONFIG_ALT0);
			mxc_request_iomux(MX51_PIN_SD2_DATA3,
				IOMUX_CONFIG_ALT0);
			mxc_iomux_set_pad(MX51_PIN_SD2_CMD,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_CLK,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_DATA0,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_DATA1,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_DATA2,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_iomux_set_pad(MX51_PIN_SD2_DATA3,
				PAD_CTL_DRV_MAX | PAD_CTL_22K_PU |
				PAD_CTL_SRE_FAST);
			mxc_request_iomux(MX51_PIN_SD2_CMD,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_request_iomux(MX51_PIN_GPIO1_6,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_GPIO1_6,
				PAD_CTL_HYS_ENABLE);
			mxc_request_iomux(MX51_PIN_GPIO1_5,
				IOMUX_CONFIG_ALT0 | IOMUX_CONFIG_SION);
			mxc_iomux_set_pad(MX51_PIN_GPIO1_5,
				PAD_CTL_HYS_ENABLE);
			break;
		default:
			printf("Warning: you configured more ESDHC controller"
				"(%d) as supported by the board(2)\n",
				CONFIG_SYS_FSL_ESDHC_NUM);
			return status;
		}
		status |= fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
	}
	return status;
}
#endif

int board_init(void)
{
	system_rev = get_cpu_rev();

	gd->bd->bi_arch_number = MACH_TYPE_MX51_BABBAGE;
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	setup_iomux_uart();
	setup_expio();
	setup_iomux_fec();
	return 0;
}

int checkboard(void)
{
	puts("Board: MX51EVK ");

	switch (system_rev & 0xff) {
	case CHIP_REV_3_0:
		puts("3.0 [");
		break;
	case CHIP_REV_2_5:
		puts("2.5 [");
		break;
	case CHIP_REV_2_0:
		puts("2.0 [");
		break;
	case CHIP_REV_1_1:
		puts("1.1 [");
		break;
	case CHIP_REV_1_0:
	default:
		puts("1.0 [");
		break;
	}

	switch (__raw_readl(SRC_BASE_ADDR + 0x8)) {
	case 0x0001:
		puts("POR");
		break;
	case 0x0009:
		puts("RST");
		break;
	case 0x0010:
	case 0x0011:
		puts("WDOG");
		break;
	default:
		puts("unknown");
	}
	puts("]\n");
	return 0;
}
