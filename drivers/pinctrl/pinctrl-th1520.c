// SPDX-License-Identifier: GPL-2.0
/*
 * Pinctrl driver for the T-Head TH1520 SoC
 *
 * Copyright (C) 2023 Emil Renner Berthing <emil.renner.berthing@canonical.com>
 * Copyright (C) 2025 Yao Zi <ziyao@disroot.org>
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/string.h>
#include <malloc.h>

#define TH1520_PADCFG_IE	BIT(9)
#define TH1520_PADCFG_SL	BIT(8)
#define TH1520_PADCFG_ST	BIT(7)
#define TH1520_PADCFG_SPU	BIT(6)
#define TH1520_PADCFG_PS	BIT(5)
#define TH1520_PADCFG_PE	BIT(4)
#define TH1520_PADCFG_BIAS	(TH1520_PADCFG_SPU | TH1520_PADCFG_PS | TH1520_PADCFG_PE)
#define TH1520_PADCFG_DS	GENMASK(3, 0)

#define TH1520_PULL_DOWN_OHM	44000 /* typ. 44kOhm */
#define TH1520_PULL_UP_OHM	48000 /* typ. 48kOhm */
#define TH1520_PULL_STRONG_OHM	 2100 /* typ. 2.1kOhm */

#define TH1520_PAD_NO_PADCFG	BIT(0)

enum th1520_muxtype {
	TH1520_MUX_____,
	TH1520_MUX_GPIO,
	TH1520_MUX_PWM,
	TH1520_MUX_UART,
	TH1520_MUX_IR,
	TH1520_MUX_I2C,
	TH1520_MUX_SPI,
	TH1520_MUX_QSPI,
	TH1520_MUX_SDIO,
	TH1520_MUX_AUD,
	TH1520_MUX_I2S,
	TH1520_MUX_MAC0,
	TH1520_MUX_MAC1,
	TH1520_MUX_DPU0,
	TH1520_MUX_DPU1,
	TH1520_MUX_ISP,
	TH1520_MUX_HDMI,
	TH1520_MUX_BSEL,
	TH1520_MUX_DBG,
	TH1520_MUX_CLK,
	TH1520_MUX_JTAG,
	TH1520_MUX_ISO,
	TH1520_MUX_FUSE,
	TH1520_MUX_RST,
};

static const char *const th1520_muxtype_string[] = {
	[TH1520_MUX_GPIO] = "gpio",
	[TH1520_MUX_PWM]  = "pwm",
	[TH1520_MUX_UART] = "uart",
	[TH1520_MUX_IR]   = "ir",
	[TH1520_MUX_I2C]  = "i2c",
	[TH1520_MUX_SPI]  = "spi",
	[TH1520_MUX_QSPI] = "qspi",
	[TH1520_MUX_SDIO] = "sdio",
	[TH1520_MUX_AUD]  = "audio",
	[TH1520_MUX_I2S]  = "i2s",
	[TH1520_MUX_MAC0] = "gmac0",
	[TH1520_MUX_MAC1] = "gmac1",
	[TH1520_MUX_DPU0] = "dpu0",
	[TH1520_MUX_DPU1] = "dpu1",
	[TH1520_MUX_ISP]  = "isp",
	[TH1520_MUX_HDMI] = "hdmi",
	[TH1520_MUX_BSEL] = "bootsel",
	[TH1520_MUX_DBG]  = "debug",
	[TH1520_MUX_CLK]  = "clock",
	[TH1520_MUX_JTAG] = "jtag",
	[TH1520_MUX_ISO]  = "iso7816",
	[TH1520_MUX_FUSE] = "efuse",
	[TH1520_MUX_RST]  = "reset",
};

struct th1520_pin_desc {
	unsigned int number;
	const char *name;
	enum th1520_muxtype muxes[6];
	u8 flags;
};

struct th1520_pad_group {
	unsigned int npins;
	const struct th1520_pin_desc *pins;
	const char *name;
};

struct th1520_pinctrl {
	const struct th1520_pad_group *group;
	void __iomem *base;
	struct pinctrl_dev *pctl;
};

static enum th1520_muxtype th1520_muxtype_get(const char *str)
{
	enum th1520_muxtype mt;

