// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 ASPEED Technology Inc.
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <errno.h>
#include <vsprintf.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>

#define AST2700_SOC1_MUX_BASE		0x400
#define AST2700_SOC1_PCIE_REG		0x908
#define AST2700_SOC1_USB_MODE_REG	0x3b0
#define AST2700_SOC1_SGMII_REG		0x47c
#define AST2700_SOC1_MUX_MASK		0x7
#define AST2700_SOC1_MUX_BITS_PER_PIN	4
#define AST2700_SOC1_MUX_PINS_PER_REG	8

#define AST2700_SOC1_BIAS_BASE		0x480
#define AST2700_SOC1_BIAS_PINS_PER_REG	32

#define AST2700_SOC1_DRV_BASE		0x4c0
#define AST2700_SOC1_DRV_MASK		0x3
#define AST2700_SOC1_DRV_BITS_PER_PIN	2
#define AST2700_SOC1_DRV_PINS_PER_REG	16
#define AST2700_SOC1_DRV_STEP_MA	4
#define AST2700_SOC1_DRV_MIN_MA		4
#define AST2700_SOC1_DRV_MAX_MA		16

enum {
	C16,
	C14,
	C11,
	D9,
	F14,
	D10,
	C12,
	C13,
	AC26,
	AA25,
	AB23,
	U22,
	V21,
	N26,
	P25,
	N25,
	V23,
	W22,
	AB26,
	AD26,
	P26,
	AE26,
	AF26,
	AF25,
	AE25,
	AD25,
	AF23,
	AF20,
	AF21,
	AE21,
	AE23,
	AD22,
	AF17,
	AA16,
	Y16,
	V17,
	J13,
	AB16,
	AC16,
	AF16,
	AA15,
	AB15,
	AC15,
	AD15,
	Y15,
	AA14,
	W16,
	V16,
	AB18,
	AC18,
	K13,
	AA17,
	AB17,
	AD16,
	AC17,
	AD17,
	AE16,
	AE17,
	AB24,
	W26,
	HOLE0,
	HOLE1,
	HOLE2,
	HOLE3,
	W25,
	Y23,
	Y24,
	W21,
	AA23,
	AC22,
	AB22,
	Y21,
	AE20,
	AF19,
	Y22,
	AA20,
	AA22,
	AB20,
	AF18,
	AE19,
	AD20,
	AC20,
	AA21,
	AB21,
	AC19,
	AE18,
	AD19,
	AD18,
	U25,
	U26,
	Y26,
	AA24,
	R25,
	AA26,
	R26,
	Y25,
	B16,
	D14,
	B15,
	B14,
	C17,
	B13,
	E14,
	C15,
	D24,
	B23,
	B22,
	C23,
	B18,
	B21,
	M15,
	B19,
	B26,
	A25,
	A24,
	B24,
	E26,
	A21,
	A19,
	A18,
	D26,
	C26,
	A23,
	A22,
	B25,
	F26,
	A26,
	A14,
	E10,
	E13,
	D12,
	F10,
	E11,
	F11,
	F13,
	N15,
	C20,
	C19,
	A8,
	R14,
	A7,
	P14,
	D20,
	A6,
	B6,
	N14,
	B7,
	B8,
	B9,
	M14,
	J11,
	E7,
	D19,
	B11,
	D15,
	B12,
	B10,
	P13,
	C18,
	C6,
	C7,
	D7,
	N13,
	C8,
	C9,
	C10,
	M16,
	A15,
	G11,
	H7,
	H8,
	H9,
	H10,
	H11,
	J9,
	J10,
	E9,
	F9,
	F8,
	M13,
	F7,
	D8,
	E8,
	L12,
	F12,
	E12,
	J12,
	G7,
	G8,
	G9,
	G10,
	K12,
	W17,
	V18,
	W18,
	Y17,
	AA18,
	AA13,
	Y18,
	AA12,
	W20,
	V20,
	Y11,
	V14,
	V19,
	W14,
	Y20,
	AB19,
	U21,
	T24,
	V24,
	V22,
	T23,
	AC25,
	AB25,
	AC24,
	PCIERC2_PERST,
	PORTC_MODE,
	PORTD_MODE,
	SGMII0,
};

struct ast2700_soc1_field {
	u32 reg;
	u32 shift;
	u32 mask;
};

struct ast2700_soc1_group {
	const char *name;
	const unsigned int *pins;
	unsigned int npins;
};

struct ast2700_soc1_function {
	const char *name;
	const char *const *groups;
	const u8 *muxvals;
	unsigned int ngroups;
};

struct ast2700_soc1_pinctrl_priv {
	void __iomem *scu;
};

#define PIN(n) \
	[n] = #n

static const char *const ast2700_soc1_pin_names[] = {
	PIN(C16),
	PIN(C14),
	PIN(C11),
	PIN(D9),
	PIN(F14),
	PIN(D10),
	PIN(C12),
	PIN(C13),
	PIN(AC26),
	PIN(AA25),
	PIN(AB23),
	PIN(U22),
	PIN(V21),
	PIN(N26),
	PIN(P25),
	PIN(N25),
	PIN(V23),
	PIN(W22),
	PIN(AB26),
	PIN(AD26),
	PIN(P26),
	PIN(AE26),
	PIN(AF26),
	PIN(AF25),
	PIN(AE25),
	PIN(AD25),
	PIN(AF23),
	PIN(AF20),
	PIN(AF21),
	PIN(AE21),
	PIN(AE23),
	PIN(AD22),
	PIN(AF17),
	PIN(AA16),
	PIN(Y16),
	PIN(V17),
	PIN(J13),
	PIN(AB16),
	PIN(AC16),
	PIN(AF16),
	PIN(AA15),
	PIN(AB15),
	PIN(AC15),
	PIN(AD15),
	PIN(Y15),
	PIN(AA14),
	PIN(W16),
	PIN(V16),
	PIN(AB18),
	PIN(AC18),
	PIN(K13),
	PIN(AA17),
	PIN(AB17),
	PIN(AD16),
	PIN(AC17),
	PIN(AD17),
	PIN(AE16),
	PIN(AE17),
	PIN(AB24),
	PIN(W26),
	PIN(HOLE0),
	PIN(HOLE1),
	PIN(HOLE2),
	PIN(HOLE3),
	PIN(W25),
	PIN(Y23),
	PIN(Y24),
	PIN(W21),
	PIN(AA23),
	PIN(AC22),
	PIN(AB22),
	PIN(Y21),
	PIN(AE20),
	PIN(AF19),
	PIN(Y22),
	PIN(AA20),
	PIN(AA22),
	PIN(AB20),
	PIN(AF18),
	PIN(AE19),
	PIN(AD20),
	PIN(AC20),
	PIN(AA21),
	PIN(AB21),
	PIN(AC19),
	PIN(AE18),
	PIN(AD19),
	PIN(AD18),
	PIN(U25),
	PIN(U26),
	PIN(Y26),
	PIN(AA24),
	PIN(R25),
	PIN(AA26),
	PIN(R26),
	PIN(Y25),
	PIN(B16),
	PIN(D14),
	PIN(B15),
	PIN(B14),
	PIN(C17),
	PIN(B13),
	PIN(E14),
	PIN(C15),
	PIN(D24),
	PIN(B23),
	PIN(B22),
	PIN(C23),
	PIN(B18),
	PIN(B21),
	PIN(M15),
	PIN(B19),
	PIN(B26),
	PIN(A25),
	PIN(A24),
	PIN(B24),
	PIN(E26),
	PIN(A21),
	PIN(A19),
	PIN(A18),
	PIN(D26),
	PIN(C26),
	PIN(A23),
	PIN(A22),
	PIN(B25),
	PIN(F26),
	PIN(A26),
	PIN(A14),
	PIN(E10),
	PIN(E13),
	PIN(D12),
	PIN(F10),
	PIN(E11),
	PIN(F11),
	PIN(F13),
	PIN(N15),
	PIN(C20),
	PIN(C19),
	PIN(A8),
	PIN(R14),
	PIN(A7),
	PIN(P14),
	PIN(D20),
	PIN(A6),
	PIN(B6),
	PIN(N14),
	PIN(B7),
	PIN(B8),
	PIN(B9),
	PIN(M14),
	PIN(J11),
	PIN(E7),
	PIN(D19),
	PIN(B11),
	PIN(D15),
	PIN(B12),
	PIN(B10),
	PIN(P13),
	PIN(C18),
	PIN(C6),
	PIN(C7),
	PIN(D7),
	PIN(N13),
	PIN(C8),
	PIN(C9),
	PIN(C10),
	PIN(M16),
	PIN(A15),
	PIN(G11),
	PIN(H7),
	PIN(H8),
	PIN(H9),
	PIN(H10),
	PIN(H11),
	PIN(J9),
	PIN(J10),
	PIN(E9),
	PIN(F9),
	PIN(F8),
	PIN(M13),
	PIN(F7),
	PIN(D8),
	PIN(E8),
	PIN(L12),
	PIN(F12),
	PIN(E12),
	PIN(J12),
	PIN(G7),
	PIN(G8),
	PIN(G9),
	PIN(G10),
	PIN(K12),
	PIN(W17),
	PIN(V18),
	PIN(W18),
	PIN(Y17),
	PIN(AA18),
	PIN(AA13),
	PIN(Y18),
	PIN(AA12),
	PIN(W20),
	PIN(V20),
	PIN(Y11),
	PIN(V14),
	PIN(V19),
	PIN(W14),
	PIN(Y20),
	PIN(AB19),
	PIN(U21),
	PIN(T24),
	PIN(V24),
	PIN(V22),
	PIN(T23),
	PIN(AC25),
	PIN(AB25),
	PIN(AC24),
	PIN(PCIERC2_PERST),
	PIN(PORTC_MODE),
	PIN(PORTD_MODE),
	PIN(SGMII0),
};

