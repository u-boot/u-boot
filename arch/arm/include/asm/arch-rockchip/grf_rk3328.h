/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SOC_ROCKCHIP_RK3328_GRF_H__
#define __SOC_ROCKCHIP_RK3328_GRF_H__

struct rk3328_grf_regs {
	u32 gpio0a_iomux;
	u32 gpio0b_iomux;
	u32 gpio0c_iomux;
	u32 gpio0d_iomux;
	u32 gpio1a_iomux;
	u32 gpio1b_iomux;
	u32 gpio1c_iomux;
	u32 gpio1d_iomux;
	u32 gpio2a_iomux;
	u32 gpio2bl_iomux;
	u32 gpio2bh_iomux;
	u32 gpio2cl_iomux;
	u32 gpio2ch_iomux;
	u32 gpio2d_iomux;
	u32 gpio3al_iomux;
	u32 gpio3ah_iomux;
	u32 gpio3bl_iomux;
	u32 gpio3bh_iomux;
	u32 gpio3c_iomux;
	u32 gpio3d_iomux;
	u32 com_iomux;
	u32 reserved1[(0x100 - 0x54) / 4];

	u32 gpio0a_p;
	u32 gpio0b_p;
	u32 gpio0c_p;
	u32 gpio0d_p;
	u32 gpio1a_p;
	u32 gpio1b_p;
	u32 gpio1c_p;
	u32 gpio1d_p;
	u32 gpio2a_p;
	u32 gpio2b_p;
	u32 gpio2c_p;
	u32 gpio2d_p;
	u32 gpio3a_p;
	u32 gpio3b_p;
	u32 gpio3c_p;
	u32 gpio3d_p;
	u32 reserved2[(0x200 - 0x140) / 4];
	u32 gpio0a_e;
	u32 gpio0b_e;
	u32 gpio0c_e;
	u32 gpio0d_e;
	u32 gpio1a_e;
	u32 gpio1b_e;
	u32 gpio1c_e;
	u32 gpio1d_e;
	u32 gpio2a_e;
	u32 gpio2b_e;
	u32 gpio2c_e;
	u32 gpio2d_e;
	u32 gpio3a_e;
	u32 gpio3b_e;
	u32 gpio3c_e;
	u32 gpio3d_e;
	u32 reserved3[(0x300 - 0x240) / 4];
	u32 gpio0l_sr;
	u32 gpio0h_sr;
	u32 gpio1l_sr;
	u32 gpio1h_sr;
	u32 gpio2l_sr;
	u32 gpio2h_sr;
	u32 gpio3l_sr;
	u32 gpio3h_sr;
	u32 reserved4[(0x380 - 0x320) / 4];
	u32 gpio0l_smt;
	u32 gpio0h_smt;
	u32 gpio1l_smt;
	u32 gpio1h_smt;
	u32 gpio2l_smt;
	u32 gpio2h_smt;
	u32 gpio3l_smt;
	u32 gpio3h_smt;
	u32 reserved5[(0x400 - 0x3a0) / 4];
	u32 soc_con[11];
	u32 reserved6[(0x480 - 0x42c) / 4];
	u32 soc_status[5];
	u32 reserved7[(0x4c0 - 0x494) / 4];
	u32 otg3_con[2];
	u32 reserved8[(0x500 - 0x4c8) / 4];
	u32 cpu_con[2];
	u32 reserved9[(0x520 - 0x508) / 4];
	u32 cpu_status[2];
	u32 reserved10[(0x5c8 - 0x528) / 4];
	u32 os_reg[8];
	u32 reserved11[(0x680 - 0x5e8) / 4];
	u32 sig_detect_con;
	u32 reserved12[3];
	u32 sig_detect_status;
	u32 reserved13[3];
	u32 sig_detect_status_clr;
	u32 reserved14[3];

	u32 sdmmc_det_counter;
	u32 reserved15[(0x700 - 0x6b4) / 4];
	u32 host0_con[3];
	u32 reserved16[(0x880 - 0x70c) / 4];
	u32 otg_con0;
	u32 reserved17[3];
	u32 host0_status;
	u32 reserved18[(0x900 - 0x894) / 4];
	u32 mac_con[3];
	u32 reserved19[(0xb00 - 0x90c) / 4];
	u32 macphy_con[4];
	u32 macphy_status;
};
check_member(rk3328_grf_regs, macphy_status, 0xb10);

struct rk3328_sgrf_regs {
	u32 soc_con[6];
	u32 reserved0[(0x100 - 0x18) / 4];
	u32 dmac_con[6];
	u32 reserved1[(0x180 - 0x118) / 4];
	u32 fast_boot_addr;
	u32 reserved2[(0x200 - 0x184) / 4];
	u32 chip_fuse_con;
	u32 reserved3[(0x280 - 0x204) / 4];
	u32 hdcp_key_reg[8];
	u32 hdcp_key_access_mask;
};
check_member(rk3328_sgrf_regs, hdcp_key_access_mask, 0x2a0);

enum {
	/* GPIO0A_IOMUX */
	GPIO0A5_SEL_SHIFT	= 10,
	GPIO0A5_SEL_MASK	= 3 << GPIO0A5_SEL_SHIFT,
	GPIO0A5_I2C3_SCL	= 2,

	GPIO0A6_SEL_SHIFT	= 12,
	GPIO0A6_SEL_MASK	= 3 << GPIO0A6_SEL_SHIFT,
	GPIO0A6_I2C3_SDA	= 2,

	GPIO0A7_SEL_SHIFT	= 14,
	GPIO0A7_SEL_MASK	= 3 << GPIO0A7_SEL_SHIFT,
	GPIO0A7_EMMC_DATA0	= 2,

