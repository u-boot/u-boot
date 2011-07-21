 /*
 * (C) Copyright 2010
 * Texas Instruments Incorporated, <www.ti.com>
 *
 *	Balaji Krishnamoorthy	<balajitk@ti.com>
 *	Aneesh V		<aneesh@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _OMAP4_MUX_DATA_H_
#define _OMAP4_MUX_DATA_H_

#include <asm/arch/mux_omap4.h>

const struct pad_conf_entry core_padconf_array_essential[] = {

{GPMC_AD0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat0 */
{GPMC_AD1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat1 */
{GPMC_AD2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat2 */
{GPMC_AD3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat3 */
{GPMC_AD4, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat4 */
{GPMC_AD5, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat5 */
{GPMC_AD6, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat6 */
{GPMC_AD7, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_dat7 */
{GPMC_NOE, (PTU | IEN | OFF_EN | OFF_OUT_PTD | M1)},	 /* sdmmc2_clk */
{GPMC_NWE, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M1)}, /* sdmmc2_cmd */
{SDMMC1_CLK, (PTU | OFF_EN | OFF_OUT_PTD | M0)},	 /* sdmmc1_clk */
{SDMMC1_CMD, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_cmd */
{SDMMC1_DAT0, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat0 */
{SDMMC1_DAT1, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat1 */
{SDMMC1_DAT2, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat2 */
{SDMMC1_DAT3, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat3 */
{SDMMC1_DAT4, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat4 */
{SDMMC1_DAT5, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat5 */
{SDMMC1_DAT6, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat6 */
{SDMMC1_DAT7, (PTU | IEN | OFF_EN | OFF_PD | OFF_IN | M0)}, /* sdmmc1_dat7 */
{I2C1_SCL, (PTU | IEN | M0)},				/* i2c1_scl */
{I2C1_SDA, (PTU | IEN | M0)},				/* i2c1_sda */
{I2C2_SCL, (PTU | IEN | M0)},				/* i2c2_scl */
{I2C2_SDA, (PTU | IEN | M0)},				/* i2c2_sda */
{I2C3_SCL, (PTU | IEN | M0)},				/* i2c3_scl */
{I2C3_SDA, (PTU | IEN | M0)},				/* i2c3_sda */
{I2C4_SCL, (PTU | IEN | M0)},				/* i2c4_scl */
{I2C4_SDA, (PTU | IEN | M0)},				/* i2c4_sda */
{UART3_CTS_RCTX, (PTU | IEN | M0)},			/* uart3_tx */
{UART3_RTS_SD, (M0)},					/* uart3_rts_sd */
{UART3_RX_IRRX, (IEN | M0)},				/* uart3_rx */
{UART3_TX_IRTX, (M0)}					/* uart3_tx */

};

const struct pad_conf_entry wkup_padconf_array_essential[] = {

{PAD1_SR_SCL, (PTU | IEN | M0)}, /* sr_scl */
{PAD0_SR_SDA, (PTU | IEN | M0)}, /* sr_sda */
{PAD1_SYS_32K, (IEN | M0)}	 /* sys_32k */

};

#endif  /* _OMAP4_MUX_DATA_H_ */