#define PIN_GROUP(name, ...) \
	static const unsigned int name##_pins[] = { __VA_ARGS__ }

/* Pin groups and functions */
PIN_GROUP(ADC0, W17);
PIN_GROUP(ADC1, V18);
PIN_GROUP(ADC10, Y11);
PIN_GROUP(ADC11, V14);
PIN_GROUP(ADC12, V19);
PIN_GROUP(ADC13, W14);
PIN_GROUP(ADC14, Y20);
PIN_GROUP(ADC15, AB19);
PIN_GROUP(ADC2, W18);
PIN_GROUP(ADC3, Y17);
PIN_GROUP(ADC4, AA18);
PIN_GROUP(ADC5, AA13);
PIN_GROUP(ADC6, Y18);
PIN_GROUP(ADC7, AA12);
PIN_GROUP(ADC8, W20);
PIN_GROUP(ADC9, V20);
PIN_GROUP(AUXPWRGOOD0, W14);
PIN_GROUP(AUXPWRGOOD1, Y20);
PIN_GROUP(CANBUS, G7, G8, G9);
PIN_GROUP(DI2C0, C16, D9);
PIN_GROUP(DI2C1, C14, F14);
PIN_GROUP(DI2C10, R25, AA26);
PIN_GROUP(DI2C11, R26, Y25);
PIN_GROUP(DI2C12, W25, Y23);
PIN_GROUP(DI2C13, Y24, W21);
PIN_GROUP(DI2C14, AA23, AC22);
PIN_GROUP(DI2C15, AB22, Y21);
PIN_GROUP(DI2C2, D10, C12);
PIN_GROUP(DI2C3, C11, C13);
PIN_GROUP(DI2C8, U25, U26);
PIN_GROUP(DI2C9, Y26, AA24);
PIN_GROUP(DSGPM0, D19, B10, C7, D7);
PIN_GROUP(ESPI0, B16, D14, B15, B14, C17, B13, E14, C15);
PIN_GROUP(ESPI1, C16, C14, C11, D9, F14, D10, C12, C13);
PIN_GROUP(FSI0, AD20, AC20);
PIN_GROUP(FSI1, AA21, AB21);
PIN_GROUP(FSI2, AC19, AE18);
PIN_GROUP(FSI3, AD19, AD18);
PIN_GROUP(FWQSPI, M16, A15);
PIN_GROUP(FWSPIABR, A14);
PIN_GROUP(FWWPN, N15);
PIN_GROUP(HBLED, V24);
PIN_GROUP(HVI3C0, U25, U26);
PIN_GROUP(HVI3C1, Y26, AA24);
PIN_GROUP(HVI3C12, W25, Y23);
PIN_GROUP(HVI3C13, Y24, W21);
PIN_GROUP(HVI3C14, AA23, AC22);
PIN_GROUP(HVI3C15, AB22, Y21);
PIN_GROUP(HVI3C2, R25, AA26);
PIN_GROUP(HVI3C3, R26, Y25);
PIN_GROUP(I2C0, G11, H7);
PIN_GROUP(I2C1, H8, H9);
PIN_GROUP(I2C10, G8, G9);
PIN_GROUP(I2C11, G10, K12);
PIN_GROUP(I2C12, AC18, AA17);
PIN_GROUP(I2C13, AB17, AD16);
PIN_GROUP(I2C14, AC17, AD17);
PIN_GROUP(I2C15, AE16, AE17);
PIN_GROUP(I2C2, H10, H11);
PIN_GROUP(I2C3, J9, J10);
PIN_GROUP(I2C4, E9, F9);
PIN_GROUP(I2C5, F8, M13);
PIN_GROUP(I2C6, F7, D8);
PIN_GROUP(I2C7, E8, L12);
PIN_GROUP(I2C8, F12, E12);
PIN_GROUP(I2C9, J12, G7);
PIN_GROUP(I2CF0, F12, E12, J12, G7);
PIN_GROUP(I2CF1, E9, F9, F8, M13);
PIN_GROUP(I2CF2, F7, D8, E8, L12);
PIN_GROUP(I3C10, AC19, AE18);
PIN_GROUP(I3C11, AD19, AD18);
PIN_GROUP(I3C4, AE20, AF19);
PIN_GROUP(I3C5, Y22, AA20);
PIN_GROUP(I3C6, AA22, AB20);
PIN_GROUP(I3C7, AF18, AE19);
PIN_GROUP(I3C8, AD20, AC20);
PIN_GROUP(I3C9, AA21, AB21);
PIN_GROUP(JTAGM1, D12, F10, E11, F11, F13);
PIN_GROUP(LPC0, AF26, AF25, B16, D14, B15, B14, C17, B13, E14, C15);
PIN_GROUP(LPC1, C16, C14, C11, D9, F14, D10, C12, C13, AE16, AE17);
PIN_GROUP(LTPI, U25, U26, Y26, AA24);
PIN_GROUP(LTPI_PS_I2C0, G11, H7);
PIN_GROUP(LTPI_PS_I2C1, H8, H9);
PIN_GROUP(LTPI_PS_I2C2, H10, H11);
PIN_GROUP(LTPI_PS_I2C3, J9, J10);
PIN_GROUP(MACLINK0, U21);
PIN_GROUP(MACLINK1, AC24);
PIN_GROUP(MACLINK2, T24);
PIN_GROUP(MDIO0, B9, M14);
PIN_GROUP(MDIO1, C9, C10);
PIN_GROUP(MDIO2, E10, E13);
PIN_GROUP(NCTS0, AF17);
PIN_GROUP(NCTS1, AA15);
PIN_GROUP(NCTS5, V21);
PIN_GROUP(NCTS6, AB26);
PIN_GROUP(NDCD0, AA16);
PIN_GROUP(NDCD1, AB15);
PIN_GROUP(NDCD5, N26);
PIN_GROUP(NDCD6, AD26);
PIN_GROUP(NDSR0, Y16);
PIN_GROUP(NDSR1, AC15);
PIN_GROUP(NDSR5, P25);
PIN_GROUP(NDSR6, P26);
PIN_GROUP(NDTR0, J13);
PIN_GROUP(NDTR1, Y15);
PIN_GROUP(NDTR5, V23);
PIN_GROUP(NDTR6, AF26);
PIN_GROUP(NRI0, V17);
PIN_GROUP(NRI1, AD15);
PIN_GROUP(NRI5, N25);
PIN_GROUP(NRI6, AE26);
PIN_GROUP(NRTS0, AB16);
PIN_GROUP(NRTS1, AA14);
PIN_GROUP(NRTS5, W22);
PIN_GROUP(NRTS6, AF25);
PIN_GROUP(OSCCLK, C17);
PIN_GROUP(PE2SGRSTN, E10, PCIERC2_PERST);
PIN_GROUP(PWM0, AE25);
PIN_GROUP(PWM1, AD25);
PIN_GROUP(PWM10, AB17);
PIN_GROUP(PWM11, AD16);
PIN_GROUP(PWM12, AC17);
PIN_GROUP(PWM13, AD17);
PIN_GROUP(PWM14, AE16);
PIN_GROUP(PWM15, AE17);
PIN_GROUP(PWM2, AF23);
PIN_GROUP(PWM3, AF20);
PIN_GROUP(PWM4, AF21);
PIN_GROUP(PWM5, AE21);
PIN_GROUP(PWM6, AE23);
PIN_GROUP(PWM7, AD22);
PIN_GROUP(PWM8, K13);
PIN_GROUP(PWM9, AA17);
PIN_GROUP(QSPI0, C23, B18);
PIN_GROUP(QSPI1, B24, E26);
PIN_GROUP(QSPI2, B25, F26);
PIN_GROUP(RGMII0, C20, C19, A8, R14, A7, P14, D20, A6, B6, N14, B7, B8);
PIN_GROUP(RGMII1, D19, B11, D15, B12, B10, P13, C18, C6, C7, D7, N13, C8);
PIN_GROUP(RMII0, C20, A8, R14, A7, P14, A6, B6, N14);
PIN_GROUP(RMII0RCLKO, D20);
PIN_GROUP(RMII1, D19, D15, B12, B10, P13, C6, C7, D7);
PIN_GROUP(RMII1RCLKO, C18);
PIN_GROUP(SALT0, AC17);
PIN_GROUP(SALT1, AD17);
PIN_GROUP(SALT10, Y18);
PIN_GROUP(SALT11, AA12);
PIN_GROUP(SALT12, AB26);
PIN_GROUP(SALT13, AD26);
PIN_GROUP(SALT14, P26);
PIN_GROUP(SALT15, AE26);
PIN_GROUP(SALT2, AC15);
PIN_GROUP(SALT3, AD15);
PIN_GROUP(SALT4, W17);
PIN_GROUP(SALT5, V18);
PIN_GROUP(SALT6, W18);
PIN_GROUP(SALT7, Y17);
PIN_GROUP(SALT8, AA18);
PIN_GROUP(SALT9, AA13);
PIN_GROUP(SD, C16, C14, C11, D9, F14, D10, C12, C13);
PIN_GROUP(SGMII, SGMII0);
PIN_GROUP(SGPM0, U21, T24, V22, T23);
PIN_GROUP(SGPM1, AC25, AB25, AB24, W26);
PIN_GROUP(SGPS, B11, C18, N13, C8);
PIN_GROUP(SIOONCTRLN0, AE23);
PIN_GROUP(SIOONCTRLN1, AA15);
PIN_GROUP(SIOPBIN0, AD25);
PIN_GROUP(SIOPBIN1, AA16);
PIN_GROUP(SIOPBON0, AE25);
PIN_GROUP(SIOPBON1, AF17);
PIN_GROUP(SIOPWREQN0, AE21);
PIN_GROUP(SIOPWREQN1, AB16);
PIN_GROUP(SIOPWRGD1, AB15);
PIN_GROUP(SIOS3N0, AF20);
PIN_GROUP(SIOS3N1, V17);
PIN_GROUP(SIOS5N0, AF21);
PIN_GROUP(SIOS5N1, J13);
PIN_GROUP(SIOSCIN0, AF23);
PIN_GROUP(SIOSCIN1, Y16);
PIN_GROUP(SMON0, U21, T24, V22, T23);
PIN_GROUP(SMON1, W26, AC25, AB25);
PIN_GROUP(SPI0, D24, B23, B22);
PIN_GROUP(SPI0ABR, M15);
PIN_GROUP(SPI0CS1, B21);
PIN_GROUP(SPI0WPN, B19);
PIN_GROUP(SPI1, B26, A25, A24);
PIN_GROUP(SPI1ABR, A19);
PIN_GROUP(SPI1CS1, A21);
PIN_GROUP(SPI1WPN, A18);
PIN_GROUP(SPI2, D26, C26, A23, A22);
PIN_GROUP(SPI2CS1, A26);
PIN_GROUP(TACH0, AC26);
PIN_GROUP(TACH1, AA25);
PIN_GROUP(TACH10, AB26);
PIN_GROUP(TACH11, AD26);
PIN_GROUP(TACH12, P26);
PIN_GROUP(TACH13, AE26);
PIN_GROUP(TACH14, AF26);
PIN_GROUP(TACH15, AF25);
PIN_GROUP(TACH2, AB23);
PIN_GROUP(TACH3, U22);
PIN_GROUP(TACH4, V21);
PIN_GROUP(TACH5, N26);
PIN_GROUP(TACH6, P25);
PIN_GROUP(TACH7, N25);
PIN_GROUP(TACH8, V23);
PIN_GROUP(TACH9, W22);
PIN_GROUP(THRU0, AC26, AA25);
PIN_GROUP(THRU1, AB23, U22);
PIN_GROUP(THRU2, A19, A18);
PIN_GROUP(THRU3, B25, F26);
PIN_GROUP(UART0, AC16, AF16);
PIN_GROUP(UART1, W16, V16);
PIN_GROUP(UART2, AB18, AC18);
PIN_GROUP(UART3, K13, AA17);
PIN_GROUP(UART5, AB17, AD16);
PIN_GROUP(UART6, AC17, AD17);
PIN_GROUP(UART7, AE16, AE17);
PIN_GROUP(UART8, M15, B19);
PIN_GROUP(UART9, B26, A25);
PIN_GROUP(UART10, A24, B24);
PIN_GROUP(UART11, E26, A21);
PIN_GROUP(USB2CD, PORTC_MODE);
PIN_GROUP(USB2CH, PORTC_MODE);
PIN_GROUP(USB2CU, PORTC_MODE);
PIN_GROUP(USB2CUD, PORTC_MODE);
PIN_GROUP(USB2DD, PORTD_MODE);
PIN_GROUP(USB2DH, PORTD_MODE);
PIN_GROUP(USBUART, G10, K12);
PIN_GROUP(VGA, J11, E7);
PIN_GROUP(VPI, C16, C14, C11, D9, F14, D10, AC26, AA25, AB23, U22, V21, N26,
	  P25, N25, V23, W22, AB26, AD26, P26, AE26, AF26, AF25, AE25, AD25,
	  AF23, AF20, AF21, AE21);
