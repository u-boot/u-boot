/*
 * Copyright(c) 2003 Motorola Inc.
 * Xianghua Xiao (x.xiao@motorola.com)
 */

#ifndef	__MPC85xx_H__
#define __MPC85xx_H__

#define EXC_OFF_SYS_RESET	0x0100	/* System reset				*/

#if defined(CONFIG_E500)
#include <e500.h>
#endif

#if defined(CONFIG_DDR_ECC)
void dma_init(void);
uint dma_check(void);
int dma_xfer(void *dest, uint count, void *src);
#endif
/*-----------------------------------------------------------------------
 * SCCR - System Clock Control Register                                  9-8
 */
#define SCCR_CLPD       0x00000004      /* CPM Low Power Disable        */
#define SCCR_DFBRG_MSK  0x00000003      /* Division factor of BRGCLK Mask */
#define SCCR_DFBRG_SHIFT 0

#define SCCR_DFBRG00    0x00000000      /* BRGCLK division by 4         */
#define SCCR_DFBRG01    0x00000001      /* BRGCLK division by 16 (normal op.)*/
#define SCCR_DFBRG10    0x00000002      /* BRGCLK division by 64        */
#define SCCR_DFBRG11    0x00000003      /* BRGCLK division by 256       */

#endif	/* __MPC85xx_H__ */
