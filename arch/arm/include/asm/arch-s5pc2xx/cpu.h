/*
 * (C) Copyright 2010 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
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
 *
 */

#ifndef _S5PC2XX_CPU_H
#define _S5PC2XX_CPU_H

#define S5PC2XX_ADDR_BASE	0x10000000

/* S5PC210 */
#define S5PC210_GPIO_PART3_BASE	0x03860000
#define S5PC210_PRO_ID		0x10000000
#define S5PC210_POWER_BASE	0x10020000
#define S5PC210_SWRESET		0x10020400
#define S5PC210_CLOCK_BASE	0x10030000
#define S5PC210_SYSTIMER_BASE	0x10050000
#define S5PC210_WATCHDOG_BASE	0x10060000
#define S5PC210_MIU_BASE	0x10600000
#define S5PC210_DMC0_BASE	0x10400000
#define S5PC210_DMC1_BASE	0x10410000
#define S5PC210_GPIO_PART2_BASE	0x11000000
#define S5PC210_GPIO_PART1_BASE	0x11400000
#define S5PC210_FIMD_BASE	0x11C00000
#define S5PC210_USBOTG_BASE	0x12480000
#define S5PC210_MMC_BASE	0x12510000
#define S5PC210_SROMC_BASE	0x12570000
#define S5PC210_USBPHY_BASE	0x125B0000
#define S5PC210_UART_BASE	0x13800000
#define S5PC210_ADC_BASE	0x13910000
#define S5PC210_PWMTIMER_BASE	0x139D0000
#define S5PC210_MODEM_BASE	0x13A00000

#ifndef __ASSEMBLY__
#include <asm/io.h>
/* CPU detection macros */
extern unsigned int s5p_cpu_id;
extern unsigned int s5p_cpu_rev;

static inline int s5p_get_cpu_rev(void)
{
	return s5p_cpu_rev;
}

static inline void s5p_set_cpu_id(void)
{
	s5p_cpu_id = readl(S5PC210_PRO_ID);
	s5p_cpu_id = (0xC000 | ((s5p_cpu_id & 0x00FFF000) >> 12));

	/*
	 * 0xC200: S5PC210 EVT0
	 * 0xC210: S5PC210 EVT1
	 */
	if (s5p_cpu_id == 0xC200) {
		s5p_cpu_id |= 0x10;
		s5p_cpu_rev = 0;
	} else if (s5p_cpu_id == 0xC210) {
		s5p_cpu_rev = 1;
	}
}

#define IS_SAMSUNG_TYPE(type, id)			\
static inline int cpu_is_##type(void)			\
{							\
	return s5p_cpu_id == id ? 1 : 0;		\
}

IS_SAMSUNG_TYPE(s5pc210, 0xc210)

#define SAMSUNG_BASE(device, base)				\
static inline unsigned int samsung_get_base_##device(void)	\
{								\
	if (cpu_is_s5pc210())					\
		return S5PC210_##base;				\
	else							\
		return 0;					\
}

SAMSUNG_BASE(adc, ADC_BASE)
SAMSUNG_BASE(clock, CLOCK_BASE)
SAMSUNG_BASE(fimd, FIMD_BASE)
SAMSUNG_BASE(gpio_part1, GPIO_PART1_BASE)
SAMSUNG_BASE(gpio_part2, GPIO_PART2_BASE)
SAMSUNG_BASE(gpio_part3, GPIO_PART3_BASE)
SAMSUNG_BASE(pro_id, PRO_ID)
SAMSUNG_BASE(mmc, MMC_BASE)
SAMSUNG_BASE(modem, MODEM_BASE)
SAMSUNG_BASE(sromc, SROMC_BASE)
SAMSUNG_BASE(swreset, SWRESET)
SAMSUNG_BASE(timer, PWMTIMER_BASE)
SAMSUNG_BASE(uart, UART_BASE)
SAMSUNG_BASE(usb_phy, USBPHY_BASE)
SAMSUNG_BASE(usb_otg, USBOTG_BASE)
SAMSUNG_BASE(watchdog, WATCHDOG_BASE)
#endif

#endif	/* _S5PC2XX_CPU_H */
