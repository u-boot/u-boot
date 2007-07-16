/*
 * (C) Copyright 2007 Schindler Lift Inc.
 * (C) Copyright 2007 Semihalf
 *
 * Author: Michel Marti <mma@objectxp.com>
 * Adapted for U-Boot 1.2 by Piotr Kruszynski <ppk@semihalf.com>:
 * - code clean-up
 * - bugfix for overwriting bootargs by user
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <image.h>
#include <usb.h>
#include <fat.h>

#include "fwupdate.h"

extern int do_bootm(cmd_tbl_t *, int, int, char *[]);
extern long do_fat_read(const char *, void *, unsigned long, int);
extern int do_fat_fsload(cmd_tbl_t *, int, int, char *[]);

static int load_rescue_image(ulong);

void cm1_fwupdate(void)
{
	cmd_tbl_t *bcmd;
	char *rsargs;
	char *tmp = NULL;
	char ka[16];
	char *argv[3] = { "bootm", ka, NULL };

	/* Check if rescue system is disabled... */
	if (getenv("norescue")) {
		printf(LOG_PREFIX "Rescue System disabled.\n");
		return;
	}

	/* Check if we have a USB storage device and load image */
	if (load_rescue_image(LOAD_ADDR))
		return;

	bcmd = find_cmd("bootm");
	if (!bcmd)
		return;

	sprintf(ka, "%lx", LOAD_ADDR);

	/* prepare our bootargs */
	rsargs = getenv("rs-args");
	if (!rsargs)
		rsargs = RS_BOOTARGS;
	else {
		tmp = malloc(strlen(rsargs+1));
		if (!tmp) {
			printf(LOG_PREFIX "Memory allocation failed\n");
			return;
		}
		strcpy(tmp, rsargs);
		rsargs = tmp;
	}

	setenv("bootargs", rsargs);

	if (rsargs == tmp)
		free(rsargs);

	printf(LOG_PREFIX "Starting update system (bootargs=%s)...\n", rsargs);
	do_bootm(bcmd, 0, 2, argv);
}

static int load_rescue_image(ulong addr)
{
	disk_partition_t info;
	int devno;
	int partno;
	int i;
	char fwdir[64];
	char nxri[128];
	char *tmp;
	char dev[7];
	char addr_str[16];
	char *argv[6] = { "fatload", "usb", dev, addr_str, nxri, NULL };
	block_dev_desc_t *stor_dev = NULL;
	cmd_tbl_t *bcmd;

	/* Get name of firmware directory */
	tmp = getenv("fw-dir");

	/* Copy it into fwdir */
	strncpy(fwdir, tmp ? tmp : FW_DIR, sizeof(fwdir));
	fwdir[sizeof(fwdir) - 1] = 0; /* Terminate string */

	printf(LOG_PREFIX "Checking for firmware image directory '%s' on USB"
		" storage...\n", fwdir);
	usb_stop();
	if (usb_init() != 0)
		return 1;

	/* Check for storage device */
	if (usb_stor_scan(1) != 0) {
		usb_stop();
		return 1;
	}

	/* Detect storage device */
	for (devno = 0; devno < USB_MAX_STOR_DEV; devno++) {
		stor_dev = usb_stor_get_dev(devno);
		if (stor_dev->type != DEV_TYPE_UNKNOWN)
			break;
	}
	if (!stor_dev || stor_dev->type == DEV_TYPE_UNKNOWN) {
		printf(LOG_PREFIX "No valid storage device found...\n");
		usb_stop();
		return 1;
	}

	/* Detect partition */
	for (partno = -1, i = 0; i < 6; i++) {
		if (get_partition_info(stor_dev, i, &info) == 0) {
			if (fat_register_device(stor_dev, i) == 0) {
				/* Check if rescue image is present */
				FW_DEBUG("Looking for firmware directory '%s'"
					" on partition %d\n", fwdir, i);
				if (do_fat_read(fwdir, NULL, 0, LS_NO) == -1) {
					FW_DEBUG("No NX rescue image on "
						"partition %d.\n", i);
				} else {
					partno = i;
					FW_DEBUG("Partition %d contains "
						"firmware directory\n", partno);
					break;
				}
			}
		}
	}

	if (partno == -1) {
		printf(LOG_PREFIX "Error: No valid (FAT) partition detected\n");
		usb_stop();
		return 1;
	}

	/* Load the rescue image */
	bcmd = find_cmd("fatload");
	if (!bcmd) {
		printf(LOG_PREFIX "Error - 'fatload' command not present.\n");
		usb_stop();
		return 1;
	}

	tmp = getenv("nx-rescue-image");
	sprintf(nxri, "%s/%s", fwdir, tmp ? tmp : RESCUE_IMAGE);
	sprintf(dev, "%d:%d", devno, partno);
	sprintf(addr_str, "%lx", addr);

	FW_DEBUG("fat_fsload device='%s', addr='%s', file: %s\n",
		dev, addr_str, nxri);

	if (do_fat_fsload(bcmd, 0, 5, argv) != 0) {
		usb_stop();
		return 1;
	}

	/* Stop USB */
	usb_stop();
	return 0;
}
