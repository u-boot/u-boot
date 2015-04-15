/*
 * Copyright (C) 2009 Renesas Solutions Corp.
 * Copyright (C) 2009 Nobuhiro Iwamatsu <iwamatsu.nobuhiro@renesas.com>
 *
 * board/espt/lowlevel_init.S
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <asm/processor.h>
#include <asm/macro.h>

	.global	lowlevel_init

	.text
	.align	2

lowlevel_init:

	write32 WDTCSR_A, WDTCSR_D

	write32 WDTST_A, WDTST_D

	write32 WDTBST_A, WDTBST_D

	write32 CCR_A, CCR_CACHE_ICI_D

	write32 MMUCR_A, MMU_CONTROL_TI_D

	write32 MSTPCR0_A, MSTPCR0_D

	write32 MSTPCR1_A, MSTPCR1_D

	write32 RAMCR_A, RAMCR_D

	/*
	 * Setting infomation from
	 * original ESPT-GIGA bootloader register
	 */
	write32 MMSEL_A, MMSEL_D

	/* dummy */
	mov.l   @r1, r2
	mov.l   @r1, r2
	synco

    write32 BCR_A, BCR_D

    write32 CS0BCR_A, CS0BCR_D

    write32 CS0WCR_A, CS0WCR_D

	/*
	 * DDR-SDRAM setting
	 */

	/* set DDR-SDRAM dummy read */
	write32 MMSEL_A, MMSEL_D

	write32 MMSEL_A, CS0_A

	/* set DDR-SDRAM bus/endian etc */
	write32 MIM_U_A, MIM_U_D

	write32 MIM_L_A, MIM_L_D0

	write32 SDR_L_A, SDR_L_A_D0

	write32 STR_L_A, STR_L_A_D0

	/* DDR-SDRAM access control */
	write32 MIM_L_A, MIM_L_D1

	write32 SCR_L_A, SCR_L_A_D0

	write32 SCR_L_A, SCR_L_A_D1

	write32 EMRS_A, EMRS_D

	write32 MRS1_A, MRS1_D

	write32 MIM_U_A, MIM_U_D

	write32 MIM_L_A, MIM_L_A_D2

	write32 SCR_L_A, SCR_L_A_D2

	write32 SCR_L_A, SCR_L_A_D2

	write32 MRS2_A, MRS2_D

	/* wait 200us */
	wait_timer REPEAT_R3

	/* GPIO setting */
	write16 PSEL0_A, PSEL0_D

	write16 PSEL1_A, PSEL1_D

	write16 PSEL2_A, PSEL2_D

	write16 PSEL3_A, PSEL3_D

	write16 PSEL4_A, PSEL4_D

	write8 PADR_A, PADR_D

	write16 PACR_A, PACR_D

	write8 PBDR_A, PBDR_D

	write16 PBCR_A, PBCR_D

	write8 PCDR_A, PCDR_D

	write16 PCCR_A, PCCR_D

	write8	PDDR_A, PDDR_D

	write16 PDCR_A, PDCR_D

	write16 PECR_A, PECR_D

	write16 PFCR_A, PFCR_D

	write16 PGCR_A, PGCR_D

	write16 PHCR_A, PHCR_D

	write16 PICR_A, PICR_D

	write8 PJDR_A, PJDR_D

	write16 PJCR_A, PJCR_D

	/* wait 50us */
	wait_timer REPEAT_R3

	write8 PKDR_A, PKDR_D

	write16 PKCR_A, PKCR_D

	write16 PLCR_A, PLCR_D

	write16 PMCR_A, PMCR_D

	write16 PNCR_A, PNCR_D

	write16 POCR_A, POCR_D


	/* ICR0 ,ICR1 */
	write32 ICR0_A, ICR0_D

	write32 ICR1_A, ICR1_D

	/* USB Host */
	write32 USB_USBHSC_A, USB_USBHSC_D

	write32 CCR_A, CCR_CACHE_D_2

	rts
	nop

	.align	2

/* GPIO Crontrol Register */
PACR_A:	.long	0xFFEF0000
PBCR_A:	.long	0xFFEF0002
PCCR_A:	.long	0xFFEF0004
PDCR_A:	.long	0xFFEF0006
PECR_A:	.long	0xFFEF0008
PFCR_A:	.long	0xFFEF000A
PGCR_A:	.long	0xFFEF000C
PHCR_A:	.long	0xFFEF000E
PICR_A:	.long	0xFFEF0010
PJCR_A:	.long	0xFFEF0012
PKCR_A:	.long	0xFFEF0014
PLCR_A:	.long	0xFFEF0016
PMCR_A:	.long	0xFFEF0018
PNCR_A:	.long	0xFFEF001A
POCR_A:	.long	0xFFEF001C

/* GPIO Data Register */
PADR_A:	.long	0xFFEF0020
PBDR_A:	.long	0xFFEF0022
PCDR_A:	.long	0xFFEF0024
PDDR_A:	.long	0xFFEF0026
PJDR_A:	.long	0xFFEF0032
PKDR_A:	.long	0xFFEF0034

