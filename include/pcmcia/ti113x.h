/*
 * ti113x.h 1.31 2002/05/12 18:19:47
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License
 * at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and
 * limitations under the License.
 *
 * The initial developer of the original code is David A. Hinds
 * <dahinds@users.sourceforge.net>.  Portions created by David A. Hinds
 * are Copyright (C) 1999 David A. Hinds.  All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU General Public License version 2 (the "GPL"), in
 * which case the provisions of the GPL are applicable instead of the
 * above.  If you wish to allow the use of your version of this file
 * only under the terms of the GPL and not to allow others to use
 * your version of this file under the MPL, indicate your decision by
 * deleting the provisions above and replace them with the notice and
 * other provisions required by the GPL.  If you do not delete the
 * provisions above, a recipient may use your version of this file
 * under either the MPL or the GPL.
 */

#ifndef _LINUX_TI113X_H
#define _LINUX_TI113X_H

#ifndef PCI_VENDOR_ID_TI
#define PCI_VENDOR_ID_TI		0x104c
#endif

#ifndef PCI_DEVICE_ID_TI_1130
#define PCI_DEVICE_ID_TI_1130		0xac12
#endif
#ifndef PCI_DEVICE_ID_TI_1031
#define PCI_DEVICE_ID_TI_1031		0xac13
#endif
#ifndef PCI_DEVICE_ID_TI_1131
#define PCI_DEVICE_ID_TI_1131		0xac15
#endif
#ifndef PCI_DEVICE_ID_TI_1210
#define PCI_DEVICE_ID_TI_1210		0xac1a
#endif
#ifndef PCI_DEVICE_ID_TI_1211
#define PCI_DEVICE_ID_TI_1211		0xac1e
#endif
#ifndef PCI_DEVICE_ID_TI_1220
#define PCI_DEVICE_ID_TI_1220		0xac17
#endif
#ifndef PCI_DEVICE_ID_TI_1221
#define PCI_DEVICE_ID_TI_1221		0xac19
#endif
#ifndef PCI_DEVICE_ID_TI_1250A
#define PCI_DEVICE_ID_TI_1250A		0xac16
#endif
#ifndef PCI_DEVICE_ID_TI_1225
#define PCI_DEVICE_ID_TI_1225		0xac1c
#endif
#ifndef PCI_DEVICE_ID_TI_1251A
#define PCI_DEVICE_ID_TI_1251A		0xac1d
#endif
#ifndef PCI_DEVICE_ID_TI_1251B
#define PCI_DEVICE_ID_TI_1251B		0xac1f
#endif
#ifndef PCI_DEVICE_ID_TI_1410
#define PCI_DEVICE_ID_TI_1410		0xac50
#endif
#ifndef PCI_DEVICE_ID_TI_1420
#define PCI_DEVICE_ID_TI_1420		0xac51
#endif
#ifndef PCI_DEVICE_ID_TI_1450
#define PCI_DEVICE_ID_TI_1450		0xac1b
#endif
#ifndef PCI_DEVICE_ID_TI_1451
#define PCI_DEVICE_ID_TI_1451		0xac52
#endif
#ifndef PCI_DEVICE_ID_TI_1510
#define PCI_DEVICE_ID_TI_1510		0xac56
#endif
#ifndef PCI_DEVICE_ID_TI_4410
#define PCI_DEVICE_ID_TI_4410		0xac41
#endif
#ifndef PCI_DEVICE_ID_TI_4450
#define PCI_DEVICE_ID_TI_4450		0xac40
#endif
#ifndef PCI_DEVICE_ID_TI_4451
#define PCI_DEVICE_ID_TI_4451		0xac42
#endif

/* Register definitions for TI 113X PCI-to-CardBus bridges */

