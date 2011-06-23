/*
 * IXP PCI Init
 *
 * (C) Copyright 2011
 * Michael Schwingen, michael@schwingen.org
 * (C) Copyright 2004 eslab.whut.edu.cn
 * Yue Hu(huyue_whut@yahoo.com.cn), Ligong Xue(lgxue@hotmail.com)
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
#include <asm/processor.h>
#include <asm/io.h>
#include <pci.h>
#include <asm/arch/ixp425.h>
#include <asm/arch/ixp425pci.h>

DECLARE_GLOBAL_DATA_PTR;

static void non_prefetch_read(unsigned int addr, unsigned int cmd,
			      unsigned int *data);
static void non_prefetch_write(unsigned int addr, unsigned int cmd,
			       unsigned int data);

/*define the sub vendor and subsystem to be used */
#define IXP425_PCI_SUB_VENDOR_SYSTEM 0x00000000

#define PCI_MEMORY_BUS		0x00000000
#define PCI_MEMORY_PHY		0x00000000
#define PCI_MEMORY_SIZE		0x04000000

#define PCI_MEM_BUS		0x48000000
#define PCI_MEM_PHY		0x00000000
#define PCI_MEM_SIZE		0x04000000

#define	PCI_IO_BUS		0x00000000
#define PCI_IO_PHY		0x00000000
#define PCI_IO_SIZE		0x00010000

/* build address value for config sycle */
static unsigned int pci_config_addr(pci_dev_t bdf, unsigned int reg)
{
	unsigned int bus = PCI_BUS(bdf);
	unsigned int dev = PCI_DEV(bdf);
	unsigned int func = PCI_FUNC(bdf);
	unsigned int addr;

	if (bus) { /* secondary bus, use type 1 config cycle */
		addr = bdf | (reg & ~3) | 1;
	} else {
		/*
		  primary bus, type 0 config cycle. address bits 31:28
		  specify the device 10:8 specify the function
		*/
		addr = BIT((31 - dev)) | (func << 8) | (reg & ~3);
	}

	return addr;
}

static int pci_config_status(void)
{
	unsigned int regval;

	regval = readl(PCI_CSR_BASE + PCI_ISR_OFFSET);
	if ((regval & PCI_ISR_PFE) == 0)
		return OK;

	/* no device present, make sure that the master abort bit is reset */
	writel(PCI_ISR_PFE, PCI_CSR_BASE + PCI_ISR_OFFSET);
	return ERROR;
}

static int pci_ixp_hose_read_config_dword(struct pci_controller *hose,
				   pci_dev_t bdf, int where, unsigned int *val)
{
	unsigned int retval;
	unsigned int addr;
	int stat;

	debug("pci_ixp_hose_read_config_dword: bdf %x, reg %x", bdf, where);
	/*Set the address to be read */
	addr = pci_config_addr(bdf, where);
	non_prefetch_read(addr, NP_CMD_CONFIGREAD, &retval);
	*val = retval;

	stat = pci_config_status();
	if (stat < 0)
		*val = -1;
	debug("-> val %x, status %x\n", *val, stat);
	return stat;
}

static int pci_ixp_hose_read_config_word(struct pci_controller *hose,
				  pci_dev_t bdf, int where, unsigned short *val)
{
	unsigned int n;
	unsigned int retval;
	unsigned int addr;
	unsigned int byteEnables;
	int stat;

	debug("pci_ixp_hose_read_config_word: bdf %x, reg %x", bdf, where);
	n = where % 4;
	/*byte enables are 4 bits active low, the position of each
	   bit maps to the byte that it enables */
	byteEnables =
		(~(BIT(n) | BIT((n + 1)))) &
		IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK;
	byteEnables = byteEnables << PCI_NP_CBE_BESL;
	/*Set the address to be read */
	addr = pci_config_addr(bdf, where);
	non_prefetch_read(addr, byteEnables | NP_CMD_CONFIGREAD, &retval);

	/*Pick out the word we are interested in */
	*val = retval >> (8 * n);

	stat = pci_config_status();
	if (stat < 0)
		*val = -1;
	debug("-> val %x, status %x\n", *val, stat);
	return stat;
}

