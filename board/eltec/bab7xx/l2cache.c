/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if defined(CFG_L2_BAB7xx)

#include <pci.h>
#include <mpc106.h>
#include <asm/processor.h>

/* defines L2CR register for MPC750 */

#define L2CR_E           0x80000000
#define L2CR_256K        0x10000000
#define L2CR_512K        0x20000000
#define L2CR_1024K       0x30000000
#define L2CR_I           0x00200000
#define L2CR_SL          0x00008000
#define L2CR_IP          0x00000001

/*----------------------------------------------------------------------------*/

static int dummy (int dummy)
{
    return (dummy+1);
}

/*----------------------------------------------------------------------------*/

int l2_cache_enable (int l2control)
{
    if (l2control)              /* BAB750 */
    {
	mtspr(SPRN_L2CR, l2control);
	mtspr(SPRN_L2CR, (l2control | L2CR_I));
	while (mfspr(SPRN_L2CR) & L2CR_IP)
	    ;
	mtspr(SPRN_L2CR, (l2control | L2CR_E));
	return (0);
    }
    else /* BAB740 */
    {
	int picr1, picr2, mask;
	int picr2CacheSize, cacheSize;
	int *d;
	int devbusfn;
	u32 reg32;

	devbusfn = pci_find_device(PCI_VENDOR_ID_MOTOROLA,
				   PCI_DEVICE_ID_MOTOROLA_MPC106, 0);
	if (devbusfn == -1)
	    return (-1);

	pci_read_config_dword  (devbusfn, PCI_PICR2, &reg32);
	reg32 &= ~PICR2_L2_EN;
	pci_write_config_dword (devbusfn, PCI_PICR2, reg32);

	/* cache size */
	if (*(volatile unsigned char *) (CFG_ISA_IO + 0x220) & 0x04)
	{
	    /* cache size is 512 KB */
	    picr2CacheSize = PICR2_L2_SIZE_512K;
	    cacheSize = 0x80000;
	}
	else
	{
	    /* cache size is 256 KB */
	    picr2CacheSize = PICR2_L2_SIZE_256K;
	    cacheSize = 0x40000;
	}

	/* setup PICR1 */
	mask =
	~(PICR1_CF_BREAD_WS(1) |
	  PICR1_CF_BREAD_WS(2) |
	  PICR1_CF_CBA(0xff) |
	  PICR1_CF_CACHE_1G |
	  PICR1_CF_DPARK |
	  PICR1_CF_APARK |
	  PICR1_CF_L2_CACHE_MASK);

	picr1 =
	(PICR1_CF_CBA(0x3f) |
	 PICR1_CF_CACHE_1G |
	 PICR1_CF_APARK |
	 PICR1_CF_DPARK |
	 PICR1_CF_L2_COPY_BACK); /* PICR1_CF_L2_WRITE_THROUGH */

	pci_read_config_dword  (devbusfn, PCI_PICR1, &reg32);
	reg32 &= mask;
	reg32 |= picr1;
	pci_write_config_dword (devbusfn, PCI_PICR1, reg32);

	/*
	 * invalidate all L2 cache
	 */
	picr2 =
	(PICR2_CF_INV_MODE |
	 PICR2_CF_HIT_HIGH |
	 PICR2_CF_MOD_HIGH |
	 PICR2_CF_L2_HIT_DELAY(1) |
	 PICR2_CF_APHASE_WS(1) |
	 picr2CacheSize);

	pci_write_config_dword (devbusfn, PCI_PICR2, picr2);

	/*
	 * dummy transactions
	 */
	for (d=0; d<(int *)(2*cacheSize); d++)
	    dummy(*d);

	pci_write_config_dword (devbusfn, PCI_PICR2,
				(picr2 | PICR2_CF_FLUSH_L2));

	/* setup PICR2 */
	picr2 =
	(PICR2_CF_FAST_CASTOUT |
	 PICR2_CF_WDATA |
	 PICR2_CF_ADDR_ONLY_DISABLE |
	 PICR2_CF_HIT_HIGH |
	 PICR2_CF_MOD_HIGH |
	 PICR2_L2_UPDATE_EN |
	 PICR2_L2_EN |
	 PICR2_CF_APHASE_WS(1) |
	 PICR2_CF_DATA_RAM_PBURST |
	 PICR2_CF_L2_HIT_DELAY(1) |
	 PICR2_CF_SNOOP_WS(2) |
	 picr2CacheSize);

	pci_write_config_dword (devbusfn, PCI_PICR2, picr2);
    }
    return (0);
}

/*----------------------------------------------------------------------------*/

#endif /* (CFG_L2_BAB7xx) */
