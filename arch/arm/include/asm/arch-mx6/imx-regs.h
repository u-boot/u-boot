/*
 * Copyright (C) 2011 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __ASM_ARCH_MX6_IMX_REGS_H__
#define __ASM_ARCH_MX6_IMX_REGS_H__

#define ARCH_MXC

#define CONFIG_SYS_CACHELINE_SIZE	32

#define ROMCP_ARB_BASE_ADDR             0x00000000
#define ROMCP_ARB_END_ADDR              0x000FFFFF
#define CAAM_ARB_BASE_ADDR              0x00100000
#define CAAM_ARB_END_ADDR               0x00103FFF
#define APBH_DMA_ARB_BASE_ADDR          0x00110000
#define APBH_DMA_ARB_END_ADDR           0x00117FFF
#define HDMI_ARB_BASE_ADDR              0x00120000
#define HDMI_ARB_END_ADDR               0x00128FFF
#define GPU_3D_ARB_BASE_ADDR            0x00130000
#define GPU_3D_ARB_END_ADDR             0x00133FFF
#define GPU_2D_ARB_BASE_ADDR            0x00134000
#define GPU_2D_ARB_END_ADDR             0x00137FFF
#define DTCP_ARB_BASE_ADDR              0x00138000
#define DTCP_ARB_END_ADDR               0x0013BFFF

/* GPV - PL301 configuration ports */
#define GPV2_BASE_ADDR			0x00200000
#define GPV3_BASE_ADDR			0x00300000
#define GPV4_BASE_ADDR			0x00800000
#define IRAM_BASE_ADDR			0x00900000
#define SCU_BASE_ADDR                   0x00A00000
#define IC_INTERFACES_BASE_ADDR         0x00A00100
#define GLOBAL_TIMER_BASE_ADDR          0x00A00200
#define PRIVATE_TIMERS_WD_BASE_ADDR     0x00A00600
#define IC_DISTRIBUTOR_BASE_ADDR        0x00A01000
#define GPV0_BASE_ADDR                  0x00B00000
#define GPV1_BASE_ADDR                  0x00C00000
#define PCIE_ARB_BASE_ADDR              0x01000000
#define PCIE_ARB_END_ADDR               0x01FFFFFF

#define AIPS1_ARB_BASE_ADDR             0x02000000
#define AIPS1_ARB_END_ADDR              0x020FFFFF
#define AIPS2_ARB_BASE_ADDR             0x02100000
#define AIPS2_ARB_END_ADDR              0x021FFFFF
#define SATA_ARB_BASE_ADDR              0x02200000
#define SATA_ARB_END_ADDR               0x02203FFF
#define OPENVG_ARB_BASE_ADDR            0x02204000
#define OPENVG_ARB_END_ADDR             0x02207FFF
#define HSI_ARB_BASE_ADDR               0x02208000
#define HSI_ARB_END_ADDR                0x0220BFFF
#define IPU1_ARB_BASE_ADDR              0x02400000
#define IPU1_ARB_END_ADDR               0x027FFFFF
#define IPU2_ARB_BASE_ADDR              0x02800000
#define IPU2_ARB_END_ADDR               0x02BFFFFF
#define WEIM_ARB_BASE_ADDR              0x08000000
#define WEIM_ARB_END_ADDR               0x0FFFFFFF

#define MMDC0_ARB_BASE_ADDR             0x10000000
#define MMDC0_ARB_END_ADDR              0x7FFFFFFF
#define MMDC1_ARB_BASE_ADDR             0x80000000
#define MMDC1_ARB_END_ADDR              0xFFFFFFFF

#define IPU_SOC_BASE_ADDR		IPU1_ARB_BASE_ADDR
#define IPU_SOC_OFFSET			0x00200000

/* Defines for Blocks connected via AIPS (SkyBlue) */
#define ATZ1_BASE_ADDR              AIPS1_ARB_BASE_ADDR
#define ATZ2_BASE_ADDR              AIPS2_ARB_BASE_ADDR
#define AIPS1_BASE_ADDR             AIPS1_ON_BASE_ADDR
#define AIPS2_BASE_ADDR             AIPS2_ON_BASE_ADDR

