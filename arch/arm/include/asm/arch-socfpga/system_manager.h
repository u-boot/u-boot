/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	_SYSTEM_MANAGER_H_
#define	_SYSTEM_MANAGER_H_

#ifndef __ASSEMBLY__

void sysmgr_pinmux_init(void);

/* declaration for handoff table type */
extern unsigned long sys_mgr_init_table[CONFIG_HPS_PINMUX_NUM];

#endif


#define CONFIG_SYSMGR_PINMUXGRP_OFFSET	(0x400)

#endif /* _SYSTEM_MANAGER_H_ */