/* System Control Register */
#define TI113X_SYSTEM_CONTROL		0x80	/* 32 bit */
#define  TI113X_SCR_SMIROUTE		0x04000000
#define  TI113X_SCR_SMISTATUS		0x02000000
#define  TI113X_SCR_SMIENB		0x01000000
#define  TI113X_SCR_VCCPROT		0x00200000
#define  TI113X_SCR_REDUCEZV		0x00100000
#define  TI113X_SCR_CDREQEN		0x00080000
#define  TI113X_SCR_CDMACHAN		0x00070000
#define  TI113X_SCR_SOCACTIVE		0x00002000
#define  TI113X_SCR_PWRSTREAM		0x00000800
#define  TI113X_SCR_DELAYUP		0x00000400
#define  TI113X_SCR_DELAYDOWN		0x00000200
#define  TI113X_SCR_INTERROGATE		0x00000100
#define  TI113X_SCR_CLKRUN_SEL		0x00000080
#define  TI113X_SCR_PWRSAVINGS		0x00000040
#define  TI113X_SCR_SUBSYSRW		0x00000020
#define  TI113X_SCR_CB_DPAR		0x00000010
#define  TI113X_SCR_CDMA_EN		0x00000008
#define  TI113X_SCR_ASYNC_IRQ		0x00000004
#define  TI113X_SCR_KEEPCLK		0x00000002
#define  TI113X_SCR_CLKRUN_ENA		0x00000001

#define  TI122X_SCR_SER_STEP		0xc0000000
#define  TI122X_SCR_INTRTIE		0x20000000
#define  TI122X_SCR_P2CCLK		0x08000000
#define  TI122X_SCR_CBRSVD		0x00400000
#define  TI122X_SCR_MRBURSTDN		0x00008000
#define  TI122X_SCR_MRBURSTUP		0x00004000
#define  TI122X_SCR_RIMUX		0x00000001

/* Multimedia Control Register */
#define TI1250_MULTIMEDIA_CTL		0x84	/* 8 bit */
#define  TI1250_MMC_ZVOUTEN		0x80
#define  TI1250_MMC_PORTSEL		0x40
#define  TI1250_MMC_ZVEN1		0x02
#define  TI1250_MMC_ZVEN0		0x01

#define TI1250_GENERAL_STATUS		0x85	/* 8 bit */
#define TI1250_GPIO0_CONTROL		0x88	/* 8 bit */
#define TI1250_GPIO1_CONTROL		0x89	/* 8 bit */
#define TI1250_GPIO2_CONTROL		0x8a	/* 8 bit */
#define TI1250_GPIO3_CONTROL		0x8b	/* 8 bit */
#define TI12XX_IRQMUX			0x8c	/* 32 bit */

/* Retry Status Register */
#define TI113X_RETRY_STATUS		0x90	/* 8 bit */
#define  TI113X_RSR_PCIRETRY		0x80
#define  TI113X_RSR_CBRETRY		0x40
#define  TI113X_RSR_TEXP_CBB		0x20
#define  TI113X_RSR_MEXP_CBB		0x10
#define  TI113X_RSR_TEXP_CBA		0x08
#define  TI113X_RSR_MEXP_CBA		0x04
#define  TI113X_RSR_TEXP_PCI		0x02
#define  TI113X_RSR_MEXP_PCI		0x01

/* Card Control Register */
#define TI113X_CARD_CONTROL		0x91	/* 8 bit */
#define  TI113X_CCR_RIENB		0x80
#define  TI113X_CCR_ZVENABLE		0x40
#define  TI113X_CCR_PCI_IRQ_ENA		0x20
#define  TI113X_CCR_PCI_IREQ		0x10
#define  TI113X_CCR_PCI_CSC		0x08
#define  TI113X_CCR_SPKROUTEN		0x02
#define  TI113X_CCR_IFG			0x01

#define  TI1220_CCR_PORT_SEL		0x20
#define  TI122X_CCR_AUD2MUX		0x04

/* Device Control Register */
#define TI113X_DEVICE_CONTROL		0x92	/* 8 bit */
#define  TI113X_DCR_5V_FORCE		0x40
#define  TI113X_DCR_3V_FORCE		0x20
#define  TI113X_DCR_IMODE_MASK		0x06
#define  TI113X_DCR_IMODE_ISA		0x02
#define  TI113X_DCR_IMODE_SERIAL	0x04