static int pci_ixp_hose_read_config_byte(struct pci_controller *hose,
				  pci_dev_t bdf, int where, unsigned char *val)
{
	unsigned int retval;
	unsigned int n;
	unsigned int byteEnables;
	unsigned int addr;
	int stat;

	debug("pci_ixp_hose_read_config_byte: bdf %x, reg %x", bdf, where);
	n = where % 4;
	/*byte enables are 4 bits, active low, the position of each
	   bit maps to the byte that it enables */
	byteEnables = (~BIT(n)) & IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK;
	byteEnables = byteEnables << PCI_NP_CBE_BESL;

	/*Set the address to be read */
	addr = pci_config_addr(bdf, where);
	non_prefetch_read(addr, byteEnables | NP_CMD_CONFIGREAD, &retval);
	/*Pick out the byte we are interested in */
	*val = retval >> (8 * n);

	stat = pci_config_status();
	if (stat < 0)
		*val = -1;
	debug("-> val %x, status %x\n", *val, stat);
	return stat;
}

static int pci_ixp_hose_write_config_byte(struct pci_controller *hose,
				   pci_dev_t bdf, int where, unsigned char val)
{
	unsigned int addr;
	unsigned int byteEnables;
	unsigned int n;
	unsigned int ldata;
	int stat;

	debug("pci_ixp_hose_write_config_byte: bdf %x, reg %x, val %x",
	      bdf, where, val);
	n = where % 4;
	/*byte enables are 4 bits active low, the position of each
	   bit maps to the byte that it enables */
	byteEnables = (~BIT(n)) & IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK;
	byteEnables = byteEnables << PCI_NP_CBE_BESL;
	ldata = val << (8 * n);
	/*Set the address to be written */
	addr = pci_config_addr(bdf, where);
	non_prefetch_write(addr, byteEnables | NP_CMD_CONFIGWRITE, ldata);

	stat = pci_config_status();
	debug("-> status %x\n", stat);
	return stat;
}

static int pci_ixp_hose_write_config_word(struct pci_controller *hose,
				   pci_dev_t bdf, int where, unsigned short val)
{
	unsigned int addr;
	unsigned int byteEnables;
	unsigned int n;
	unsigned int ldata;
	int stat;

	debug("pci_ixp_hose_write_config_word: bdf %x, reg %x, val %x",
	      bdf, where, val);
	n = where % 4;
	/*byte enables are 4 bits active low, the position of each
	   bit maps to the byte that it enables */
	byteEnables =
		(~(BIT(n) | BIT((n + 1)))) &
		IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK;
	byteEnables = byteEnables << PCI_NP_CBE_BESL;
	ldata = val << (8 * n);
	/*Set the address to be written */
	addr = pci_config_addr(bdf, where);
	non_prefetch_write(addr, byteEnables | NP_CMD_CONFIGWRITE, ldata);

	stat = pci_config_status();
	debug("-> status %x\n", stat);
	return stat;
}

static int pci_ixp_hose_write_config_dword(struct pci_controller *hose,
				    pci_dev_t bdf, int where, unsigned int val)
{
	unsigned int addr;
	int stat;

	debug("pci_ixp_hose_write_config_dword: bdf %x, reg %x, val %x",
	      bdf, where, val);
	/*Set the address to be written */
	addr = pci_config_addr(bdf, where);
	non_prefetch_write(addr, NP_CMD_CONFIGWRITE, val);

	stat = pci_config_status();
	debug("-> status %x\n", stat);
	return stat;
}

static void non_prefetch_read(unsigned int addr,
		       unsigned int cmd, unsigned int *data)
{
	writel(addr, PCI_CSR_BASE + PCI_NP_AD_OFFSET);

	/*set up and execute the read */
	writel(cmd, PCI_CSR_BASE + PCI_NP_CBE_OFFSET);

	/*The result of the read is now in np_rdata */
	*data = readl(PCI_CSR_BASE + PCI_NP_RDATA_OFFSET);

	return;
}

static void non_prefetch_write(unsigned int addr,
			unsigned int cmd, unsigned int data)
{

	writel(addr, PCI_CSR_BASE + PCI_NP_AD_OFFSET);
	/*set up the write */
	writel(cmd, PCI_CSR_BASE + PCI_NP_CBE_OFFSET);
	/*Execute the write by writing to NP_WDATA */
	writel(data, PCI_CSR_BASE + PCI_NP_WDATA_OFFSET);

	return;
}

