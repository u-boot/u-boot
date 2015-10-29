/* GRLIB AMBA Plug&Play information scanning, relies on assembler
 * routines.
 *
 * (C) Copyright 2010, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* #define DEBUG */

#include <common.h>
#include <malloc.h>
#include <ambapp.h>
#include <config.h>

/************ C INTERFACE OF ASSEMBLER SCAN ROUTINES ************/
struct ambapp_find_apb_info {
	/* Address of APB device Plug&Play information */
	struct ambapp_pnp_apb	*pnp;
	/* AHB Bus index of where the APB-Master Bridge device was found */
	int			ahb_bus_index;
	int			dec_index;
};

struct ambapp_find_ahb_info {
	/* Address of AHB device Plug&Play information */
	struct ambapp_pnp_ahb	*pnp;
	/* AHB Bus index of where the AHB device was found */
	int			ahb_bus_index;
	int			dec_index;
};

extern void ambapp_find_buses(unsigned int ioarea, struct ambapp_bus *abus);

extern int ambapp_find_apb(struct ambapp_bus *abus, unsigned int dev_vend,
	int index, struct ambapp_find_apb_info *result);

extern int ambapp_find_ahb(struct ambapp_bus *abus, unsigned int dev_vend,
	int index, int type, struct ambapp_find_ahb_info *result);

/************ C ROUTINES USED BY U-BOOT AMBA CORE DRIVERS ************/
struct ambapp_bus ambapp_plb __section(.data);

void ambapp_bus_init(
	unsigned int ioarea,
	unsigned int freq,
	struct ambapp_bus *abus)
{
	int i;

	ambapp_find_buses(ioarea, abus);
	for (i = 0; i < 6; i++)
		if (abus->ioareas[i] == 0)
			break;
	abus->buses = i;
	abus->freq = freq;
}

/* Parse APB PnP Information */
void ambapp_apb_parse(struct ambapp_find_apb_info *info, ambapp_apbdev *dev)
{
	struct ambapp_pnp_apb *apb = info->pnp;
	unsigned int apbbase = (unsigned int)apb & 0xfff00000;

	dev->vendor = amba_vendor(apb->id);
	dev->device = amba_device(apb->id);
	dev->irq = amba_irq(apb->id);
	dev->ver = amba_ver(apb->id);
	dev->address = (apbbase | (((apb->iobar & 0xfff00000) >> 12))) &
			(((apb->iobar &	0x0000fff0) << 4) | 0xfff00000);
	dev->mask = amba_apb_mask(apb->iobar);
	dev->ahb_bus_index = info->ahb_bus_index - 1;
}

/* Parse AHB PnP information */
void ambapp_ahb_parse(struct ambapp_find_ahb_info *info, ambapp_ahbdev *dev)
{
	struct ambapp_pnp_ahb *ahb = info->pnp;
	unsigned int ahbbase = (unsigned int)ahb & 0xfff00000;
	int i, type;
	unsigned int addr, mask, mbar;

	dev->vendor = amba_vendor(ahb->id);
	dev->device = amba_device(ahb->id);
	dev->irq = amba_irq(ahb->id);
	dev->ver = amba_ver(ahb->id);
	dev->userdef[0] = ahb->custom[0];
	dev->userdef[1] = ahb->custom[1];
	dev->userdef[2] = ahb->custom[2];
	dev->ahb_bus_index = info->ahb_bus_index - 1;
	for (i = 0; i < 4; i++) {
		mbar = ahb->mbar[i];
		addr = amba_membar_start(mbar);
		type = amba_membar_type(mbar);
		if (type == AMBA_TYPE_AHBIO) {
			addr = amba_ahbio_adr(addr, ahbbase);
			mask = (((unsigned int)
				(amba_membar_mask((~mbar))<<8)|0xff))+1;
		} else {
			/* AHB memory area, absolute address */
			mask = (~((unsigned int)
				(amba_membar_mask(mbar)<<20)))+1;
		}
		dev->address[i] = addr;
		dev->mask[i] = mask;
		dev->type[i] = type;
	}
}

