/*
 * (C) Copyright 2009
 * Vipin Kumar, STMicroelectronics, <vipin.kumar@st.com>
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

#ifndef _ASM_ARCH_HARDWARE_H
#define _ASM_ARCH_HARDWARE_H

#define CONFIG_SYS_USBD_BASE			0xE1100000
#define CONFIG_SYS_PLUG_BASE			0xE1200000
#define CONFIG_SYS_FIFO_BASE			0xE1000800
#define CONFIG_SYS_SMI_BASE			0xFC000000
#define CONFIG_SPEAR_SYSCNTLBASE		0xFCA00000
#define CONFIG_SPEAR_TIMERBASE			0xFC800000
#define CONFIG_SPEAR_MISCBASE			0xFCA80000
#define CONFIG_SPEAR_ETHBASE			0xE0800000
#define CONFIG_SPEAR_MPMCBASE			0xFC600000
#define CONFIG_SSP1_BASE			0xD0100000
#define CONFIG_SSP2_BASE			0xD0180000
#define CONFIG_SSP3_BASE			0xD8180000
#define CONFIG_GPIO_BASE			0xD8100000

#define CONFIG_SYS_NAND_CLE			(1 << 16)
#define CONFIG_SYS_NAND_ALE			(1 << 17)

#if defined(CONFIG_SPEAR600)
#define CONFIG_SYS_I2C_BASE			0xD0200000
#define CONFIG_SYS_FSMC_BASE			0xD1800000
#define CONFIG_FSMC_NAND_BASE			0xD2000000

#define CONFIG_SPEAR_BOOTSTRAPCFG		0xFCA80000
#define CONFIG_SPEAR_BOOTSTRAPSHFT		16
#define CONFIG_SPEAR_BOOTSTRAPMASK		0xB
#define CONFIG_SPEAR_ONLYSNORBOOT		0xA
#define CONFIG_SPEAR_NORNANDBOOT		0xB
#define CONFIG_SPEAR_NORNAND8BOOT		0x8
#define CONFIG_SPEAR_NORNAND16BOOT		0x9
#define CONFIG_SPEAR_USBBOOT			0x8

#define CONFIG_SPEAR_MPMCREGS			100

#elif defined(CONFIG_SPEAR300)
#define CONFIG_SYS_I2C_BASE			0xD0180000
#define CONFIG_SYS_FSMC_BASE			0x94000000

#elif defined(CONFIG_SPEAR310)
#define CONFIG_SYS_I2C_BASE			0xD0180000
#define CONFIG_SYS_FSMC_BASE			0x44000000

#undef CONFIG_SYS_NAND_CLE
#undef CONFIG_SYS_NAND_ALE
#define CONFIG_SYS_NAND_CLE			(1 << 17)
#define CONFIG_SYS_NAND_ALE			(1 << 16)

#define CONFIG_SPEAR_EMIBASE			0x4F000000
#define CONFIG_SPEAR_RASBASE			0xB4000000

#define CONFIG_SYS_MACB0_BASE			0xB0000000
#define CONFIG_SYS_MACB1_BASE			0xB0800000
#define CONFIG_SYS_MACB2_BASE			0xB1000000
#define CONFIG_SYS_MACB3_BASE			0xB1800000

#elif defined(CONFIG_SPEAR320)
#define CONFIG_SYS_I2C_BASE			0xD0180000
#define CONFIG_SYS_FSMC_BASE			0x4C000000

#define CONFIG_SPEAR_EMIBASE			0x40000000
#define CONFIG_SPEAR_RASBASE			0xB3000000

#define CONFIG_SYS_MACB0_BASE			0xAA000000

#endif
#endif /* _ASM_ARCH_HARDWARE_H */