#define SPDIF_BASE_ADDR             (ATZ1_BASE_ADDR + 0x04000)
#define ECSPI1_BASE_ADDR            (ATZ1_BASE_ADDR + 0x08000)
#define ECSPI2_BASE_ADDR            (ATZ1_BASE_ADDR + 0x0C000)
#define ECSPI3_BASE_ADDR            (ATZ1_BASE_ADDR + 0x10000)
#define ECSPI4_BASE_ADDR            (ATZ1_BASE_ADDR + 0x14000)
#define ECSPI5_BASE_ADDR            (ATZ1_BASE_ADDR + 0x18000)
#define UART1_BASE                  (ATZ1_BASE_ADDR + 0x20000)
#define ESAI1_BASE_ADDR             (ATZ1_BASE_ADDR + 0x24000)
#define SSI1_BASE_ADDR              (ATZ1_BASE_ADDR + 0x28000)
#define SSI2_BASE_ADDR              (ATZ1_BASE_ADDR + 0x2C000)
#define SSI3_BASE_ADDR              (ATZ1_BASE_ADDR + 0x30000)
#define ASRC_BASE_ADDR              (ATZ1_BASE_ADDR + 0x34000)
#define SPBA_BASE_ADDR              (ATZ1_BASE_ADDR + 0x3C000)
#define VPU_BASE_ADDR               (ATZ1_BASE_ADDR + 0x40000)
#define AIPS1_ON_BASE_ADDR          (ATZ1_BASE_ADDR + 0x7C000)

#define AIPS1_OFF_BASE_ADDR         (ATZ1_BASE_ADDR + 0x80000)
#define PWM1_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x0000)
#define PWM2_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x4000)
#define PWM3_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x8000)
#define PWM4_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0xC000)
#define CAN1_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x10000)
#define CAN2_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x14000)
#define GPT1_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x18000)
#define GPIO1_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x1C000)
#define GPIO2_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x20000)
#define GPIO3_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x24000)
#define GPIO4_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x28000)
#define GPIO5_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x2C000)
#define GPIO6_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x30000)
#define GPIO7_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x34000)
#define KPP_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x38000)
#define WDOG1_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x3C000)
#define WDOG2_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x40000)
#define ANATOP_BASE_ADDR            (AIPS1_OFF_BASE_ADDR + 0x48000)
#define USB_PHY0_BASE_ADDR          (AIPS1_OFF_BASE_ADDR + 0x49000)
#define USB_PHY1_BASE_ADDR          (AIPS1_OFF_BASE_ADDR + 0x4a000)
#define CCM_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x44000)
#define SNVS_BASE_ADDR              (AIPS1_OFF_BASE_ADDR + 0x4C000)
#define EPIT1_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x50000)
#define EPIT2_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x54000)
#define SRC_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x58000)
#define GPC_BASE_ADDR               (AIPS1_OFF_BASE_ADDR + 0x5C000)
#define IOMUXC_BASE_ADDR            (AIPS1_OFF_BASE_ADDR + 0x60000)
#define DCIC1_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x64000)
#define DCIC2_BASE_ADDR             (AIPS1_OFF_BASE_ADDR + 0x68000)
#define DMA_REQ_PORT_HOST_BASE_ADDR (AIPS1_OFF_BASE_ADDR + 0x6C000)

#define AIPS2_ON_BASE_ADDR          (ATZ2_BASE_ADDR + 0x7C000)
#define AIPS2_OFF_BASE_ADDR         (ATZ2_BASE_ADDR + 0x80000)
#define CAAM_BASE_ADDR              (ATZ2_BASE_ADDR)
#define ARM_BASE_ADDR		    (ATZ2_BASE_ADDR + 0x40000)
#define USBOH3_PL301_BASE_ADDR      (AIPS2_OFF_BASE_ADDR + 0x0000)
#define USBOH3_USB_BASE_ADDR        (AIPS2_OFF_BASE_ADDR + 0x4000)
#define ENET_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x8000)
#define MLB_BASE_ADDR               (AIPS2_OFF_BASE_ADDR + 0xC000)
#define USDHC1_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x10000)
#define USDHC2_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x14000)
#define USDHC3_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x18000)
#define USDHC4_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x1C000)
#define I2C1_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x20000)
#define I2C2_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x24000)
#define I2C3_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x28000)
#define ROMCP_BASE_ADDR             (AIPS2_OFF_BASE_ADDR + 0x2C000)
#define MMDC_P0_BASE_ADDR           (AIPS2_OFF_BASE_ADDR + 0x30000)
#define MMDC_P1_BASE_ADDR           (AIPS2_OFF_BASE_ADDR + 0x34000)
#define WEIM_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x38000)
#define OCOTP_BASE_ADDR             (AIPS2_OFF_BASE_ADDR + 0x3C000)
#define CSU_BASE_ADDR               (AIPS2_OFF_BASE_ADDR + 0x40000)
#define IP2APB_PERFMON1_BASE_ADDR   (AIPS2_OFF_BASE_ADDR + 0x44000)
#define IP2APB_PERFMON2_BASE_ADDR   (AIPS2_OFF_BASE_ADDR + 0x48000)
#define IP2APB_PERFMON3_BASE_ADDR   (AIPS2_OFF_BASE_ADDR + 0x4C000)
#define IP2APB_TZASC1_BASE_ADDR     (AIPS2_OFF_BASE_ADDR + 0x50000)
#define IP2APB_TZASC2_BASE_ADDR     (AIPS2_OFF_BASE_ADDR + 0x54000)
#define AUDMUX_BASE_ADDR            (AIPS2_OFF_BASE_ADDR + 0x58000)
#define MIPI_CSI2_BASE_ADDR         (AIPS2_OFF_BASE_ADDR + 0x5C000)
#define MIPI_DSI_BASE_ADDR          (AIPS2_OFF_BASE_ADDR + 0x60000)
#define VDOA_BASE_ADDR              (AIPS2_OFF_BASE_ADDR + 0x64000)
#define UART2_BASE                  (AIPS2_OFF_BASE_ADDR + 0x68000)
#define UART3_BASE                  (AIPS2_OFF_BASE_ADDR + 0x6C000)
#define UART4_BASE                  (AIPS2_OFF_BASE_ADDR + 0x70000)
#define UART5_BASE                  (AIPS2_OFF_BASE_ADDR + 0x74000)
#define IP2APB_USBPHY1_BASE_ADDR    (AIPS2_OFF_BASE_ADDR + 0x78000)
#define IP2APB_USBPHY2_BASE_ADDR    (AIPS2_OFF_BASE_ADDR + 0x7C000)

