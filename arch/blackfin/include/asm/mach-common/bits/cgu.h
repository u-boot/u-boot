/*
 * CGU Masks
 */

#ifndef __BFIN_PERIPHERAL_CGU__
#define __BFIN_PERIPHERAL_CGU__

/* CGU_CTL Masks */
#define DF			(1 << 0)
#define MSEL			(0x7f << MSEL_P)
#define WIDLE			(1 << WIDLE_P)
#define LOCK			(1 << LOCK_P)

#define DF_P			0
#define MSEL_P			8
#define WIDLE_P			30
#define LOCK_P			31
#define MSEL_MASK               0x7F00
#define DF_MASK                 0x1

/* CGU_STAT Masks */
#define PLLEN			(1 << 0)
#define PLLBP			(1 << 1)
#define PLLLK			(1 << 2)
#define CLKSALGN		(1 << 3)
#define CCBF0EN			(1 << 4)
#define CCBF1EN			(1 << 5)
#define SCBF0EN			(1 << 6)
#define SCBF1EN			(1 << 7)
#define DCBFEN			(1 << 8)
#define OCBFEN			(1 << 9)
#define ADRERR			(1 << 16)
#define LWERR			(1 << 17)
#define DIVERR			(1 << 18)
#define WDFMSERR		(1 << 19)
#define WDIVERR			(1 << 20)
#define PLLLKERR		(1 << 21)

/* CGU_DIV Masks */
#define CSEL			(0x1f << CSEL_P)
#define S0SEL			(3 << S0SEL_P)
#define SYSSEL			(0x1f << SYSSEL_P)
#define S1SEL			(3 << S1SEL_P)
#define DSEL			(0x1f << DSEL_P)
#define OSEL			(0x7f << OSEL_P)
#define ALGN			(1 << ALGN_P)
#define UPDT			(1 << UPDT_P)
#define LOCK			(1 << LOCK_P)

#define CSEL_P			0
#define S0SEL_P			5
#define SYSSEL_P		8
#define S1SEL_P			13
#define DSEL_P			16
#define OSEL_P			22
#define ALGN_P			29
#define UPDT_P			30
#define LOCK_P			31

/* CGU_CLKOUTSEL Masks */
#define CLKOUTSEL		(0xf << 0)
#define USBCLKSEL		(0x3f << 16)
#define LOCK			(1 << LOCK_P)

#define LOCK_P			31

#define CLKOUTSEL_CLKIN		0x0
#define CLKOUTSEL_CCLK		0x1
#define CLKOUTSEL_SYSCLK	0x2
#define CLKOUTSEL_SCLK0		0x3
#define CLKOUTSEL_SCLK1		0x4
#define CLKOUTSEL_DCLK		0x5
#define CLKOUTSEL_USB_PLL	0x6
#define CLKOUTSEL_OUTCLK	0x7
#define CLKOUTSEL_USB_CLKIN	0x8
#define CLKOUTSEL_WDOG		0x9
#define CLKOUTSEL_PMON		0xA
#define CLKOUTSEL_GND		0xB

#endif
