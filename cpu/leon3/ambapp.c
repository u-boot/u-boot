/* Gaisler AMBA Plug&Play bus scanning. Functions
 * ending on _nomem is inteded to be used only during
 * initialization, only registers are used (no ram).
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
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
#include <command.h>
#include <ambapp.h>

static int ambapp_apb_scan(unsigned int vendor,	/* Plug&Play Vendor ID */
			   unsigned int driver,	/* Plug&Play Device ID */
			   ambapp_apbdev * dev,	/* Result(s) is placed here */
			   int index,	/* Index of device to start copying Plug&Play
					 * info into dev
					 */
			   int max_cnt	/* Maximal count that dev can hold, if dev
					 * is NULL function will stop scanning after
					 * max_cnt devices are found.
					 */
    )
{
	int i, cnt = 0;
	unsigned int apbmst_base;
	ambapp_ahbdev apbmst;
	apbctrl_pp_dev *apb;

	if (max_cnt == 0)
		return 0;

	/* Get AMBA APB Master */
	if (ambapp_ahbslv_first(VENDOR_GAISLER, GAISLER_APBMST, &apbmst) != 1) {
		return 0;
	}

	/* Get APB CTRL Plug&Play info area */
	apbmst_base = apbmst.address[0] & LEON3_IO_AREA;
	apb = (apbctrl_pp_dev *) (apbmst_base | LEON3_CONF_AREA);

	for (i = 0; i < LEON3_APB_SLAVES; i++) {
		if ((amba_vendor(apb->conf) == vendor) &&
		    (amba_device(apb->conf) == driver) && ((index < 0)
							   || (index-- == 0))) {
			/* Convert Plug&Play info into a more readable format */
			cnt++;
			if (dev) {
				dev->irq = amba_irq(apb->conf);
				dev->ver = amba_ver(apb->conf);
				dev->address =
				    (apbmst_base |
				     (((apb->
					bar & 0xfff00000) >> 12))) & (((apb->
									bar &
									0x0000fff0)
								       << 4) |
								      0xfff00000);
				dev++;
			}
			/* found max devices? */
			if (cnt >= max_cnt)
				return cnt;
		}
		/* Get next Plug&Play entry */
		apb++;
	}
	return cnt;
}

unsigned int ambapp_apb_next_nomem(register unsigned int vendor,	/* Plug&Play Vendor ID */
				   register unsigned int driver,	/* Plug&Play Device ID */
				   register int index)
{
	register int i;
	register ahbctrl_pp_dev *apbmst;
	register apbctrl_pp_dev *apb;
	register unsigned int apbmst_base;

	/* APBMST is a AHB Slave */
	apbmst = ambapp_ahb_next_nomem(VENDOR_GAISLER, GAISLER_APBMST, 1, 0);
	if (!apbmst)
		return 0;

	apbmst_base = amba_membar_start(apbmst->bars[0]);
	if (amba_membar_type(apbmst->bars[0]) == AMBA_TYPE_AHBIO)
		apbmst_base = AMBA_TYPE_AHBIO_ADDR(apbmst_base);
	apbmst_base &= LEON3_IO_AREA;

	/* Find the vendor/driver device on the first APB bus */
	apb = (apbctrl_pp_dev *) (apbmst_base | LEON3_CONF_AREA);

	for (i = 0; i < LEON3_APB_SLAVES; i++) {
		if ((amba_vendor(apb->conf) == vendor) &&
		    (amba_device(apb->conf) == driver) && ((index < 0)
							   || (index-- == 0))) {
			/* Convert Plug&Play info info a more readable format */
			return (apbmst_base | (((apb->bar & 0xfff00000) >> 12)))
			    & (((apb->bar & 0x0000fff0) << 4) | 0xfff00000);
		}
		/* Get next Plug&Play entry */
		apb++;
	}
	return 0;
}

/****************************** APB SLAVES ******************************/

int ambapp_apb_count(unsigned int vendor, unsigned int driver)
{
	return ambapp_apb_scan(vendor, driver, NULL, 0, LEON3_APB_SLAVES);
}

int ambapp_apb_first(unsigned int vendor,
		     unsigned int driver, ambapp_apbdev * dev)
{
	return ambapp_apb_scan(vendor, driver, dev, 0, 1);
}

int ambapp_apb_next(unsigned int vendor,
		    unsigned int driver, ambapp_apbdev * dev, int index)
{
	return ambapp_apb_scan(vendor, driver, dev, index, 1);
}

int ambapp_apbs_first(unsigned int vendor,
		      unsigned int driver, ambapp_apbdev * dev, int max_cnt)
{
	return ambapp_apb_scan(vendor, driver, dev, 0, max_cnt);
}

enum {
	AHB_SCAN_MASTER = 0,
	AHB_SCAN_SLAVE = 1
};

/* Scan AMBA Plug&Play bus for AMBA AHB Masters or AHB Slaves
 * for a certain matching Vendor and Device ID.
 *
 * Return number of devices found.
 *
 * Compact edition...
 */
