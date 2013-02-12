/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Sricharan R	<r.sricharan@ti.com>
 * Nishant Kamat <nskamat@ti.com>
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
#ifndef _MUX_DATA_DRA7XX_H_
#define _MUX_DATA_DRA7XX_H_

#include <asm/arch/mux_dra7xx.h>

const struct pad_conf_entry core_padconf_array_essential[] = {
	{MMC1_CLK, (PTU | IEN | M0)},	/* MMC1_CLK */
	{MMC1_CMD, (PTU | IEN | M0)},   /* MMC1_CMD */
	{MMC1_DAT0, (PTU | IEN | M0)},  /* MMC1_DAT0 */
	{MMC1_DAT1, (PTU | IEN | M0)},  /* MMC1_DAT1 */
	{MMC1_DAT2, (PTU | IEN | M0)},  /* MMC1_DAT2 */
	{MMC1_DAT3, (PTU | IEN | M0)},  /* MMC1_DAT3 */
	{MMC1_SDCD, (PTU | IEN | M0)},  /* MMC1_SDCD */
	{MMC1_SDWP, (PTU | IEN | M0)},  /* MMC1_SDWP */
	{UART1_RXD, (PTU | IEN | M0)},  /* UART1_RXD */
	{UART1_TXD, (M0)},              /* UART1_TXD */
	{UART1_CTSN, (PTU | IEN | M0)}, /* UART1_CTSN */
	{UART1_RTSN, (M0)},             /* UART1_RTSN */
	{I2C1_SDA, (PTU | IEN | M0)},   /* I2C1_SDA */
	{I2C1_SCL, (PTU | IEN | M0)},   /* I2C1_SCL */
};
#endif /* _MUX_DATA_DRA7XX_H_ */
