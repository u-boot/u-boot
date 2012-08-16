/*
 * PLL Masks
 */

#ifndef __BFIN_PERIPHERAL_PLL__
#define __BFIN_PERIPHERAL_PLL__

/* PLL_CTL Masks */
#define DF			0x0001		/* 0: PLL = CLKIN, 1: PLL = CLKIN/2 */
#define PLL_OFF			0x0002		/* PLL Not Powered */
#define STOPCK			0x0008		/* Core Clock Off */
#define PDWN			0x0020		/* Enter Deep Sleep Mode */
#define IN_DELAY		0x0040		/* Add 200ps Delay To EBIU Input Latches */
#define OUT_DELAY		0x0080		/* Add 200ps Delay To EBIU Output Signals */
#define BYPASS			0x0100		/* Bypass the PLL */
#define MSEL			0x7E00		/* Multiplier Select For CCLK/VCO Factors */
#define SPORT_HYST		0x8000		/* Enable Additional Hysteresis on SPORT Input Pins */

#define MSEL_P			9

/* PLL_DIV Masks */
#define SSEL			0x000F		/* System Select */
#define CSEL			0x0030		/* Core Select */
#define CSEL_DIV1		0x0000		/* CCLK = VCO / 1 */
#define CSEL_DIV2		0x0010		/* CCLK = VCO / 2 */
#define CSEL_DIV4		0x0020		/* CCLK = VCO / 4 */
#define CSEL_DIV8		0x0030		/* CCLK = VCO / 8 */

#define CCLK_DIV1		CSEL_DIV1
#define CCLK_DIV2		CSEL_DIV2
#define CCLK_DIV4		CSEL_DIV4
#define CCLK_DIV8		CSEL_DIV8

#define SSEL_P			0
#define CSEL_P			4

/* PLL_STAT Masks */
#define ACTIVE_PLLENABLED	0x0001		/* Processor In Active Mode With PLL Enabled */
#define FULL_ON			0x0002		/* Processor In Full On Mode */
#define ACTIVE_PLLDISABLED	0x0004		/* Processor In Active Mode With PLL Disabled */
#define DEEP_SLEEP		0x0008		/* Processor In Deep Sleep Mode */
#define SLEEP			0x0010		/* Processor In Sleep Mode */
#define PLL_LOCKED		0x0020		/* PLL_LOCKCNT Has Been Reached */
#define CORE_IDLE		0x0040		/* Processor In IDLE Mode */
#define VSTAT			0x0080		/* Voltage Regulator Has Reached Programmed Voltage */

/* VR_CTL Masks */
#ifdef __ADSPBF52x__
#define FREQ_MASK		0x3000		/* Switching Oscillator Frequency For Regulator */
#define FREQ_HIBERNATE		0x0000		/* Powerdown/Bypass On-Board Regulation */
#define FREQ_1000		0x3000		/* Switching Frequency Is 1 MHz */
#else
#define FREQ_MASK		0x0003		/* Switching Oscillator Frequency For Regulator */
#define FREQ_HIBERNATE		0x0000		/* Powerdown/Bypass On-Board Regulation */
#define FREQ_333		0x0001		/* Switching Frequency Is 333 kHz */
#define FREQ_667		0x0002		/* Switching Frequency Is 667 kHz */
#define FREQ_1000		0x0003		/* Switching Frequency Is 1 MHz */
#endif

#define GAIN_MASK		0x000C		/* Voltage Level Gain */
#define GAIN_5			0x0000		/* GAIN = 5 */
#define GAIN_10			0x0004		/* GAIN = 10 */
#define GAIN_20			0x0008		/* GAIN = 20 */
#define GAIN_50			0x000C		/* GAIN = 50 */

#ifdef __ADSPBF52x__
#define VLEV_MASK		0x00F0		/* Internal Voltage Level */
#define VLEV_085		0x0040		/* VLEV = 0.85 V (-5% - +10% Accuracy) */
#define VLEV_090		0x0050		/* VLEV = 0.90 V (-5% - +10% Accuracy) */
#define VLEV_095		0x0060		/* VLEV = 0.95 V (-5% - +10% Accuracy) */
#define VLEV_100		0x0070		/* VLEV = 1.00 V (-5% - +10% Accuracy) */
#define VLEV_105		0x0080		/* VLEV = 1.05 V (-5% - +10% Accuracy) */
#define VLEV_110		0x0090		/* VLEV = 1.10 V (-5% - +10% Accuracy) */
#define VLEV_115		0x00A0		/* VLEV = 1.15 V (-5% - +10% Accuracy) */
#define VLEV_120		0x00B0		/* VLEV = 1.20 V (-5% - +10% Accuracy) */
#else
#define VLEV_MASK		0x00F0		/* Internal Voltage Level */
#define VLEV_085		0x0060		/* VLEV = 0.85 V (-5% - +10% Accuracy) */
#define VLEV_090		0x0070		/* VLEV = 0.90 V (-5% - +10% Accuracy) */
#define VLEV_095		0x0080		/* VLEV = 0.95 V (-5% - +10% Accuracy) */
#define VLEV_100		0x0090		/* VLEV = 1.00 V (-5% - +10% Accuracy) */
#define VLEV_105		0x00A0		/* VLEV = 1.05 V (-5% - +10% Accuracy) */
#define VLEV_110		0x00B0		/* VLEV = 1.10 V (-5% - +10% Accuracy) */
#define VLEV_115		0x00C0		/* VLEV = 1.15 V (-5% - +10% Accuracy) */
#define VLEV_120		0x00D0		/* VLEV = 1.20 V (-5% - +10% Accuracy) */
#define VLEV_125		0x00E0		/* VLEV = 1.25 V (-5% - +10% Accuracy) */
#define VLEV_130		0x00F0		/* VLEV = 1.30 V (-5% - +10% Accuracy) */
#endif

#define WAKE			0x0100		/* Enable RTC/Reset Wakeup From Hibernate */
#define CANWE			0x0200		/* Enable CAN Wakeup From Hibernate */
#define PHYWE			0x0400		/* Enable PHY Wakeup From Hibernate */
#define GPWE			0x0400		/* General-purpose Wakeup From Hibernate */
#define MXVRWE			0x0400		/* MXVR Wakeup From Hibernate */
#define USBWE			0x0800		/* USB Wakeup From Hibernate */
#define KPADWE			0x1000		/* Keypad Wakeup From Hibernate */
#define ROTWE			0x2000		/* Rotary Counter Wakeup From Hibernate */
#define CLKBUFOE		0x4000		/* CLKIN Buffer Output Enable */
#define CKELOW			0x8000		/* Enable Drive CKE Low During Reset */

#endif
