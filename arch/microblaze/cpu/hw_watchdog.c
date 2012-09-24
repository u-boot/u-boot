/******************************************************************************
*
*     This program is free software; you can redistribute it and/or modify it
*     under the terms of the GNU General Public License as published by the
*     Free Software Foundation; either version 2 of the License, or (at your
*     option) any later version.
*
*     (c) Copyright 2011 Xilinx Inc.
*
*     You should have received a copy of the GNU General Public License along
*     with this program; if not, write to the Free Software Foundation, Inc.,
*     675 Mass Ave, Cambridge, MA 02139, USA.
*
******************************************************************************/

#include <common.h>
#include <asm/io.h>
#include <watchdog.h>

#ifdef CONFIG_HW_WATCHDOG

    #define XWT_TWCSR0_OFFSET	0x0 /**< Control/Status Register 0 Offset */
    #define XWT_TWCSR1_OFFSET	0x4 /**< Control/Status Register 1 Offset */
    #define XWT_TBR_OFFSET		0x8 /**< Timebase Register Offset */

    #define XWT_CSR0_WRS_MASK	0x00000008 /**< Reset status Mask */
    #define XWT_CSR0_WDS_MASK	0x00000004 /**< Timer state Mask */
    #define XWT_CSR0_EWDT1_MASK	0x00000002 /**< Enable bit 1 Mask*/
    #define XWT_CSRX_EWDT2_MASK	0x00000001 /**< Enable bit 2 Mask */

void hw_watchdog_reset(void)
{
    unsigned int CSRRegister;

#ifdef XPAR_MICROBLAZE_USE_DCACHE
    microblaze_invalidate_dcache_range(WATCHDOG_BASEADDR, 4);
#endif

    /* Read the current contents of TCSR0 */
    CSRRegister = inl(WATCHDOG_BASEADDR + XWT_TWCSR0_OFFSET);

    /* Clear the watchdog WDS bit */
    if (CSRRegister & (XWT_CSR0_EWDT1_MASK | XWT_CSRX_EWDT2_MASK))
    {
        outl(CSRRegister | XWT_CSR0_WDS_MASK, WATCHDOG_BASEADDR + XWT_TWCSR0_OFFSET);
#ifdef XPAR_MICROBLAZE_USE_DCACHE
        microblaze_flush_dcache_range(WATCHDOG_BASEADDR, 4);
#endif
    }
}

void hw_watchdog_init(void)
{
    outl((XWT_CSR0_WRS_MASK | XWT_CSR0_WDS_MASK | XWT_CSR0_EWDT1_MASK), WATCHDOG_BASEADDR + XWT_TWCSR0_OFFSET);
    outl(XWT_CSRX_EWDT2_MASK, WATCHDOG_BASEADDR + XWT_TWCSR1_OFFSET);

#ifdef XPAR_MICROBLAZE_USE_DCACHE
    microblaze_flush_dcache_range(WATCHDOG_BASEADDR, 8);
#endif
}

void hw_watchdog_disable(void)
{
    unsigned int CSRRegister;

#ifdef XPAR_MICROBLAZE_USE_DCACHE
    microblaze_invalidate_dcache_range(WATCHDOG_BASEADDR, 4);
#endif

    /* Read the current contents of TCSR0 */
    CSRRegister = inl(WATCHDOG_BASEADDR + XWT_TWCSR0_OFFSET);

    outl(CSRRegister & ~XWT_CSR0_EWDT1_MASK, WATCHDOG_BASEADDR + XWT_TWCSR0_OFFSET);
    outl(~XWT_CSRX_EWDT2_MASK, WATCHDOG_BASEADDR + XWT_TWCSR1_OFFSET);
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    microblaze_flush_dcache_range(WATCHDOG_BASEADDR, 8);
#endif

}

#endif
