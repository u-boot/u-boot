/*
 * VESA frame buffer driver
 *
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <pci_rom.h>
#include <video_fb.h>
#include <vbe.h>

/*
 * The Graphic Device
 */
GraphicDevice ctfb;

void *video_hw_init(void)
{
	GraphicDevice *gdev = &ctfb;
	struct udevice *dev;
	int bits_per_pixel;
	int ret;

	printf("Video: ");
	if (!ll_boot_init()) {
		/*
		 * If we are running from EFI or coreboot, this driver can't
		 * work.
		 */
		printf("Not available (previous bootloader prevents it)\n");
		return NULL;
	}
	if (vbe_get_video_info(gdev)) {
		ret = dm_pci_find_class(PCI_CLASS_DISPLAY_VGA << 8, 0, &dev);
		if (ret) {
			printf("no card detected\n");
			return NULL;
		}
		bootstage_start(BOOTSTAGE_ID_ACCUM_LCD, "vesa display");
		ret = dm_pci_run_vga_bios(dev, NULL, PCI_ROM_USE_NATIVE |
					  PCI_ROM_ALLOW_FALLBACK);
		bootstage_accum(BOOTSTAGE_ID_ACCUM_LCD);
		if (ret) {
			printf("failed to run video BIOS: %d\n", ret);
			return NULL;
		}
	}

	if (vbe_get_video_info(gdev)) {
		printf("No video mode configured\n");
		return NULL;
	}

	bits_per_pixel = gdev->gdfBytesPP * 8;
	sprintf(gdev->modeIdent, "%dx%dx%d", gdev->winSizeX, gdev->winSizeY,
		bits_per_pixel);
	printf("%s\n", gdev->modeIdent);
	debug("Frame buffer at %x\n", gdev->pciBase);

	return (void *)gdev;
}