PIN_GROUP(WDTRST0N, K13);
PIN_GROUP(WDTRST1N, AA17);
PIN_GROUP(WDTRST2N, AB17);
PIN_GROUP(WDTRST3N, AD16);
PIN_GROUP(WDTRST4N, AC25);
PIN_GROUP(WDTRST5N, AB25);
PIN_GROUP(WDTRST6N, AC24);
PIN_GROUP(WDTRST7N, AB24);

#define GROUP(n) \
	{ .name = #n, .pins = n##_pins, .npins = ARRAY_SIZE(n##_pins) }

static const struct ast2700_soc1_group ast2700_soc1_groups[] = {
	GROUP(ADC0),
	GROUP(ADC1),
	GROUP(ADC10),
	GROUP(ADC11),
	GROUP(ADC12),
	GROUP(ADC13),
	GROUP(ADC14),
	GROUP(ADC15),
	GROUP(ADC2),
	GROUP(ADC3),
	GROUP(ADC4),
	GROUP(ADC5),
	GROUP(ADC6),
	GROUP(ADC7),
	GROUP(ADC8),
	GROUP(ADC9),
	GROUP(AUXPWRGOOD0),
	GROUP(AUXPWRGOOD1),
	GROUP(CANBUS),
	GROUP(DI2C0),
	GROUP(DI2C1),
	GROUP(DI2C10),
	GROUP(DI2C11),
	GROUP(DI2C12),
	GROUP(DI2C13),
	GROUP(DI2C14),
	GROUP(DI2C15),
	GROUP(DI2C2),
	GROUP(DI2C3),
	GROUP(DI2C8),
	GROUP(DI2C9),
	GROUP(DSGPM0),
	GROUP(ESPI0),
	GROUP(ESPI1),
	GROUP(FSI0),
	GROUP(FSI1),
	GROUP(FSI2),
	GROUP(FSI3),
	GROUP(FWQSPI),
	GROUP(FWSPIABR),
	GROUP(FWWPN),
	GROUP(HBLED),
	GROUP(HVI3C0),
	GROUP(HVI3C1),
	GROUP(HVI3C12),
	GROUP(HVI3C13),
	GROUP(HVI3C14),
	GROUP(HVI3C15),
	GROUP(HVI3C2),
	GROUP(HVI3C3),
	GROUP(I2C0),
	GROUP(I2C1),
	GROUP(I2C10),
	GROUP(I2C11),
	GROUP(I2C12),
	GROUP(I2C13),
	GROUP(I2C14),
	GROUP(I2C15),
	GROUP(I2C2),
	GROUP(I2C3),
	GROUP(I2C4),
	GROUP(I2C5),
	GROUP(I2C6),
	GROUP(I2C7),
	GROUP(I2C8),
	GROUP(I2C9),
	GROUP(I2CF0),
	GROUP(I2CF1),
	GROUP(I2CF2),
	GROUP(I3C10),
	GROUP(I3C11),
	GROUP(I3C4),
	GROUP(I3C5),
	GROUP(I3C6),
	GROUP(I3C7),
	GROUP(I3C8),
	GROUP(I3C9),
	GROUP(JTAGM1),
	GROUP(LPC0),
	GROUP(LPC1),
	GROUP(LTPI),
	GROUP(LTPI_PS_I2C0),
	GROUP(LTPI_PS_I2C1),
	GROUP(LTPI_PS_I2C2),
	GROUP(LTPI_PS_I2C3),
	GROUP(MACLINK0),
	GROUP(MACLINK1),
	GROUP(MACLINK2),
	GROUP(MDIO0),
	GROUP(MDIO1),
	GROUP(MDIO2),
	GROUP(NCTS0),
	GROUP(NCTS1),
	GROUP(NCTS5),
	GROUP(NCTS6),
	GROUP(NDCD0),
	GROUP(NDCD1),
	GROUP(NDCD5),
	GROUP(NDCD6),
	GROUP(NDSR0),
	GROUP(NDSR1),
	GROUP(NDSR5),
	GROUP(NDSR6),
	GROUP(NDTR0),
	GROUP(NDTR1),
	GROUP(NDTR5),
	GROUP(NDTR6),
	GROUP(NRI0),
	GROUP(NRI1),
	GROUP(NRI5),
	GROUP(NRI6),
	GROUP(NRTS0),
	GROUP(NRTS1),
	GROUP(NRTS5),
	GROUP(NRTS6),
	GROUP(OSCCLK),
	GROUP(PE2SGRSTN),
	GROUP(PWM0),
	GROUP(PWM1),
	GROUP(PWM10),
	GROUP(PWM11),
	GROUP(PWM12),
	GROUP(PWM13),
	GROUP(PWM14),
	GROUP(PWM15),
	GROUP(PWM2),
	GROUP(PWM3),
	GROUP(PWM4),
	GROUP(PWM5),
	GROUP(PWM6),
	GROUP(PWM7),
	GROUP(PWM8),
	GROUP(PWM9),
	GROUP(QSPI0),
	GROUP(QSPI1),
	GROUP(QSPI2),
	GROUP(RGMII0),
	GROUP(RGMII1),
	GROUP(RMII0),
	GROUP(RMII0RCLKO),
	GROUP(RMII1),
	GROUP(RMII1RCLKO),
	GROUP(SALT0),
	GROUP(SALT1),
	GROUP(SALT10),
	GROUP(SALT11),
	GROUP(SALT12),
	GROUP(SALT13),
	GROUP(SALT14),
	GROUP(SALT15),
	GROUP(SALT2),
	GROUP(SALT3),
	GROUP(SALT4),
	GROUP(SALT5),
	GROUP(SALT6),
	GROUP(SALT7),
	GROUP(SALT8),
	GROUP(SALT9),
	GROUP(SD),
	GROUP(SGMII),
	GROUP(SGPM0),
	GROUP(SGPM1),
	GROUP(SGPS),
	GROUP(SIOONCTRLN0),
	GROUP(SIOONCTRLN1),
	GROUP(SIOPBIN0),
	GROUP(SIOPBIN1),
	GROUP(SIOPBON0),
	GROUP(SIOPBON1),
	GROUP(SIOPWREQN0),
	GROUP(SIOPWREQN1),
	GROUP(SIOPWRGD1),
	GROUP(SIOS3N0),
	GROUP(SIOS3N1),
	GROUP(SIOS5N0),
	GROUP(SIOS5N1),
	GROUP(SIOSCIN0),
	GROUP(SIOSCIN1),
	GROUP(SMON0),
	GROUP(SMON1),
	GROUP(SPI0),
	GROUP(SPI0ABR),
	GROUP(SPI0CS1),
	GROUP(SPI0WPN),
	GROUP(SPI1),
	GROUP(SPI1ABR),
	GROUP(SPI1CS1),
	GROUP(SPI1WPN),
	GROUP(SPI2),
	GROUP(SPI2CS1),
	GROUP(TACH0),
	GROUP(TACH1),
	GROUP(TACH10),
	GROUP(TACH11),
	GROUP(TACH12),
	GROUP(TACH13),
	GROUP(TACH14),
	GROUP(TACH15),
	GROUP(TACH2),
	GROUP(TACH3),
	GROUP(TACH4),
	GROUP(TACH5),
	GROUP(TACH6),
	GROUP(TACH7),
	GROUP(TACH8),
	GROUP(TACH9),
	GROUP(THRU0),
	GROUP(THRU1),
	GROUP(THRU2),
	GROUP(THRU3),
	GROUP(UART0),
	GROUP(UART1),
	GROUP(UART10),
	GROUP(UART11),
	GROUP(UART2),
	GROUP(UART3),
	GROUP(UART5),
	GROUP(UART6),
	GROUP(UART7),
	GROUP(UART8),
	GROUP(UART9),
	GROUP(USB2CD),
	GROUP(USB2CH),
	GROUP(USB2CU),
	GROUP(USB2CUD),
	GROUP(USB2DD),
	GROUP(USB2DH),
	GROUP(USBUART),
	GROUP(VGA),
	GROUP(VPI),
	GROUP(WDTRST0N),
	GROUP(WDTRST1N),
	GROUP(WDTRST2N),
	GROUP(WDTRST3N),
	GROUP(WDTRST4N),
	GROUP(WDTRST5N),
	GROUP(WDTRST6N),
	GROUP(WDTRST7N),
};

/**
 * VM() - Helper macro to unwrap a parenthesized list of arguments.
 * @...: The parenthesized list to be unwrapped.
 *
 * Since the C preprocessor treats commas inside braces {} as argument
 * separators for macros, wrap mux value lists in parentheses and strip
 * them here when initializing the compound literal.
 */
#define VM(...) __VA_ARGS__

/**
 * FUNC() - Initialize an ast2700_soc1_function entry.
 * @n: Name of the pin function.
 * @m: Parenthesized list of mux values mapped 1:1 to the groups list.
 * @...: Variable list of pin group names associated with this function.
 */
#define FUNC(n, m, ...) \
	{ \
		.name = #n, \
		.groups = (const char *const[]){ __VA_ARGS__ }, \
		.muxvals = (const u8[]){ VM m }, \
		.ngroups = sizeof((const char *const[]){ __VA_ARGS__ }) / \
			   sizeof(char *), \
	}

static const struct ast2700_soc1_function ast2700_soc1_functions[] = {
	FUNC(ADC0, (0), "ADC0"),
	FUNC(ADC1, (0), "ADC1"),
	FUNC(ADC10, (0), "ADC10"),
	FUNC(ADC11, (0), "ADC11"),
	FUNC(ADC12, (0), "ADC12"),
	FUNC(ADC13, (0), "ADC13"),
	FUNC(ADC14, (0), "ADC14"),
	FUNC(ADC15, (0), "ADC15"),
	FUNC(ADC2, (0), "ADC2"),
	FUNC(ADC3, (0), "ADC3"),
	FUNC(ADC4, (0), "ADC4"),
	FUNC(ADC5, (0), "ADC5"),
	FUNC(ADC6, (0), "ADC6"),
	FUNC(ADC7, (0), "ADC7"),
	FUNC(ADC8, (0), "ADC8"),
	FUNC(ADC9, (0), "ADC9"),
	FUNC(AUXPWRGOOD0, (2), "AUXPWRGOOD0"),
	FUNC(AUXPWRGOOD1, (2), "AUXPWRGOOD1"),
	FUNC(CANBUS, (2), "CANBUS"),
	FUNC(ESPI0, (1), "ESPI0"),
	FUNC(ESPI1, (1), "ESPI1"),
	FUNC(FSI0, (2), "FSI0"),
	FUNC(FSI1, (2), "FSI1"),
	FUNC(FSI2, (2), "FSI2"),
	FUNC(FSI3, (2), "FSI3"),
	FUNC(FWQSPI, (1), "FWQSPI"),
	FUNC(FWSPIABR, (1), "FWSPIABR"),
	FUNC(FWWPN, (1), "FWWPN"),
	FUNC(HBLED, (2), "HBLED"),
	FUNC(I2C0, (1, 2, 4), "I2C0", "LTPI_PS_I2C0", "DI2C0"),
	FUNC(I2C1, (1, 2, 4), "I2C1", "LTPI_PS_I2C1", "DI2C1"),
	FUNC(I2C10, (1, 2), "I2C10", "DI2C10"),
	FUNC(I2C11, (1, 2), "I2C11", "DI2C11"),
	FUNC(I2C12, (4, 2), "I2C12", "DI2C12"),
	FUNC(I2C13, (4, 2), "I2C13", "DI2C13"),
	FUNC(I2C14, (4, 2), "I2C14", "DI2C14"),
	FUNC(I2C15, (2, 2), "I2C15", "DI2C15"),
	FUNC(I2C2, (1, 2, 4), "I2C2", "LTPI_PS_I2C2", "DI2C2"),
	FUNC(I2C3, (1, 2, 4), "I2C3", "LTPI_PS_I2C3", "DI2C3"),
	FUNC(I2C4, (1), "I2C4"),
	FUNC(I2C5, (1), "I2C5"),
	FUNC(I2C6, (1), "I2C6"),
	FUNC(I2C7, (1), "I2C7"),
	FUNC(I2C8, (1, 2), "I2C8", "DI2C8"),
	FUNC(I2C9, (1, 2), "I2C9", "DI2C9"),
	FUNC(I2CF0, (5), "I2CF0"),
	FUNC(I2CF1, (5), "I2CF1"),
	FUNC(I2CF2, (5), "I2CF2"),
	FUNC(I3C0, (1), "HVI3C0"),
	FUNC(I3C1, (1), "HVI3C1"),
	FUNC(I3C10, (1), "I3C10"),
	FUNC(I3C11, (1), "I3C11"),
	FUNC(I3C12, (1), "HVI3C12"),
	FUNC(I3C13, (1), "HVI3C13"),
	FUNC(I3C14, (1), "HVI3C14"),
	FUNC(I3C15, (1), "HVI3C15"),
	FUNC(I3C2, (1), "HVI3C2"),
	FUNC(I3C3, (1), "HVI3C3"),
	FUNC(I3C4, (1), "I3C4"),
	FUNC(I3C5, (1), "I3C5"),
	FUNC(I3C6, (1), "I3C6"),
	FUNC(I3C7, (1), "I3C7"),
	FUNC(I3C8, (1), "I3C8"),
	FUNC(I3C9, (1), "I3C9"),
	FUNC(JTAGM1, (1), "JTAGM1"),
	FUNC(LPC0, (2), "LPC0"),
	FUNC(LPC1, (2), "LPC1"),
	FUNC(LTPI, (2), "LTPI"),
	FUNC(MACLINK0, (4), "MACLINK0"),
	FUNC(MACLINK1, (3), "MACLINK1"),
	FUNC(MACLINK2, (4), "MACLINK2"),
	FUNC(MDIO0, (1), "MDIO0"),
	FUNC(MDIO1, (1), "MDIO1"),
	FUNC(MDIO2, (1), "MDIO2"),
	FUNC(NCTS0, (1), "NCTS0"),
	FUNC(NCTS1, (1), "NCTS1"),
	FUNC(NCTS5, (4), "NCTS5"),
	FUNC(NCTS6, (4), "NCTS6"),
	FUNC(NDCD0, (1), "NDCD0"),
	FUNC(NDCD1, (1), "NDCD1"),
	FUNC(NDCD5, (4), "NDCD5"),
	FUNC(NDCD6, (4), "NDCD6"),
	FUNC(NDSR0, (1), "NDSR0"),
	FUNC(NDSR1, (1), "NDSR1"),
	FUNC(NDSR5, (4), "NDSR5"),
	FUNC(NDSR6, (4), "NDSR6"),
	FUNC(NDTR0, (1), "NDTR0"),
	FUNC(NDTR1, (1), "NDTR1"),
	FUNC(NDTR5, (4), "NDTR5"),
	FUNC(NDTR6, (4), "NDTR6"),
	FUNC(NRI0, (1), "NRI0"),
	FUNC(NRI1, (1), "NRI1"),
	FUNC(NRI5, (4), "NRI5"),
	FUNC(NRI6, (4), "NRI6"),
	FUNC(NRTS0, (1), "NRTS0"),
	FUNC(NRTS1, (1), "NRTS1"),
	FUNC(NRTS5, (4), "NRTS5"),
	FUNC(NRTS6, (4), "NRTS6"),
	FUNC(OSCCLK, (3), "OSCCLK"),
	FUNC(PCIERC, (2), "PE2SGRSTN"),
	FUNC(PWM0, (1), "PWM0"),
	FUNC(PWM1, (1), "PWM1"),
	FUNC(PWM10, (3), "PWM10"),
	FUNC(PWM11, (3), "PWM11"),
	FUNC(PWM12, (3), "PWM12"),
	FUNC(PWM13, (3), "PWM13"),
	FUNC(PWM14, (3), "PWM14"),
	FUNC(PWM15, (3), "PWM15"),
	FUNC(PWM2, (1), "PWM2"),
	FUNC(PWM3, (1), "PWM3"),
	FUNC(PWM4, (1), "PWM4"),
	FUNC(PWM5, (1), "PWM5"),
	FUNC(PWM6, (1), "PWM6"),
	FUNC(PWM7, (1), "PWM7"),
	FUNC(PWM8, (3), "PWM8"),
	FUNC(PWM9, (3), "PWM9"),
	FUNC(QSPI0, (1), "QSPI0"),
	FUNC(QSPI1, (1), "QSPI1"),
	FUNC(QSPI2, (1), "QSPI2"),
	FUNC(RGMII0, (1), "RGMII0"),
	FUNC(RGMII1, (1), "RGMII1"),
	FUNC(RMII0, (2), "RMII0"),
	FUNC(RMII0RCLKO, (2), "RMII0RCLKO"),
	FUNC(RMII1, (2), "RMII1"),
	FUNC(RMII1RCLKO, (2), "RMII1RCLKO"),
	FUNC(SALT0, (2), "SALT0"),
	FUNC(SALT1, (2), "SALT1"),
	FUNC(SALT10, (2), "SALT10"),
	FUNC(SALT11, (2), "SALT11"),
	FUNC(SALT12, (2), "SALT12"),
	FUNC(SALT13, (2), "SALT13"),
	FUNC(SALT14, (2), "SALT14"),
	FUNC(SALT15, (2), "SALT15"),
	FUNC(SALT2, (2), "SALT2"),
	FUNC(SALT3, (2), "SALT3"),
	FUNC(SALT4, (2), "SALT4"),
	FUNC(SALT5, (2), "SALT5"),
	FUNC(SALT6, (2), "SALT6"),
	FUNC(SALT7, (2), "SALT7"),
	FUNC(SALT8, (2), "SALT8"),
	FUNC(SALT9, (2), "SALT9"),
	FUNC(SD, (3), "SD"),
	FUNC(SGMII, (1), "SGMII"),
	FUNC(SGPM0, (1, 4), "SGPM0", "DSGPM0"),
	FUNC(SGPM1, (1), "SGPM1"),
	FUNC(SGPS, (5), "SGPS"),
	FUNC(SIOONCTRLN0, (2), "SIOONCTRLN0"),
	FUNC(SIOONCTRLN1, (2), "SIOONCTRLN1"),
	FUNC(SIOPBIN0, (2), "SIOPBIN0"),
	FUNC(SIOPBIN1, (2), "SIOPBIN1"),
	FUNC(SIOPBON0, (2), "SIOPBON0"),
	FUNC(SIOPBON1, (2), "SIOPBON1"),
	FUNC(SIOPWREQN0, (2), "SIOPWREQN0"),
	FUNC(SIOPWREQN1, (2), "SIOPWREQN1"),
	FUNC(SIOPWRGD1, (2), "SIOPWRGD1"),
	FUNC(SIOS3N0, (2), "SIOS3N0"),
	FUNC(SIOS3N1, (2), "SIOS3N1"),
	FUNC(SIOS5N0, (2), "SIOS5N0"),
	FUNC(SIOS5N1, (2), "SIOS5N1"),
	FUNC(SIOSCIN0, (2), "SIOSCIN0"),
	FUNC(SIOSCIN1, (2), "SIOSCIN1"),
	FUNC(SMON0, (2), "SMON0"),
	FUNC(SMON1, (4), "SMON1"),
	FUNC(SPI0, (1), "SPI0"),
	FUNC(SPI0ABR, (1), "SPI0ABR"),
	FUNC(SPI0CS1, (1), "SPI0CS1"),
	FUNC(SPI0WPN, (1), "SPI0WPN"),
	FUNC(SPI1, (1), "SPI1"),
	FUNC(SPI1ABR, (1), "SPI1ABR"),
	FUNC(SPI1CS1, (1), "SPI1CS1"),
	FUNC(SPI1WPN, (1), "SPI1WPN"),
	FUNC(SPI2, (1), "SPI2"),
	FUNC(SPI2CS1, (1), "SPI2CS1"),
	FUNC(TACH0, (1), "TACH0"),
	FUNC(TACH1, (1), "TACH1"),
	FUNC(TACH10, (1), "TACH10"),
	FUNC(TACH11, (1), "TACH11"),
	FUNC(TACH12, (1), "TACH12"),
	FUNC(TACH13, (1), "TACH13"),
	FUNC(TACH14, (1), "TACH14"),
	FUNC(TACH15, (1), "TACH15"),
	FUNC(TACH2, (1), "TACH2"),
	FUNC(TACH3, (1), "TACH3"),
	FUNC(TACH4, (1), "TACH4"),
	FUNC(TACH5, (1), "TACH5"),
	FUNC(TACH6, (1), "TACH6"),
	FUNC(TACH7, (1), "TACH7"),
	FUNC(TACH8, (1), "TACH8"),
	FUNC(TACH9, (1), "TACH9"),
	FUNC(THRU0, (2), "THRU0"),
	FUNC(THRU1, (2), "THRU1"),
	FUNC(THRU2, (4), "THRU2"),
	FUNC(THRU3, (4), "THRU3"),
	FUNC(UART0, (1), "UART0"),
	FUNC(UART1, (1), "UART1"),
	FUNC(UART10, (3), "UART10"),
	FUNC(UART11, (3), "UART11"),
	FUNC(UART2, (1), "UART2"),
	FUNC(UART3, (1), "UART3"),
	FUNC(UART5, (1), "UART5"),
	FUNC(UART6, (1), "UART6"),
	FUNC(UART7, (1), "UART7"),
	FUNC(UART8, (3), "UART8"),
	FUNC(UART9, (3), "UART9"),
	FUNC(USB2C, (0, 1, 2, 3), "USB2CUD", "USB2CD", "USB2CH", "USB2CU"),
	FUNC(USB2D, (1, 2), "USB2DD", "USB2DH"),
	FUNC(USBUART, (2), "USBUART"),
	FUNC(VGA, (1), "VGA"),
	FUNC(VPI, (5), "VPI"),
	FUNC(WDTRST0N, (2), "WDTRST0N"),
	FUNC(WDTRST1N, (2), "WDTRST1N"),
	FUNC(WDTRST2N, (2), "WDTRST2N"),
	FUNC(WDTRST3N, (2), "WDTRST3N"),
	FUNC(WDTRST4N, (2), "WDTRST4N"),
	FUNC(WDTRST5N, (2), "WDTRST5N"),
	FUNC(WDTRST6N, (2), "WDTRST6N"),
	FUNC(WDTRST7N, (2), "WDTRST7N"),
};

static struct ast2700_soc1_field
ast2700_soc1_pinmux_field_from_pin(unsigned int pin)
{
	return (struct ast2700_soc1_field){
		.reg = AST2700_SOC1_MUX_BASE +
		       (pin / AST2700_SOC1_MUX_PINS_PER_REG) * sizeof(u32),
		.shift = (pin % AST2700_SOC1_MUX_PINS_PER_REG) *
			 AST2700_SOC1_MUX_BITS_PER_PIN,
		.mask = AST2700_SOC1_MUX_MASK,
	};
}

static void __iomem *ast2700_soc1_reg(struct ast2700_soc1_pinctrl_priv *priv,
				      unsigned int offset)
{
	return (u8 __iomem *)priv->scu + offset;
}

static struct ast2700_soc1_field
ast2700_soc1_field_from_pin(unsigned int pin)
{
	switch (pin) {
	case PCIERC2_PERST:
		return (struct ast2700_soc1_field){
			.reg = AST2700_SOC1_PCIE_REG,
			.shift = 0,
			.mask = 0x1,
		};
	case PORTC_MODE:
		return (struct ast2700_soc1_field){
			.reg = AST2700_SOC1_USB_MODE_REG,
			.shift = 0,
			.mask = 0x3,
		};
	case PORTD_MODE:
		return (struct ast2700_soc1_field){
			.reg = AST2700_SOC1_USB_MODE_REG,
			.shift = 2,
			.mask = 0x3,
		};
	case SGMII0:
		return (struct ast2700_soc1_field){
			.reg = AST2700_SOC1_SGMII_REG,
			.shift = 0,
			.mask = 0x1,
		};
	default:
		return ast2700_soc1_pinmux_field_from_pin(pin);
	}
}

static void
ast2700_soc1_set_muxval(struct ast2700_soc1_pinctrl_priv *priv,
			unsigned int pin, unsigned int muxval)
{
	struct ast2700_soc1_field field = ast2700_soc1_field_from_pin(pin);

	/*
	 * PCIERC2_PERST is controlled by a 1-bit enable instead of the
	 * function mux value.
	 */
	if (pin == PCIERC2_PERST)
		muxval = 1;

	clrsetbits_le32(ast2700_soc1_reg(priv, field.reg),
			field.mask << field.shift,
			(muxval & field.mask) << field.shift);
}

static int ast2700_soc1_pinctrl_get_pins_count(struct udevice *dev)
{
	return ARRAY_SIZE(ast2700_soc1_pin_names);
}

static const char *
ast2700_soc1_pinctrl_get_pin_name(struct udevice *dev, unsigned int selector)
{
	if (selector >= ARRAY_SIZE(ast2700_soc1_pin_names))
		return NULL;

	return ast2700_soc1_pin_names[selector];
}

static const struct ast2700_soc1_group *
ast2700_soc1_find_group(const char *name)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(ast2700_soc1_groups); i++) {
		if (!strcmp(ast2700_soc1_groups[i].name, name))
			return &ast2700_soc1_groups[i];
	}

	return NULL;
}

