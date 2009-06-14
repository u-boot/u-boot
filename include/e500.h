/*
 * Copyright 2003 Motorola,Inc.
 * Xianghua Xiao(x.xiao@motorola.com)
 */

#ifndef	__E500_H__
#define __E500_H__

#ifndef __ASSEMBLY__

#ifndef CONFIG_NUM_CPUS
#define CONFIG_NUM_CPUS 1
#endif

typedef struct
{
  unsigned long freqProcessor[CONFIG_NUM_CPUS];
  unsigned long freqSystemBus;
  unsigned long freqDDRBus;
  unsigned long freqLocalBus;
  unsigned long freqQE;
} MPC85xx_SYS_INFO;

#endif  /* _ASMLANGUAGE */

#define RESET_VECTOR	0xfffffffc

#endif	/* __E500_H__ */
