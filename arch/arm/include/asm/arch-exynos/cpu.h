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

#ifndef _EXYNOS4_CPU_H
#define _EXYNOS4_CPU_H

#define DEVICE_NOT_AVAILABLE		0

#define EXYNOS_CPU_NAME			"Exynos"
#define EXYNOS4_ADDR_BASE		0x10000000

/* EXYNOS4 */
#define EXYNOS4_I2C_SPACING		0x10000

#define EXYNOS4_GPIO_PART3_BASE		0x03860000
#define EXYNOS4_PRO_ID			0x10000000
#define EXYNOS4_SYSREG_BASE		0x10010000
#define EXYNOS4_POWER_BASE		0x10020000
#define EXYNOS4_SWRESET			0x10020400
#define EXYNOS4_CLOCK_BASE		0x10030000
#define EXYNOS4_SYSTIMER_BASE		0x10050000
#define EXYNOS4_WATCHDOG_BASE		0x10060000
#define EXYNOS4_MIU_BASE		0x10600000
#define EXYNOS4_DMC0_BASE		0x10400000
#define EXYNOS4_DMC1_BASE		0x10410000
#define EXYNOS4_GPIO_PART2_BASE		0x11000000
#define EXYNOS4_GPIO_PART1_BASE		0x11400000
#define EXYNOS4_FIMD_BASE		0x11C00000
#define EXYNOS4_MIPI_DSIM_BASE		0x11C80000
#define EXYNOS4_USBOTG_BASE		0x12480000
#define EXYNOS4_MMC_BASE		0x12510000
#define EXYNOS4_SROMC_BASE		0x12570000
#define EXYNOS4_USB_HOST_EHCI_BASE	0x12580000
#define EXYNOS4_USBPHY_BASE		0x125B0000
#define EXYNOS4_UART_BASE		0x13800000
#define EXYNOS4_I2C_BASE		0x13860000
#define EXYNOS4_ADC_BASE		0x13910000
#define EXYNOS4_SPI_BASE		0x13920000
#define EXYNOS4_PWMTIMER_BASE		0x139D0000
#define EXYNOS4_MODEM_BASE		0x13A00000
#define EXYNOS4_USBPHY_CONTROL		0x10020704
#define EXYNOS4_I2S_BASE		0xE2100000

#define EXYNOS4_GPIO_PART4_BASE		DEVICE_NOT_AVAILABLE
#define EXYNOS4_DP_BASE			DEVICE_NOT_AVAILABLE
#define EXYNOS4_SPI_ISP_BASE		DEVICE_NOT_AVAILABLE

/* EXYNOS5 */
#define EXYNOS5_I2C_SPACING		0x10000

#define EXYNOS5_GPIO_PART4_BASE		0x03860000
#define EXYNOS5_PRO_ID			0x10000000
#define EXYNOS5_CLOCK_BASE		0x10010000
#define EXYNOS5_POWER_BASE		0x10040000
#define EXYNOS5_SWRESET			0x10040400
#define EXYNOS5_SYSREG_BASE		0x10050000
#define EXYNOS5_WATCHDOG_BASE		0x101D0000
#define EXYNOS5_DMC_PHY0_BASE		0x10C00000
#define EXYNOS5_DMC_PHY1_BASE		0x10C10000
#define EXYNOS5_GPIO_PART3_BASE		0x10D10000
#define EXYNOS5_DMC_CTRL_BASE		0x10DD0000
#define EXYNOS5_GPIO_PART1_BASE		0x11400000
#define EXYNOS5_MIPI_DSIM_BASE		0x11D00000
#define EXYNOS5_USB_HOST_EHCI_BASE	0x12110000
#define EXYNOS5_USBPHY_BASE		0x12130000
#define EXYNOS5_USBOTG_BASE		0x12140000
#define EXYNOS5_MMC_BASE		0x12200000
#define EXYNOS5_SROMC_BASE		0x12250000
#define EXYNOS5_UART_BASE		0x12C00000
#define EXYNOS5_I2C_BASE		0x12C60000
#define EXYNOS5_SPI_BASE		0x12D20000
#define EXYNOS5_I2S_BASE		0x12D60000
#define EXYNOS5_PWMTIMER_BASE		0x12DD0000
#define EXYNOS5_SPI_ISP_BASE		0x131A0000
#define EXYNOS5_GPIO_PART2_BASE		0x13400000
#define EXYNOS5_FIMD_BASE		0x14400000
#define EXYNOS5_DP_BASE			0x145B0000

