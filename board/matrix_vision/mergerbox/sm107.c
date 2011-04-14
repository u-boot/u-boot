/*
 * Copyright (C) 2011 Matrix Vision GmbH
 * Andre Schwarz <andre.schwarz@matrix-vision.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>
#include <asm/io.h>
#include <ns16550.h>
#include <netdev.h>
#include <sm501.h>
#include <pci.h>
#include "../common/mv_common.h"

#ifdef CONFIG_VIDEO
static const SMI_REGS init_regs_800x480[] = {
	/* set endianess to little endian */
	{0x0005c, 0x00000000},
	/* PCI drive 12mA */
	{0x00004, 0x42401001},
	/* current clock */
	{0x0003c, 0x310a1818},
	/* clocks for pm0... */
	{0x00040, 0x0002184f},
	{0x00044, 0x2a1a0a01},
	/* GPIO */
	{0x10008, 0x00000000},
	{0x1000C, 0x00000000},
	/* panel control regs */
	{0x80000, 0x0f017106},
	{0x80004, 0x0},
	{0x80008, 0x0},
	{0x8000C, 0x00000000},
	{0x80010, 0x0c800c80},
	/* width 0x320 */
	{0x80014, 0x03200000},
	/* height 0x1e0 */
	{0x80018, 0x01E00000},
	{0x8001C, 0x0},
	{0x80020, 0x01df031f},
	{0x80024, 0x041f031f},
	{0x80028, 0x00800347},
	{0x8002C, 0x020c01df},
	{0x80030, 0x000201e9},
	{0x80200, 0x00000000},
	/* ZV[0:7] */
	{0x00008, 0x00ff0000},
	/* 24-Bit TFT */
	{0x0000c, 0x3f000000},
	{0, 0}
};

/*
 * Returns SM107 register base address. First thing called in the driver.
 */
unsigned int board_video_init(void)
{
	pci_dev_t devbusfn;
	u32 addr;

	devbusfn = pci_find_device(PCI_VENDOR_SM, PCI_DEVICE_SM501, 0);
	if (devbusfn != -1) {
		pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_1,
			(u32 *)&addr);
		return addr & 0xfffffffe;
	}

	return 0;
}

/*
 * Called after initializing the SM501 and before clearing the screen.
 */
void board_validate_screen(unsigned int base)
{
}

/*
 * Returns SM107 framebuffer address
 */
unsigned int board_video_get_fb(void)
{
	pci_dev_t devbusfn;
	u32 addr;

	devbusfn = pci_find_device(PCI_VENDOR_SM, PCI_DEVICE_SM501, 0);
	if (devbusfn != -1) {
		pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_0,
			(u32 *)&addr);
		addr &= 0xfffffffe;
#ifdef CONFIG_VIDEO_SM501_FBMEM_OFFSET
		addr += CONFIG_VIDEO_SM501_FBMEM_OFFSET;
#endif
		return addr;
	}

	printf("board_video_get_fb(): FAILED\n");

	return 0;
}

/*
 * Return a pointer to the initialization sequence.
 */
const SMI_REGS *board_get_regs(void)
{
	return init_regs_800x480;
}

int board_get_width(void)
{
	return 800;
}

int board_get_height(void)
{
	return 480;
}
#endif
