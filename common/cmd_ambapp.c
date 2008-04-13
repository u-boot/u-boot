/*
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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

/*
 * AMBA Plug&Play information list command
 *
 */
#include <common.h>
#include <command.h>
#include <ambapp.h>

DECLARE_GLOBAL_DATA_PTR;

/* We put these variables into .data section so that they are zero
 * when entering the AMBA Plug & Play routines (in cpu/cpu/ambapp.c)
 * the first time. BSS is not garantueed to be zero since BSS
 * hasn't been cleared the first times entering the CPU AMBA functions.
 *
 * The AMBA PnP routines call these functions if ambapp_???_print is set.
 *
 */
int ambapp_apb_print __attribute__ ((section(".data"))) = 0;
int ambapp_ahb_print __attribute__ ((section(".data"))) = 0;

typedef struct {
	int device_id;
	char *name;
} ambapp_device_name;

static ambapp_device_name gaisler_devices[] = {
	{GAISLER_LEON3, "GAISLER_LEON3"},
	{GAISLER_LEON3DSU, "GAISLER_LEON3DSU"},
	{GAISLER_ETHAHB, "GAISLER_ETHAHB"},
	{GAISLER_ETHMAC, "GAISLER_ETHMAC"},
	{GAISLER_APBMST, "GAISLER_APBMST"},
	{GAISLER_AHBUART, "GAISLER_AHBUART"},
	{GAISLER_SRCTRL, "GAISLER_SRCTRL"},
	{GAISLER_SDCTRL, "GAISLER_SDCTRL"},
	{GAISLER_APBUART, "GAISLER_APBUART"},
	{GAISLER_IRQMP, "GAISLER_IRQMP"},
	{GAISLER_AHBRAM, "GAISLER_AHBRAM"},
	{GAISLER_GPTIMER, "GAISLER_GPTIMER"},
	{GAISLER_PCITRG, "GAISLER_PCITRG"},
	{GAISLER_PCISBRG, "GAISLER_PCISBRG"},
	{GAISLER_PCIFBRG, "GAISLER_PCIFBRG"},
	{GAISLER_PCITRACE, "GAISLER_PCITRACE"},
	{GAISLER_AHBTRACE, "GAISLER_AHBTRACE"},
	{GAISLER_ETHDSU, "GAISLER_ETHDSU"},
	{GAISLER_PIOPORT, "GAISLER_PIOPORT"},
	{GAISLER_AHBJTAG, "GAISLER_AHBJTAG"},
	{GAISLER_ATACTRL, "GAISLER_ATACTRL"},
	{GAISLER_VGA, "GAISLER_VGA"},
	{GAISLER_KBD, "GAISLER_KBD"},
	{GAISLER_L2TIME, "GAISLER_L2TIME"},
	{GAISLER_L2C, "GAISLER_L2C"},
	{GAISLER_PLUGPLAY, "GAISLER_PLUGPLAY"},
	{GAISLER_SPW, "GAISLER_SPW"},
	{GAISLER_SPW2, "GAISLER_SPW2"},
	{GAISLER_EHCI, "GAISLER_EHCI"},
	{GAISLER_UHCI, "GAISLER_UHCI"},
	{GAISLER_AHBSTAT, "GAISLER_AHBSTAT"},
	{GAISLER_DDR2SPA, "GAISLER_DDR2SPA"},
	{GAISLER_DDRSPA, "GAISLER_DDRSPA"},
	{0, NULL}
};

static ambapp_device_name esa_devices[] = {
	{ESA_LEON2, "ESA_LEON2"},
	{ESA_MCTRL, "ESA_MCTRL"},
	{0, NULL}
};

static ambapp_device_name opencores_devices[] = {
	{OPENCORES_PCIBR, "OPENCORES_PCIBR"},
	{OPENCORES_ETHMAC, "OPENCORES_ETHMAC"},
	{0, NULL}
};

typedef struct {
	unsigned int vendor_id;
	char *name;
	ambapp_device_name *devices;
} ambapp_vendor_devnames;

static ambapp_vendor_devnames vendors[] = {
	{VENDOR_GAISLER, "VENDOR_GAISLER", gaisler_devices},
	{VENDOR_ESA, "VENDOR_ESA", esa_devices},
	{VENDOR_OPENCORES, "VENDOR_OPENCORES", opencores_devices},
	{0, NULL, 0}
};

static char *ambapp_get_devname(ambapp_device_name * devs, int id)
{
	if (!devs)
		return NULL;

	while (devs->device_id > 0) {
		if (devs->device_id == id)
			return devs->name;
		devs++;
	}
	return NULL;
}

char *ambapp_device_id2str(int vendor, int id)
{
	ambapp_vendor_devnames *ven = &vendors[0];

	while (ven->vendor_id > 0) {
		if (ven->vendor_id == vendor) {
			return ambapp_get_devname(ven->devices, id);
		}
		ven++;
	}
	return NULL;
}