#define CHIP_REV_1_0                 0x10
#define IRAM_SIZE                    0x00040000
#define IMX_IIM_BASE                 OCOTP_BASE_ADDR
#define FEC_QUIRK_ENET_MAC

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

extern void imx_get_mac_from_fuse(int dev_id, unsigned char *mac);

/* System Reset Controller (SRC) */
struct src {
	u32	scr;
	u32	sbmr1;
	u32	srsr;
	u32	reserved1[2];
	u32	sisr;
	u32	simr;
	u32     sbmr2;
	u32     gpr1;
	u32     gpr2;
	u32     gpr3;
	u32     gpr4;
	u32     gpr5;
	u32     gpr6;
	u32     gpr7;
	u32     gpr8;
	u32     gpr9;
	u32     gpr10;
};

/* OCOTP Registers */
struct ocotp_regs {
	u32	reserved[0x198];
	u32	gp1;	/* 0x660 */
};

/* GPR3 bitfields */
#define IOMUXC_GPR3_GPU_DBG_OFFSET		29
#define IOMUXC_GPR3_GPU_DBG_MASK		(3<<IOMUXC_GPR3_GPU_DBG_OFFSET)
#define IOMUXC_GPR3_BCH_WR_CACHE_CTL_OFFSET	28
#define IOMUXC_GPR3_BCH_WR_CACHE_CTL_MASK	(1<<IOMUXC_GPR3_BCH_WR_CACHE_CTL_OFFSET)
#define IOMUXC_GPR3_BCH_RD_CACHE_CTL_OFFSET	27
#define IOMUXC_GPR3_BCH_RD_CACHE_CTL_MASK	(1<<IOMUXC_GPR3_BCH_RD_CACHE_CTL_OFFSET)
#define IOMUXC_GPR3_uSDHCx_WR_CACHE_CTL_OFFSET	26
#define IOMUXC_GPR3_uSDHCx_WR_CACHE_CTL_MASK	(1<<IOMUXC_GPR3_uSDHCx_WR_CACHE_CTL_OFFSET)
#define IOMUXC_GPR3_uSDHCx_RD_CACHE_CTL_OFFSET	25
#define IOMUXC_GPR3_uSDHCx_RD_CACHE_CTL_MASK	(1<<IOMUXC_GPR3_uSDHCx_RD_CACHE_CTL_OFFSET)
#define IOMUXC_GPR3_OCRAM_CTL_OFFSET		21
#define IOMUXC_GPR3_OCRAM_CTL_MASK		(0xf<<IOMUXC_GPR3_OCRAM_CTL_OFFSET)
#define IOMUXC_GPR3_OCRAM_STATUS_OFFSET		17
#define IOMUXC_GPR3_OCRAM_STATUS_MASK		(0xf<<IOMUXC_GPR3_OCRAM_STATUS_OFFSET)
#define IOMUXC_GPR3_CORE3_DBG_ACK_EN_OFFSET	16
#define IOMUXC_GPR3_CORE3_DBG_ACK_EN_MASK	(1<<IOMUXC_GPR3_CORE3_DBG_ACK_EN_OFFSET)
#define IOMUXC_GPR3_CORE2_DBG_ACK_EN_OFFSET	15
#define IOMUXC_GPR3_CORE2_DBG_ACK_EN_MASK	(1<<IOMUXC_GPR3_CORE2_DBG_ACK_EN_OFFSET)
#define IOMUXC_GPR3_CORE1_DBG_ACK_EN_OFFSET	14
#define IOMUXC_GPR3_CORE1_DBG_ACK_EN_MASK	(1<<IOMUXC_GPR3_CORE1_DBG_ACK_EN_OFFSET)
#define IOMUXC_GPR3_CORE0_DBG_ACK_EN_OFFSET	13
#define IOMUXC_GPR3_CORE0_DBG_ACK_EN_MASK	(1<<IOMUXC_GPR3_CORE0_DBG_ACK_EN_OFFSET)
#define IOMUXC_GPR3_TZASC2_BOOT_LOCK_OFFSET	12
#define IOMUXC_GPR3_TZASC2_BOOT_LOCK_MASK	(1<<IOMUXC_GPR3_TZASC2_BOOT_LOCK_OFFSET)
#define IOMUXC_GPR3_TZASC1_BOOT_LOCK_OFFSET	11
#define IOMUXC_GPR3_TZASC1_BOOT_LOCK_MASK	(1<<IOMUXC_GPR3_TZASC1_BOOT_LOCK_OFFSET)
#define IOMUXC_GPR3_IPU_DIAG_OFFSET		10
#define IOMUXC_GPR3_IPU_DIAG_MASK		(1<<IOMUXC_GPR3_IPU_DIAG_OFFSET)

