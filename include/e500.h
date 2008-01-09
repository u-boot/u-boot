/*
 * Copyright 2003 Motorola,Inc.
 * Xianghua Xiao(x.xiao@motorola.com)
 */

#ifndef	__E500_H__
#define __E500_H__

#ifndef __ASSEMBLY__

typedef struct
{
  unsigned long freqProcessor;
  unsigned long freqSystemBus;
  unsigned long freqDDRBus;
} MPC85xx_SYS_INFO;

#endif  /* _ASMLANGUAGE */

#define RESET_VECTOR	0xfffffffc

#endif	/* __E500_H__ */
