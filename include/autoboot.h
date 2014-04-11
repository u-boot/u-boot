/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AUTOBOOT_H
#define __AUTOBOOT_H

#ifdef CONFIG_BOOTDELAY
void bootdelay_process(void);
#else
static inline void bootdelay_process(void)
{
}
#endif

#endif