#define  TI12XX_DCR_IMODE_PCI_ONLY	0x00
#define  TI12XX_DCR_IMODE_ALL_SERIAL	0x06

/* Buffer Control Register */
#define TI113X_BUFFER_CONTROL		0x93	/* 8 bit */
#define  TI113X_BCR_CB_READ_DEPTH	0x08
#define  TI113X_BCR_CB_WRITE_DEPTH	0x04
#define  TI113X_BCR_PCI_READ_DEPTH	0x02
#define  TI113X_BCR_PCI_WRITE_DEPTH	0x01

/* Diagnostic Register */
#define TI1250_DIAGNOSTIC		0x93	/* 8 bit */
#define  TI1250_DIAG_TRUE_VALUE		0x80
#define  TI1250_DIAG_PCI_IREQ		0x40
#define  TI1250_DIAG_PCI_CSC		0x20
#define  TI1250_DIAG_ASYNC_CSC		0x01

/* DMA Registers */
#define TI113X_DMA_0			0x94	/* 32 bit */
#define TI113X_DMA_1			0x98	/* 32 bit */

/* ExCA IO offset registers */
#define TI113X_IO_OFFSET(map)		(0x36+((map)<<1))

/* Data structure for tracking vendor-specific state */
typedef struct ti113x_state_t {
    u32			sysctl;		/* TI113X_SYSTEM_CONTROL */
    u8			cardctl;	/* TI113X_CARD_CONTROL */
    u8			devctl;		/* TI113X_DEVICE_CONTROL */
    u8			diag;		/* TI1250_DIAGNOSTIC */
    u32			irqmux;		/* TI12XX_IRQMUX */
} ti113x_state_t;

#define TI_PCIC_ID \
    IS_TI1130, IS_TI1131, IS_TI1031, IS_TI1210, IS_TI1211,	\
    IS_TI1220, IS_TI1221, IS_TI1225, IS_TI1250A, IS_TI1251A,	\
    IS_TI1251B, IS_TI1410, IS_TI1420, IS_TI1450, IS_TI1451,	\
    IS_TI1510, IS_TI4410, IS_TI4450, IS_TI4451

#define TI_PCIC_INFO \
    { "TI 1130",  IS_TI|IS_CARDBUS, ID(TI, 1130) }, \
    { "TI 1131",  IS_TI|IS_CARDBUS, ID(TI, 1131) }, \
    { "TI 1031",  IS_TI|IS_CARDBUS, ID(TI, 1031) }, \
    { "TI 1210",  IS_TI|IS_CARDBUS, ID(TI, 1210) }, \
    { "TI 1211",  IS_TI|IS_CARDBUS, ID(TI, 1211) }, \
    { "TI 1220",  IS_TI|IS_CARDBUS, ID(TI, 1220) }, \
    { "TI 1221",  IS_TI|IS_CARDBUS, ID(TI, 1221) }, \
    { "TI 1225",  IS_TI|IS_CARDBUS, ID(TI, 1225) }, \
    { "TI 1250A", IS_TI|IS_CARDBUS, ID(TI, 1250A) }, \
    { "TI 1251A", IS_TI|IS_CARDBUS, ID(TI, 1251A) }, \
    { "TI 1251B", IS_TI|IS_CARDBUS, ID(TI, 1251B) }, \
    { "TI 1410",  IS_TI|IS_CARDBUS, ID(TI, 1410) }, \
    { "TI 1420",  IS_TI|IS_CARDBUS, ID(TI, 1420) }, \
    { "TI 1450",  IS_TI|IS_CARDBUS, ID(TI, 1450) }, \
    { "TI 1451",  IS_TI|IS_CARDBUS, ID(TI, 1451) }, \
    { "TI 1510",  IS_TI|IS_CARDBUS, ID(TI, 1510) }, \
    { "TI 4410",  IS_TI|IS_CARDBUS, ID(TI, 4410) }, \
    { "TI 4450",  IS_TI|IS_CARDBUS, ID(TI, 4450) }, \
    { "TI 4451",  IS_TI|IS_CARDBUS, ID(TI, 4451) }

#endif /* _LINUX_TI113X_H */
