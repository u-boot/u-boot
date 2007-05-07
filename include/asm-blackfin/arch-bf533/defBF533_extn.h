/*
 * defBF533_extn.h
 *
 * This file is subject to the terms and conditions of the GNU Public
 * License. See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Non-GPL License also available as part of VisualDSP++
 *
 * http://www.analog.com/processors/resources/crosscore/visualDspDevSoftware.html
 *
 * (c) Copyright 2001-2005 Analog Devices, Inc. All rights reserved
 *
 * This file under source code control, please send bugs or changes to:
 * dsptools.support@analog.com
 *
 */

#ifndef _DEF_BF533_EXTN_H
#define _DEF_BF533_EXTN_H

/* define macro for offset */
#define OFFSET_( x )		((x) & 0x0000FFFF)
/* Delay inserted for PLL transition */
#define PLL_DELAY			0x1000

#define L1_ISRAM		0xFFA00000
#define L1_ISRAM_END		0xFFA10000
#define DATA_BANKA_SRAM		0xFF800000
#define DATA_BANKA_SRAM_END	0xFF808000
#define DATA_BANKB_SRAM		0xFF900000
#define DATA_BANKB_SRAM_END	0xFF908000
#define SYSMMR_BASE		0xFFC00000
#define WDSIZE16		0x00000004

/* Event Vector Table Address */
#define EVT_EMULATION_ADDR	0xffe02000
#define EVT_RESET_ADDR		0xffe02004
#define EVT_NMI_ADDR		0xffe02008
#define EVT_EXCEPTION_ADDR	0xffe0200c
#define EVT_GLOBAL_INT_ENB_ADDR	0xffe02010
#define EVT_HARDWARE_ERROR_ADDR	0xffe02014
#define EVT_TIMER_ADDR		0xffe02018
#define EVT_IVG7_ADDR		0xffe0201c
#define EVT_IVG8_ADDR		0xffe02020
#define EVT_IVG9_ADDR		0xffe02024
#define EVT_IVG10_ADDR		0xffe02028
#define EVT_IVG11_ADDR		0xffe0202c
#define EVT_IVG12_ADDR		0xffe02030
#define EVT_IVG13_ADDR		0xffe02034
#define EVT_IVG14_ADDR		0xffe02038
#define EVT_IVG15_ADDR		0xffe0203c
#define EVT_OVERRIDE_ADDR	0xffe02100

/* IMASK Bit values */
#define IVG15_POS		0x00008000
#define IVG14_POS		0x00004000
#define IVG13_POS		0x00002000
#define IVG12_POS		0x00001000
#define IVG11_POS		0x00000800
#define IVG10_POS		0x00000400
#define IVG9_POS		0x00000200
#define IVG8_POS		0x00000100
#define IVG7_POS		0x00000080
#define IVGTMR_POS		0x00000040
#define IVGHW_POS		0x00000020

#define WDOG_TMR_DISABLE	(0xAD << 4)
#define ICTL_RST		0x00000000
#define ICTL_NMI		0x00000002
#define ICTL_GP			0x00000004
#define ICTL_DISABLE		0x00000003

/* Watch Dog timer values setup */
#define WATCHDOG_DISABLE	WDOG_TMR_DISABLE | ICTL_DISABLE

#endif	/* _DEF_BF533_EXTN_H */
