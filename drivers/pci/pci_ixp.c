/*
 * IXP PCI Init
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

static void non_prefetch_read (unsigned int addr, unsigned int cmd,
			       unsigned int *data);
static void non_prefetch_write (unsigned int addr, unsigned int cmd,
				unsigned int data);
static void configure_pins (void);
static void sys_pci_gpio_clock_config (void);
static void pci_bus_scan (void);
static int pci_device_exists (unsigned int deviceNo);
static void sys_pci_bar_info_get (unsigned int devnum, unsigned int bus,
				  unsigned int dev, unsigned int func);
static void sys_pci_device_bars_write (void);
static void calc_bars (PciBar * Bars[], unsigned int nBars,
		       unsigned int startAddr);

#define PCI_MEMORY_BUS		0x00000000
#define PCI_MEMORY_PHY		0x48000000
#define PCI_MEMORY_SIZE		0x04000000

#define PCI_MEM_BUS		0x40000000
#define PCI_MEM_PHY		0x00000000
#define PCI_MEM_SIZE		0x04000000

#define	PCI_IO_BUS		0x40000000
#define PCI_IO_PHY		0x50000000
#define PCI_IO_SIZE		0x10000000

struct pci_controller hose;

unsigned int nDevices;
unsigned int nMBars;
unsigned int nIOBars;
PciBar *memBars[IXP425_PCI_MAX_BAR];
PciBar *ioBars[IXP425_PCI_MAX_BAR];
PciDevice devices[IXP425_PCI_MAX_FUNC_ON_BUS];

int pci_read_config_dword (pci_dev_t dev, int where, unsigned int *val)
{
	unsigned int retval;
	unsigned int addr;

	/*address bits 31:28 specify the device 10:8 specify the function */
	/*Set the address to be read */
	addr = BIT ((31 - dev)) | (where & ~3);
	non_prefetch_read (addr, NP_CMD_CONFIGREAD, &retval);

	*val = retval;

	return (OK);
}

int pci_read_config_word (pci_dev_t dev, int where, unsigned short *val)
{
	unsigned int n;
	unsigned int retval;
	unsigned int addr;
	unsigned int byteEnables;

	n = where % 4;
	/*byte enables are 4 bits active low, the position of each
	   bit maps to the byte that it enables */
	byteEnables =
		(~(BIT (n) | BIT ((n + 1)))) &
		IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK;
	byteEnables = byteEnables << PCI_NP_CBE_BESL;
	/*address bits 31:28 specify the device 10:8 specify the function */
	/*Set the address to be read */
	addr = BIT ((31 - dev)) | (where & ~3);
	non_prefetch_read (addr, byteEnables | NP_CMD_CONFIGREAD, &retval);

	/*Pick out the word we are interested in */
	*val = (retval >> (8 * n));

	return (OK);
}

int pci_read_config_byte (pci_dev_t dev, int where, unsigned char *val)
{
	unsigned int retval;
	unsigned int n;
	unsigned int byteEnables;
	unsigned int addr;

	n = where % 4;
	/*byte enables are 4 bits, active low, the position of each
	   bit maps to the byte that it enables */
	byteEnables = (~BIT (n)) & IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK;
	byteEnables = byteEnables << PCI_NP_CBE_BESL;

	/*address bits 31:28 specify the device, 10:8 specify the function */
	/*Set the address to be read */
	addr = BIT ((31 - dev)) | (where & ~3);
	non_prefetch_read (addr, byteEnables | NP_CMD_CONFIGREAD, &retval);
	/*Pick out the byte we are interested in */
	*val = (retval >> (8 * n));

	return (OK);
}

int pci_write_config_byte (pci_dev_t dev, int where, unsigned char val)
{
	unsigned int addr;
	unsigned int byteEnables;
	unsigned int n;
	unsigned int ldata;

	n = where % 4;
	/*byte enables are 4 bits active low, the position of each
	   bit maps to the byte that it enables */
	byteEnables = (~BIT (n)) & IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK;
	byteEnables = byteEnables << PCI_NP_CBE_BESL;
	ldata = val << (8 * n);
	/*address bits 31:28 specify the device 10:8 specify the function */
	/*Set the address to be written */
	addr = BIT ((31 - dev)) | (where & ~3);
	non_prefetch_write (addr, byteEnables | NP_CMD_CONFIGWRITE, ldata);

	return (OK);
}