	for (mt = TH1520_MUX_GPIO; mt < ARRAY_SIZE(th1520_muxtype_string); mt++) {
		if (!strcmp(str, th1520_muxtype_string[mt]))
			return mt;
	}
	return TH1520_MUX_____;
}

#define TH1520_PAD(_nr, _name, m0, m1, m2, m3, m4, m5, _flags)	\
	{							\
		.number = _nr,					\
		.name = #_name,					\
		.muxes = {					\
			TH1520_MUX_##m0, TH1520_MUX_##m1,	\
			TH1520_MUX_##m2, TH1520_MUX_##m3,	\
			TH1520_MUX_##m4, TH1520_MUX_##m5	\
		},						\
		.flags = _flags,				\
	}

static bool th1520_pad_no_padcfg(const struct th1520_pin_desc *pin)
{
	return pin->flags & TH1520_PAD_NO_PADCFG;
}

static const struct th1520_pin_desc th1520_group1_pins[] = {
	TH1520_PAD(0,  OSC_CLK_IN,    ____, ____, ____, ____, ____, ____, TH1520_PAD_NO_PADCFG),
	TH1520_PAD(1,  OSC_CLK_OUT,   ____, ____, ____, ____, ____, ____, TH1520_PAD_NO_PADCFG),
	TH1520_PAD(2,  SYS_RST_N,     ____, ____, ____, ____, ____, ____, TH1520_PAD_NO_PADCFG),
	TH1520_PAD(3,  RTC_CLK_IN,    ____, ____, ____, ____, ____, ____, TH1520_PAD_NO_PADCFG),
	TH1520_PAD(4,  RTC_CLK_OUT,   ____, ____, ____, ____, ____, ____, TH1520_PAD_NO_PADCFG),
	/* skip number 5 so we can calculate register offsets and shifts from the pin number */
	TH1520_PAD(6,  TEST_MODE,     ____, ____, ____, ____, ____, ____, TH1520_PAD_NO_PADCFG),
	TH1520_PAD(7,  DEBUG_MODE,    DBG,  ____, ____, GPIO, ____, ____, TH1520_PAD_NO_PADCFG),
	TH1520_PAD(8,  POR_SEL,       ____, ____, ____, ____, ____, ____, TH1520_PAD_NO_PADCFG),
	TH1520_PAD(9,  I2C_AON_SCL,   I2C,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(10, I2C_AON_SDA,   I2C,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(11, CPU_JTG_TCLK,  JTAG, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(12, CPU_JTG_TMS,   JTAG, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(13, CPU_JTG_TDI,   JTAG, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(14, CPU_JTG_TDO,   JTAG, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(15, CPU_JTG_TRST,  JTAG, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(16, AOGPIO_7,      CLK,  AUD,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(17, AOGPIO_8,      UART, AUD,  IR,   GPIO, ____, ____, 0),
	TH1520_PAD(18, AOGPIO_9,      UART, AUD,  IR,   GPIO, ____, ____, 0),
	TH1520_PAD(19, AOGPIO_10,     CLK,  AUD,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(20, AOGPIO_11,     GPIO, AUD,  ____, ____, ____, ____, 0),
	TH1520_PAD(21, AOGPIO_12,     GPIO, AUD,  ____, ____, ____, ____, 0),
	TH1520_PAD(22, AOGPIO_13,     GPIO, AUD,  ____, ____, ____, ____, 0),
	TH1520_PAD(23, AOGPIO_14,     GPIO, AUD,  ____, ____, ____, ____, 0),
	TH1520_PAD(24, AOGPIO_15,     GPIO, AUD,  ____, ____, ____, ____, 0),
	TH1520_PAD(25, AUDIO_PA0,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(26, AUDIO_PA1,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(27, AUDIO_PA2,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(28, AUDIO_PA3,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(29, AUDIO_PA4,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(30, AUDIO_PA5,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(31, AUDIO_PA6,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(32, AUDIO_PA7,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(33, AUDIO_PA8,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(34, AUDIO_PA9,     AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(35, AUDIO_PA10,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(36, AUDIO_PA11,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(37, AUDIO_PA12,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(38, AUDIO_PA13,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(39, AUDIO_PA14,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(40, AUDIO_PA15,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(41, AUDIO_PA16,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(42, AUDIO_PA17,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(43, AUDIO_PA27,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(44, AUDIO_PA28,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(45, AUDIO_PA29,    AUD,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(46, AUDIO_PA30,    AUD,  RST,  ____, GPIO, ____, ____, 0),
};

static const struct th1520_pin_desc th1520_group2_pins[] = {
	TH1520_PAD(0,  QSPI1_SCLK,    QSPI, ISO,  ____, GPIO, FUSE, ____, 0),
	TH1520_PAD(1,  QSPI1_CSN0,    QSPI, ____, I2C,  GPIO, FUSE, ____, 0),
	TH1520_PAD(2,  QSPI1_D0_MOSI, QSPI, ISO,  I2C,  GPIO, FUSE, ____, 0),
	TH1520_PAD(3,  QSPI1_D1_MISO, QSPI, ISO,  ____, GPIO, FUSE, ____, 0),
	TH1520_PAD(4,  QSPI1_D2_WP,   QSPI, ISO,  UART, GPIO, FUSE, ____, 0),
	TH1520_PAD(5,  QSPI1_D3_HOLD, QSPI, ISO,  UART, GPIO, ____, ____, 0),
	TH1520_PAD(6,  I2C0_SCL,      I2C,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(7,  I2C0_SDA,      I2C,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(8,  I2C1_SCL,      I2C,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(9,  I2C1_SDA,      I2C,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(10, UART1_TXD,     UART, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(11, UART1_RXD,     UART, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(12, UART4_TXD,     UART, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(13, UART4_RXD,     UART, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(14, UART4_CTSN,    UART, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(15, UART4_RTSN,    UART, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(16, UART3_TXD,     DBG,  UART, ____, GPIO, ____, ____, 0),
	TH1520_PAD(17, UART3_RXD,     DBG,  UART, ____, GPIO, ____, ____, 0),
	TH1520_PAD(18, GPIO0_18,      GPIO, I2C,  ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(19, GPIO0_19,      GPIO, I2C,  ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(20, GPIO0_20,      GPIO, UART, IR,   ____, DPU0, DPU1, 0),
	TH1520_PAD(21, GPIO0_21,      GPIO, UART, IR,   ____, DPU0, DPU1, 0),
	TH1520_PAD(22, GPIO0_22,      GPIO, JTAG, I2C,  ____, DPU0, DPU1, 0),
	TH1520_PAD(23, GPIO0_23,      GPIO, JTAG, I2C,  ____, DPU0, DPU1, 0),
	TH1520_PAD(24, GPIO0_24,      GPIO, JTAG, QSPI, ____, DPU0, DPU1, 0),
	TH1520_PAD(25, GPIO0_25,      GPIO, JTAG, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(26, GPIO0_26,      GPIO, JTAG, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(27, GPIO0_27,      GPIO, ____, I2C,  ____, DPU0, DPU1, 0),
	TH1520_PAD(28, GPIO0_28,      GPIO, ____, I2C,  ____, DPU0, DPU1, 0),
	TH1520_PAD(29, GPIO0_29,      GPIO, ____, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(30, GPIO0_30,      GPIO, ____, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(31, GPIO0_31,      GPIO, ____, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(32, GPIO1_0,       GPIO, JTAG, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(33, GPIO1_1,       GPIO, JTAG, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(34, GPIO1_2,       GPIO, JTAG, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(35, GPIO1_3,       GPIO, JTAG, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(36, GPIO1_4,       GPIO, JTAG, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(37, GPIO1_5,       GPIO, ____, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(38, GPIO1_6,       GPIO, QSPI, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(39, GPIO1_7,       GPIO, QSPI, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(40, GPIO1_8,       GPIO, QSPI, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(41, GPIO1_9,       GPIO, QSPI, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(42, GPIO1_10,      GPIO, QSPI, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(43, GPIO1_11,      GPIO, QSPI, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(44, GPIO1_12,      GPIO, QSPI, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(45, GPIO1_13,      GPIO, UART, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(46, GPIO1_14,      GPIO, UART, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(47, GPIO1_15,      GPIO, UART, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(48, GPIO1_16,      GPIO, UART, ____, ____, DPU0, DPU1, 0),
	TH1520_PAD(49, CLK_OUT_0,     BSEL, CLK,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(50, CLK_OUT_1,     BSEL, CLK,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(51, CLK_OUT_2,     BSEL, CLK,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(52, CLK_OUT_3,     BSEL, CLK,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(53, GPIO1_21,      JTAG, ____, ISP,  GPIO, ____, ____, 0),
	TH1520_PAD(54, GPIO1_22,      JTAG, ____, ISP,  GPIO, ____, ____, 0),
	TH1520_PAD(55, GPIO1_23,      JTAG, ____, ISP,  GPIO, ____, ____, 0),
	TH1520_PAD(56, GPIO1_24,      JTAG, ____, ISP,  GPIO, ____, ____, 0),
	TH1520_PAD(57, GPIO1_25,      JTAG, ____, ISP,  GPIO, ____, ____, 0),
	TH1520_PAD(58, GPIO1_26,      GPIO, ____, ISP,  ____, ____, ____, 0),
	TH1520_PAD(59, GPIO1_27,      GPIO, ____, ISP,  ____, ____, ____, 0),
	TH1520_PAD(60, GPIO1_28,      GPIO, ____, ISP,  ____, ____, ____, 0),
	TH1520_PAD(61, GPIO1_29,      GPIO, ____, ISP,  ____, ____, ____, 0),
	TH1520_PAD(62, GPIO1_30,      GPIO, ____, ISP,  ____, ____, ____, 0),
};

static const struct th1520_pin_desc th1520_group3_pins[] = {
	TH1520_PAD(0,  UART0_TXD,     UART, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(1,  UART0_RXD,     UART, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(2,  QSPI0_SCLK,    QSPI, PWM,  I2S,  GPIO, ____, ____, 0),
	TH1520_PAD(3,  QSPI0_CSN0,    QSPI, PWM,  I2S,  GPIO, ____, ____, 0),
	TH1520_PAD(4,  QSPI0_CSN1,    QSPI, PWM,  I2S,  GPIO, ____, ____, 0),
	TH1520_PAD(5,  QSPI0_D0_MOSI, QSPI, PWM,  I2S,  GPIO, ____, ____, 0),
	TH1520_PAD(6,  QSPI0_D1_MISO, QSPI, PWM,  I2S,  GPIO, ____, ____, 0),
	TH1520_PAD(7,  QSPI0_D2_WP,   QSPI, PWM,  I2S,  GPIO, ____, ____, 0),
	TH1520_PAD(8,  QSPI1_D3_HOLD, QSPI, ____, I2S,  GPIO, ____, ____, 0),
	TH1520_PAD(9,  I2C2_SCL,      I2C,  UART, ____, GPIO, ____, ____, 0),
	TH1520_PAD(10, I2C2_SDA,      I2C,  UART, ____, GPIO, ____, ____, 0),
	TH1520_PAD(11, I2C3_SCL,      I2C,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(12, I2C3_SDA,      I2C,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(13, GPIO2_13,      GPIO, SPI,  ____, ____, ____, ____, 0),
	TH1520_PAD(14, SPI_SCLK,      SPI,  UART, IR,   GPIO, ____, ____, 0),
	TH1520_PAD(15, SPI_CSN,       SPI,  UART, IR,   GPIO, ____, ____, 0),
	TH1520_PAD(16, SPI_MOSI,      SPI,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(17, SPI_MISO,      SPI,  ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(18, GPIO2_18,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(19, GPIO2_19,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(20, GPIO2_20,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(21, GPIO2_21,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(22, GPIO2_22,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(23, GPIO2_23,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(24, GPIO2_24,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(25, GPIO2_25,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(26, SDIO0_WPRTN,   SDIO, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(27, SDIO0_DETN,    SDIO, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(28, SDIO1_WPRTN,   SDIO, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(29, SDIO1_DETN,    SDIO, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(30, GPIO2_30,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(31, GPIO2_31,      GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(32, GPIO3_0,       GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(33, GPIO3_1,       GPIO, MAC1, ____, ____, ____, ____, 0),
	TH1520_PAD(34, GPIO3_2,       GPIO, PWM,  ____, ____, ____, ____, 0),
	TH1520_PAD(35, GPIO3_3,       GPIO, PWM,  ____, ____, ____, ____, 0),
	TH1520_PAD(36, HDMI_SCL,      HDMI, PWM,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(37, HDMI_SDA,      HDMI, PWM,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(38, HDMI_CEC,      HDMI, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(39, GMAC0_TX_CLK,  MAC0, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(40, GMAC0_RX_CLK,  MAC0, ____, ____, GPIO, ____, ____, 0),
	TH1520_PAD(41, GMAC0_TXEN,    MAC0, UART, ____, GPIO, ____, ____, 0),
	TH1520_PAD(42, GMAC0_TXD0,    MAC0, UART, ____, GPIO, ____, ____, 0),
	TH1520_PAD(43, GMAC0_TXD1,    MAC0, UART, ____, GPIO, ____, ____, 0),
	TH1520_PAD(44, GMAC0_TXD2,    MAC0, UART, ____, GPIO, ____, ____, 0),
	TH1520_PAD(45, GMAC0_TXD3,    MAC0, I2C,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(46, GMAC0_RXDV,    MAC0, I2C,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(47, GMAC0_RXD0,    MAC0, I2C,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(48, GMAC0_RXD1,    MAC0, I2C,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(49, GMAC0_RXD2,    MAC0, SPI,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(50, GMAC0_RXD3,    MAC0, SPI,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(51, GMAC0_MDC,     MAC0, SPI,  MAC1, GPIO, ____, ____, 0),
	TH1520_PAD(52, GMAC0_MDIO,    MAC0, SPI,  MAC1, GPIO, ____, ____, 0),
	TH1520_PAD(53, GMAC0_COL,     MAC0, PWM,  ____, GPIO, ____, ____, 0),
	TH1520_PAD(54, GMAC0_CRS,     MAC0, PWM,  ____, GPIO, ____, ____, 0),
};

static const struct th1520_pad_group th1520_group1 = {
	.name = "th1520-group1",
	.pins = th1520_group1_pins,
	.npins = ARRAY_SIZE(th1520_group1_pins),
};

static const struct th1520_pad_group th1520_group2 = {
	.name = "th1520-group2",
	.pins = th1520_group2_pins,
	.npins = ARRAY_SIZE(th1520_group2_pins),
};

static const struct th1520_pad_group th1520_group3 = {
	.name = "th1520-group3",
	.pins = th1520_group3_pins,
	.npins = ARRAY_SIZE(th1520_group3_pins),
};

static void __iomem *th1520_padcfg(struct th1520_pinctrl *thp,
				   unsigned int pin)
{
	return thp->base + 4 * (pin / 2);
}

static unsigned int th1520_padcfg_shift(unsigned int pin)
{
	return 16 * (pin & BIT(0));
}

static void __iomem *th1520_muxcfg(struct th1520_pinctrl *thp,
				   unsigned int pin)
{
	return thp->base + 0x400 + 4 * (pin / 8);
}

static unsigned int th1520_muxcfg_shift(unsigned int pin)
{
	return 4 * (pin & GENMASK(2, 0));
}

static const u8 th1520_drive_strength_in_ma[16] = {
	1, 2, 3, 5, 7, 8, 10, 12, 13, 15, 16, 18, 20, 21, 23, 25,
};

static u16 th1520_drive_strength_from_ma(u32 arg)
{
	u16 ds;

	for (ds = 0; ds < TH1520_PADCFG_DS; ds++) {
		if (arg <= th1520_drive_strength_in_ma[ds])
			return ds;
	}
	return TH1520_PADCFG_DS;
}

static int th1520_padcfg_rmw(struct th1520_pinctrl *thp, unsigned int pin,
			     u32 mask, u32 value)
{
	void __iomem *padcfg = th1520_padcfg(thp, pin);
	unsigned int shift = th1520_padcfg_shift(pin);
	u32 tmp;

	mask <<= shift;
	value <<= shift;

	tmp = readl_relaxed(padcfg);
	tmp = (tmp & ~mask) | value;
	writel_relaxed(tmp, padcfg);

	return 0;
}

static int th1520_pinconf_apply_one(struct th1520_pinctrl *thp,
				    const struct th1520_pin_desc *desc,
				    enum pin_config_param param, u32 arg)

{
	u16 mask = 0, value = 0;

	if (th1520_pad_no_padcfg(desc))
		return -EOPNOTSUPP;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
		mask |= TH1520_PADCFG_BIAS;
		value &= ~TH1520_PADCFG_BIAS;
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		if (arg == 0)
			return -EOPNOTSUPP;
		mask |= TH1520_PADCFG_BIAS;
		value &= ~TH1520_PADCFG_BIAS;
		value |= TH1520_PADCFG_PE;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		if (arg == 0)
			return -EOPNOTSUPP;
		mask |= TH1520_PADCFG_BIAS;
		value &= ~TH1520_PADCFG_BIAS;
		if (arg == TH1520_PULL_STRONG_OHM)
			value |= TH1520_PADCFG_SPU;
		else
			value |= TH1520_PADCFG_PE | TH1520_PADCFG_PS;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		mask |= TH1520_PADCFG_DS;
		value &= ~TH1520_PADCFG_DS;
		value |= th1520_drive_strength_from_ma(arg);
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		mask |= TH1520_PADCFG_IE;
		if (arg)
			value |= TH1520_PADCFG_IE;
		else
			value &= ~TH1520_PADCFG_IE;
		break;
	case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
		mask |= TH1520_PADCFG_ST;
		if (arg)
			value |= TH1520_PADCFG_ST;
		else
			value &= ~TH1520_PADCFG_ST;
		break;
	case PIN_CONFIG_SLEW_RATE:
		mask |= TH1520_PADCFG_SL;
		if (arg)
			value |= TH1520_PADCFG_SL;
		else
			value &= ~TH1520_PADCFG_SL;
		break;
	default:
		return -EOPNOTSUPP;
	}

	return th1520_padcfg_rmw(thp, desc->number, mask, value);
}

static int th1520_pinmux_apply_one(struct th1520_pinctrl *thp,
				   const struct th1520_pin_desc *desc,
				   enum th1520_muxtype muxtype)
{
	void __iomem *muxcfg = th1520_muxcfg(thp, desc->number);
	unsigned int shift = th1520_muxcfg_shift(desc->number);
	u32 mask, value, tmp;

	for (value = 0; value < ARRAY_SIZE(desc->muxes); value++) {
		if (desc->muxes[value] == muxtype)
			break;
	}
	if (value == ARRAY_SIZE(desc->muxes)) {
		pr_err("invalid mux %s for pin \"%s\"\n",
		       th1520_muxtype_string[muxtype], desc->name);
		return -EINVAL;
	}

	mask = GENMASK(3, 0) << shift;
	value = value << shift;

	tmp = readl_relaxed(muxcfg);
	tmp = (tmp & ~mask) | value;
	writel_relaxed(tmp, muxcfg);

	return 0;
}

const struct pinconf_param th1520_pinconf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
	{ "input-disable", PIN_CONFIG_INPUT_ENABLE, 0 },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "input-schmitt-disable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 0 },
	{ "input-schmitt-enable", PIN_CONFIG_INPUT_SCHMITT_ENABLE, 1 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 0 },
};

static const struct th1520_pin_desc *
th1520_pinctrl_search_pin(struct th1520_pinctrl *thp,
			  const char *name)
{
	const struct th1520_pad_group *pg = thp->group;
	int i;

	for (i = 0; i < pg->npins; i++) {
		if (!strcmp(pg->pins[i].name, name))
			return &pg->pins[i];
	}

	return NULL;
}

static int th1520_pinctrl_apply_group(struct th1520_pinctrl *thp, ofnode group)
{
	struct th1520_pin_desc const **pins;
	enum th1520_muxtype muxtype;
	int pin_count, ret, i, j;
	const char *muxname;

	pin_count = ofnode_read_string_count(group, "pins");
	if (pin_count < 0) {
		pr_err("missing property pins");
		return -EINVAL;
	}

	pins = calloc(pin_count, sizeof(pins[0]));
	if (!pins)
		return -ENOMEM;

	for (i = 0; i < pin_count; i++) {
		const char *pinname;

		ret = ofnode_read_string_index(group, "pins", i, &pinname);
		if (ret)
			goto out;

		pins[i] = th1520_pinctrl_search_pin(thp, pinname);
		if (!pins[i]) {
			pr_err("unknown pin name \"%s\"\n", pinname);
			goto out;
		}
	}

	for (i = 0; i < ARRAY_SIZE(th1520_pinconf_params); i++) {
		const struct pinconf_param *param = &th1520_pinconf_params[i];
		u32 val;

		ret = ofnode_read_u32(group, param->property, &val);
		if (ret == -EINVAL)
			continue;
		else if (ret)
			val = param->default_value;

		for (j = 0; j < pin_count; j++) {
			ret = th1520_pinconf_apply_one(thp, pins[j],
						       param->param, val);
			if (ret) {
				pr_err("failed to apply pinconf for \"%s\": %d\n",
				       pins[j]->name, ret);
				goto out;
			}
		}
	}

	muxname = ofnode_read_string(group, "function");
	if (!muxname)
		goto out;

	muxtype = th1520_muxtype_get(muxname);
	if (!muxtype) {
		pr_err("invalid mux type \"%s\"", muxname);
		ret = -EINVAL;
		goto out;
	}

	for (i = 0; i < pin_count; i++) {
		ret = th1520_pinmux_apply_one(thp, pins[i], muxtype);
		if (ret) {
			pr_err("failed to set pinmux function: %d\n", ret);
			break;
		}
	}

out:
	free(pins);

	return ret;
}

static int th1520_pinctrl_set_state(struct udevice *dev, struct udevice *pcfg)
{
	struct th1520_pinctrl *thp = dev_get_priv(dev);
	ofnode group;
	int ret = 0;

	dev_for_each_subnode(group, pcfg) {
		ret = th1520_pinctrl_apply_group(thp, group);
		if (ret) {
			pr_err("failed to apply pin group \"%s\": %d\n",
			       ofnode_get_name(group), ret);
			break;
		}
	}

	return ret;
}

static int th1520_pinctrl_get_pins_count(struct udevice *dev)
{
	struct th1520_pinctrl *thp = dev_get_priv(dev);

	return thp->group->npins;
}

static const char *th1520_pinctrl_get_pin_name(struct udevice *dev,
					       unsigned int selector)
{
	struct th1520_pinctrl *p = dev_get_priv(dev);

	if (selector >= p->group->npins)
		return ERR_PTR(-EINVAL);

	return p->group->pins[selector].name;
}

static int th1520_pinctrl_get_pin_muxing(struct udevice *dev,
					 unsigned int selector,
					 char *buf, int size)
{
	struct th1520_pinctrl *thp = dev_get_priv(dev);
	const struct th1520_pad_group *group = thp->group;
	const struct th1520_pin_desc *desc;
	void __iomem *muxcfg;
	unsigned int shift;
	u32 val;

	if (selector >= group->npins)
		return -EINVAL;

	desc = &group->pins[selector];
	if (th1520_pad_no_padcfg(desc)) {
		strlcpy(buf, "unsupported", size - 1);
		return 0;
	}

	muxcfg	= th1520_muxcfg(thp, desc->number);
	shift	= th1520_muxcfg_shift(desc->number);

	val = (readl_relaxed(muxcfg) >> shift) & GENMASK(3, 0);
	strlcpy(buf, th1520_muxtype_string[desc->muxes[val]], size);

	return 0;
}

static const struct pinctrl_ops th1520_pinctrl_ops = {
	.get_pins_count = th1520_pinctrl_get_pins_count,
	.get_pin_name = th1520_pinctrl_get_pin_name,
	.get_pin_muxing = th1520_pinctrl_get_pin_muxing,
	.set_state = th1520_pinctrl_set_state,
};

static int th1520_pinctrl_probe(struct udevice *dev)
{
	struct th1520_pinctrl *thp = dev_get_priv(dev);
	struct clk clk;
	u32 pin_group;
	int ret;

	thp->base = dev_read_addr_ptr(dev);
	if (!thp->base)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret) {
		pr_err("failed to enable pinctrl clock: %d\n", ret);
		return ret;
	}

	ret = dev_read_u32(dev, "thead,pad-group", &pin_group);
	if (ret) {
		pr_err("failed to read thead,pad-group property: %d\n", ret);
		return ret;
	}

	switch (pin_group) {
	case 1:
		thp->group = &th1520_group1;
		break;
	case 2:
		thp->group = &th1520_group2;
		break;
	case 3:
		thp->group = &th1520_group3;
		break;
	default:
		pr_err("invalid thead,pad-group property: %u\n", pin_group);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id th1520_pinctrl_ids[] = {
	{ .compatible = "thead,th1520-pinctrl"},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(th1520_pinctrl) = {
	.name = "th1520-pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = th1520_pinctrl_ids,
	.probe = th1520_pinctrl_probe,
	.priv_auto = sizeof(struct th1520_pinctrl),
	.ops = &th1520_pinctrl_ops,
};
