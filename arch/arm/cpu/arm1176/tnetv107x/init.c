/*
 * TNETV107X: Architecture initialization
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>

void chip_configuration_unlock(void)
{
       __raw_writel(TNETV107X_KICK0_MAGIC, TNETV107X_KICK0);
       __raw_writel(TNETV107X_KICK1_MAGIC, TNETV107X_KICK1);
}

int arch_cpu_init(void)
{
       icache_enable();
       chip_configuration_unlock();

       return 0;
}
