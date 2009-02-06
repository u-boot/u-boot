/*
 * (C) Copyright 2003
 * AMIRIX Systems Inc.
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
#include <ppc4xx.h>
#include <asm/processor.h>
#include <pci.h>

#define PCI_MEM_82559ER_CSR_BASE    0x30200000
#define PCI_IO_82559ER_CSR_BASE     0x40000200

/** AP1100 specific values */
#define PSII_BASE                   0x30000000	  /**< PowerSpan II dual bridge local bus register address */
#define PSII_CONFIG_ADDR            0x30000290	  /**< PowerSpan II Configuration Cycle Address configuration register */
#define PSII_CONFIG_DATA            0x30000294	  /**< PowerSpan II Configuration Cycle Data register. */
#define PSII_CONFIG_DEST_PCI2       0x01000000	  /**< PowerSpan II configuration cycle destination selection, set for PCI2 bus */
#define PSII_PCI_MEM_BASE           0x30200000	  /**< Local Bus address for start of PCI memory space on PCI2 bus. */
#define PSII_PCI_MEM_SIZE           0x1BE00000	  /**< PCI Memory space about 510 Meg. */
#define AP1000_SYS_MEM_START        0x00000000	  /**< System memory starts at 0. */
#define AP1000_SYS_MEM_SIZE         0x08000000	  /**< System memory is 128 Meg. */

/* static int G_verbosity_level = 1; */
#define G_verbosity_level 1

void write1 (unsigned long addr, unsigned char val)
{
	volatile unsigned char *p = (volatile unsigned char *) addr;

	if (G_verbosity_level > 1)
		printf ("write1: addr=%08x val=%02x\n", (unsigned int) addr,
			val);
	*p = val;
	asm ("eieio");
}

unsigned char read1 (unsigned long addr)
{
	unsigned char val;
	volatile unsigned char *p = (volatile unsigned char *) addr;

	if (G_verbosity_level > 1)
		printf ("read1: addr=%08x ", (unsigned int) addr);
	val = *p;
	asm ("eieio");
	if (G_verbosity_level > 1)
		printf ("val=%08x\n", val);
	return val;
}

void write2 (unsigned long addr, unsigned short val)
{
	volatile unsigned short *p = (volatile unsigned short *) addr;

	if (G_verbosity_level > 1)
		printf ("write2: addr=%08x val=%04x -> *p=%04x\n",
			(unsigned int) addr, val,
			((val & 0xFF00) >> 8) | ((val & 0x00FF) << 8));

	*p = ((val & 0xFF00) >> 8) | ((val & 0x00FF) << 8);
	asm ("eieio");
}

unsigned short read2 (unsigned long addr)
{
	unsigned short val;
	volatile unsigned short *p = (volatile unsigned short *) addr;

	if (G_verbosity_level > 1)
		printf ("read2: addr=%08x ", (unsigned int) addr);
	val = *p;
	val = ((val & 0xFF00) >> 8) | ((val & 0x00FF) << 8);
	asm ("eieio");
	if (G_verbosity_level > 1)
		printf ("*p=%04x -> val=%04x\n",
			((val & 0xFF00) >> 8) | ((val & 0x00FF) << 8), val);
	return val;
}

void write4 (unsigned long addr, unsigned long val)
{
	volatile unsigned long *p = (volatile unsigned long *) addr;

	if (G_verbosity_level > 1)
		printf ("write4: addr=%08x val=%08x -> *p=%08x\n",
			(unsigned int) addr, (unsigned int) val,
			(unsigned int) (((val & 0xFF000000) >> 24) |
					((val & 0x000000FF) << 24) |
					((val & 0x00FF0000) >> 8) |
					((val & 0x0000FF00) << 8)));

	*p = ((val & 0xFF000000) >> 24) | ((val & 0x000000FF) << 24) |
		((val & 0x00FF0000) >> 8) | ((val & 0x0000FF00) << 8);
	asm ("eieio");
}

