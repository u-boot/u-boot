/*
 * Copyright 2004, 2007 Freescale Semiconductor.
 * Copyright(c) 2003 Motorola Inc.
 */

#ifndef	__MPC85xx_H__
#define __MPC85xx_H__

#include <asm/fsl_lbc.h>

/* define for common ppc_asm.tmpl */
#define EXC_OFF_SYS_RESET	0x100	/* System reset */
#define _START_OFFSET		0

#if defined(CONFIG_E500)
#include <e500.h>
#endif

/*
 * SCCR - System Clock Control Register, 9-8
 */
#define SCCR_CLPD       0x00000004      /* CPM Low Power Disable */
#define SCCR_DFBRG_MSK  0x00000003      /* Division by BRGCLK Mask */
#define SCCR_DFBRG_SHIFT 0

#define SCCR_DFBRG00    0x00000000      /* BRGCLK division by 4 */
#define SCCR_DFBRG01    0x00000001      /* BRGCLK div by 16 (normal) */
#define SCCR_DFBRG10    0x00000002      /* BRGCLK division by 64 */
#define SCCR_DFBRG11    0x00000003      /* BRGCLK division by 256 */

#endif	/* __MPC85xx_H__ */