static int ast2700_soc1_pinctrl_get_pin_muxing(struct udevice *dev,
					       unsigned int selector,
					       char *buf, int size)
{
	struct ast2700_soc1_pinctrl_priv *priv = dev_get_priv(dev);
	const struct ast2700_soc1_function *func;
	const struct ast2700_soc1_group *group;
	struct ast2700_soc1_field field;
	unsigned int muxval, want, f, g, i;
	bool adc_pin;

	if (selector >= ARRAY_SIZE(ast2700_soc1_pin_names))
		return -EINVAL;

	field = ast2700_soc1_field_from_pin(selector);
	muxval = (readl(ast2700_soc1_reg(priv, field.reg)) >> field.shift) &
		 field.mask;

	/* Mux value 0 selects GPIO, except on the ADC balls where it is 1 */
	adc_pin = selector >= W17 && selector <= AB19;
	if (selector <= AC24 && muxval == (adc_pin ? 1 : 0)) {
		snprintf(buf, size, "GPIO");
		return 0;
	}

	for (f = 0; f < ARRAY_SIZE(ast2700_soc1_functions); f++) {
		func = &ast2700_soc1_functions[f];
		for (g = 0; g < func->ngroups; g++) {
			want = func->muxvals[g];
			/* PCIERC2_PERST is a 1-bit enable, not a mux value */
			if (selector == PCIERC2_PERST)
				want = 1;
			if (want != muxval)
				continue;
			group = ast2700_soc1_find_group(func->groups[g]);
			if (!group)
				continue;
			for (i = 0; i < group->npins; i++) {
				if (group->pins[i] != selector)
					continue;
				snprintf(buf, size, "%s (%s)", func->name,
					 group->name);
				return 0;
			}
		}
	}

	snprintf(buf, size, "unknown (mux 0x%x)", muxval);

	return 0;
}