unsigned long read4 (unsigned long addr)
{
	unsigned long val;
	volatile unsigned long *p = (volatile unsigned long *) addr;

	if (G_verbosity_level > 1)
		printf ("read4: addr=%08x", (unsigned int) addr);

	val = *p;
	val = ((val & 0xFF000000) >> 24) | ((val & 0x000000FF) << 24) |
		((val & 0x00FF0000) >> 8) | ((val & 0x0000FF00) << 8);
	asm ("eieio");

	if (G_verbosity_level > 1)
		printf ("*p=%04x -> val=%04x\n",
			(unsigned int) (((val & 0xFF000000) >> 24) |
					((val & 0x000000FF) << 24) |
					((val & 0x00FF0000) >> 8) |
					((val & 0x0000FF00) << 8)),
			(unsigned int) val);
	return val;
}

void write4be (unsigned long addr, unsigned long val)
{
	volatile unsigned long *p = (volatile unsigned long *) addr;

	if (G_verbosity_level > 1)
		printf ("write4: addr=%08x val=%08x\n", (unsigned int) addr,
			(unsigned int) val);
	*p = val;
	asm ("eieio");
}

/** One byte configuration write on PSII.
 *  Currently fixes destination PCI bus to PCI2, onboard
 *  pci.
 *  @param    hose    PCI Host controller information. Ignored.
 *  @param    dev        Encoded PCI device/Bus and Function value.
 *  @param    reg        PCI Configuration register number.
 *  @param    val        Address of location for received byte.
 *  @return Always Zero.
 */
static int psII_read_config_byte (struct pci_controller *hose,
				  pci_dev_t dev, int reg, u8 * val)
{
	write4be (PSII_CONFIG_ADDR, PSII_CONFIG_DEST_PCI2 |	/* Operate on PCI2 bus interface . */
		  (PCI_BUS (dev) << 16) | (PCI_DEV (dev) << 11) | (PCI_FUNC (dev) << 8) | ((reg & 0xFF) & ~3));	/* Configuation cycle type 0 */

	*val = read1 (PSII_CONFIG_DATA + (reg & 0x03));
	return (0);
}

/** One byte configuration write on PSII.
 *  Currently fixes destination bus to PCI2, onboard
 *  pci.
 *  @param    hose    PCI Host controller information. Ignored.
 *  @param    dev        Encoded PCI device/Bus and Function value.
 *  @param    reg        PCI Configuration register number.
 *  @param    val        Output byte.
 *  @return Always Zero.
 */
static int psII_write_config_byte (struct pci_controller *hose,
				   pci_dev_t dev, int reg, u8 val)
{
	write4be (PSII_CONFIG_ADDR, PSII_CONFIG_DEST_PCI2 |	/* Operate on PCI2 bus interface . */
		  (PCI_BUS (dev) << 16) | (PCI_DEV (dev) << 11) | (PCI_FUNC (dev) << 8) | ((reg & 0xFF) & ~3));	/* Configuation cycle type 0 */

	write1 (PSII_CONFIG_DATA + (reg & 0x03), (unsigned char) val);

	return (0);
}

/** One word (16 bit) configuration read on PSII.
 *  Currently fixes destination PCI bus to PCI2, onboard
 *  pci.
 *  @param    hose    PCI Host controller information. Ignored.
 *  @param    dev        Encoded PCI device/Bus and Function value.
 *  @param    reg        PCI Configuration register number.
 *  @param    val        Address of location for received word.
 *  @return Always Zero.
 */
static int psII_read_config_word (struct pci_controller *hose,
				  pci_dev_t dev, int reg, u16 * val)
{
	write4be (PSII_CONFIG_ADDR, PSII_CONFIG_DEST_PCI2 |	/* Operate on PCI2 bus interface . */
		  (PCI_BUS (dev) << 16) | (PCI_DEV (dev) << 11) | (PCI_FUNC (dev) << 8) | ((reg & 0xFF) & ~3));	/* Configuation cycle type 0 */

	*val = read2 (PSII_CONFIG_DATA + (reg & 0x03));
	return (0);
}

/** One word (16 bit) configuration write on PSII.
 *  Currently fixes destination bus to PCI2, onboard
 *  pci.
 *  @param    hose    PCI Host controller information. Ignored.
 *  @param    dev        Encoded PCI device/Bus and Function value.
 *  @param    reg        PCI Configuration register number.
 *  @param    val        Output word.
 *  @return Always Zero.
 */