static int ambapp_ahb_scan(unsigned int vendor,	/* Plug&Play Vendor ID */
			   unsigned int driver,	/* Plug&Play Device ID */
			   ambapp_ahbdev * dev,	/* Result(s) is placed here */
			   int index,	/* Index of device to start copying Plug&Play
					 * info into dev
					 */
			   int max_cnt,	/* Maximal count that dev can hold, if dev
					 * is NULL function will stop scanning after
					 * max_cnt devices are found.
					 */
			   int type	/* Selectes what type of devices to scan.
					 * 0=AHB Masters
					 * 1=AHB Slaves
					 */
    )
{
	int i, j, cnt = 0, max_pp_devs;
	unsigned int addr;
	ahbctrl_info *info = (ahbctrl_info *) (LEON3_IO_AREA | LEON3_CONF_AREA);
	ahbctrl_pp_dev *ahb;

	if (max_cnt == 0)
		return 0;

	if (type == 0) {
		max_pp_devs = LEON3_AHB_MASTERS;
		ahb = info->masters;
	} else {
		max_pp_devs = LEON3_AHB_SLAVES;
		ahb = info->slaves;
	}

	for (i = 0; i < max_pp_devs; i++) {
		if ((amba_vendor(ahb->conf) == vendor) &&
		    (amba_device(ahb->conf) == driver) &&
		    ((index < 0) || (index-- == 0))) {
			/* Convert Plug&Play info info a more readable format */
			cnt++;
			if (dev) {
				dev->irq = amba_irq(ahb->conf);
				dev->ver = amba_ver(ahb->conf);
				dev->userdef[0] = ahb->userdef[0];
				dev->userdef[1] = ahb->userdef[1];
				dev->userdef[2] = ahb->userdef[2];
				for (j = 0; j < 4; j++) {
					addr = amba_membar_start(ahb->bars[j]);
					if (amba_membar_type(ahb->bars[j]) ==
					    AMBA_TYPE_AHBIO)
						addr =
						    AMBA_TYPE_AHBIO_ADDR(addr);
					dev->address[j] = addr;
				}
				dev++;
			}
			/* found max devices? */
			if (cnt >= max_cnt)
				return cnt;
		}
		/* Get next Plug&Play entry */
		ahb++;
	}
	return cnt;
}

unsigned int ambapp_ahb_get_info(ahbctrl_pp_dev * ahb, int info)
{
	register unsigned int ret;

	if (!ahb)
		return 0;

	switch (info) {
	default:
		info = 0;
	case 0:
	case 1:
	case 2:
	case 3:
		/* Get Address from PnP Info */
		ret = amba_membar_start(ahb->bars[info]);
		if (amba_membar_type(ahb->bars[info]) == AMBA_TYPE_AHBIO)
			ret = AMBA_TYPE_AHBIO_ADDR(ret);
		return ret;
	}
	return 0;

}

ahbctrl_pp_dev *ambapp_ahb_next_nomem(register unsigned int vendor,	/* Plug&Play Vendor ID */
				      register unsigned int driver,	/* Plug&Play Device ID */
				      register unsigned int opts,	/* 1=slave, 0=master */
				      register int index)
{
	register ahbctrl_pp_dev *ahb;
	register ahbctrl_info *info =
	    (ahbctrl_info *) (LEON3_IO_AREA | LEON3_CONF_AREA);
	register int i;
	register int max_pp_devs;

	if (opts == 0) {
		max_pp_devs = LEON3_AHB_MASTERS;
		ahb = info->masters;
	} else {
		max_pp_devs = LEON3_AHB_SLAVES;
		ahb = info->slaves;
	}

	for (i = 0; i < max_pp_devs; i++) {
		if ((amba_vendor(ahb->conf) == vendor) &&
		    (amba_device(ahb->conf) == driver) &&
		    ((index < 0) || (index-- == 0))) {
			/* Convert Plug&Play info info a more readable format */
			return ahb;
		}
		/* Get next Plug&Play entry */
		ahb++;
	}
	return 0;
}

/****************************** AHB MASTERS ******************************/
int ambapp_ahbmst_count(unsigned int vendor, unsigned int driver)
{
	/* Get number of devices of this vendor&device ID */
	return ambapp_ahb_scan(vendor, driver, NULL, 0, LEON3_AHB_MASTERS,
			       AHB_SCAN_MASTER);
}

int ambapp_ahbmst_first(unsigned int vendor, unsigned int driver,
			ambapp_ahbdev * dev)
{
	/* find first device of this */
	return ambapp_ahb_scan(vendor, driver, dev, 0, 1, AHB_SCAN_MASTER);
}

int ambapp_ahbmst_next(unsigned int vendor,
		       unsigned int driver, ambapp_ahbdev * dev, int index)
{
	/* find first device of this */
	return ambapp_ahb_scan(vendor, driver, dev, index, 1, AHB_SCAN_MASTER);
}

int ambapp_ahbmsts_first(unsigned int vendor,
			 unsigned int driver, ambapp_ahbdev * dev, int max_cnt)
{
	/* find first device of this */
	return ambapp_ahb_scan(vendor, driver, dev, 0, max_cnt,
			       AHB_SCAN_MASTER);
}

/****************************** AHB SLAVES ******************************/
int ambapp_ahbslv_count(unsigned int vendor, unsigned int driver)
{
	/* Get number of devices of this vendor&device ID */
	return ambapp_ahb_scan(vendor, driver, NULL, 0, LEON3_AHB_SLAVES,
			       AHB_SCAN_SLAVE);
}

int ambapp_ahbslv_first(unsigned int vendor, unsigned int driver,
			ambapp_ahbdev * dev)
{
	/* find first device of this */
	return ambapp_ahb_scan(vendor, driver, dev, 0, 1, AHB_SCAN_SLAVE);
}

int ambapp_ahbslv_next(unsigned int vendor,
		       unsigned int driver, ambapp_ahbdev * dev, int index)
{
	/* find first device of this */
	return ambapp_ahb_scan(vendor, driver, dev, index, 1, AHB_SCAN_SLAVE);
}

int ambapp_ahbslvs_first(unsigned int vendor,
			 unsigned int driver, ambapp_ahbdev * dev, int max_cnt)
{
	/* find first device of this */
	return ambapp_ahb_scan(vendor, driver, dev, 0, max_cnt, AHB_SCAN_SLAVE);
}
