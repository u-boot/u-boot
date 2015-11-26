/*
 * Copyright (C) 2014 Google, Inc
 *
 * From coreboot, originally based on the Linux kernel (drivers/pci/pci.c).
 *
 * Modifications are:
 * Copyright (C) 2003-2004 Linux Networx
 * (Written by Eric Biederman <ebiederman@lnxi.com> for Linux Networx)
 * Copyright (C) 2003-2006 Ronald G. Minnich <rminnich@gmail.com>
 * Copyright (C) 2004-2005 Li-Ta Lo <ollie@lanl.gov>
 * Copyright (C) 2005-2006 Tyan
 * (Written by Yinghai Lu <yhlu@tyan.com> for Tyan)
 * Copyright (C) 2005-2009 coresystems GmbH
 * (Written by Stefan Reinauer <stepan@coresystems.de> for coresystems GmbH)
 *
 * PCI Bus Services, see include/linux/pci.h for further explanation.
 *
 * Copyright 1993 -- 1997 Drew Eckhardt, Frederic Potter,
 * David Mosberger-Tang
 *
 * Copyright 1997 -- 1999 Martin Mares <mj@atrey.karlin.mff.cuni.cz>

 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <bios_emul.h>
#include <errno.h>
#include <malloc.h>
#include <pci.h>
#include <pci_rom.h>
#include <vbe.h>
#include <video_fb.h>
#include <linux/screen_info.h>

__weak bool board_should_run_oprom(pci_dev_t dev)
{
	return true;
}

static bool should_load_oprom(pci_dev_t dev)
{
	if (IS_ENABLED(CONFIG_ALWAYS_LOAD_OPROM))
		return 1;
	if (board_should_run_oprom(dev))
		return 1;

	return 0;
}

__weak uint32_t board_map_oprom_vendev(uint32_t vendev)
{
	return vendev;
}

static int pci_rom_probe(pci_dev_t dev, uint class,
			 struct pci_rom_header **hdrp)
{
	struct pci_rom_header *rom_header;
	struct pci_rom_data *rom_data;
	u16 vendor, device;
	u16 rom_vendor, rom_device;
	u32 rom_class;
	u32 vendev;
	u32 mapped_vendev;
	u32 rom_address;

	pci_read_config_word(dev, PCI_VENDOR_ID, &vendor);
	pci_read_config_word(dev, PCI_DEVICE_ID, &device);
	vendev = vendor << 16 | device;
	mapped_vendev = board_map_oprom_vendev(vendev);
	if (vendev != mapped_vendev)
		debug("Device ID mapped to %#08x\n", mapped_vendev);

#ifdef CONFIG_VGA_BIOS_ADDR
	rom_address = CONFIG_VGA_BIOS_ADDR;
#else

	pci_read_config_dword(dev, PCI_ROM_ADDRESS, &rom_address);
	if (rom_address == 0x00000000 || rom_address == 0xffffffff) {
		debug("%s: rom_address=%x\n", __func__, rom_address);
		return -ENOENT;
	}

	/* Enable expansion ROM address decoding. */
	pci_write_config_dword(dev, PCI_ROM_ADDRESS,
			       rom_address | PCI_ROM_ADDRESS_ENABLE);