int ambapp_apb_find(struct ambapp_bus *abus, int vendor, int device,
	int index, ambapp_apbdev *dev)
{
	unsigned int devid = AMBA_PNP_ID(vendor, device);
	int found;
	struct ambapp_find_apb_info apbdev;

	found = ambapp_find_apb(abus, devid, index, &apbdev);
	if (found == 1)
		ambapp_apb_parse(&apbdev, dev);

	return found;
}

int ambapp_apb_count(struct ambapp_bus *abus, int vendor, int device)
{
	unsigned int devid = AMBA_PNP_ID(vendor, device);
	int found;
	struct ambapp_find_apb_info apbdev;

	found = ambapp_find_apb(abus, devid, 63, &apbdev);
	if (found == 1)
		return 64;
	else
		return 63 - apbdev.dec_index;
}

int ambapp_ahb_find(struct ambapp_bus *abus, int vendor, int device,
	int index, ambapp_ahbdev *dev, int type)
{
	int found;
	struct ambapp_find_ahb_info ahbdev;
	unsigned int devid = AMBA_PNP_ID(vendor, device);

	found = ambapp_find_ahb(abus, devid, index, type, &ahbdev);
	if (found == 1)
		ambapp_ahb_parse(&ahbdev, dev);

	return found;
}

int ambapp_ahbmst_find(struct ambapp_bus *abus, int vendor, int device,
	int index, ambapp_ahbdev *dev)
{
	return ambapp_ahb_find(abus, vendor, device, index, dev, DEV_AHB_MST);
}

int ambapp_ahbslv_find(struct ambapp_bus *abus, int vendor, int device,
	int index, ambapp_ahbdev *dev)
{
	return ambapp_ahb_find(abus, vendor, device, index, dev, DEV_AHB_SLV);
}

int ambapp_ahb_count(struct ambapp_bus *abus, int vendor, int device, int type)
{
	int found;
	struct ambapp_find_ahb_info ahbdev;
	unsigned int devid = AMBA_PNP_ID(vendor, device);

	found = ambapp_find_ahb(abus, devid, 63, type, &ahbdev);
	if (found == 1)
		return 64;
	else
		return 63 - ahbdev.dec_index;
}

int ambapp_ahbmst_count(struct ambapp_bus *abus, int vendor, int device)
{
	return ambapp_ahb_count(abus, vendor, device, DEV_AHB_MST);
}

int ambapp_ahbslv_count(struct ambapp_bus *abus, int vendor, int device)
{
	return ambapp_ahb_count(abus, vendor, device, DEV_AHB_SLV);
}

/* The define CONFIG_SYS_GRLIB_SINGLE_BUS may be defined on GRLIB systems
 * where only one AHB Bus is available - no bridges are present. This option
 * is available only to reduce the footprint.
 *
 * Defining this on a multi-bus GRLIB system may also work depending on the
 * design.
 */

#ifndef CONFIG_SYS_GRLIB_SINGLE_BUS

/* GAISLER AHB2AHB Version 1 Bridge Definitions */
#define AHB2AHB_V1_FLAG_FFACT     0x0f0	/* Frequency factor against top bus */
#define AHB2AHB_V1_FLAG_FFACT_DIR 0x100	/* Factor direction, 0=down, 1=up */
#define AHB2AHB_V1_FLAG_MBUS      0x00c	/* Master bus number mask */
#define AHB2AHB_V1_FLAG_SBUS      0x003	/* Slave bus number mask */

/* Get Parent bus frequency. Note that since we go from a "child" bus
 * to a parent bus, the frequency factor direction is inverted.
 */
unsigned int gaisler_ahb2ahb_v1_freq(ambapp_ahbdev *ahb, unsigned int freq)
{
	int dir;
	unsigned char ffact;

	/* Get division/multiple factor */
	ffact = (ahb->userdef[0] & AHB2AHB_V1_FLAG_FFACT) >> 4;
	if (ffact != 0) {
		dir = ahb->userdef[0] & AHB2AHB_V1_FLAG_FFACT_DIR;

		/* Calculate frequency by dividing or
		 * multiplying system frequency
		 */
		if (dir)
			freq = freq * ffact;
		else
			freq = freq / ffact;
	}

	return freq;
}

