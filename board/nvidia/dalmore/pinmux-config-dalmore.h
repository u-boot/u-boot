/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PINMUX_CONFIG_DALMORE_H_
#define _PINMUX_CONFIG_DALMORE_H_

#define DEFAULT_PINMUX(_pingroup, _mux, _pull, _tri, _io)	\
	{							\
		.pingroup	= PINGRP_##_pingroup,		\
		.func		= PMUX_FUNC_##_mux,		\
		.pull		= PMUX_PULL_##_pull,		\
		.tristate	= PMUX_TRI_##_tri,		\
		.io		= PMUX_PIN_##_io,		\
		.lock		= PMUX_PIN_LOCK_DEFAULT,	\
		.od		= PMUX_PIN_OD_DEFAULT,		\
		.ioreset	= PMUX_PIN_IO_RESET_DEFAULT,	\
	}

#define I2C_PINMUX(_pingroup, _mux, _pull, _tri, _io, _lock, _od) \
	{							\
		.pingroup	= PINGRP_##_pingroup,		\
		.func		= PMUX_FUNC_##_mux,		\
		.pull		= PMUX_PULL_##_pull,		\
		.tristate	= PMUX_TRI_##_tri,		\
		.io		= PMUX_PIN_##_io,		\
		.lock		= PMUX_PIN_LOCK_##_lock,	\
		.od		= PMUX_PIN_OD_##_od,		\
		.ioreset	= PMUX_PIN_IO_RESET_DEFAULT,	\
	}

#define DDC_PINMUX(_pingroup, _mux, _pull, _tri, _io, _lock, _rcv_sel) \
	{							\
		.pingroup	= PINGRP_##_pingroup,		\
		.func		= PMUX_FUNC_##_mux,		\
		.pull		= PMUX_PULL_##_pull,		\
		.tristate	= PMUX_TRI_##_tri,		\
		.io		= PMUX_PIN_##_io,		\
		.lock		= PMUX_PIN_LOCK_##_lock,	\
		.rcv_sel	= PMUX_PIN_RCV_SEL_##_rcv_sel,	\
		.ioreset	= PMUX_PIN_IO_RESET_DEFAULT,	\
	}

#define VI_PINMUX(_pingroup, _mux, _pull, _tri, _io, _lock, _ioreset) \
	{							\
		.pingroup	= PINGRP_##_pingroup,		\
		.func		= PMUX_FUNC_##_mux,		\
		.pull		= PMUX_PULL_##_pull,		\
		.tristate	= PMUX_TRI_##_tri,		\
		.io		= PMUX_PIN_##_io,		\
		.lock		= PMUX_PIN_LOCK_##_lock,	\
		.od		= PMUX_PIN_OD_DEFAULT,		\
		.ioreset	= PMUX_PIN_IO_RESET_##_ioreset	\
	}

#define CEC_PINMUX(_pingroup, _mux, _pull, _tri, _io, _lock, _od)	\
	{								\
		.pingroup   = PINGRP_##_pingroup,			\
		.func       = PMUX_FUNC_##_mux,				\
		.pull       = PMUX_PULL_##_pull,			\
		.tristate   = PMUX_TRI_##_tri,				\
		.io         = PMUX_PIN_##_io,				\
		.lock       = PMUX_PIN_LOCK_##_lock,			\
		.od         = PMUX_PIN_OD_##_od,			\
		.ioreset    = PMUX_PIN_IO_RESET_DEFAULT,		\
	}

#define USB_PINMUX CEC_PINMUX

#define DEFAULT_PADCFG(_padgrp, _slwf, _slwr, _drvup, _drvdn, _lpmd, _schmt, _hsm) \
	{						\
		.padgrp = PDRIVE_PINGROUP_##_padgrp,	\
		.slwf   = _slwf,			\
		.slwr   = _slwr,			\
		.drvup  = _drvup,			\
		.drvdn  = _drvdn,			\
		.lpmd   = PGRP_LPMD_##_lpmd,		\
		.schmt  = PGRP_SCHMT_##_schmt,		\
		.hsm    = PGRP_HSM_##_hsm,		\
	}

