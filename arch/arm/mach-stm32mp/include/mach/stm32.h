/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier:	GPL-2.0+	BSD-3-Clause
 */

#ifndef _MACH_STM32_H_
#define _MACH_STM32_H_

/*
 * Peripheral memory map
 * only address used before device tree parsing
 */
#define STM32_RCC_BASE			0x50000000
#define STM32_PWR_BASE			0x50001000
#define STM32_DBGMCU_BASE		0x50081000
#define STM32_TZC_BASE			0x5C006000
#define STM32_ETZPC_BASE		0x5C007000
#define STM32_TAMP_BASE			0x5C00A000

#define STM32_SYSRAM_BASE		0x2FFC0000
#define STM32_SYSRAM_SIZE		SZ_256K

#define STM32_DDR_BASE			0xC0000000
#define STM32_DDR_SIZE			SZ_1G

#endif /* _MACH_STM32_H_ */
