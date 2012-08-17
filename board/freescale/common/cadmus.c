/*
 * Copyright 2004, 2011 Freescale Semiconductor.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>


/*
 * CADMUS Board System Registers
 */
#ifndef CONFIG_SYS_CADMUS_BASE_REG
#define CONFIG_SYS_CADMUS_BASE_REG	(CADMUS_BASE_ADDR + 0x4000)
#endif

typedef struct cadmus_reg {
    u_char cm_ver;		/* Board version */
    u_char cm_csr;		/* General control/status */
    u_char cm_rst;		/* Reset control */
    u_char cm_hsclk;		/* High speed clock */
    u_char cm_hsxclk;		/* High speed clock extended */
    u_char cm_led;		/* LED data */
    u_char cm_pci;		/* PCI control/status */
    u_char cm_dma;		/* DMA control */
    u_char cm_reserved[248];	/* Total 256 bytes */
} cadmus_reg_t;


unsigned int
get_board_version(void)
{
	volatile cadmus_reg_t *cadmus = (cadmus_reg_t *)CONFIG_SYS_CADMUS_BASE_REG;

	return cadmus->cm_ver;
}


unsigned long
get_clock_freq(void)
{
	volatile cadmus_reg_t *cadmus = (cadmus_reg_t *)CONFIG_SYS_CADMUS_BASE_REG;

	uint pci1_speed = (cadmus->cm_pci >> 2) & 0x3; /* PSPEED in [4:5] */

	if (pci1_speed == 0) {
		return 33333333;
	} else if (pci1_speed == 1) {
		return 66666666;
	} else {
		/* Really, unknown. Be safe? */
		return 33333333;
	}
}


unsigned int
get_pci_slot(void)
{
	volatile cadmus_reg_t *cadmus = (cadmus_reg_t *)CONFIG_SYS_CADMUS_BASE_REG;

	/*
	 * PCI slot in USER bits CSR[6:7] by convention.
	 */
	return ((cadmus->cm_csr >> 6) & 0x3) + 1;
}


unsigned int
get_pci_dual(void)
{
	volatile cadmus_reg_t *cadmus = (cadmus_reg_t *)CONFIG_SYS_CADMUS_BASE_REG;

	/*
	 * PCI DUAL in CM_PCI[3]
	 */
	return cadmus->cm_pci & 0x10;
}