int pci_write_config_word (pci_dev_t dev, int where, unsigned short val)
{
	unsigned int addr;
	unsigned int byteEnables;
	unsigned int n;
	unsigned int ldata;

	n = where % 4;
	/*byte enables are 4 bits active low, the position of each
	   bit maps to the byte that it enables */
	byteEnables =
		(~(BIT (n) | BIT ((n + 1)))) &
		IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK;
	byteEnables = byteEnables << PCI_NP_CBE_BESL;
	ldata = val << (8 * n);
	/*address bits 31:28 specify the device 10:8 specify the function */
	/*Set the address to be written */
	addr = BIT (31 - dev) | (where & ~3);
	non_prefetch_write (addr, byteEnables | NP_CMD_CONFIGWRITE, ldata);

	return (OK);
}

int pci_write_config_dword (pci_dev_t dev, int where, unsigned int val)
{
	unsigned int addr;

	/*address bits 31:28 specify the device 10:8 specify the function */
	/*Set the address to be written */
	addr = BIT (31 - dev) | (where & ~3);
	non_prefetch_write (addr, NP_CMD_CONFIGWRITE, val);

	return (OK);
}

void non_prefetch_read (unsigned int addr,
			unsigned int cmd, unsigned int *data)
{
	REG_WRITE (PCI_CSR_BASE, PCI_NP_AD_OFFSET, addr);

	/*set up and execute the read */
	REG_WRITE (PCI_CSR_BASE, PCI_NP_CBE_OFFSET, cmd);

	/*The result of the read is now in np_rdata */
	REG_READ (PCI_CSR_BASE, PCI_NP_RDATA_OFFSET, *data);

	return;
}

void non_prefetch_write (unsigned int addr,
			 unsigned int cmd, unsigned int data)
{

	REG_WRITE (PCI_CSR_BASE, PCI_NP_AD_OFFSET, addr);
	/*set up the write */
	REG_WRITE (PCI_CSR_BASE, PCI_NP_CBE_OFFSET, cmd);
	/*Execute the write by writing to NP_WDATA */
	REG_WRITE (PCI_CSR_BASE, PCI_NP_WDATA_OFFSET, data);

	return;
}

/*
 * PCI controller config registers are accessed through these functions
 * i.e. these allow us to set up our own BARs etc.
 */
void crp_read (unsigned int offset, unsigned int *data)
{
	REG_WRITE (PCI_CSR_BASE, PCI_CRP_AD_CBE_OFFSET, offset);
	REG_READ (PCI_CSR_BASE, PCI_CRP_RDATA_OFFSET, *data);
}

void crp_write (unsigned int offset, unsigned int data)
{
	/*The CRP address register bit 16 indicates that we want to do a write */
	REG_WRITE (PCI_CSR_BASE, PCI_CRP_AD_CBE_OFFSET,
		   PCI_CRP_WRITE | offset);
	REG_WRITE (PCI_CSR_BASE, PCI_CRP_WDATA_OFFSET, data);
}