static struct pingroup_config tegra114_pinmux_common[] = {
	/* EXTPERIPH1 pinmux */
	DEFAULT_PINMUX(CLK1_OUT,      EXTPERIPH1,  NORMAL,    NORMAL,   OUTPUT),

	/* I2S0 pinmux */
	DEFAULT_PINMUX(DAP1_DIN,      I2S0,        NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(DAP1_DOUT,     I2S0,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(DAP1_FS,       I2S0,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(DAP1_SCLK,     I2S0,        NORMAL,    NORMAL,   INPUT),

	/* I2S1 pinmux */
	DEFAULT_PINMUX(DAP2_DIN,      I2S1,        NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(DAP2_DOUT,     I2S1,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(DAP2_FS,       I2S1,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(DAP2_SCLK,     I2S1,        NORMAL,    NORMAL,   INPUT),

	/* I2S3 pinmux */
	DEFAULT_PINMUX(DAP4_DIN,      I2S3,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(DAP4_DOUT,     I2S3,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(DAP4_FS,       I2S3,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(DAP4_SCLK,     I2S3,        NORMAL,    NORMAL,   INPUT),

	/* CLDVFS pinmux */
	DEFAULT_PINMUX(DVFS_PWM,      CLDVFS,      NORMAL,    NORMAL,   OUTPUT),
	DEFAULT_PINMUX(DVFS_CLK,      CLDVFS,      NORMAL,    NORMAL,   OUTPUT),

	/* ULPI pinmux */
	DEFAULT_PINMUX(ULPI_CLK,      ULPI,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(ULPI_DATA0,    ULPI,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(ULPI_DATA1,    ULPI,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(ULPI_DATA2,    ULPI,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(ULPI_DATA3,    ULPI,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(ULPI_DATA4,    ULPI,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(ULPI_DATA5,    ULPI,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(ULPI_DATA6,    ULPI,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(ULPI_DATA7,    ULPI,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(ULPI_DIR,      ULPI,        NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(ULPI_NXT,      ULPI,        NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(ULPI_STP,      ULPI,        NORMAL,    NORMAL,   OUTPUT),

	/* I2C3 pinmux */
	I2C_PINMUX(CAM_I2C_SCL, I2C3, NORMAL, NORMAL, INPUT, DEFAULT, DISABLE),
	I2C_PINMUX(CAM_I2C_SDA, I2C3, NORMAL, NORMAL, INPUT, DEFAULT, DISABLE),

	/* VI pinmux */
	VI_PINMUX(CAM_MCLK, VI_ALT3,  NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT),

	/* VI_ALT1 pinmux */
	VI_PINMUX(GPIO_PBB0, VI_ALT3, NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT),

	/* VGP4 pinmux */
	VI_PINMUX(GPIO_PBB4, VGP4,    NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT),

	/* I2C2 pinmux */
	I2C_PINMUX(GEN2_I2C_SCL, I2C2, NORMAL, NORMAL, INPUT, DEFAULT, DISABLE),
	I2C_PINMUX(GEN2_I2C_SDA, I2C2, NORMAL, NORMAL, INPUT, DEFAULT, DISABLE),

	/* UARTD pinmux */
	DEFAULT_PINMUX(GMI_A16,       UARTD,       NORMAL,    NORMAL,   OUTPUT),
	DEFAULT_PINMUX(GMI_A17,       UARTD,       NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(GMI_A18,       UARTD,       NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(GMI_A19,       UARTD,       NORMAL,    NORMAL,   OUTPUT),

	/* SPI4 pinmux */
	DEFAULT_PINMUX(GMI_AD5,       SPI4,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(GMI_AD6,       SPI4,        UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(GMI_AD7,       SPI4,        UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(GMI_AD12,      RSVD1,       NORMAL,    NORMAL,   OUTPUT),
	DEFAULT_PINMUX(GMI_CS6_N,     SPI4,        NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(GMI_WR_N,      SPI4,        NORMAL,    NORMAL,   INPUT),

	/* PWM1 pinmux */
	DEFAULT_PINMUX(GMI_AD9,       PWM1,        NORMAL,    NORMAL,   OUTPUT),

	/* SOC pinmux */
	DEFAULT_PINMUX(GMI_CS1_N,     SOC,         NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(GMI_OE_N,      SOC,         NORMAL,    TRISTATE, INPUT),

	/* EXTPERIPH2 pinmux */
	DEFAULT_PINMUX(CLK2_OUT,      EXTPERIPH2,  NORMAL,    NORMAL,   OUTPUT),

	/* SDMMC1 pinmux */
	DEFAULT_PINMUX(SDMMC1_CLK,    SDMMC1,      NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC1_CMD,    SDMMC1,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC1_DAT0,   SDMMC1,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC1_DAT1,   SDMMC1,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC1_DAT2,   SDMMC1,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC1_DAT3,   SDMMC1,      UP,        NORMAL,   INPUT),

	/* SDMMC3 pinmux */
	DEFAULT_PINMUX(SDMMC3_CLK,    SDMMC3,      NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC3_CMD,    SDMMC3,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC3_DAT0,   SDMMC3,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC3_DAT1,   SDMMC3,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC3_DAT2,   SDMMC3,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC3_DAT3,   SDMMC3,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC3_CLK_LB_IN,  SDMMC3,  UP,        TRISTATE, INPUT),
	DEFAULT_PINMUX(SDMMC3_CLK_LB_OUT, SDMMC3,  DOWN,      NORMAL,   INPUT),

	/* SDMMC4 pinmux */
	DEFAULT_PINMUX(SDMMC4_CLK,    SDMMC4,      NORMAL,    NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC4_CMD,    SDMMC4,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC4_DAT0,   SDMMC4,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC4_DAT1,   SDMMC4,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC4_DAT2,   SDMMC4,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC4_DAT3,   SDMMC4,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC4_DAT4,   SDMMC4,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC4_DAT5,   SDMMC4,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC4_DAT6,   SDMMC4,      UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(SDMMC4_DAT7,   SDMMC4,      UP,        NORMAL,   INPUT),

	/* BLINK pinmux */
	DEFAULT_PINMUX(CLK_32K_OUT,   BLINK,       NORMAL,    NORMAL,   OUTPUT),

	/* KBC pinmux */
	DEFAULT_PINMUX(KB_COL0,       KBC,         UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(KB_COL1,       KBC,         UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(KB_COL2,       KBC,         UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(KB_ROW0,       KBC,         UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(KB_ROW1,       KBC,         UP,        NORMAL,   INPUT),
	DEFAULT_PINMUX(KB_ROW2,       KBC,         UP,        NORMAL,   INPUT),

	/*Audio Codec*/
	DEFAULT_PINMUX(DAP3_DIN,      RSVD1,       NORMAL,    TRISTATE, OUTPUT),
	DEFAULT_PINMUX(DAP3_SCLK,     RSVD1,       NORMAL,    TRISTATE, OUTPUT),
	DEFAULT_PINMUX(GPIO_PV0,      RSVD1,       NORMAL,    TRISTATE, OUTPUT),
	DEFAULT_PINMUX(KB_ROW7,       RSVD1,       UP,        NORMAL,   INPUT),

	/* UARTA pinmux */
	DEFAULT_PINMUX(KB_ROW10,      UARTA,       NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(KB_ROW9,       UARTA,       NORMAL,    NORMAL,   OUTPUT),

	/* I2CPWR pinmux (I2C5) */
	I2C_PINMUX(PWR_I2C_SCL, I2CPWR, NORMAL, NORMAL, INPUT, DEFAULT, DISABLE),
	I2C_PINMUX(PWR_I2C_SDA, I2CPWR, NORMAL, NORMAL, INPUT, DEFAULT, DISABLE),

	/* SYSCLK pinmux */
	DEFAULT_PINMUX(SYS_CLK_REQ,   SYSCLK,      NORMAL,    NORMAL,   OUTPUT),

	/* RTCK pinmux */
	DEFAULT_PINMUX(JTAG_RTCK,     RTCK,        NORMAL,    NORMAL,   INPUT),

	/* CLK pinmux */
	DEFAULT_PINMUX(CLK_32K_IN,    CLK,         NORMAL,    TRISTATE, INPUT),

	/* PWRON pinmux */
	DEFAULT_PINMUX(CORE_PWR_REQ,  PWRON,       NORMAL,    NORMAL,   OUTPUT),

	/* CPU pinmux */
	DEFAULT_PINMUX(CPU_PWR_REQ,   CPU,         NORMAL,    NORMAL,   OUTPUT),

	/* PMI pinmux */
	DEFAULT_PINMUX(PWR_INT_N,     PMI,         NORMAL,    TRISTATE, INPUT),

	/* RESET_OUT_N pinmux */
	DEFAULT_PINMUX(RESET_OUT_N,   RESET_OUT_N, NORMAL,    NORMAL,   OUTPUT),

	/* EXTPERIPH3 pinmux */
	DEFAULT_PINMUX(CLK3_OUT,      EXTPERIPH3,  NORMAL,    NORMAL,   OUTPUT),

	/* I2C1 pinmux */
	I2C_PINMUX(GEN1_I2C_SCL, I2C1, NORMAL, NORMAL, INPUT, DEFAULT, DISABLE),
	I2C_PINMUX(GEN1_I2C_SDA, I2C1, NORMAL, NORMAL, INPUT, DEFAULT, DISABLE),

	/* UARTB pinmux */
	DEFAULT_PINMUX(UART2_CTS_N,   UARTB,       NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(UART2_RTS_N,   UARTB,       NORMAL,    NORMAL,   OUTPUT),

	/* IRDA pinmux */
	DEFAULT_PINMUX(UART2_RXD,     UARTB,       NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(UART2_TXD,     UARTB,       NORMAL,    NORMAL,   OUTPUT),

	/* UARTC pinmux */
	DEFAULT_PINMUX(UART3_CTS_N,   UARTC,       NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(UART3_RTS_N,   UARTC,       NORMAL,    NORMAL,   OUTPUT),
	DEFAULT_PINMUX(UART3_RXD,     UARTC,       NORMAL,    TRISTATE, INPUT),
	DEFAULT_PINMUX(UART3_TXD,     UARTC,       NORMAL,    NORMAL,   OUTPUT),

	/* OWR pinmux */
	DEFAULT_PINMUX(OWR,           OWR,         NORMAL,    NORMAL,   INPUT),

	/* CEC pinmux */
	CEC_PINMUX(HDMI_CEC, CEC, NORMAL, NORMAL, INPUT, DEFAULT, DISABLE),

	/* I2C4 pinmux */
	DDC_PINMUX(DDC_SCL, I2C4, NORMAL, NORMAL, INPUT, DEFAULT, HIGH),
	DDC_PINMUX(DDC_SDA, I2C4, NORMAL, NORMAL, INPUT, DEFAULT, HIGH),

	/* USB pinmux */
	USB_PINMUX(USB_VBUS_EN0, USB, NORMAL, NORMAL, INPUT, DEFAULT, ENABLE),

	/* nct */
	DEFAULT_PINMUX(GPIO_X6_AUD,   SPI6,        UP,        TRISTATE, INPUT),
};

static struct pingroup_config unused_pins_lowpower[] = {
	DEFAULT_PINMUX(CLK1_REQ,      RSVD3,       DOWN, TRISTATE, OUTPUT),
	DEFAULT_PINMUX(USB_VBUS_EN1,  RSVD3,       DOWN, TRISTATE, OUTPUT),
};

/* Initially setting all used GPIO's to non-TRISTATE */
static struct pingroup_config tegra114_pinmux_set_nontristate[] = {
	DEFAULT_PINMUX(GPIO_X4_AUD,     RSVD1,  DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GPIO_X5_AUD,     RSVD1,  UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GPIO_X6_AUD,     RSVD3,  UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GPIO_X7_AUD,     RSVD1,  DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GPIO_W2_AUD,     RSVD1,  UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GPIO_W3_AUD,     SPI6,   UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GPIO_X1_AUD,     RSVD3,  DOWN,    NORMAL,    INPUT),
	DEFAULT_PINMUX(GPIO_X3_AUD,     RSVD3,  UP,      NORMAL,    INPUT),

	DEFAULT_PINMUX(DAP3_FS,         I2S2,   DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(DAP3_DIN,        I2S2,   DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(DAP3_DOUT,       I2S2,   DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(DAP3_SCLK,       I2S2,   DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GPIO_PV0,        RSVD3,  NORMAL,  NORMAL,    INPUT),
	DEFAULT_PINMUX(GPIO_PV1,        RSVD1,  NORMAL,  NORMAL,    INPUT),

	DEFAULT_PINMUX(GPIO_PBB3,       RSVD3,  DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GPIO_PBB5,       RSVD3,  DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GPIO_PBB6,       RSVD3,  DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GPIO_PBB7,       RSVD3,  DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GPIO_PCC1,       RSVD3,  DOWN,    NORMAL,    INPUT),
	DEFAULT_PINMUX(GPIO_PCC2,       RSVD3,  DOWN,    NORMAL,    INPUT),

	DEFAULT_PINMUX(GMI_AD0,         GMI,    NORMAL,  NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GMI_AD1,         GMI,    NORMAL,  NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GMI_AD10,        GMI,    DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GMI_AD11,        GMI,    DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GMI_AD12,        GMI,    UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_AD13,        GMI,    DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GMI_AD2,         GMI,    NORMAL,  NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_AD3,         GMI,    NORMAL,  NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_AD8,         GMI,    DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GMI_ADV_N,       GMI,    UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_CLK,         GMI,    DOWN,    NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GMI_CS0_N,       GMI,    UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_CS2_N,       GMI,    UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_CS3_N,       GMI,    UP,      NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GMI_CS4_N,       GMI,    UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_CS7_N,       GMI,    UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_DQS,         GMI,    UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_IORDY,       GMI,    UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(GMI_WP_N,        GMI,    UP,      NORMAL,    INPUT),

	DEFAULT_PINMUX(SDMMC1_WP_N,     SPI4,   UP,      NORMAL,    OUTPUT),
	DEFAULT_PINMUX(CLK2_REQ,        RSVD3,  NORMAL,  NORMAL,    OUTPUT),

	DEFAULT_PINMUX(KB_COL3,         KBC,    UP,      NORMAL,    OUTPUT),
	DEFAULT_PINMUX(KB_COL4,		SDMMC3, UP,	 NORMAL,    INPUT),
	DEFAULT_PINMUX(KB_COL5,         KBC,    UP,      NORMAL,    INPUT),
	DEFAULT_PINMUX(KB_COL6,         KBC,    UP,      NORMAL,    OUTPUT),
	DEFAULT_PINMUX(KB_COL7,         KBC,    UP,      NORMAL,    OUTPUT),
	DEFAULT_PINMUX(KB_ROW3,         KBC,    DOWN,    NORMAL,    INPUT),
	DEFAULT_PINMUX(KB_ROW4,         KBC,    DOWN,    NORMAL,    INPUT),
	DEFAULT_PINMUX(KB_ROW6,         KBC,    DOWN,    NORMAL,    INPUT),
	DEFAULT_PINMUX(KB_ROW8,         KBC,    UP,      NORMAL,    INPUT),

	DEFAULT_PINMUX(CLK3_REQ,        RSVD3,  NORMAL,  NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GPIO_PU4,        RSVD3,  NORMAL,  NORMAL,    OUTPUT),
	DEFAULT_PINMUX(GPIO_PU5,        RSVD3,  NORMAL,  NORMAL,    INPUT),
	DEFAULT_PINMUX(GPIO_PU6,        RSVD3,  NORMAL,  NORMAL,    INPUT),

	DEFAULT_PINMUX(HDMI_INT,        RSVD1,   DOWN,    NORMAL,   INPUT),

	DEFAULT_PINMUX(GMI_AD9,         PWM1,   NORMAL,   NORMAL,   OUTPUT),
	DEFAULT_PINMUX(SPDIF_IN,	USB,	NORMAL,   NORMAL,   INPUT),

	DEFAULT_PINMUX(SDMMC3_CD_N,     SDMMC3, UP,       NORMAL,   INPUT),
};

static struct padctrl_config dalmore_padctrl[] = {
	/* (_padgrp, _slwf, _slwr, _drvup, _drvdn, _lpmd, _schmt, _hsm) */
	DEFAULT_PADCFG(SDIO3, SDIOCFG_DRVUP_SLWF, SDIOCFG_DRVDN_SLWR, \
		SDIOCFG_DRVUP, SDIOCFG_DRVDN, NONE, NONE, NONE),
};
#endif /* PINMUX_CONFIG_COMMON_H */