static int psII_write_config_word (struct pci_controller *hose,
				   pci_dev_t dev, int reg, u16 val)
{
	write4be (PSII_CONFIG_ADDR, PSII_CONFIG_DEST_PCI2 |	/* Operate on PCI2 bus interface . */
		  (PCI_BUS (dev) << 16) | (PCI_DEV (dev) << 11) | (PCI_FUNC (dev) << 8) | ((reg & 0xFF) & ~3));	/* Configuation cycle type 0 */

	write2 (PSII_CONFIG_DATA + (reg & 0x03), (unsigned short) val);

	return (0);
}

/** One DWord (32 bit) configuration read on PSII.
 *  Currently fixes destination PCI bus to PCI2, onboard
 *  pci.
 *  @param    hose    PCI Host controller information. Ignored.
 *  @param    dev        Encoded PCI device/Bus and Function value.
 *  @param    reg        PCI Configuration register number.
 *  @param    val        Address of location for received byte.
 *  @return Always Zero.
 */
static int psII_read_config_dword (struct pci_controller *hose,
				   pci_dev_t dev, int reg, u32 * val)
{
	write4be (PSII_CONFIG_ADDR, PSII_CONFIG_DEST_PCI2 |	/* Operate on PCI2 bus interface . */
		  (PCI_BUS (dev) << 16) | (PCI_DEV (dev) << 11) | (PCI_FUNC (dev) << 8) | ((reg & 0xFF) & ~3));	/* Configuation cycle type 0 */

	*val = read4 (PSII_CONFIG_DATA);
	return (0);
}

/** One DWord (32 bit) configuration write on PSII.
 *  Currently fixes destination bus to PCI2, onboard
 *  pci.
 *  @param    hose    PCI Host controller information. Ignored.
 *  @param    dev        Encoded PCI device/Bus and Function value.
 *  @param    reg        PCI Configuration register number.
 *  @param    val        Output Dword.
 *  @return Always Zero.
 */
static int psII_write_config_dword (struct pci_controller *hose,
				    pci_dev_t dev, int reg, u32 val)
{
	write4be (PSII_CONFIG_ADDR, PSII_CONFIG_DEST_PCI2 |	/* Operate on PCI2 bus interface . */
		  (PCI_BUS (dev) << 16) | (PCI_DEV (dev) << 11) | (PCI_FUNC (dev) << 8) | ((reg & 0xFF) & ~3));	/* Configuation cycle type 0 */

	write4 (PSII_CONFIG_DATA, (unsigned long) val);

	return (0);
}

static struct pci_config_table ap1000_config_table[] = {
#ifdef CONFIG_AP1000
	{PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 PCI_BUS (CONFIG_SYS_ETH_DEV_FN), PCI_DEV (CONFIG_SYS_ETH_DEV_FN),
	 PCI_FUNC (CONFIG_SYS_ETH_DEV_FN),
	 pci_cfgfunc_config_device,
	 {CONFIG_SYS_ETH_IOBASE, CONFIG_SYS_ETH_MEMBASE,
	  PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER}},
#endif
	{}
};

static struct pci_controller psII_hose = {
      config_table:ap1000_config_table,
};

void pci_init_board (void)
{
	struct pci_controller *hose = &psII_hose;

	/*
	 * Register the hose
	 */
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* System memory space */
	pci_set_region (hose->regions + 0,
			AP1000_SYS_MEM_START, AP1000_SYS_MEM_START,
			AP1000_SYS_MEM_SIZE,
			PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	/* PCI Memory space */
	pci_set_region (hose->regions + 1,
			PSII_PCI_MEM_BASE, PSII_PCI_MEM_BASE,
			PSII_PCI_MEM_SIZE, PCI_REGION_MEM);

	/* No IO Memory space  - for now */

	pci_set_ops (hose,
		     psII_read_config_byte,
		     psII_read_config_word,
		     psII_read_config_dword,
		     psII_write_config_byte,
		     psII_write_config_word, psII_write_config_dword);

	hose->region_count = 2;

	pci_register_hose (hose);

	hose->last_busno = pci_hose_scan (hose);
}