#define EXYNOS5_ADC_BASE		DEVICE_NOT_AVAILABLE
#define EXYNOS5_MODEM_BASE		DEVICE_NOT_AVAILABLE

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
	unsigned int pro_id = (readl(EXYNOS4_PRO_ID) & 0x00FFF000) >> 12;

	switch (pro_id) {
	case 0x200:
		/* Exynos4210 EVT0 */
		s5p_cpu_id = 0x4210;
		s5p_cpu_rev = 0;
		break;
	case 0x210:
		/* Exynos4210 EVT1 */
		s5p_cpu_id = 0x4210;
		break;
	case 0x412:
		/* Exynos4412 */
		s5p_cpu_id = 0x4412;
		break;
	case 0x520:
		/* Exynos5250 */
		s5p_cpu_id = 0x5250;
		break;
	}
}

static inline char *s5p_get_cpu_name(void)
{
	return EXYNOS_CPU_NAME;
}

#define IS_SAMSUNG_TYPE(type, id)			\
static inline int cpu_is_##type(void)			\
{							\
	return (s5p_cpu_id >> 12) == id;		\
}

IS_SAMSUNG_TYPE(exynos4, 0x4)
IS_SAMSUNG_TYPE(exynos5, 0x5)

#define IS_EXYNOS_TYPE(type, id)			\
static inline int proid_is_##type(void)			\
{							\
	return s5p_cpu_id == id;			\
}

IS_EXYNOS_TYPE(exynos4210, 0x4210)
IS_EXYNOS_TYPE(exynos5250, 0x5250)

#define SAMSUNG_BASE(device, base)				\
static inline unsigned int samsung_get_base_##device(void)	\
{								\
	if (cpu_is_exynos4())					\
		return EXYNOS4_##base;				\
	else if (cpu_is_exynos5())				\
		return EXYNOS5_##base;				\
	else							\
		return 0;					\
}

SAMSUNG_BASE(adc, ADC_BASE)
SAMSUNG_BASE(clock, CLOCK_BASE)
SAMSUNG_BASE(dp, DP_BASE)
SAMSUNG_BASE(sysreg, SYSREG_BASE)
SAMSUNG_BASE(fimd, FIMD_BASE)
SAMSUNG_BASE(i2c, I2C_BASE)
SAMSUNG_BASE(i2s, I2S_BASE)
SAMSUNG_BASE(mipi_dsim, MIPI_DSIM_BASE)
SAMSUNG_BASE(gpio_part1, GPIO_PART1_BASE)
SAMSUNG_BASE(gpio_part2, GPIO_PART2_BASE)
SAMSUNG_BASE(gpio_part3, GPIO_PART3_BASE)
SAMSUNG_BASE(gpio_part4, GPIO_PART4_BASE)
SAMSUNG_BASE(pro_id, PRO_ID)
SAMSUNG_BASE(mmc, MMC_BASE)
SAMSUNG_BASE(modem, MODEM_BASE)
SAMSUNG_BASE(sromc, SROMC_BASE)
SAMSUNG_BASE(swreset, SWRESET)
SAMSUNG_BASE(timer, PWMTIMER_BASE)
SAMSUNG_BASE(uart, UART_BASE)
SAMSUNG_BASE(usb_phy, USBPHY_BASE)
SAMSUNG_BASE(usb_ehci, USB_HOST_EHCI_BASE)
SAMSUNG_BASE(usb_otg, USBOTG_BASE)
SAMSUNG_BASE(watchdog, WATCHDOG_BASE)
SAMSUNG_BASE(power, POWER_BASE)
SAMSUNG_BASE(spi, SPI_BASE)
SAMSUNG_BASE(spi_isp, SPI_ISP_BASE)
#endif

#endif	/* _EXYNOS4_CPU_H */