static int ast2700_soc1_pinctrl_get_groups_count(struct udevice *dev)
{
	return ARRAY_SIZE(ast2700_soc1_groups);
}

static const char *
ast2700_soc1_pinctrl_get_group_name(struct udevice *dev,
				    unsigned int selector)
{
	if (selector >= ARRAY_SIZE(ast2700_soc1_groups))
		return NULL;

	return ast2700_soc1_groups[selector].name;
}

static int ast2700_soc1_pinctrl_get_functions_count(struct udevice *dev)
{
	return ARRAY_SIZE(ast2700_soc1_functions);
}

static const char *
ast2700_soc1_pinctrl_get_function_name(struct udevice *dev,
				       unsigned int selector)
{
	if (selector >= ARRAY_SIZE(ast2700_soc1_functions))
		return NULL;

	return ast2700_soc1_functions[selector].name;
}

static int
ast2700_soc1_pinctrl_group_set(struct udevice *dev,
			       unsigned int group_selector,
			       unsigned int func_selector)
{
	struct ast2700_soc1_pinctrl_priv *priv = dev_get_priv(dev);
	const struct ast2700_soc1_function *func;
	const struct ast2700_soc1_group *group;
	unsigned int muxval = 0;
	unsigned int i;

	if (group_selector >= ARRAY_SIZE(ast2700_soc1_groups) ||
	    func_selector >= ARRAY_SIZE(ast2700_soc1_functions))
		return -EINVAL;

	group = &ast2700_soc1_groups[group_selector];
	func = &ast2700_soc1_functions[func_selector];

	for (i = 0; i < func->ngroups; i++) {
		if (!strcmp(group->name, func->groups[i])) {
			muxval = func->muxvals[i];
			break;
		}
	}

	if (i == func->ngroups)
		return -EINVAL;

	for (i = 0; i < group->npins; i++)
		ast2700_soc1_set_muxval(priv, group->pins[i], muxval);

	return 0;
}

