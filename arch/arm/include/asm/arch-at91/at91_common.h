/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
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

#ifndef AT91_COMMON_H
#define AT91_COMMON_H

void at91_can_hw_init(void);
void at91_macb_hw_init(void);
void at91_mci_hw_init(void);
void at91_serial0_hw_init(void);
void at91_serial1_hw_init(void);
void at91_serial2_hw_init(void);
void at91_seriald_hw_init(void);
void at91_spi0_hw_init(unsigned long cs_mask);
void at91_spi1_hw_init(unsigned long cs_mask);
void at91_uhp_hw_init(void);

#endif /* AT91_COMMON_H */
