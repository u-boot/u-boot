/*
 * system_map.S - optional symbol lookup for debugging
 *
 * Copyright (c) 2007 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <config.h>

#ifdef CONFIG_DEBUG_DUMP_SYMS
.data
.global _system_map
.type _system_map,@object
_system_map:
#include SYM_FILE
.asciz ""
.size _system_map,.-_system_map
#endif
