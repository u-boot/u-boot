#ifndef __ARM7_HW_H
#define __ARM7_HW_H

/*
 * Copyright (c) 2004	Cucy Systems (http://www.cucy.com)
 * Curt Brune <curt@cucy.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#if defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
/* include IntegratorCP/CM720T specific hardware file if there was one */
#else
#error No hardware file defined for this configuration
#endif

#endif /* __ARM7_HW_H */
