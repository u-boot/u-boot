/*
 * text_base
 *
 * (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <config.h>

#ifdef CONFIG_SYS_MONITOR_BASE
	.text
	/* text base used in link script u-boot.lds */
	.global text_base
	.equ text_base,CONFIG_SYS_MONITOR_BASE
	/* dummy func to let linker include this file */
	.global text_base_hook
text_base_hook:
	ret
#endif