static void crp_write(unsigned int offset, unsigned int data)
{
	/*
	 * The CRP address register bit 16 indicates that we want to do a
	 * write
	 */
	writel(PCI_CRP_WRITE | offset, PCI_CSR_BASE + PCI_CRP_AD_CBE_OFFSET);
	writel(data, PCI_CSR_BASE + PCI_CRP_WDATA_OFFSET);
}

void pci_ixp_init(struct pci_controller *hose)
{
	unsigned int csr;

	/*
	 * Specify that the AHB bus is operating in big endian mode. Set up
	 * byte lane swapping between little-endian PCI and the big-endian
	 * AHB bus
	 */
#ifdef __ARMEB__
	csr =  PCI_CSR_ABE | PCI_CSR_PDS | PCI_CSR_ADS;
#else
	csr = PCI_CSR_ABE;
#endif
	writel(csr, PCI_CSR_BASE + PCI_CSR_OFFSET);

	writel(0, PCI_CSR_BASE + PCI_INTEN_OFFSET);

	/*
	 * We configure the PCI inbound memory windows to be
	 * 1:1 mapped to SDRAM
	 */
	crp_write(PCI_CFG_BASE_ADDRESS_0, 0x00000000);
	crp_write(PCI_CFG_BASE_ADDRESS_1, 0x01000000);
	crp_write(PCI_CFG_BASE_ADDRESS_2, 0x02000000);
	crp_write(PCI_CFG_BASE_ADDRESS_3, 0x03000000);

	/*
	 * Enable CSR window at 64 MiB to allow PCI masters
	 * to continue prefetching past 64 MiB boundary.
	 */
	crp_write(PCI_CFG_BASE_ADDRESS_4, 0x04000000);
	/*
	 * Enable the IO window to be way up high, at 0xfffffc00
	 */
	crp_write(PCI_CFG_BASE_ADDRESS_5, 0xfffffc01);

	/*Setup PCI-AHB and AHB-PCI address mappings */
	writel(0x00010203, PCI_CSR_BASE + PCI_AHBMEMBASE_OFFSET);

	writel(0x00000000, PCI_CSR_BASE + PCI_AHBIOBASE_OFFSET);

	writel(0x48494a4b, PCI_CSR_BASE + PCI_PCIMEMBASE_OFFSET);

	crp_write(PCI_CFG_SUB_VENDOR_ID, IXP425_PCI_SUB_VENDOR_SYSTEM);

	crp_write(PCI_CFG_COMMAND, PCI_CFG_CMD_MAE | PCI_CFG_CMD_BME);
	udelay(1000);

	/* clear error bits in status register */
	writel(PCI_ISR_PSE | PCI_ISR_PFE | PCI_ISR_PPE | PCI_ISR_AHBE,
	       PCI_CSR_BASE + PCI_ISR_OFFSET);

	/*
	 * Set Initialize Complete in PCI Control Register: allow IXP4XX to
	 * respond to PCI configuration cycles.
	 */
	csr |= PCI_CSR_IC;
	writel(csr, PCI_CSR_BASE + PCI_CSR_OFFSET);

	hose->first_busno = 0;
	hose->last_busno = 0;

	/* System memory space */
	pci_set_region(hose->regions + 0,
		       PCI_MEMORY_BUS,
		       PCI_MEMORY_PHY, PCI_MEMORY_SIZE, PCI_REGION_SYS_MEMORY);

	/* PCI memory space */
	pci_set_region(hose->regions + 1,
		       PCI_MEM_BUS,
		       PCI_MEM_PHY, PCI_MEM_SIZE, PCI_REGION_MEM);
	/* PCI I/O space */
	pci_set_region(hose->regions + 2,
		       PCI_IO_BUS, PCI_IO_PHY, PCI_IO_SIZE, PCI_REGION_IO);

	hose->region_count = 3;

	pci_set_ops(hose,
		    pci_ixp_hose_read_config_byte,
		    pci_ixp_hose_read_config_word,
		    pci_ixp_hose_read_config_dword,
		    pci_ixp_hose_write_config_byte,
		    pci_ixp_hose_write_config_word,
		    pci_ixp_hose_write_config_dword);

	pci_register_hose(hose);
	hose->last_busno = pci_hose_scan(hose);
}