char *ambapp_vendor_id2str(int vendor)
{
	ambapp_vendor_devnames *ven = &vendors[0];

	while (ven->vendor_id > 0) {
		if (ven->vendor_id == vendor) {
			return ven->name;
		}
		ven++;
	}
	return NULL;
}

static char *unknown = "unknown";

/* Print one APB device */
void ambapp_print_apb(apbctrl_pp_dev * apb, ambapp_ahbdev * apbmst, int index)
{
	char *dev_str, *ven_str;
	int irq, ver, vendor, deviceid;
	unsigned int address, apbmst_base, mask;

	vendor = amba_vendor(apb->conf);
	deviceid = amba_device(apb->conf);
	irq = amba_irq(apb->conf);
	ver = amba_ver(apb->conf);
	apbmst_base = apbmst->address[0] & LEON3_IO_AREA;
	address = (apbmst_base | (((apb->bar & 0xfff00000) >> 12))) &
	    (((apb->bar & 0x0000fff0) << 4) | 0xfff00000);

	mask = amba_membar_mask(apb->bar) << 8;
	mask = ((~mask) & 0x000fffff) + 1;

	ven_str = ambapp_vendor_id2str(vendor);
	if (!ven_str) {
		ven_str = unknown;
		dev_str = unknown;
	} else {
		dev_str = ambapp_device_id2str(vendor, deviceid);
		if (!dev_str)
			dev_str = unknown;
	}

	printf("0x%02x:0x%02x:0x%02x: %s  %s\n"
	       "   apb: 0x%08x - 0x%08x\n"
	       "   irq: %-2d (ver: %-2d)\n",
	       index, vendor, deviceid, ven_str, dev_str, address,
	       address + mask, irq, ver);
}

void ambapp_print_ahb(ahbctrl_pp_dev * ahb, int index)
{
	char *dev_str, *ven_str;
	int irq, ver, vendor, deviceid;
	unsigned int addr, mask;
	int j;

	vendor = amba_vendor(ahb->conf);
	deviceid = amba_device(ahb->conf);
	irq = amba_irq(ahb->conf);
	ver = amba_ver(ahb->conf);

	ven_str = ambapp_vendor_id2str(vendor);
	if (!ven_str) {
		ven_str = unknown;
		dev_str = unknown;
	} else {
		dev_str = ambapp_device_id2str(vendor, deviceid);
		if (!dev_str)
			dev_str = unknown;
	}

	printf("0x%02x:0x%02x:0x%02x: %s  %s\n",
	       index, vendor, deviceid, ven_str, dev_str);

	for (j = 0; j < 4; j++) {
		addr = amba_membar_start(ahb->bars[j]);
		if (amba_membar_type(ahb->bars[j]) == 0)
			continue;
		if (amba_membar_type(ahb->bars[j]) == AMBA_TYPE_AHBIO)
			addr = AMBA_TYPE_AHBIO_ADDR(addr);
		mask = amba_membar_mask(ahb->bars[j]) << 20;
		printf("   mem: 0x%08x - 0x%08x\n", addr, addr + ((~mask) + 1));
	}

	printf("   irq: %-2d (ver: %d)\n", irq, ver);
}

int do_ambapp_print(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{

	/* Print AHB Masters */
	puts("--------- AHB Masters ---------\n");
	ambapp_apb_print = 0;
	ambapp_ahb_print = 1;
	ambapp_ahbmst_count(99, 99);	/* Get vendor&device 99 = nonexistent... */

	/* Print AHB Slaves */
	puts("--------- AHB Slaves  ---------\n");
	ambapp_ahbslv_count(99, 99);	/* Get vendor&device 99 = nonexistent... */

	/* Print APB Slaves */
	puts("--------- APB Slaves  ---------\n");
	ambapp_apb_print = 1;
	ambapp_ahb_print = 0;
	ambapp_apb_count(99, 99);	/* Get vendor&device 99 = nonexistent... */

	/* Reset, no futher printing */
	ambapp_apb_print = 0;
	ambapp_ahb_print = 0;
	puts("\n");
	return 0;
}

int ambapp_init_reloc(void)
{
	ambapp_vendor_devnames *vend = vendors;
	ambapp_device_name *dev;

	while (vend->vendor_id && vend->name) {
		vend->name = (char *)((unsigned int)vend->name + gd->reloc_off);
		vend->devices =
		    (ambapp_device_name *) ((unsigned int)vend->devices +
					    gd->reloc_off);;
		dev = vend->devices;
		vend++;
		if (!dev)
			continue;
		while (dev->device_id && dev->name) {
			dev->name =
			    (char *)((unsigned int)dev->name + gd->reloc_off);;
			dev++;
		}
	}
	return 0;
}

U_BOOT_CMD(ambapp, 1, 1, do_ambapp_print,
	   "ambapp  - list AMBA Plug&Play information\n",
	   "ambapp\n"
	   "    - lists AMBA (AHB & APB) Plug&Play devices present on the system\n");