/* AHB2AHB and L2CACHE ver 2 is not supported yet. */
unsigned int gaisler_ahb2ahb_v2_freq(ambapp_ahbdev *ahb, unsigned int freq)
{
	panic("gaisler_ahb2ahb_v2_freq: AHB2AHB ver 2 not supported\n");
	return -1;
}
#endif

/* Return the frequency of a AHB bus identified by index found
 * note that this is not the AHB Bus number.
 */
unsigned int ambapp_bus_freq(struct ambapp_bus *abus, int ahb_bus_index)
{
	unsigned int freq = abus->freq;
#ifndef CONFIG_SYS_GRLIB_SINGLE_BUS
	unsigned int ioarea, ioarea_parent, bridge_pnp_ofs;
	struct ambapp_find_ahb_info ahbinfo;
	ambapp_ahbdev ahb;
	int parent;

	debug("ambapp_bus_freq: get freq on bus %d\n", ahb_bus_index);

	while (ahb_bus_index != 0) {
		debug("  BUS[0]: 0x%08x\n", abus->ioareas[0]);
		debug("  BUS[1]: 0x%08x\n", abus->ioareas[1]);
		debug("  BUS[2]: 0x%08x\n", abus->ioareas[2]);
		debug("  BUS[3]: 0x%08x\n", abus->ioareas[3]);
		debug("  BUS[4]: 0x%08x\n", abus->ioareas[4]);
		debug("  BUS[5]: 0x%08x\n", abus->ioareas[5]);

		/* Get I/O area of AHB bus */
		ioarea = abus->ioareas[ahb_bus_index];

		debug("  IOAREA: 0x%08x\n", ioarea);

		/* Get parent bus */
		parent = (ioarea & 0x7);
		if (parent == 0) {
			panic("%s: parent=0 indicates no parent! Stopping.\n",
				__func__);
			return -1;
		}
		parent = parent - 1;
		bridge_pnp_ofs = ioarea & 0x7e0;

		debug("  PARENT: %d\n", parent);
		debug("  BRIDGE_OFS: 0x%08x\n", bridge_pnp_ofs);

		/* Get AHB/AHB bridge PnP address */
		ioarea_parent = (abus->ioareas[parent] & 0xfff00000) |
				AMBA_CONF_AREA | AMBA_AHB_SLAVE_CONF_AREA;
		ahbinfo.pnp = (struct ambapp_pnp_ahb *)
				(ioarea_parent | bridge_pnp_ofs);

		debug("  IOAREA PARENT: 0x%08x\n", ioarea_parent);
		debug("  BRIDGE PNP: 0x%p\n", ahbinfo.pnp);

		/* Parse the AHB information */
		ahbinfo.ahb_bus_index = parent;
		ambapp_ahb_parse(&ahbinfo, &ahb);

		debug("  BRIDGE ID: VENDOR=%d(0x%x), DEVICE=%d(0x%x)\n",
			ahb.vendor, ahb.vendor, ahb.device, ahb.device);

		/* Different bridges may convert frequency differently */
		if ((ahb.vendor == VENDOR_GAISLER) &&
			((ahb.device == GAISLER_AHB2AHB) ||
			(ahb.device == GAISLER_L2CACHE))) {
			/* Get new frequency */
			if (ahb.ver > 1)
				freq = gaisler_ahb2ahb_v2_freq(&ahb, freq);
			else
				freq = gaisler_ahb2ahb_v1_freq(&ahb, freq);

			debug("  NEW FREQ: %dHz\n", freq);
		} else {
			panic("%s: unsupported AMBA bridge\n", __func__);
			return -1;
		}

		/* Step upwards towards system top bus */
		ahb_bus_index = parent;
	}
#endif

	debug("ambapp_bus_freq: %dHz\n", freq);

	return freq;
}