static int
ast2700_soc1_pinctrl_gpio_request_enable(struct udevice *dev,
					 unsigned int selector)
{
	struct ast2700_soc1_pinctrl_priv *priv = dev_get_priv(dev);
	unsigned int muxval = 0;

	if (selector > AC24)
		return -ENOTSUPP;

	if (selector >= W17 && selector <= AB19)
		muxval = 1;

	ast2700_soc1_set_muxval(priv, selector, muxval);

	return 0;
}

/*
 * Drive strength is controlled by 2-bit fields in the registers at
 * DRV_BASE, but the mapping from pins to fields is sparse and
 * non-linear: only a subset of the pins supports drive strength
 * selection. The table stores (field index + 1) so that the implicit
 * zero initialization marks the unsupported pins.
 */
static const u8 ast2700_soc1_drv_map[] = {
	[C16] = 1,   [C14] = 2,   [C11] = 3,   [D9] = 4,    [F14] = 5,
	[D10] = 6,   [C12] = 7,   [C13] = 8,   [W25] = 9,   [Y23] = 10,
	[Y24] = 11,  [W21] = 12,  [AA23] = 13, [AC22] = 14, [AB22] = 15,
	[Y21] = 16,  [AE20] = 17, [AF19] = 18, [Y22] = 19,  [AA20] = 20,
	[AA22] = 21, [AB20] = 22, [AF18] = 23, [AE19] = 24, [AD20] = 25,
	[AC20] = 26, [AA21] = 27, [AB21] = 28, [AC19] = 29, [AE18] = 30,
	[AD19] = 31, [AD18] = 32, [U25] = 33,  [U26] = 34,  [Y26] = 35,
	[AA24] = 36, [R25] = 37,  [AA26] = 38, [R26] = 39,  [Y25] = 40,
	[B16] = 41,  [D14] = 42,  [B15] = 43,  [B14] = 44,  [C17] = 45,
	[B13] = 46,  [E14] = 47,  [C15] = 48,  [D24] = 49,  [B23] = 50,
	[B22] = 51,  [C23] = 52,  [B18] = 53,  [B21] = 54,  [M15] = 55,
	[B19] = 56,  [B26] = 57,  [A25] = 58,  [A24] = 59,  [B24] = 60,
	[E26] = 61,  [A21] = 62,  [A19] = 63,  [A18] = 64,  [D26] = 65,
	[C26] = 66,  [A23] = 67,  [A22] = 68,  [B25] = 69,  [F26] = 70,
	[A26] = 71,  [A14] = 72,  [E10] = 73,  [E13] = 74,  [D12] = 75,
	[F10] = 76,  [E11] = 77,  [F11] = 78,  [F13] = 79,  [N15] = 80,
	[C20] = 81,  [C19] = 82,  [A8] = 83,   [R14] = 84,  [A7] = 85,
	[P14] = 86,  [D20] = 87,  [A6] = 88,   [B6] = 89,   [N14] = 90,
	[B7] = 91,   [B8] = 92,   [B9] = 93,   [M14] = 94,  [J11] = 95,
	[E7] = 96,   [D19] = 97,  [B11] = 98,  [D15] = 99,  [B12] = 100,
	[B10] = 101, [P13] = 102, [C18] = 103, [C6] = 104,  [C7] = 105,
	[D7] = 106,  [N13] = 107, [C8] = 108,  [C9] = 109,  [C10] = 110,
	[M16] = 111, [A15] = 112, [E9] = 113,  [F9] = 114,  [F8] = 115,
	[M13] = 116, [F7] = 117,  [D8] = 118,  [E8] = 119,  [L12] = 120,
};