#endif
	debug("Option ROM address %x\n", rom_address);
	rom_header = (struct pci_rom_header *)(unsigned long)rom_address;

	debug("PCI expansion ROM, signature %#04x, INIT size %#04x, data ptr %#04x\n",
	      le16_to_cpu(rom_header->signature),
	      rom_header->size * 512, le16_to_cpu(rom_header->data));

	if (le16_to_cpu(rom_header->signature) != PCI_ROM_HDR) {
		printf("Incorrect expansion ROM header signature %04x\n",
		       le16_to_cpu(rom_header->signature));
#ifndef CONFIG_VGA_BIOS_ADDR
		/* Disable expansion ROM address decoding */
		pci_write_config_dword(dev, PCI_ROM_ADDRESS, rom_address);
#endif
		return -EINVAL;
	}

	rom_data = (((void *)rom_header) + le16_to_cpu(rom_header->data));
	rom_vendor = le16_to_cpu(rom_data->vendor);
	rom_device = le16_to_cpu(rom_data->device);

	debug("PCI ROM image, vendor ID %04x, device ID %04x,\n",
	      rom_vendor, rom_device);

	/* If the device id is mapped, a mismatch is expected */
	if ((vendor != rom_vendor || device != rom_device) &&
	    (vendev == mapped_vendev)) {
		printf("ID mismatch: vendor ID %04x, device ID %04x\n",
		       rom_vendor, rom_device);
		/* Continue anyway */
	}

	rom_class = (le16_to_cpu(rom_data->class_hi) << 8) | rom_data->class_lo;
	debug("PCI ROM image, Class Code %06x, Code Type %02x\n",
	      rom_class, rom_data->type);

	if (class != rom_class) {
		debug("Class Code mismatch ROM %06x, dev %06x\n",
		      rom_class, class);
	}
	*hdrp = rom_header;

	return 0;
}

int pci_rom_load(struct pci_rom_header *rom_header,
		 struct pci_rom_header **ram_headerp)
{
	struct pci_rom_data *rom_data;
	unsigned int rom_size;
	unsigned int image_size = 0;
	void *target;

	do {
		/* Get next image, until we see an x86 version */
		rom_header = (struct pci_rom_header *)((void *)rom_header +
							    image_size);

		rom_data = (struct pci_rom_data *)((void *)rom_header +
				le16_to_cpu(rom_header->data));

		image_size = le16_to_cpu(rom_data->ilen) * 512;
	} while ((rom_data->type != 0) && (rom_data->indicator == 0));

	if (rom_data->type != 0)
		return -EACCES;

	rom_size = rom_header->size * 512;

#ifdef PCI_VGA_RAM_IMAGE_START
	target = (void *)PCI_VGA_RAM_IMAGE_START;
#else
	target = (void *)malloc(rom_size);
	if (!target)
		return -ENOMEM;
#endif
	if (target != rom_header) {
		ulong start = get_timer(0);

		debug("Copying VGA ROM Image from %p to %p, 0x%x bytes\n",
		      rom_header, target, rom_size);
		memcpy(target, rom_header, rom_size);
		if (memcmp(target, rom_header, rom_size)) {
			printf("VGA ROM copy failed\n");
			return -EFAULT;
		}
		debug("Copy took %lums\n", get_timer(start));
	}
	*ram_headerp = target;

	return 0;
}

struct vbe_mode_info mode_info;

int vbe_get_video_info(struct graphic_device *gdev)
{
#ifdef CONFIG_FRAMEBUFFER_SET_VESA_MODE
	struct vesa_mode_info *vesa = &mode_info.vesa;

	gdev->winSizeX = vesa->x_resolution;
	gdev->winSizeY = vesa->y_resolution;

	gdev->plnSizeX = vesa->x_resolution;
	gdev->plnSizeY = vesa->y_resolution;

	gdev->gdfBytesPP = vesa->bits_per_pixel / 8;

	switch (vesa->bits_per_pixel) {
	case 32:
	case 24:
		gdev->gdfIndex = GDF_32BIT_X888RGB;
		break;
	case 16:
		gdev->gdfIndex = GDF_16BIT_565RGB;
		break;
	default:
		gdev->gdfIndex = GDF__8BIT_INDEX;
		break;
	}

	gdev->isaBase = CONFIG_SYS_ISA_IO_BASE_ADDRESS;
	gdev->pciBase = vesa->phys_base_ptr;

	gdev->frameAdrs = vesa->phys_base_ptr;
	gdev->memSize = vesa->bytes_per_scanline * vesa->y_resolution;

	gdev->vprBase = vesa->phys_base_ptr;
	gdev->cprBase = vesa->phys_base_ptr;

	return gdev->winSizeX ? 0 : -ENOSYS;
#else
	return -ENOSYS;
#endif
}

