/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 *
 * (C) Copyright 2005
 * Martin Krause TQ-Systems GmbH martin.krause@tqs.de
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
 * Basic video support for SMI SM501 "Voyager" graphic controller
 */

#include <common.h>

#include <asm/io.h>
#include <video_fb.h>
#include <sm501.h>

#define read8(ptrReg)                \
    *(volatile unsigned char *)(sm501.isaBase + ptrReg)

#define write8(ptrReg,value) \
    *(volatile unsigned char *)(sm501.isaBase + ptrReg) = value

#define read16(ptrReg) \
    (*(volatile unsigned short *)(sm501.isaBase + ptrReg))

#define write16(ptrReg,value) \
    (*(volatile unsigned short *)(sm501.isaBase + ptrReg) = value)

#define read32(ptrReg) \
    (*(volatile unsigned int *)(sm501.isaBase + ptrReg))

#define write32(ptrReg, value) \
    (*(volatile unsigned int *)(sm501.isaBase + ptrReg) = value)

GraphicDevice sm501;

void write_be32(int off, unsigned int val)
{
	out_be32((unsigned __iomem *)(sm501.isaBase + off), val);
}

void write_le32(int off, unsigned int val)
{
	out_le32((unsigned __iomem *)(sm501.isaBase + off), val);
}

void (*write_reg32)(int off, unsigned int val) = write_be32;

/*-----------------------------------------------------------------------------
 * SmiSetRegs --
 *-----------------------------------------------------------------------------
 */
static void SmiSetRegs (void)
{
	/*
	 * The content of the chipset register depends on the board (clocks,
	 * ...)
	 */
	const SMI_REGS *preg = board_get_regs ();
	while (preg->Index) {
		write_reg32 (preg->Index, preg->Value);
		/*
		 * Insert a delay between
		 */
		udelay (1000);
		preg ++;
	}
}

#ifdef CONFIG_VIDEO_SM501_PCI
static struct pci_device_id sm501_pci_tbl[] = {
	{ PCI_VENDOR_ID_SMI, PCI_DEVICE_ID_SMI_501 },
	{}
};
#endif

/*
 * We do not enforce board code to provide empty/unused
 * functions for this driver and define weak default
 * functions here.
 */
unsigned int __board_video_init (void)
{
	return 0;
}

unsigned int board_video_init (void)
			__attribute__((weak, alias("__board_video_init")));

unsigned int __board_video_get_fb (void)
{
	return 0;
}

unsigned int board_video_get_fb (void)
			__attribute__((weak, alias("__board_video_get_fb")));

void __board_validate_screen (unsigned int base)
{
}

void board_validate_screen (unsigned int base)
			__attribute__((weak, alias("__board_validate_screen")));

/*-----------------------------------------------------------------------------
 * video_hw_init --
 *-----------------------------------------------------------------------------
 */
void *video_hw_init (void)
{
#ifdef CONFIG_VIDEO_SM501_PCI
	unsigned int pci_mem_base, pci_mmio_base;
	unsigned int id;
	unsigned short device_id;
	pci_dev_t devbusfn;
	int mem;
#endif
	unsigned int *vm, i;

	memset (&sm501, 0, sizeof (GraphicDevice));

#ifdef CONFIG_VIDEO_SM501_PCI
	printf("Video: ");

	/* Look for SM501/SM502 chips */
	devbusfn = pci_find_devices(sm501_pci_tbl, 0);
	if (devbusfn < 0) {
		printf ("PCI Controller not found.\n");
		goto not_pci;
	}

	/* Setup */
	pci_write_config_dword (devbusfn, PCI_COMMAND,
				(PCI_COMMAND_MEMORY | PCI_COMMAND_IO));
	pci_read_config_word (devbusfn, PCI_DEVICE_ID, &device_id);
	pci_read_config_dword (devbusfn, PCI_REVISION_ID, &id);
	pci_read_config_dword (devbusfn, PCI_BASE_ADDRESS_0, &pci_mem_base);
	pci_read_config_dword (devbusfn, PCI_BASE_ADDRESS_1, &pci_mmio_base);
	sm501.frameAdrs = pci_mem_to_phys (devbusfn, pci_mem_base);
	sm501.isaBase = pci_mem_to_phys (devbusfn, pci_mmio_base);

	if (sm501.isaBase)
		write_reg32 = write_le32;

	mem = in_le32 ((unsigned __iomem *)(sm501.isaBase + 0x10));
	mem = (mem & 0x0000e000) >> 13;
	switch (mem) {
	case 1:
		mem = 8;
		break;
	case 2:
		mem = 16;
		break;
	case 3:
		mem = 32;
		break;
	case 4:
		mem = 64;
		break;
	case 5:
		mem = 2;
		break;
	case 0:
	default:
		mem = 4;
	}
	printf ("PCI SM50%d %d MB\n", ((id & 0xff) == 0xC0) ? 2 : 1, mem);
not_pci:
#endif
	/*
	 * Initialization of the access to the graphic chipset Retreive base
	 * address of the chipset (see board/RPXClassic/eccx.c)
	 */
	if (!sm501.isaBase) {
		sm501.isaBase = board_video_init ();
		if (!sm501.isaBase)
			return NULL;
	}

	if (!sm501.frameAdrs) {
		sm501.frameAdrs = board_video_get_fb ();
		if (!sm501.frameAdrs)
			return NULL;
	}

	sm501.winSizeX = board_get_width ();
	sm501.winSizeY = board_get_height ();

#if defined(CONFIG_VIDEO_SM501_8BPP)
	sm501.gdfIndex = GDF__8BIT_INDEX;
	sm501.gdfBytesPP = 1;

#elif defined(CONFIG_VIDEO_SM501_16BPP)
	sm501.gdfIndex = GDF_16BIT_565RGB;
	sm501.gdfBytesPP = 2;

#elif defined(CONFIG_VIDEO_SM501_32BPP)
	sm501.gdfIndex = GDF_32BIT_X888RGB;
	sm501.gdfBytesPP = 4;
#else
#error Unsupported SM501 BPP
#endif

	sm501.memSize = sm501.winSizeX * sm501.winSizeY * sm501.gdfBytesPP;

	/* Load Smi registers */
	SmiSetRegs ();

	/* (see board/RPXClassic/RPXClassic.c) */
	board_validate_screen (sm501.isaBase);

	/* Clear video memory */
	i = sm501.memSize/4;
	vm = (unsigned int *)sm501.frameAdrs;
	while(i--)
		*vm++ = 0;

	return (&sm501);
}