static struct ast2700_soc1_field
ast2700_soc1_bias_field_from_pin(unsigned int pin)
{
	return (struct ast2700_soc1_field){
		.reg = AST2700_SOC1_BIAS_BASE +
		       (pin / AST2700_SOC1_BIAS_PINS_PER_REG) * sizeof(u32),
		.shift = pin % AST2700_SOC1_BIAS_PINS_PER_REG,
		.mask = 0x1,
	};
}

static struct ast2700_soc1_field
ast2700_soc1_drv_field_from_idx(unsigned int idx)
{
	return (struct ast2700_soc1_field){
		.reg = AST2700_SOC1_DRV_BASE +
		       (idx / AST2700_SOC1_DRV_PINS_PER_REG) * sizeof(u32),
		.shift = (idx % AST2700_SOC1_DRV_PINS_PER_REG) *
			 AST2700_SOC1_DRV_BITS_PER_PIN,
		.mask = AST2700_SOC1_DRV_MASK,
	};
}

static const struct pinconf_param ast2700_soc1_pinconf_params[] = {
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 0 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 0 },
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 0 },
	{ "drive-strength", PIN_CONFIG_DRIVE_STRENGTH, 0 },
};

static int ast2700_soc1_pinctrl_pinconf_set(struct udevice *dev,
					    unsigned int selector,
					    unsigned int param,
					    unsigned int arg)
{
	struct ast2700_soc1_pinctrl_priv *priv = dev_get_priv(dev);
	struct ast2700_soc1_field field;
	unsigned int val, idx;

	if (selector > AC24)
		return -EINVAL;

	switch (param) {
	case PIN_CONFIG_BIAS_DISABLE:
	case PIN_CONFIG_BIAS_PULL_DOWN:
	case PIN_CONFIG_BIAS_PULL_UP:
		/*
		 * One bias enable bit per pin; the pull direction is fixed
		 * in silicon. Setting the bit disables the bias.
		 */
		field = ast2700_soc1_bias_field_from_pin(selector);
		val = param == PIN_CONFIG_BIAS_DISABLE ? 1 : 0;
		break;
	case PIN_CONFIG_DRIVE_STRENGTH:
		if (selector >= ARRAY_SIZE(ast2700_soc1_drv_map) ||
		    !ast2700_soc1_drv_map[selector])
			return -ENOTSUPP;
		if (arg < AST2700_SOC1_DRV_MIN_MA ||
		    arg > AST2700_SOC1_DRV_MAX_MA ||
		    arg % AST2700_SOC1_DRV_STEP_MA)
			return -EINVAL;
		idx = ast2700_soc1_drv_map[selector] - 1;
		field = ast2700_soc1_drv_field_from_idx(idx);
		val = arg / AST2700_SOC1_DRV_STEP_MA - 1;
		break;
	default:
		return -ENOTSUPP;
	}

	clrsetbits_le32(ast2700_soc1_reg(priv, field.reg),
			field.mask << field.shift, val << field.shift);

	return 0;
}