#define IOMUXC_GPR3_MUX_SRC_IPU1_DI0	0
#define IOMUXC_GPR3_MUX_SRC_IPU1_DI1	1
#define IOMUXC_GPR3_MUX_SRC_IPU2_DI0	2
#define IOMUXC_GPR3_MUX_SRC_IPU2_DI1	3

#define IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET	8
#define IOMUXC_GPR3_LVDS1_MUX_CTL_MASK		(3<<IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET)

#define IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET	6
#define IOMUXC_GPR3_LVDS0_MUX_CTL_MASK		(3<<IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET)

#define IOMUXC_GPR3_MIPI_MUX_CTL_OFFSET		4
#define IOMUXC_GPR3_MIPI_MUX_CTL_MASK		(3<<IOMUXC_GPR3_MIPI_MUX_CTL_OFFSET)

#define IOMUXC_GPR3_HDMI_MUX_CTL_OFFSET		2
#define IOMUXC_GPR3_HDMI_MUX_CTL_MASK		(3<<IOMUXC_GPR3_HDMI_MUX_CTL_OFFSET)


struct iomuxc {
	u32 gpr[14];
	u32 omux[5];
	/* mux and pad registers */
};

#define IOMUXC_GPR2_COUNTER_RESET_VAL_OFFSET		20
#define IOMUXC_GPR2_COUNTER_RESET_VAL_MASK		(3<<IOMUXC_GPR2_COUNTER_RESET_VAL_OFFSET)
#define IOMUXC_GPR2_LVDS_CLK_SHIFT_OFFSET		16
#define IOMUXC_GPR2_LVDS_CLK_SHIFT_MASK			(7<<IOMUXC_GPR2_LVDS_CLK_SHIFT_OFFSET)

#define IOMUXC_GPR2_BGREF_RRMODE_OFFSET			15
#define IOMUXC_GPR2_BGREF_RRMODE_MASK			(1<<IOMUXC_GPR2_BGREF_RRMODE_OFFSET)
#define IOMUXC_GPR2_BGREF_RRMODE_INTERNAL_RES		(1<<IOMUXC_GPR2_BGREF_RRMODE_OFFSET)
#define IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES		(0<<IOMUXC_GPR2_BGREF_RRMODE_OFFSET)
#define IOMUXC_GPR2_VSYNC_ACTIVE_HIGH	0
#define IOMUXC_GPR2_VSYNC_ACTIVE_LOW	1

#define IOMUXC_GPR2_DI1_VS_POLARITY_OFFSET		10
#define IOMUXC_GPR2_DI1_VS_POLARITY_MASK		(1<<IOMUXC_GPR2_DI1_VS_POLARITY_OFFSET)
#define IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_HIGH		(IOMUXC_GPR2_VSYNC_ACTIVE_HIGH<<IOMUXC_GPR2_DI1_VS_POLARITY_OFFSET)
#define IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW		(IOMUXC_GPR2_VSYNC_ACTIVE_LOW<<IOMUXC_GPR2_DI1_VS_POLARITY_OFFSET)