/*struct pci_controller *hose*/
void pci_ixp_init (struct pci_controller *hose)
{
	unsigned int regval;

	hose->first_busno = 0;
	hose->last_busno = 0x00;

	/* System memory space */
	pci_set_region (hose->regions + 0,
			PCI_MEMORY_BUS,
			PCI_MEMORY_PHY, PCI_MEMORY_SIZE, PCI_REGION_SYS_MEMORY);

	/* PCI memory space */
	pci_set_region (hose->regions + 1,
			PCI_MEM_BUS,
			PCI_MEM_PHY, PCI_MEM_SIZE, PCI_REGION_MEM);
	/* PCI I/O space */
	pci_set_region (hose->regions + 2,
			PCI_IO_BUS, PCI_IO_PHY, PCI_IO_SIZE, PCI_REGION_IO);

	hose->region_count = 3;

	pci_register_hose (hose);

/*
 ==========================================================
		Init IXP PCI
 ==========================================================
*/
	REG_READ (PCI_CSR_BASE, PCI_CSR_OFFSET, regval);
	regval |= 1 << 2;
	REG_WRITE (PCI_CSR_BASE, PCI_CSR_OFFSET, regval);

	configure_pins ();

	READ_GPIO_REG (IXP425_GPIO_GPOUTR, regval);
	WRITE_GPIO_REG (IXP425_GPIO_GPOUTR, regval & (~(1 << 13)));
	udelay (533);
	sys_pci_gpio_clock_config ();
	REG_WRITE (PCI_CSR_BASE, PCI_INTEN_OFFSET, 0);
	udelay (100);
	READ_GPIO_REG (IXP425_GPIO_GPOUTR, regval);
	WRITE_GPIO_REG (IXP425_GPIO_GPOUTR, regval | (1 << 13));
	udelay (533);
	crp_write (PCI_CFG_BASE_ADDRESS_0, IXP425_PCI_BAR_0_DEFAULT);
	crp_write (PCI_CFG_BASE_ADDRESS_1, IXP425_PCI_BAR_1_DEFAULT);
	crp_write (PCI_CFG_BASE_ADDRESS_2, IXP425_PCI_BAR_2_DEFAULT);
	crp_write (PCI_CFG_BASE_ADDRESS_3, IXP425_PCI_BAR_3_DEFAULT);
	crp_write (PCI_CFG_BASE_ADDRESS_4, IXP425_PCI_BAR_4_DEFAULT);
	crp_write (PCI_CFG_BASE_ADDRESS_5, IXP425_PCI_BAR_5_DEFAULT);
	/*Setup PCI-AHB and AHB-PCI address mappings */
	REG_WRITE (PCI_CSR_BASE, PCI_AHBMEMBASE_OFFSET,
		   IXP425_PCI_AHBMEMBASE_DEFAULT);

	REG_WRITE (PCI_CSR_BASE, PCI_AHBIOBASE_OFFSET,
		   IXP425_PCI_AHBIOBASE_DEFAULT);

	REG_WRITE (PCI_CSR_BASE, PCI_PCIMEMBASE_OFFSET,
		   IXP425_PCI_PCIMEMBASE_DEFAULT);

	crp_write (PCI_CFG_SUB_VENDOR_ID, IXP425_PCI_SUB_VENDOR_SYSTEM);

	REG_READ (PCI_CSR_BASE, PCI_CSR_OFFSET, regval);
	regval |= PCI_CSR_IC | PCI_CSR_ABE | PCI_CSR_PDS;
	REG_WRITE (PCI_CSR_BASE, PCI_CSR_OFFSET, regval);
	crp_write (PCI_CFG_COMMAND, PCI_CFG_CMD_MAE | PCI_CFG_CMD_BME);
	udelay (1000);

	pci_write_config_word (0, PCI_CFG_COMMAND, INITIAL_PCI_CMD);
	REG_WRITE (PCI_CSR_BASE, PCI_ISR_OFFSET, PCI_ISR_PSE
		   | PCI_ISR_PFE | PCI_ISR_PPE | PCI_ISR_AHBE);
#ifdef CONFIG_PCI_SCAN_SHOW
	printf ("Device  bus  dev  func  deviceID  vendorID \n");
#endif
	pci_bus_scan ();
}

void configure_pins (void)
{
	unsigned int regval;

	/* Disable clock on GPIO PIN 14 */
	READ_GPIO_REG (IXP425_GPIO_GPCLKR, regval);
	WRITE_GPIO_REG (IXP425_GPIO_GPCLKR, regval & (~(1 << 8)));
	READ_GPIO_REG (IXP425_GPIO_GPCLKR, regval);

	READ_GPIO_REG (IXP425_GPIO_GPOER, regval);
	WRITE_GPIO_REG (IXP425_GPIO_GPOER,
			(((~(3 << 13)) & regval) | (0xf << 8)));
	READ_GPIO_REG (IXP425_GPIO_GPOER, regval);

	READ_GPIO_REG (IXP425_GPIO_GPIT2R, regval);
	WRITE_GPIO_REG (IXP425_GPIO_GPIT2R,
			(regval &
			 ((0x1 << 9) | (0x1 << 6) | (0x1 << 3) | 0x1)));
	READ_GPIO_REG (IXP425_GPIO_GPIT2R, regval);

	READ_GPIO_REG (IXP425_GPIO_GPISR, regval);
	WRITE_GPIO_REG (IXP425_GPIO_GPISR, (regval | (0xf << 8)));
	READ_GPIO_REG (IXP425_GPIO_GPISR, regval);
}

