/*
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
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
 * Initialisation of the PCI-to-ISA bridge and disabling the BIOS
 * write protection (for flash) in function 0 of the chip.
 * Enabling function 1 (IDE controller of the chip.
 */

#include <common.h>
#include <config.h>

#include <asm/io.h>
#include <pci.h>

#include <w83c553f.h>

#define out8(addr,val)	do { \
			out_8((u8*) (addr),(val)); udelay(1); \
			} while (0)
#define out16(addr,val)	do { \
			out_be16((u16*) (addr),(val)); udelay(1); \
			} while (0)

extern uint ide_bus_offset[CONFIG_SYS_IDE_MAXBUS];

void initialise_pic(void);
void initialise_dma(void);

void initialise_w83c553f(void)
{
	pci_dev_t devbusfn;
	unsigned char reg8;
	unsigned short reg16;
	unsigned int reg32;

	devbusfn = pci_find_device(W83C553F_VID, W83C553F_DID, 0);
	if (devbusfn == -1)
	{
		printf("Error: Cannot find W83C553F controller on any PCI bus.");
		return;
	}

	pci_read_config_word(devbusfn, PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
	pci_write_config_word(devbusfn, PCI_COMMAND, reg16);

	pci_read_config_byte(devbusfn, WINBOND_IPADCR, &reg8);
	/* 16 MB ISA memory space */
	reg8 |= (IPADCR_IPATOM4 | IPADCR_IPATOM5 | IPADCR_IPATOM6 | IPADCR_IPATOM7);
	reg8 &= ~IPADCR_MBE512;
	pci_write_config_byte(devbusfn, WINBOND_IPADCR, reg8);

	pci_read_config_byte(devbusfn, WINBOND_CSCR, &reg8);
	/* switch off BIOS write protection */
	reg8 |= CSCR_UBIOSCSE;
	reg8 &= ~CSCR_BIOSWP;
	pci_write_config_byte(devbusfn, WINBOND_CSCR, reg8);

	/*
	 * Interrupt routing:
	 *  - IDE  -> IRQ 9/0
	 *  - INTA -> IRQ 10
	 *  - INTB -> IRQ 11
	 *  - INTC -> IRQ 14
	 *  - INTD -> IRQ 15
	 */
	pci_write_config_byte(devbusfn, WINBOND_IDEIRCR, 0x90);
	pci_write_config_word(devbusfn, WINBOND_PCIIRCR, 0xABEF);

	/*
	 * Read IDE bus offsets from function 1 device.
	 * We must unmask the LSB indicating that ist is an IO address.
	 */
	devbusfn |= PCI_BDF(0,0,1);

	/*
	 * Switch off legacy IRQ for IDE and IDE port 1.
	 */
	pci_write_config_byte(devbusfn, 0x09, 0x8F);

	pci_read_config_dword(devbusfn, WINDOND_IDECSR, &reg32);
	reg32 &= ~(IDECSR_LEGIRQ | IDECSR_P1EN | IDECSR_P1F16);
	pci_write_config_dword(devbusfn, WINDOND_IDECSR, reg32);

	pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_0, &ide_bus_offset[0]);
	ide_bus_offset[0] &= ~1;
#if CONFIG_SYS_IDE_MAXBUS > 1
	pci_read_config_dword(devbusfn, PCI_BASE_ADDRESS_2, &ide_bus_offset[1]);
	ide_bus_offset[1] &= ~1;
#endif

	/*
	 * Enable function 1, IDE -> busmastering and IO space access
	 */
	pci_read_config_word(devbusfn, PCI_COMMAND, &reg16);
	reg16 |= PCI_COMMAND_MASTER | PCI_COMMAND_IO;
	pci_write_config_word(devbusfn, PCI_COMMAND, reg16);

	/*
	 * Initialise ISA interrupt controller
	 */
	initialise_pic();

	/*
	 * Initialise DMA controller
	 */
	initialise_dma();
}