#define IOMUXC_GPR2_DI0_VS_POLARITY_OFFSET		9
#define IOMUXC_GPR2_DI0_VS_POLARITY_MASK		(1<<IOMUXC_GPR2_DI0_VS_POLARITY_OFFSET)
#define IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_HIGH		(IOMUXC_GPR2_VSYNC_ACTIVE_HIGH<<IOMUXC_GPR2_DI0_VS_POLARITY_OFFSET)
#define IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW		(IOMUXC_GPR2_VSYNC_ACTIVE_LOW<<IOMUXC_GPR2_DI0_VS_POLARITY_OFFSET)

#define IOMUXC_GPR2_BITMAP_SPWG	0
#define IOMUXC_GPR2_BITMAP_JEIDA	1

#define IOMUXC_GPR2_BIT_MAPPING_CH1_OFFSET		8
#define IOMUXC_GPR2_BIT_MAPPING_CH1_MASK		(1<<IOMUXC_GPR2_BIT_MAPPING_CH1_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH1_JEIDA		(IOMUXC_GPR2_BITMAP_JEIDA<<IOMUXC_GPR2_BIT_MAPPING_CH1_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG		(IOMUXC_GPR2_BITMAP_SPWG<<IOMUXC_GPR2_BIT_MAPPING_CH1_OFFSET)

#define IOMUXC_GPR2_DATA_WIDTH_18	0
#define IOMUXC_GPR2_DATA_WIDTH_24	1

#define IOMUXC_GPR2_DATA_WIDTH_CH1_OFFSET		7
#define IOMUXC_GPR2_DATA_WIDTH_CH1_MASK			(1<<IOMUXC_GPR2_DATA_WIDTH_CH1_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT		(IOMUXC_GPR2_DATA_WIDTH_18<<IOMUXC_GPR2_DATA_WIDTH_CH1_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH1_24BIT		(IOMUXC_GPR2_DATA_WIDTH_24<<IOMUXC_GPR2_DATA_WIDTH_CH1_OFFSET)

#define IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET		6
#define IOMUXC_GPR2_BIT_MAPPING_CH0_MASK		(1<<IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH0_JEIDA		(IOMUXC_GPR2_BITMAP_JEIDA<<IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET)
#define IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG		(IOMUXC_GPR2_BITMAP_SPWG<<IOMUXC_GPR2_BIT_MAPPING_CH0_OFFSET)

#define IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET		5
#define IOMUXC_GPR2_DATA_WIDTH_CH0_MASK			(1<<IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT		(IOMUXC_GPR2_DATA_WIDTH_18<<IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET)
#define IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT		(IOMUXC_GPR2_DATA_WIDTH_24<<IOMUXC_GPR2_DATA_WIDTH_CH0_OFFSET)

#define IOMUXC_GPR2_SPLIT_MODE_EN_OFFSET		4
#define IOMUXC_GPR2_SPLIT_MODE_EN_MASK			(1<<IOMUXC_GPR2_SPLIT_MODE_EN_OFFSET)

#define IOMUXC_GPR2_MODE_DISABLED	0
#define IOMUXC_GPR2_MODE_ENABLED_DI0	1
#define IOMUXC_GPR2_MODE_ENABLED_DI1	2

#define IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET		2
#define IOMUXC_GPR2_LVDS_CH1_MODE_MASK			(3<<IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED		(IOMUXC_GPR2_MODE_DISABLED<<IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0		(IOMUXC_GPR2_MODE_ENABLED_DI0<<IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI1		(IOMUXC_GPR2_MODE_ENABLED_DI1<<IOMUXC_GPR2_LVDS_CH1_MODE_OFFSET)

#define IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET		0
#define IOMUXC_GPR2_LVDS_CH0_MODE_MASK			(3<<IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED		(IOMUXC_GPR2_MODE_DISABLED<<IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0		(IOMUXC_GPR2_MODE_ENABLED_DI0<<IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)
#define IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI1		(IOMUXC_GPR2_MODE_ENABLED_DI1<<IOMUXC_GPR2_LVDS_CH0_MODE_OFFSET)

/* ECSPI registers */
struct cspi_regs {
	u32 rxdata;
	u32 txdata;
	u32 ctrl;
	u32 cfg;
	u32 intr;
	u32 dma;
	u32 stat;
	u32 period;
};

/*
 * CSPI register definitions
 */
#define MXC_ECSPI
#define MXC_CSPICTRL_EN		(1 << 0)
#define MXC_CSPICTRL_MODE	(1 << 1)
#define MXC_CSPICTRL_XCH	(1 << 2)
#define MXC_CSPICTRL_CHIPSELECT(x)	(((x) & 0x3) << 12)
#define MXC_CSPICTRL_BITCOUNT(x)	(((x) & 0xfff) << 20)
#define MXC_CSPICTRL_PREDIV(x)	(((x) & 0xF) << 12)
#define MXC_CSPICTRL_POSTDIV(x)	(((x) & 0xF) << 8)
#define MXC_CSPICTRL_SELCHAN(x)	(((x) & 0x3) << 18)
#define MXC_CSPICTRL_MAXBITS	0xfff
#define MXC_CSPICTRL_TC		(1 << 7)
#define MXC_CSPICTRL_RXOVF	(1 << 6)
#define MXC_CSPIPERIOD_32KHZ	(1 << 15)
#define MAX_SPI_BYTES	32

