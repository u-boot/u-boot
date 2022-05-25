/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __SYSTEM_CONSTANTS_H__
#define __SYSTEM_CONSTANTS_H__

/*
 * The most common case for our initial stack pointer address is to
 * say that we have defined a static intiial ram address location and
 * size and from that we subtract the generated global data size.
 */
#ifdef CONFIG_HAS_CUSTOM_SYS_INIT_SP_ADDR
#define SYS_INIT_SP_ADDR	CONFIG_CUSTOM_SYS_INIT_SP_ADDR
#else
#ifdef CONFIG_MIPS
#define SYS_INIT_SP_ADDR	(CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_INIT_SP_OFFSET)
#else
#define SYS_INIT_SP_ADDR	\
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#endif
#endif

#endif
