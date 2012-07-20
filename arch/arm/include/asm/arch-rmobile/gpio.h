#ifndef __ASM_ARCH_GPIO_H
#define __ASM_ARCH_GPIO_H

#if defined(CONFIG_SH73A0)
#include "sh73a0-gpio.h"
void sh73a0_pinmux_init(void);
#endif

#endif /* __ASM_ARCH_GPIO_H */