void sys_pci_gpio_clock_config (void)
{
	unsigned int regval;

	READ_GPIO_REG (IXP425_GPIO_GPCLKR, regval);
	regval |= 0x1 << 4;
	WRITE_GPIO_REG (IXP425_GPIO_GPCLKR, regval);
	READ_GPIO_REG (IXP425_GPIO_GPCLKR, regval);
	regval |= 0x1 << 8;
	WRITE_GPIO_REG (IXP425_GPIO_GPCLKR, regval);
}

void pci_bus_scan (void)
{
	unsigned int bus = 0, dev, func = 0;
	unsigned short data16;
	unsigned int data32;
	unsigned char intPin;

	/* Assign first device to ourselves */
	devices[0].bus = 0;
	devices[0].device = 0;
	devices[0].func = 0;

	crp_read (PCI_CFG_VENDOR_ID, &data32);

	devices[0].vendor_id = data32 & IXP425_PCI_BOTTOM_WORD_OF_LONG_MASK;
	devices[0].device_id = data32 >> 16;
	devices[0].error = FALSE;
	devices[0].bar[NO_BAR].size = 0;	/*dummy - required */

	nDevices = 1;

	nMBars = 0;
	nIOBars = 0;

	for (dev = 0; dev < IXP425_PCI_MAX_DEV; dev++) {

		/*Check whether a device is present */
		if (pci_device_exists (dev) != TRUE) {

			/*Clear error bits in ISR, write 1 to clear */
			REG_WRITE (PCI_CSR_BASE, PCI_ISR_OFFSET, PCI_ISR_PSE
				   | PCI_ISR_PFE | PCI_ISR_PPE |
				   PCI_ISR_AHBE);
			continue;
		}

		/*A device is present, add an entry to the array */
		devices[nDevices].bus = bus;
		devices[nDevices].device = dev;
		devices[nDevices].func = func;

		pci_read_config_word (dev, PCI_CFG_VENDOR_ID, &data16);

		devices[nDevices].vendor_id = data16;

		pci_read_config_word (dev, PCI_CFG_DEVICE_ID, &data16);
		devices[nDevices].device_id = data16;

		/*The device is functioning correctly, set error to FALSE */
		devices[nDevices].error = FALSE;

		/*Figure out what BARs are on this device */
		sys_pci_bar_info_get (nDevices, bus, dev, func);
		/*Figure out what INTX# line the card uses */
		pci_read_config_byte (dev, PCI_CFG_DEV_INT_PIN, &intPin);

		/*assign the appropriate irq line */
		if (intPin > PCI_IRQ_LINES) {
			devices[nDevices].error = TRUE;
		} else if (intPin != 0) {
			/*This device uses an interrupt line */
			/*devices[nDevices].irq = ixp425PciIntTranslate[dev][intPin-1]; */
			devices[nDevices].irq = intPin;
		}
#ifdef CONFIG_PCI_SCAN_SHOW
		printf ("%06d    %03d %03d %04d  %08d      %08x\n", nDevices,
			devices[nDevices].vendor_id);
#endif
		nDevices++;

	}

	calc_bars (memBars, nMBars, IXP425_PCI_BAR_MEM_BASE);
	sys_pci_device_bars_write ();

	REG_WRITE (PCI_CSR_BASE, PCI_ISR_OFFSET, PCI_ISR_PSE
		   | PCI_ISR_PFE | PCI_ISR_PPE | PCI_ISR_AHBE);
}

