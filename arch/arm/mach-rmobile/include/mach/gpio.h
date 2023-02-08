#ifndef __ASM_ARCH_GPIO_H
#define __ASM_ARCH_GPIO_H

#if defined(CONFIG_R8A7740)
#include "r8a7740-gpio.h"
void r8a7740_pinmux_init(void);
#endif

#endif /* __ASM_ARCH_GPIO_H */
