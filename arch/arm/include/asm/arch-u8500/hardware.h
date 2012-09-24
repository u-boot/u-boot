/*
 * Copyright (C) ST-Ericsson SA 2009
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

/* Peripheral clusters */

#define U8500_PER3_BASE		0x80000000
#define U8500_PER2_BASE		0x80110000
#define U8500_PER1_BASE		0x80120000
#define U8500_PER4_BASE		0x80150000

#define U8500_PER6_BASE		0xa03c0000
#define U8500_PER7_BASE		0xa03d0000
#define U8500_PER5_BASE		0xa03e0000

/* GPIO */

#define U8500_GPIO0_BASE	(U8500_PER1_BASE + 0xE000)
#define U8500_GPIO1_BASE	(U8500_PER1_BASE + 0xE000 + 0x80)

#define U8500_GPIO2_BASE	(U8500_PER3_BASE + 0xE000)
#define U8500_GPIO3_BASE	(U8500_PER3_BASE + 0xE000 + 0x80)
#define U8500_GPIO4_BASE	(U8500_PER3_BASE + 0xE000 + 0x100)
#define U8500_GPIO5_BASE	(U8500_PER3_BASE + 0xE000 + 0x180)

#define U8500_GPIO6_BASE	(U8500_PER2_BASE + 0xE000)
#define U8500_GPIO7_BASE	(U8500_PER2_BASE + 0xE000 + 0x80)

#define U8500_GPIO8_BASE	(U8500_PER5_BASE + 0x1E000)

/* Per7 */
#define U8500_CLKRST7_BASE	(U8500_PER7_BASE + 0xf000)

/* Per6 */
#define U8500_MTU0_BASE_V1	(U8500_PER6_BASE + 0x6000)
#define U8500_MTU1_BASE_V1	(U8500_PER6_BASE + 0x7000)
#define U8500_CLKRST6_BASE	(U8500_PER6_BASE + 0xf000)

/* Per5 */
#define U8500_CLKRST5_BASE	(U8500_PER5_BASE + 0x1f000)

/* Per4 */
#define U8500_PRCMU_BASE	(U8500_PER4_BASE + 0x07000)
#define U8500_PRCMU_TCDM_BASE   (U8500_PER4_BASE + 0x68000)

/* Per3 */
#define U8500_UART2_BASE	(U8500_PER3_BASE + 0x7000)
#define U8500_CLKRST3_BASE	(U8500_PER3_BASE + 0xf000)

/* Per2 */
#define U8500_CLKRST2_BASE	(U8500_PER2_BASE + 0xf000)

/* Per1 */
#define U8500_UART0_BASE	(U8500_PER1_BASE + 0x0000)
#define U8500_UART1_BASE	(U8500_PER1_BASE + 0x1000)
#define U8500_CLKRST1_BASE	(U8500_PER1_BASE + 0xf000)

/* Last page of Boot ROM */
#define U8500_BOOTROM_BASE      0x90000000
#define U8500_ASIC_ID_LOC_ED_V1 (U8500_BOOTROM_BASE + 0x1FFF4)
#define U8500_ASIC_ID_LOC_V2    (U8500_BOOTROM_BASE + 0x1DBF4)

/* AB8500 specifics */

/* address bank */
#define AB8500_REGU_CTRL2	0x0004
#define AB8500_MISC		0x0010

/* registers */
#define AB8500_REGU_VRF1VAUX3_REGU_REG	0x040A
#define AB8500_REGU_VRF1VAUX3_SEL_REG	0x0421
#define AB8500_REV_REG			0x1080

#define AB8500_GPIO_SEL2_REG	0x1001
#define AB8500_GPIO_DIR2_REG	0x1011
#define AB8500_GPIO_DIR4_REG	0x1013
#define AB8500_GPIO_SEL4_REG	0x1003
#define AB8500_GPIO_OUT2_REG	0x1021
#define AB8500_GPIO_OUT4_REG	0x1023

#define LDO_VAUX3_ENABLE_MASK	0x3
#define LDO_VAUX3_ENABLE_VAL	0x1
#define LDO_VAUX3_SEL_MASK	0xf
#define LDO_VAUX3_SEL_2V9	0xd
#define LDO_VAUX3_V2_SEL_MASK	0x7
#define LDO_VAUX3_V2_SEL_2V91	0x7


#endif /* __ASM_ARCH_HARDWARE_H */