void setup_video(struct screen_info *screen_info)
{
	struct vesa_mode_info *vesa = &mode_info.vesa;

	/* Sanity test on VESA parameters */
	if (!vesa->x_resolution || !vesa->y_resolution)
		return;

	screen_info->orig_video_isVGA = VIDEO_TYPE_VLFB;

	screen_info->lfb_width = vesa->x_resolution;
	screen_info->lfb_height = vesa->y_resolution;
	screen_info->lfb_depth = vesa->bits_per_pixel;
	screen_info->lfb_linelength = vesa->bytes_per_scanline;
	screen_info->lfb_base = vesa->phys_base_ptr;
	screen_info->lfb_size =
		ALIGN(screen_info->lfb_linelength * screen_info->lfb_height,
		      65536);
	screen_info->lfb_size >>= 16;
	screen_info->red_size = vesa->red_mask_size;
	screen_info->red_pos = vesa->red_mask_pos;
	screen_info->green_size = vesa->green_mask_size;
	screen_info->green_pos = vesa->green_mask_pos;
	screen_info->blue_size = vesa->blue_mask_size;
	screen_info->blue_pos = vesa->blue_mask_pos;
	screen_info->rsvd_size = vesa->reserved_mask_size;
	screen_info->rsvd_pos = vesa->reserved_mask_pos;
}

int pci_run_vga_bios(pci_dev_t dev, int (*int15_handler)(void), int exec_method)
{
	struct pci_rom_header *rom, *ram;
	int vesa_mode = -1;
	uint class;
	bool emulate;
	int ret;

	/* Only execute VGA ROMs */
	pci_read_config_dword(dev, PCI_REVISION_ID, &class);
	if (((class >> 16) ^ PCI_CLASS_DISPLAY_VGA) & 0xff00) {
		debug("%s: Class %#x, should be %#x\n", __func__, class,
		      PCI_CLASS_DISPLAY_VGA);
		return -ENODEV;
	}
	class >>= 8;

	if (!should_load_oprom(dev))
		return -ENXIO;

	ret = pci_rom_probe(dev, class, &rom);
	if (ret)
		return ret;

	ret = pci_rom_load(rom, &ram);
	if (ret)
		return ret;

	if (!board_should_run_oprom(dev))
		return -ENXIO;

#if defined(CONFIG_FRAMEBUFFER_SET_VESA_MODE) && \
		defined(CONFIG_FRAMEBUFFER_VESA_MODE)
	vesa_mode = CONFIG_FRAMEBUFFER_VESA_MODE;
#endif
	debug("Selected vesa mode %#x\n", vesa_mode);

	if (exec_method & PCI_ROM_USE_NATIVE) {
#ifdef CONFIG_X86
		emulate = false;
#else
		if (!(exec_method & PCI_ROM_ALLOW_FALLBACK)) {
			printf("BIOS native execution is only available on x86\n");
			return -ENOSYS;
		}
		emulate = true;
#endif
	} else {
#ifdef CONFIG_BIOSEMU
		emulate = true;
#else
		if (!(exec_method & PCI_ROM_ALLOW_FALLBACK)) {
			printf("BIOS emulation not available - see CONFIG_BIOSEMU\n");
			return -ENOSYS;
		}
		emulate = false;
#endif
	}

	if (emulate) {
#ifdef CONFIG_BIOSEMU
		BE_VGAInfo *info;

		ret = biosemu_setup(dev, &info);
		if (ret)
			return ret;
		biosemu_set_interrupt_handler(0x15, int15_handler);
		ret = biosemu_run(dev, (uchar *)ram, 1 << 16, info, true,
				  vesa_mode, &mode_info);
		if (ret)
			return ret;
#endif
	} else {
#ifdef CONFIG_X86
		bios_set_interrupt_handler(0x15, int15_handler);

		bios_run_on_x86(dev, (unsigned long)ram, vesa_mode,
				&mode_info);
#endif
	}
	debug("Final vesa mode %#x\n", mode_info.video_mode);

	return 0;
}
