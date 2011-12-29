/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc.
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
#include <asm/arch/mx6x_pins.h>
#include <asm/arch/iomux-v3.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <mmc.h>
#include <fsl_esdhc.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |            \
       PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
       PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |            \
       PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |               \
       PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

int dram_init(void)
{
       gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

       return 0;
}

iomux_v3_cfg_t uart2_pads[] = {
       MX6Q_PAD_EIM_D26__UART2_TXD | MUX_PAD_CTRL(UART_PAD_CTRL),
       MX6Q_PAD_EIM_D27__UART2_RXD | MUX_PAD_CTRL(UART_PAD_CTRL),
};

iomux_v3_cfg_t usdhc3_pads[] = {
       MX6Q_PAD_SD3_CLK__USDHC3_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_CMD__USDHC3_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT0__USDHC3_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT1__USDHC3_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT2__USDHC3_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT3__USDHC3_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD3_DAT5__GPIO_7_0    | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

iomux_v3_cfg_t usdhc4_pads[] = {
       MX6Q_PAD_SD4_CLK__USDHC4_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_CMD__USDHC4_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_DAT0__USDHC4_DAT0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_DAT1__USDHC4_DAT1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_DAT2__USDHC4_DAT2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_SD4_DAT3__USDHC4_DAT3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
       MX6Q_PAD_NANDF_D6__GPIO_2_6    | MUX_PAD_CTRL(NO_PAD_CTRL), /* CD */
};

static void setup_iomux_uart(void)
{
       imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg usdhc_cfg[2] = {
       {USDHC3_BASE_ADDR, 1},
       {USDHC4_BASE_ADDR, 1},
};

int board_mmc_getcd(struct mmc *mmc)
{
       struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
       int ret;

       if (cfg->esdhc_base == USDHC3_BASE_ADDR) {
               gpio_direction_input(192); /*GPIO7_0*/
               ret = !gpio_get_value(192);
       } else {
               gpio_direction_input(38); /*GPIO2_6*/
               ret = !gpio_get_value(38);
       }

       return ret;
}

int board_mmc_init(bd_t *bis)
{
       s32 status = 0;
       u32 index = 0;

       for (index = 0; index < CONFIG_SYS_FSL_USDHC_NUM; ++index) {
               switch (index) {
               case 0:
                       imx_iomux_v3_setup_multiple_pads(
                               usdhc3_pads, ARRAY_SIZE(usdhc3_pads));
                       break;
               case 1:
                       imx_iomux_v3_setup_multiple_pads(
                               usdhc4_pads, ARRAY_SIZE(usdhc4_pads));
                       break;
               default:
                       printf("Warning: you configured more USDHC controllers"
                               "(%d) then supported by the board (%d)\n",
                               index + 1, CONFIG_SYS_FSL_USDHC_NUM);
                       return status;
               }

               status |= fsl_esdhc_initialize(bis, &usdhc_cfg[index]);
       }

       return status;
}
#endif

int board_early_init_f(void)
{
       setup_iomux_uart();

       return 0;
}

int board_init(void)
{
       /* address of boot parameters */
       gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

       return 0;
}

int checkboard(void)
{
       puts("Board: MX6Q-Sabre Lite\n");

       return 0;
}