static int ast2700_soc1_pinctrl_pinconf_group_set(struct udevice *dev,
						  unsigned int selector,
						  unsigned int param,
						  unsigned int arg)
{
	const struct ast2700_soc1_group *group;
	unsigned int i;
	int ret;

	if (selector >= ARRAY_SIZE(ast2700_soc1_groups))
		return -EINVAL;

	group = &ast2700_soc1_groups[selector];
	for (i = 0; i < group->npins; i++) {
		ret = ast2700_soc1_pinctrl_pinconf_set(dev, group->pins[i],
						       param, arg);
		if (ret)
			return ret;
	}

	return 0;
}

static int ast2700_soc1_pinctrl_probe(struct udevice *dev)
{
	struct ast2700_soc1_pinctrl_priv *priv = dev_get_priv(dev);

	priv->scu = dev_remap_addr(dev->parent);
	if (!priv->scu)
		return -ENOMEM;

	return 0;
}

static const struct pinctrl_ops ast2700_soc1_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.get_pins_count = ast2700_soc1_pinctrl_get_pins_count,
	.get_pin_name = ast2700_soc1_pinctrl_get_pin_name,
	.get_pin_muxing = ast2700_soc1_pinctrl_get_pin_muxing,
	.get_groups_count = ast2700_soc1_pinctrl_get_groups_count,
	.get_group_name = ast2700_soc1_pinctrl_get_group_name,
	.get_functions_count = ast2700_soc1_pinctrl_get_functions_count,
	.get_function_name = ast2700_soc1_pinctrl_get_function_name,
	.pinmux_group_set = ast2700_soc1_pinctrl_group_set,
	.gpio_request_enable = ast2700_soc1_pinctrl_gpio_request_enable,
	.pinconf_num_params = ARRAY_SIZE(ast2700_soc1_pinconf_params),
	.pinconf_params = ast2700_soc1_pinconf_params,
	.pinconf_set = ast2700_soc1_pinctrl_pinconf_set,
	.pinconf_group_set = ast2700_soc1_pinctrl_pinconf_group_set,
};

static const struct udevice_id ast2700_soc1_pinctrl_ids[] = {
	{ .compatible = "aspeed,ast2700-soc1-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_ast2700_soc1) = {
	.name = "aspeed_ast2700_soc1_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = ast2700_soc1_pinctrl_ids,
	.priv_auto = sizeof(struct ast2700_soc1_pinctrl_priv),
	.ops = &ast2700_soc1_pinctrl_ops,
	.probe = ast2700_soc1_pinctrl_probe,
};
