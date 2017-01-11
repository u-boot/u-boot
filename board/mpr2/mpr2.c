/*
 * Copyright (C) 2008
 * Mark Jonas <mark.jonas@de.bosch.com>
 *
 * board/mpr2/mpr2.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>

int checkboard(void)
{
	puts("BOARD: MPR2\n");
	return 0;
}

int board_init(void)
{
	/*
	 * For MPR2 A.3 through A.7
	 */

	/* CS2: Ethernet (0xA8000000 - 0xABFFFFFF) */
	__raw_writel(0x36db0400, CS2BCR);    /* 4 idle cycles, normal space, 16 bit data bus */
	__raw_writel(0x000003c0, CS2WCR);    /* (WR:8), no ext. wait */

	/* CS4: CAN1 (0xB0000000 - 0xB3FFFFFF) */
	__raw_writel(0x00000200, CS4BCR);    /* no idle cycles, normal space, 8 bit data bus */
	__raw_writel(0x00100981, CS4WCR);    /* (SW:1.5 WR:3 HW:1.5), ext. wait */

	/* CS5a: CAN2 (0xB4000000 - 0xB5FFFFFF) */
	__raw_writel(0x00000200, CS5ABCR);   /* no idle cycles, normal space, 8 bit data bus */
	__raw_writel(0x00100981, CS5AWCR);   /* (SW:1.5 WR:3 HW:1.5), ext. wait */

	/* CS5b: CAN3 (0xB6000000 - 0xB7FFFFFF) */
	__raw_writel(0x00000200, CS5BBCR);   /* no idle cycles, normal space, 8 bit data bus */
	__raw_writel(0x00100981, CS5BWCR);   /* (SW:1.5 WR:3 HW:1.5), ext. wait */

	/* CS6a: Rotary (0xB8000000 - 0xB9FFFFFF) */
	__raw_writel(0x00000200, CS6ABCR);   /* no idle cycles, normal space, 8 bit data bus */
	__raw_writel(0x001009C1, CS6AWCR);   /* (SW:1.5 WR:3 HW:1.5), no ext. wait */

	/* set Pin Select Register A: /PCC_CD1, /PCC_CD2, PCC_BVD1, PCC_BVD2, /IOIS16, IRQ4, IRQ5, USB1d_SUSPEND */
	__raw_writew(0xAABC, PSELA);  /*    10        10        10        10       10    11    11             00 */

	/* set Pin Select Register B: /SCIF0_RTS, /SCIF0_CTS, LCD_VCPWC, LCD_VEPWC, IIC_SDA, IIC_SCL, Reserved */
	__raw_writew(0x3C00, PSELB);  /*       0           0         11         11        0        0  00000000 */

	/* set Pin Select Register C: SIOF1_SCK, SIOF1_RxD, SCIF1_RxD, SCIF1_TxD, Reserved */
	__raw_writew(0x0000, PSELC);  /*     00         00         00         00  00000000 */

	/* set Pin Select Register D: Reserved, SIOF1_TxD, Reserved, SIOF1_MCLK, Reserved, SIOF1_SYNC, Reserved, SCIF1_SCK, Reserved */
	__raw_writew(0x0000, PSELD);  /*     0         00        00          00        00          00        00         00         0 */

	/* OTH:  (00) Other fuction
	 * GPO:  (01) General Purpose Output
	 * GPI:  (11) General Purpose Input
	 * GPI+: (10) General Purpose Input with internal pull-up
	 *-------------------------------------------------------
	 * A7 GPO(LED8);     A6 GPO(LED7);     A5 GPO(LED6);        A4 GPO(LED5);
	 * A3 GPO(LED4);     A2 GPO(LED3);     A1 GPO(LED2);        A0 GPO(LED1); */
	__raw_writew(0x5555, PACR);   /* 01 01 01 01 01 01 01 01 */

	/* B7 GPO(RST4);     B6 GPO(RST3);     B5 GPO(RST2);        B4 GPO(RST1);
	 * B3 GPO(PB3);      B2 GPO(PB2);      B1 GPO(PB1);         B0 GPO(PB0); */
	__raw_writew(0x5555, PBCR);   /* 01 01 01 01 01 01 01 01 */

	/* C7 GPO(PC7);      C6 GPO(PC6);      C5 GPO(PC5);         C4 GPO(PC4);
	 * C3 LCD_DATA3;     C2 LCD_DATA2;     C1 LCD_DATA1;        C0 LCD_DATA0; */
	__raw_writew(0x5500, PCCR);   /* 01 01 01 01 00 00 00 00 */

	/* D7 GPO(PD7);      D6 GPO(PD6);      D5 GPO(PD5);         D4 GPO(PD4);
	 * D3 GPO(PD3);      D2 GPO(PD2);      D1 GPO(PD1);         D0 GPO(PD0); */
	__raw_writew(0x5555, PDCR);   /* 01 01 01 01 01 01 01 01 */

	/* E7 (x);           E6 GPI(nu);       E5 GPI(nu);          E4 LCD_M_DISP;
	 * E3 LCD_CL1;       E2 LCD_CL2;       E1 LCD_DON;          E0 LCD_FLM; */
	__raw_writew(0x2800, PECR);   /* 00 10 10 00 00 00 00 00 */

	/* F7 (x);           F6 DA1(VLCD);     F5 DA0(nc);          F4 AN3;
	 * F3 AN2(MID_AD);   F2 AN1(EARTH_AD); F1 AN0(TEMP);        F0 GPI+(nc); */
	__raw_writew(0x0002, PFCR);   /* 00 00 00 00 00 00 00 10 */

	/* G7 (x);          G6 IRQ5(TOUCH_BUSY); G5 IRQ4(TOUCH_IRQ);G4 GPI(KEY2);
	 * G3 GPI(KEY1);     G2 GPO(LED11);      G1 GPO(LED10);     G0 GPO(LED9); */
	__raw_writew(0x03D5, PGCR);   /* 00 00 00 11 11 01 01 01 */

	/* H7 (x);            H6 /RAS(BRAS);      H5 /CAS(BCAS);    H4 CKE(BCKE);
	 * H3 GPO(EARTH_OFF); H2 GPO(EARTH_TEST); H1 USB2_PWR;      H0 USB1_PWR; */
	__raw_writew(0x0050, PHCR);   /* 00 00 00 00 01 01 00 00 */

	/* J7 (x);           J6 AUDCK;         J5 ASEBRKAK;         J4 AUDATA3;
	 * J3 AUDATA2;       J2 AUDATA1;       J1 AUDATA0;          J0 AUDSYNC; */
	__raw_writew(0x0000, PJCR);   /* 00 00 00 00 00 00 00 00 */

	/* K7 (x);           K6 (x);           K5 (x);              K4 (x)
	 * K3 PINT7(/PWR2);  K2 PINT6(/PWR1);  K1 PINT5(nc);        K0 PINT4(FLASH_READY); */
	__raw_writew(0x00FB, PKCR);   /* 00 00 00 00 11 11 10 11 */

	/* L7 TRST;          L6 TMS;           L5 TDO;              L4 TDI;
	 * L3 TCK;           L2 (x);           L1 (x);              L0 (x); */
	__raw_writew(0x0000, PLCR);    /* 00 00 00 00 00 00 00 00 */

	/* M7 GPO(CURRENT_SINK);M6 GPO(PWR_SWITCH);  M5 GPO(LAN_SPEED);   M4 GPO(LAN_RESET);
	 * M3 GPO(BUZZER);      M2 GPO(LCD_BL);      M1 CS5B(CAN3_CS);    M0 GPI+(nc); */
	__raw_writew(0x5552, PMCR);   /* 01 01 01 01 01 01 00 10 */
	__raw_writeb(0xF0, PMDR);     /* CURRENT_SINK=off, PWR_SWITCH=off, LAN_SPEED=100MBit, LAN_RESET=off, BUZZER=off, LCD_BL=off */

	/* P7 (x);           P6 (x);           P5 (x);              P4 GPO(on pullup);
	 * P3 IRQ3(LAN_IRQ); P2 IRQ2(CAN3_IRQ);P1 IRQ1(CAN2_IRQ);   P0 IRQ0(CAN1_IRQ); */
	__raw_writew(0x0100, PPCR);   /* 00 00 00 01 00 00 00 00 */
	__raw_writeb(0x10, PPDR);     /* no current flow through pullup */

	/* R7 A25;           R6 A24;           R5 A23;              R4 A22;
	 * R3 A21;           R2 A20;           R1 A19;              R0 A0; */
	__raw_writew(0x0000, PRCR);   /* 00 00 00 00 00 00 00 00 */

	/* S7 (x);              S6 (x);        S5 (x);              S4 GPO(EEPROM_CS2);
	 * S3 GPO(EEPROM_CS1);  S2 SIOF0_TXD;  S1 SIOF0_RXD;        S0 SIOF0_SCK; */
	__raw_writew(0x0140, PSCR);   /* 00 00 00 01 01 00 00 00 */

	/* T7 (x);           T6 (x);           T5 (x);              T4 COM1_CTS;
	 * T3 COM1_RTS;      T2 COM1_TXD;      T1 COM1_RXD;         T0 GPO(WDOG); */
	__raw_writew(0x0001, PTCR);   /* 00 00 00 00 00 00 00 01 */

	/* U7 (x);           U6 (x);           U5 (x);              U4 GPI+(/AC_FAULT);
	 * U3 GPO(TOUCH_CS); U2 TOUCH_TXD;     U1 TOUCH_RXD;        U0 TOUCH_SCK; */
	__raw_writew(0x0240, PUCR);   /* 00 00 00 10 01 00 00 00 */

	/* V7 (x);           V6 (x);           V5 (x);              V4 GPO(MID2);
	 * V3 GPO(MID1);     V2 CARD_TxD;      V1 CARD_RxD;         V0 GPI+(/BAT_FAULT); */
	__raw_writew(0x0142, PVCR);   /* 00 00 00 01 01 00 00 10 */

	return 0;
}