/* Bit position inside CTRL register to be associated with SS */
#define MXC_CSPICTRL_CHAN	18

/* Bit position inside CON register to be associated with SS */
#define MXC_CSPICON_POL		4
#define MXC_CSPICON_PHA		0
#define MXC_CSPICON_SSPOL	12
#define MXC_SPI_BASE_ADDRESSES \
	ECSPI1_BASE_ADDR, \
	ECSPI2_BASE_ADDR, \
	ECSPI3_BASE_ADDR, \
	ECSPI4_BASE_ADDR, \
	ECSPI5_BASE_ADDR

struct iim_regs {
	u32	ctrl;
	u32	ctrl_set;
	u32     ctrl_clr;
	u32	ctrl_tog;
	u32	timing;
	u32     rsvd0[3];
	u32     data;
	u32     rsvd1[3];
	u32     read_ctrl;
	u32     rsvd2[3];
	u32     fuse_data;
	u32     rsvd3[3];
	u32     sticky;
	u32     rsvd4[3];
	u32     scs;
	u32     scs_set;
	u32     scs_clr;
	u32     scs_tog;
	u32     crc_addr;
	u32     rsvd5[3];
	u32     crc_value;
	u32     rsvd6[3];
	u32     version;
	u32     rsvd7[0xdb];

	struct fuse_bank {
		u32	fuse_regs[0x20];
	} bank[15];
};

struct fuse_bank4_regs {
	u32	sjc_resp_low;
	u32     rsvd0[3];
	u32     sjc_resp_high;
	u32     rsvd1[3];
	u32	mac_addr_low;
	u32     rsvd2[3];
	u32     mac_addr_high;
	u32	rsvd3[0x13];
};

struct aipstz_regs {
	u32	mprot0;
	u32	mprot1;
	u32	rsvd[0xe];
	u32	opacr0;
	u32	opacr1;
	u32	opacr2;
	u32	opacr3;
	u32	opacr4;
};

