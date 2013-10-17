/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef AT91_COMMON_H
#define AT91_COMMON_H

void at91_can_hw_init(void);
void at91_gmac_hw_init(void);
void at91_macb_hw_init(void);
void at91_mci_hw_init(void);
void at91_serial0_hw_init(void);
void at91_serial1_hw_init(void);
void at91_serial2_hw_init(void);
void at91_seriald_hw_init(void);
void at91_spi0_hw_init(unsigned long cs_mask);
void at91_spi1_hw_init(unsigned long cs_mask);
void at91_udp_hw_init(void);
void at91_uhp_hw_init(void);
void at91_lcd_hw_init(void);

#endif /* AT91_COMMON_H */
