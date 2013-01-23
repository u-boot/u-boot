/*
 * Copyright 2003 Motorola,Inc.
 * Xianghua Xiao(x.xiao@motorola.com)
 */

#ifndef	__E500_H__
#define __E500_H__

#ifndef __ASSEMBLY__

typedef struct
{
  unsigned long freqProcessor[CONFIG_MAX_CPUS];
  unsigned long freqSystemBus;
  unsigned long freqDDRBus;
  unsigned long freqLocalBus;
  unsigned long freqQE;
#ifdef CONFIG_SYS_DPAA_FMAN
  unsigned long freqFMan[CONFIG_SYS_NUM_FMAN];
#endif
#ifdef CONFIG_SYS_DPAA_QBMAN
  unsigned long freqQMAN;
#endif
#ifdef CONFIG_SYS_DPAA_PME
  unsigned long freqPME;
#endif
} MPC85xx_SYS_INFO;

#endif  /* _ASMLANGUAGE */

#define RESET_VECTOR	0xfffffffc

#endif	/* __E500_H__ */