struct anatop_regs {
	u32	pll_sys;		/* 0x000 */
	u32	pll_sys_set;		/* 0x004 */
	u32	pll_sys_clr;		/* 0x008 */
	u32	pll_sys_tog;		/* 0x00c */
	u32	usb1_pll_480_ctrl;	/* 0x010 */
	u32	usb1_pll_480_ctrl_set;	/* 0x014 */
	u32	usb1_pll_480_ctrl_clr;	/* 0x018 */
	u32	usb1_pll_480_ctrl_tog;	/* 0x01c */
	u32	usb2_pll_480_ctrl;	/* 0x020 */
	u32	usb2_pll_480_ctrl_set;	/* 0x024 */
	u32	usb2_pll_480_ctrl_clr;	/* 0x028 */
	u32	usb2_pll_480_ctrl_tog;	/* 0x02c */
	u32	pll_528;		/* 0x030 */
	u32	pll_528_set;		/* 0x034 */
	u32	pll_528_clr;		/* 0x038 */
	u32	pll_528_tog;		/* 0x03c */
	u32	pll_528_ss;		/* 0x040 */
	u32	rsvd0[3];
	u32	pll_528_num;		/* 0x050 */
	u32	rsvd1[3];
	u32	pll_528_denom;		/* 0x060 */
	u32	rsvd2[3];
	u32	pll_audio;		/* 0x070 */
	u32	pll_audio_set;		/* 0x074 */
	u32	pll_audio_clr;		/* 0x078 */
	u32	pll_audio_tog;		/* 0x07c */
	u32	pll_audio_num;		/* 0x080 */
	u32	rsvd3[3];
	u32	pll_audio_denom;	/* 0x090 */
	u32	rsvd4[3];
	u32	pll_video;		/* 0x0a0 */
	u32	pll_video_set;		/* 0x0a4 */
	u32	pll_video_clr;		/* 0x0a8 */
	u32	pll_video_tog;		/* 0x0ac */
	u32	pll_video_num;		/* 0x0b0 */
	u32	rsvd5[3];
	u32	pll_video_denom;	/* 0x0c0 */
	u32	rsvd6[3];
	u32	pll_mlb;		/* 0x0d0 */
	u32	pll_mlb_set;		/* 0x0d4 */
	u32	pll_mlb_clr;		/* 0x0d8 */
	u32	pll_mlb_tog;		/* 0x0dc */
	u32	pll_enet;		/* 0x0e0 */
	u32	pll_enet_set;		/* 0x0e4 */
	u32	pll_enet_clr;		/* 0x0e8 */
	u32	pll_enet_tog;		/* 0x0ec */
	u32	pfd_480;		/* 0x0f0 */
	u32	pfd_480_set;		/* 0x0f4 */
	u32	pfd_480_clr;		/* 0x0f8 */
	u32	pfd_480_tog;		/* 0x0fc */
	u32	pfd_528;		/* 0x100 */
	u32	pfd_528_set;		/* 0x104 */
	u32	pfd_528_clr;		/* 0x108 */
	u32	pfd_528_tog;		/* 0x10c */
	u32	reg_1p1;		/* 0x110 */
	u32	reg_1p1_set;		/* 0x114 */
	u32	reg_1p1_clr;		/* 0x118 */
	u32	reg_1p1_tog;		/* 0x11c */
	u32	reg_3p0;		/* 0x120 */
	u32	reg_3p0_set;		/* 0x124 */
	u32	reg_3p0_clr;		/* 0x128 */
	u32	reg_3p0_tog;		/* 0x12c */
	u32	reg_2p5;		/* 0x130 */
	u32	reg_2p5_set;		/* 0x134 */
	u32	reg_2p5_clr;		/* 0x138 */
	u32	reg_2p5_tog;		/* 0x13c */
	u32	reg_core;		/* 0x140 */
	u32	reg_core_set;		/* 0x144 */
	u32	reg_core_clr;		/* 0x148 */
	u32	reg_core_tog;		/* 0x14c */
	u32	ana_misc0;		/* 0x150 */
	u32	ana_misc0_set;		/* 0x154 */
	u32	ana_misc0_clr;		/* 0x158 */
	u32	ana_misc0_tog;		/* 0x15c */
	u32	ana_misc1;		/* 0x160 */
	u32	ana_misc1_set;		/* 0x164 */
	u32	ana_misc1_clr;		/* 0x168 */
	u32	ana_misc1_tog;		/* 0x16c */
	u32	ana_misc2;		/* 0x170 */
	u32	ana_misc2_set;		/* 0x174 */
	u32	ana_misc2_clr;		/* 0x178 */
	u32	ana_misc2_tog;		/* 0x17c */
	u32	tempsense0;		/* 0x180 */
	u32	tempsense0_set;		/* 0x184 */
	u32	tempsense0_clr;		/* 0x188 */
	u32	tempsense0_tog;		/* 0x18c */
	u32	tempsense1;		/* 0x190 */
	u32	tempsense1_set;		/* 0x194 */
	u32	tempsense1_clr;		/* 0x198 */
	u32	tempsense1_tog;		/* 0x19c */
	u32	usb1_vbus_detect;	/* 0x1a0 */
	u32	usb1_vbus_detect_set;	/* 0x1a4 */
	u32	usb1_vbus_detect_clr;	/* 0x1a8 */
	u32	usb1_vbus_detect_tog;	/* 0x1ac */
	u32	usb1_chrg_detect;	/* 0x1b0 */
	u32	usb1_chrg_detect_set;	/* 0x1b4 */
	u32	usb1_chrg_detect_clr;	/* 0x1b8 */
	u32	usb1_chrg_detect_tog;	/* 0x1bc */
	u32	usb1_vbus_det_stat;	/* 0x1c0 */
	u32	usb1_vbus_det_stat_set;	/* 0x1c4 */
	u32	usb1_vbus_det_stat_clr;	/* 0x1c8 */
	u32	usb1_vbus_det_stat_tog;	/* 0x1cc */
	u32	usb1_chrg_det_stat;	/* 0x1d0 */
	u32	usb1_chrg_det_stat_set;	/* 0x1d4 */
	u32	usb1_chrg_det_stat_clr;	/* 0x1d8 */
	u32	usb1_chrg_det_stat_tog;	/* 0x1dc */
	u32	usb1_loopback;		/* 0x1e0 */
	u32	usb1_loopback_set;	/* 0x1e4 */
	u32	usb1_loopback_clr;	/* 0x1e8 */
	u32	usb1_loopback_tog;	/* 0x1ec */
	u32	usb1_misc;		/* 0x1f0 */
	u32	usb1_misc_set;		/* 0x1f4 */
	u32	usb1_misc_clr;		/* 0x1f8 */
	u32	usb1_misc_tog;		/* 0x1fc */
	u32	usb2_vbus_detect;	/* 0x200 */
	u32	usb2_vbus_detect_set;	/* 0x204 */
	u32	usb2_vbus_detect_clr;	/* 0x208 */
	u32	usb2_vbus_detect_tog;	/* 0x20c */
	u32	usb2_chrg_detect;	/* 0x210 */
	u32	usb2_chrg_detect_set;	/* 0x214 */
	u32	usb2_chrg_detect_clr;	/* 0x218 */
	u32	usb2_chrg_detect_tog;	/* 0x21c */
	u32	usb2_vbus_det_stat;	/* 0x220 */
	u32	usb2_vbus_det_stat_set;	/* 0x224 */
	u32	usb2_vbus_det_stat_clr;	/* 0x228 */
	u32	usb2_vbus_det_stat_tog;	/* 0x22c */
	u32	usb2_chrg_det_stat;	/* 0x230 */
	u32	usb2_chrg_det_stat_set;	/* 0x234 */
	u32	usb2_chrg_det_stat_clr;	/* 0x238 */
	u32	usb2_chrg_det_stat_tog;	/* 0x23c */
	u32	usb2_loopback;		/* 0x240 */
	u32	usb2_loopback_set;	/* 0x244 */
	u32	usb2_loopback_clr;	/* 0x248 */
	u32	usb2_loopback_tog;	/* 0x24c */
	u32	usb2_misc;		/* 0x250 */
	u32	usb2_misc_set;		/* 0x254 */
	u32	usb2_misc_clr;		/* 0x258 */
	u32	usb2_misc_tog;		/* 0x25c */
	u32	digprog;		/* 0x260 */
	u32	reserved1[7];
	u32	digprog_sololite;	/* 0x280 */
};

