/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * W7O board level hardware watchdog.
 */
#include <common.h>
#include <config.h>

#ifdef CONFIG_HW_WATCHDOG
#include <watchdog.h>

void hw_watchdog_reset(void)
{
    volatile ushort *hwd = (ushort *)(CONFIG_SYS_W7O_EBC_PB7CR & 0xfff00000);

    /*
     * Read the LMG's hwd register and toggle the
     * watchdog bit to reset it.   On the LMC, just
     * reading it is enough, but toggling the bit
     * doen't hurt either.
     */
    *hwd = *hwd ^ 0x8000;

} /* hw_watchdog_reset() */

#endif /* CONFIG_HW_WATCHDOG */
