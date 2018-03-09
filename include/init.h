/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copy the startup prototype, previously defined in common.h
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __INIT_H_
#define __INIT_H_	1

#ifndef __ASSEMBLY__		/* put C only stuff in this section */

/*
 * Function Prototypes
 */

/* common/board_f.c */

/**
 * arch_cpu_init() - basic cpu-dependent setup for an architecture
 *
 * This is called after early malloc is available. It should handle any
 * CPU- or SoC- specific init needed to continue the init sequence. See
 * board_f.c for where it is called. If this is not provided, a default
 * version (which does nothing) will be used.
 *
 * @return: 0 on success, otherwise error
 */
int arch_cpu_init(void);

/**
 * mach_cpu_init() - SoC/machine dependent CPU setup
 *
 * This is called after arch_cpu_init(). It should handle any
 * SoC or machine specific init needed to continue the init sequence. See
 * board_f.c for where it is called. If this is not provided, a default
 * version (which does nothing) will be used.
 *
 * @return: 0 on success, otherwise error
 */
int mach_cpu_init(void);

/* common/board_r.c */

#endif	/* __ASSEMBLY__ */
/* Put only stuff here that the assembler can digest */

#endif	/* __INIT_H_ */