	/* GPIO0D_IOMUX*/
	GPIO0D6_SEL_SHIFT	= 12,
	GPIO0D6_SEL_MASK	= 3 << GPIO0D6_SEL_SHIFT,
	GPIO0D6_GPIO		= 0,
	GPIO0D6_SDMMC0_PWRENM1	= 3,

	/* GPIO1A_IOMUX */
	GPIO1A0_SEL_SHIFT	= 0,
	GPIO1A0_SEL_MASK	= 0x3fff << GPIO1A0_SEL_SHIFT,
	GPIO1A0_CARD_DATA_CLK_CMD_DETN	= 0x1555,

	/* GPIO2A_IOMUX */
	GPIO2A0_SEL_SHIFT	= 0,
	GPIO2A0_SEL_MASK	= 3 << GPIO2A0_SEL_SHIFT,
	GPIO2A0_UART2_TX_M1	= 1,

	GPIO2A1_SEL_SHIFT	= 2,
	GPIO2A1_SEL_MASK	= 3 << GPIO2A1_SEL_SHIFT,
	GPIO2A1_UART2_RX_M1	= 1,

	GPIO2A2_SEL_SHIFT	= 4,
	GPIO2A2_SEL_MASK	= 3 << GPIO2A2_SEL_SHIFT,
	GPIO2A2_PWM_IR		= 1,

	GPIO2A4_SEL_SHIFT	= 8,
	GPIO2A4_SEL_MASK	= 3 << GPIO2A4_SEL_SHIFT,
	GPIO2A4_PWM_0		= 1,
	GPIO2A4_I2C1_SDA,

	GPIO2A5_SEL_SHIFT	= 10,
	GPIO2A5_SEL_MASK	= 3 << GPIO2A5_SEL_SHIFT,
	GPIO2A5_PWM_1		= 1,
	GPIO2A5_I2C1_SCL,

	GPIO2A6_SEL_SHIFT	= 12,
	GPIO2A6_SEL_MASK	= 3 << GPIO2A6_SEL_SHIFT,
	GPIO2A6_PWM_2		= 1,

	GPIO2A7_SEL_SHIFT	= 14,
	GPIO2A7_SEL_MASK	= 3 << GPIO2A7_SEL_SHIFT,
	GPIO2A7_GPIO		= 0,
	GPIO2A7_SDMMC0_PWRENM0,

	/* GPIO2BL_IOMUX */
	GPIO2BL0_SEL_SHIFT	= 0,
	GPIO2BL0_SEL_MASK	= 0x3f << GPIO2BL0_SEL_SHIFT,
	GPIO2BL0_SPI_CLK_TX_RX_M0	= 0x15,

	GPIO2BL3_SEL_SHIFT	= 6,
	GPIO2BL3_SEL_MASK	= 3 << GPIO2BL3_SEL_SHIFT,
	GPIO2BL3_SPI_CSN0_M0	= 1,

	GPIO2BL4_SEL_SHIFT	= 8,
	GPIO2BL4_SEL_MASK	= 3 << GPIO2BL4_SEL_SHIFT,
	GPIO2BL4_SPI_CSN1_M0	= 1,

	GPIO2BL5_SEL_SHIFT	= 10,
	GPIO2BL5_SEL_MASK	= 3 << GPIO2BL5_SEL_SHIFT,
	GPIO2BL5_I2C2_SDA	= 1,

	GPIO2BL6_SEL_SHIFT	= 12,
	GPIO2BL6_SEL_MASK	= 3 << GPIO2BL6_SEL_SHIFT,
	GPIO2BL6_I2C2_SCL	= 1,

	/* GPIO2D_IOMUX */
	GPIO2D0_SEL_SHIFT	= 0,
	GPIO2D0_SEL_MASK	= 3 << GPIO2D0_SEL_SHIFT,
	GPIO2D0_I2C0_SCL	= 1,

	GPIO2D1_SEL_SHIFT	= 2,
	GPIO2D1_SEL_MASK	= 3 << GPIO2D1_SEL_SHIFT,
	GPIO2D1_I2C0_SDA	= 1,

	GPIO2D4_SEL_SHIFT	= 8,
	GPIO2D4_SEL_MASK	= 0xff << GPIO2D4_SEL_SHIFT,
	GPIO2D4_EMMC_DATA1234	= 0xaa,

	/* GPIO3C_IOMUX */
	GPIO3C0_SEL_SHIFT	= 0,
	GPIO3C0_SEL_MASK	= 0x3fff << GPIO3C0_SEL_SHIFT,
	GPIO3C0_EMMC_DATA567_PWR_CLK_RSTN_CMD	= 0x2aaa,

	/* COM_IOMUX */
	IOMUX_SEL_UART2_SHIFT	= 0,
	IOMUX_SEL_UART2_MASK	= 3 << IOMUX_SEL_UART2_SHIFT,
	IOMUX_SEL_UART2_M0	= 0,
	IOMUX_SEL_UART2_M1,

	IOMUX_SEL_SPI_SHIFT	= 4,
	IOMUX_SEL_SPI_MASK	= 3 << IOMUX_SEL_SPI_SHIFT,
	IOMUX_SEL_SPI_M0	= 0,
	IOMUX_SEL_SPI_M1,
	IOMUX_SEL_SPI_M2,

	IOMUX_SEL_SDMMC_SHIFT	= 7,
	IOMUX_SEL_SDMMC_MASK	= 1 << IOMUX_SEL_SDMMC_SHIFT,
	IOMUX_SEL_SDMMC_M0	= 0,
	IOMUX_SEL_SDMMC_M1,
};

#endif	/* __SOC_ROCKCHIP_RK3328_GRF_H__ */