#define ANATOP_PFD_480_PFD0_FRAC_SHIFT		0
#define ANATOP_PFD_480_PFD0_FRAC_MASK		(0x3f<<ANATOP_PFD_480_PFD0_FRAC_SHIFT)
#define ANATOP_PFD_480_PFD0_STABLE_SHIFT	6
#define ANATOP_PFD_480_PFD0_STABLE_MASK		(1<<ANATOP_PFD_480_PFD0_STABLE_SHIFT)
#define ANATOP_PFD_480_PFD0_CLKGATE_SHIFT	7
#define ANATOP_PFD_480_PFD0_CLKGATE_MASK	(1<<ANATOP_PFD_480_PFD0_CLKGATE_SHIFT)
#define ANATOP_PFD_480_PFD1_FRAC_SHIFT		8
#define ANATOP_PFD_480_PFD1_FRAC_MASK		(0x3f<<ANATOP_PFD_480_PFD1_FRAC_SHIFT)
#define ANATOP_PFD_480_PFD1_STABLE_SHIFT	14
#define ANATOP_PFD_480_PFD1_STABLE_MASK		(1<<ANATOP_PFD_480_PFD1_STABLE_SHIFT)
#define ANATOP_PFD_480_PFD1_CLKGATE_SHIFT	15
#define ANATOP_PFD_480_PFD1_CLKGATE_MASK	(0x3f<<ANATOP_PFD_480_PFD1_CLKGATE_SHIFT)
#define ANATOP_PFD_480_PFD2_FRAC_SHIFT		16
#define ANATOP_PFD_480_PFD2_FRAC_MASK		(1<<ANATOP_PFD_480_PFD2_FRAC_SHIFT)
#define ANATOP_PFD_480_PFD2_STABLE_SHIFT	22
#define ANATOP_PFD_480_PFD2_STABLE_MASK	(1<<ANATOP_PFD_480_PFD2_STABLE_SHIFT)
#define ANATOP_PFD_480_PFD2_CLKGATE_SHIFT	23
#define ANATOP_PFD_480_PFD2_CLKGATE_MASK	(0x3f<<ANATOP_PFD_480_PFD2_CLKGATE_SHIFT)
#define ANATOP_PFD_480_PFD3_FRAC_SHIFT		24
#define ANATOP_PFD_480_PFD3_FRAC_MASK		(1<<ANATOP_PFD_480_PFD3_FRAC_SHIFT)
#define ANATOP_PFD_480_PFD3_STABLE_SHIFT	30
#define ANATOP_PFD_480_PFD3_STABLE_MASK		(1<<ANATOP_PFD_480_PFD3_STABLE_SHIFT)
#define ANATOP_PFD_480_PFD3_CLKGATE_SHIFT	31

struct iomuxc_base_regs {
	u32     gpr[14];        /* 0x000 */
	u32     obsrv[5];       /* 0x038 */
	u32     swmux_ctl[197]; /* 0x04c */
	u32     swpad_ctl[250]; /* 0x360 */
	u32     swgrp[26];      /* 0x748 */
	u32     daisy[104];     /* 0x7b0..94c */
};

#endif /* __ASSEMBLER__*/
#endif /* __ASM_ARCH_MX6_IMX_REGS_H__ */