void initialise_pic(void)
{
	out8(W83C553F_PIC1_ICW1, 0x11);
	out8(W83C553F_PIC1_ICW2, 0x08);
	out8(W83C553F_PIC1_ICW3, 0x04);
	out8(W83C553F_PIC1_ICW4, 0x01);
	out8(W83C553F_PIC1_OCW1, 0xfb);
	out8(W83C553F_PIC1_ELC, 0x20);

	out8(W83C553F_PIC2_ICW1, 0x11);
	out8(W83C553F_PIC2_ICW2, 0x08);
	out8(W83C553F_PIC2_ICW3, 0x02);
	out8(W83C553F_PIC2_ICW4, 0x01);
	out8(W83C553F_PIC2_OCW1, 0xff);
	out8(W83C553F_PIC2_ELC, 0xce);

	out8(W83C553F_TMR1_CMOD, 0x74);

	out8(W83C553F_PIC2_OCW1, 0x20);
	out8(W83C553F_PIC1_OCW1, 0x20);

	out8(W83C553F_PIC2_OCW1, 0x2b);
	out8(W83C553F_PIC1_OCW1, 0x2b);
}

void initialise_dma(void)
{
	unsigned int channel;
	unsigned int rvalue1, rvalue2;

	/* perform a H/W reset of the devices */

	out8(W83C553F_DMA1 + W83C553F_DMA1_MC, 0x00);
	out16(W83C553F_DMA2 + W83C553F_DMA2_MC, 0x0000);

	/* initialise all channels to a sane state */

	for (channel = 0; channel < 4; channel++) {
		/*
		 * dependent upon the channel, setup the specifics:
		 *
		 * demand
		 * address-increment
		 * autoinitialize-disable
		 * verify-transfer
		 */

		switch (channel) {
		case 0:
			rvalue1 = (W83C553F_MODE_TM_DEMAND|W83C553F_MODE_CH0SEL|W83C553F_MODE_TT_VERIFY);
			rvalue2 = (W83C553F_MODE_TM_CASCADE|W83C553F_MODE_CH0SEL);
			break;
		case 1:
			rvalue1 = (W83C553F_MODE_TM_DEMAND|W83C553F_MODE_CH1SEL|W83C553F_MODE_TT_VERIFY);
			rvalue2 = (W83C553F_MODE_TM_DEMAND|W83C553F_MODE_CH1SEL|W83C553F_MODE_TT_VERIFY);
			break;
		case 2:
			rvalue1 = (W83C553F_MODE_TM_DEMAND|W83C553F_MODE_CH2SEL|W83C553F_MODE_TT_VERIFY);
			rvalue2 = (W83C553F_MODE_TM_DEMAND|W83C553F_MODE_CH2SEL|W83C553F_MODE_TT_VERIFY);
			break;
		case 3:
			rvalue1 = (W83C553F_MODE_TM_DEMAND|W83C553F_MODE_CH3SEL|W83C553F_MODE_TT_VERIFY);
			rvalue2 = (W83C553F_MODE_TM_DEMAND|W83C553F_MODE_CH3SEL|W83C553F_MODE_TT_VERIFY);
			break;
		default:
			rvalue1 = 0x00;
			rvalue2 = 0x00;
			break;
		}

		/* write to write mode registers */

		out8(W83C553F_DMA1 + W83C553F_DMA1_WM, rvalue1 & 0xFF);
		out16(W83C553F_DMA2 + W83C553F_DMA2_WM, rvalue2 & 0x00FF);
	}

	/* enable all channels */

	out8(W83C553F_DMA1 + W83C553F_DMA1_CM, 0x00);
	out16(W83C553F_DMA2 + W83C553F_DMA2_CM, 0x0000);
	/*
	 * initialize the global DMA configuration
	 *
	 * DACK# active low
	 * DREQ active high
	 * fixed priority
	 * channel group enable
	 */

	out8(W83C553F_DMA1 + W83C553F_DMA1_CS, 0x00);
	out16(W83C553F_DMA2 + W83C553F_DMA2_CS, 0x0000);
}
