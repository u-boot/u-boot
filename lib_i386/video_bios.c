/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
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
#include <pci.h>
#include <malloc.h>
#include <asm/ptrace.h>
#include <asm/realmode.h>
#include <asm/io.h>
#include <asm/pci.h>

#undef PCI_BIOS_DEBUG
#undef VGA_BIOS_DEBUG

#ifdef	VGA_BIOS_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#ifdef CONFIG_PCI

#ifdef PCI_BIOS_DEBUG
#define RELOC_16(seg, off) *(u32*)(seg << 4 | (u32)&off)
extern u32 num_pci_bios_present;
extern u32 num_pci_bios_find_device;
extern u32 num_pci_bios_find_class;
extern u32 num_pci_bios_generate_special_cycle;
extern u32 num_pci_bios_read_cfg_byte;
extern u32 num_pci_bios_read_cfg_word;
extern u32 num_pci_bios_read_cfg_dword;
extern u32 num_pci_bios_write_cfg_byte;
extern u32 num_pci_bios_write_cfg_word;
extern u32 num_pci_bios_write_cfg_dword;
extern u32 num_pci_bios_get_irq_routing;
extern u32 num_pci_bios_set_irq;
extern u32 num_pci_bios_unknown_function;

void print_bios_bios_stat(void)
{
	printf("16 bit functions:\n");
	printf("pci_bios_present:                %d\n", RELOC_16(0xf000, num_pci_bios_present));
	printf("pci_bios_find_device:            %d\n", RELOC_16(0xf000, num_pci_bios_find_device));
	printf("pci_bios_find_class:             %d\n", RELOC_16(0xf000, num_pci_bios_find_class));
	printf("pci_bios_generate_special_cycle: %d\n", RELOC_16(0xf000, num_pci_bios_generate_special_cycle));
	printf("pci_bios_read_cfg_byte:          %d\n", RELOC_16(0xf000, num_pci_bios_read_cfg_byte));
	printf("pci_bios_read_cfg_word:          %d\n", RELOC_16(0xf000, num_pci_bios_read_cfg_word));
	printf("pci_bios_read_cfg_dword:         %d\n", RELOC_16(0xf000, num_pci_bios_read_cfg_dword));
	printf("pci_bios_write_cfg_byte:         %d\n", RELOC_16(0xf000, num_pci_bios_write_cfg_byte));
	printf("pci_bios_write_cfg_word:         %d\n", RELOC_16(0xf000, num_pci_bios_write_cfg_word));
	printf("pci_bios_write_cfg_dword:        %d\n", RELOC_16(0xf000, num_pci_bios_write_cfg_dword));
	printf("pci_bios_get_irq_routing:        %d\n", RELOC_16(0xf000, num_pci_bios_get_irq_routing));
	printf("pci_bios_set_irq:                %d\n", RELOC_16(0xf000, num_pci_bios_set_irq));
	printf("pci_bios_unknown_function:       %d\n", RELOC_16(0xf000, num_pci_bios_unknown_function));

}
#endif

#define PCI_CLASS_VIDEO             3
#define PCI_CLASS_VIDEO_STD         0
#define PCI_CLASS_VIDEO_PROG_IF_VGA 0


static u32 probe_pci_video(void)
{
	pci_dev_t devbusfn;

	if ((devbusfn = pci_find_class(PCI_CLASS_VIDEO,
				       PCI_CLASS_VIDEO_STD,
				       PCI_CLASS_VIDEO_PROG_IF_VGA, 0)) != -1) {
		u32 old;
		u32 addr;

		/* PCI video device detected */
		printf("Found PCI VGA device at %02x.%02x.%x\n",
		       PCI_BUS(devbusfn), PCI_DEV(devbusfn), PCI_FUNC(devbusfn));

		/* Enable I/O decoding as well, PCI viudeo boards
		 * support I/O accesses, but they provide no
		 * bar register for this since the ports are fixed.
		 */
		pci_write_config_word(devbusfn, PCI_COMMAND, PCI_COMMAND_MEMORY | PCI_COMMAND_IO | PCI_COMMAND_MASTER);

		/* Test the ROM decoder, do the device support a rom? */
		pci_read_config_dword(devbusfn, PCI_ROM_ADDRESS, &old);
		pci_write_config_dword(devbusfn, PCI_ROM_ADDRESS, PCI_ROM_ADDRESS_MASK);
		pci_read_config_dword(devbusfn, PCI_ROM_ADDRESS, &addr);
		pci_write_config_dword(devbusfn, PCI_ROM_ADDRESS, old);

		if (!addr) {
			printf("PCI VGA have no ROM?\n");
			return 0;
		}

		/* device have a rom */
		if (pci_shadow_rom(devbusfn, (void*)0xc0000)) {
			printf("Shadowing of PCI VGA BIOS failed\n");
			return 0;
		}

		/* Now enable lagacy VGA port access */
		if (pci_enable_legacy_video_ports(pci_bus_to_hose(PCI_BUS(devbusfn)))) {
			printf("PCI VGA enable failed\n");
			return 0;
		}


		/* return the pci device info, that we'll need later */
		return PCI_BUS(devbusfn) << 8 |
			PCI_DEV(devbusfn) << 3 | (PCI_FUNC(devbusfn)&7);
	}

	return 0;
}


#endif

static int probe_isa_video(void)
{
	u32 ptr;
	char *buf;

	if (0 == (ptr = isa_map_rom(0xc0000, 0x8000))) {
		return -1;
	}
	if (NULL == (buf=malloc(0x8000))) {
		isa_unmap_rom(ptr);
		return -1;
	}
	if (readw(ptr) != 0xaa55) {
		free(buf);
		isa_unmap_rom(ptr);
		return -1;
	}

	/* shadow the rom */
	memcpy(buf, (void*)ptr, 0x8000);
	isa_unmap_rom(ptr);
	memcpy((void*)0xc0000, buf, 0x8000);

	free(buf);

	return 0;
}

int video_bios_init(void)
{
	struct pt_regs regs;

	/* clear the video bios area in case we warmbooted */
	memset((void*)0xc0000, 0, 0x8000);
	memset(&regs, 0, sizeof(struct pt_regs));

	if (probe_isa_video()) {
		/* No ISA board found, try the PCI bus */
		regs.eax = probe_pci_video();
	}

	/* Did we succeed in mapping any video bios */
	if (readw(0xc0000) == 0xaa55) {
	        int size;
		int i;
		u8 sum;

		PRINTF("Found video bios signature\n");
		size = 512*readb(0xc0002);
		PRINTF("size %d\n", size);
		sum=0;
		for (i=0;i<size;i++) {
			sum += readb(0xc0000 + i);
		}
		PRINTF("Checksum is %sOK\n",sum?"NOT ":"");
		if (sum) {
			return 1;
		}

		/* some video bioses (ATI Mach64) seem to think that
		 * the original int 10 handler is always at
		 * 0xf000:0xf065 , place an iret instruction there
		 */
		writeb(0xcf, 0xff065);

		regs.esp = 0x8000;
		regs.xss = 0x2000;
		enter_realmode(0xc000, 3, &regs, &regs);
		PRINTF("INT 0x10 vector after:  %04x:%04x\n",
		       readw(0x42), readw(0x40));
		PRINTF("BIOS returned %scarry\n", regs.eflags & 1?"":"NOT ");
#ifdef PCI_BIOS_DEBUG
		print_bios_bios_stat();
#endif
		return (regs.eflags & 1);

	}

	return 1;

}