void sys_pci_bar_info_get (unsigned int devnum,
			   unsigned int bus,
			   unsigned int dev, unsigned int func)
{
	unsigned int data32;
	unsigned int tmp;
	unsigned int size;

	pci_write_config_dword (devnum,
				PCI_CFG_BASE_ADDRESS_0, IXP425_PCI_BAR_QUERY);
	pci_read_config_dword (devnum, PCI_CFG_BASE_ADDRESS_0, &data32);

	devices[devnum].bar[0].address = (data32 & 1);

	if (data32 & 1) {
		/* IO space */
		tmp = data32 & ~0x3;
		size = ~(tmp - 1);
		devices[devnum].bar[0].size = size;

		if (nIOBars < IXP425_PCI_MAX_BAR) {
			ioBars[nIOBars++] = &devices[devnum].bar[0];
		}
	} else {
		/* Mem space */
		tmp = data32 & ~IXP425_PCI_BOTTOM_NIBBLE_OF_LONG_MASK;
		size = ~(tmp - 1);
		devices[devnum].bar[0].size = size;

		if (nMBars < IXP425_PCI_MAX_BAR) {
			memBars[nMBars++] = &devices[devnum].bar[0];
		} else {
			devices[devnum].error = TRUE;
		}

	}

	devices[devnum].bar[1].size = 0;
}

void sortBars (PciBar * Bars[], unsigned int nBars)
{
	unsigned int i, j;
	PciBar *tmp;

	if (nBars == 0) {
		return;
	}

	/* Sort biggest to smallest */
	for (i = 0; i < nBars - 1; i++) {
		for (j = i + 1; j < nBars; j++) {
			if (Bars[j]->size > Bars[i]->size) {
				/* swap them */
				tmp = Bars[i];
				Bars[i] = Bars[j];
				Bars[j] = tmp;
			}
		}
	}
}

void calc_bars (PciBar * Bars[], unsigned int nBars, unsigned int startAddr)
{
	unsigned int i;

	if (nBars == 0) {
		return;
	}

	for (i = 0; i < nBars; i++) {
		Bars[i]->address |= startAddr;
		startAddr += Bars[i]->size;
	}
}

void sys_pci_device_bars_write (void)
{
	unsigned int i;
	int addr;

	for (i = 1; i < nDevices; i++) {
		if (devices[i].error) {
			continue;
		}

		pci_write_config_dword (devices[i].device,
					PCI_CFG_BASE_ADDRESS_0,
					devices[i].bar[0].address);
		addr = BIT (31 - devices[i].device) |
			(0 << PCI_NP_AD_FUNCSL) |
			(PCI_CFG_BASE_ADDRESS_0 & ~3);
		pci_write_config_dword (devices[i].device,
					PCI_CFG_DEV_INT_LINE, devices[i].irq);

		pci_write_config_word (devices[i].device,
				       PCI_CFG_COMMAND, INITIAL_PCI_CMD);

	}
}


int pci_device_exists (unsigned int deviceNo)
{
	unsigned int vendorId;
	unsigned int regval;

	pci_read_config_dword (deviceNo, PCI_CFG_VENDOR_ID, &vendorId);

	/* There are two ways to find out an empty device.
	 *   1. check Master Abort bit after the access.
	 *   2. check whether the vendor id read back is 0x0.
	 */
	REG_READ (PCI_CSR_BASE, PCI_ISR_OFFSET, regval);
	if ((vendorId != 0x0) && ((regval & PCI_ISR_PFE) == 0)) {
		return TRUE;
	}
	/*no device present, make sure that the master abort bit is reset */

	REG_WRITE (PCI_CSR_BASE, PCI_ISR_OFFSET, PCI_ISR_PFE);
	return FALSE;
}

pci_dev_t pci_find_devices (struct pci_device_id * ids, int devNo)
{
	unsigned int i;
	unsigned int devdidvid;
	unsigned int didvid;
	unsigned int vendorId, deviceId;

	vendorId = ids->vendor;
	deviceId = ids->device;
	didvid = ((deviceId << 16) & IXP425_PCI_TOP_WORD_OF_LONG_MASK) |
		(vendorId & IXP425_PCI_BOTTOM_WORD_OF_LONG_MASK);

	for (i = devNo + 1; i < nDevices; i++) {

		pci_read_config_dword (devices[i].device, PCI_CFG_VENDOR_ID,
				       &devdidvid);

		if (devdidvid == didvid) {
			return devices[i].device;
		}
	}
	return -1;
}
