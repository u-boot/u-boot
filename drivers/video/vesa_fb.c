/*
 *
 * Vesa frame buffer driver for x86
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

/* Devices to allow - only the last one works fully */
struct pci_device_id vesa_video_ids[] = {
	{ .vendor = 0x102b, .device = 0x0525 },
	{ .vendor = 0x1002, .device = 0x5159 },
	{ .vendor = 0x1002, .device = 0x4752 },
	{ .vendor = 0x1002, .device = 0x5452 },
	{ .vendor = 0x8086, .device = 0x0f31 },
	{},
};

void *video_hw_init(void)
{
	GraphicDevice *gdev = &ctfb;
	int bits_per_pixel;
	pci_dev_t dev;
	int ret;

	printf("Video: ");
	if (vbe_get_video_info(gdev)) {
		/* TODO: Should we look these up by class? */
		dev = pci_find_devices(vesa_video_ids, 0);
		if (dev == -1) {
			printf("no card detected\n");
			return NULL;
		}
		bootstage_start(BOOTSTAGE_ID_ACCUM_LCD, "vesa display");
		ret = pci_run_vga_bios(dev, NULL, PCI_ROM_USE_NATIVE |
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