/* GPIO Set data */
PADR_D:	.long	0x00000000
PACR_D:	.word 	0x1400
.align 2
PBDR_D:	.long	0x00000000
PBCR_D:	.word	0x555A
.align 2
PCDR_D:	.long	0x00000000
PCCR_D:	.word	0x5555
.align 2
PDDR_D:	.long	0x00000000
PDCR_D:	.word	0x0155
PECR_D:	.word	0x0000
PFCR_D:	.word	0x0000
PGCR_D:	.word	0x0000
PHCR_D:	.word	0x0000
PICR_D:	.word	0x0800
PJDR_D:	.long	0x00000006
PJCR_D:	.word	0x5A57
.align 2
PKDR_D:	.long	0x00000000
PKCR_D:	.word	0xFFF9
.align 2
PLCR_D:	.word 	0xC330
PMCR_D:	.word	0xFFFF
PNCR_D:	.word	0x0242
POCR_D:	.word	0x0000

/* Pin Select */
PSEL0_A:	.long	0xFFEF0070
PSEL1_A:	.long	0xFFEF0072
PSEL2_A:	.long	0xFFEF0074
PSEL3_A:	.long	0xFFEF0076
PSEL4_A:	.long	0xFFEF0078
PSEL0_D:	.word	0x0001
PSEL1_D:	.word	0x2400
PSEL2_D:	.word	0x0000
PSEL3_D:	.word	0x2421
PSEL4_D:	.word	0x0000
.align 2

MMSEL_A:	.long	0xFE600020
BCR_A:		.long	0xFF801000
CS0BCR_A:	.long	0xFF802000
CS0WCR_A:	.long	0xFF802008
ICR0_A:		.long	0xFFD00000
ICR1_A:		.long	0xFFD0001C

MMSEL_D:	.long	0xA5A50000
BCR_D:		.long	0x05000000
CS0BCR_D:	.long	0x232306F0
CS0WCR_D:	.long	0x00011104
ICR0_D:		.long	0x80C00000
ICR1_D:		.long	0x00020000

/* RWBT Address */
WDTST_A:	.long	0xFFCC0000
WDTCSR_A:	.long	0xFFCC0004
WDTBST_A:	.long	0xFFCC0008
/* RWBT Data */
WDTST_D:	.long	0x5A000FFF
WDTCSR_D:	.long	0xA5000000
WDTBST_D:	.long	0x55000000

/* Cache Address */
CCR_A:		.long	0xFF00001C
MMUCR_A:	.long	0xFF000010
RAMCR_A:	.long	0xFF000074

/* Cache Data */
CCR_CACHE_ICI_D:.long	0x00000800
CCR_CACHE_D_2:	.long	0x00000103
MMU_CONTROL_TI_D:.long	0x00000004
RAMCR_D:	.long	0x00000200

/* Low power mode control Address */
MSTPCR0_A:	.long	0xFFC80030
MSTPCR1_A:	.long	0xFFC80038
/* Low power mode control Data */
MSTPCR0_D:	.long	0x00000000
MSTPCR1_D:	.long	0x00000000

REPEAT0_R3:	.long	0x00002000
REPEAT_R3:	.long	0x00000200
CS0_A:		.long	0xA8000000

MIM_U_A:	.long	0xFE800008
MIM_L_A:	.long	0xFE80000C
SCR_U_A:	.long	0xFE800010
SCR_L_A:	.long	0xFE800014
STR_U_A:	.long	0xFE800018
STR_L_A:	.long	0xFE80001C
SDR_U_A:	.long	0xFE800030
SDR_L_A:	.long	0xFE800034
EMRS_A:		.long	0xFE902000
MRS1_A:		.long	0xFE900B08
MRS2_A:		.long	0xFE900308

MIM_U_D:	.long	0x00000000
MIM_L_D0:	.long	0x04100008
MIM_L_D1:	.long	0x02EE0009
MIM_L_D2:	.long	0x02EE0209

SDR_L_A_D0:	.long	0x00000300
STR_L_A_D0:	.long	0x00010040
MIM_L_A_D1:	.long	0x04100009
SCR_L_A_D0:	.long 	0x00000003
SCR_L_A_D1:	.long 	0x00000002
MIM_L_A_D2:	.long	0x04100209
SCR_L_A_D2:	.long	0x00000004

SCR_L_NORMAL:	.long	0x00000000
SCR_L_NOP:		.long	0x00000001
SCR_L_PALL:		.long	0x00000002
SCR_L_CKE_EN:	.long	0x00000003
SCR_L_CBR:		.long	0x00000004

STR_L_D:	.long	0x000F3980
SDR_L_D:	.long	0x00000400
EMRS_D:		.long	0x00000000
MRS1_D:		.long	0x00000000
MRS2_D:		.long	0x00000000

/* USB */
USB_USBHSC_A:	.long	0xFFEC80F0
USB_USBHSC_D:	.long	0x00000000
