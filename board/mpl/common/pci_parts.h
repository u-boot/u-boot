 /*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
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
 *
 */
#ifndef _PCI_PARTS_H_
#define _PCI_PARTS_H_


/* Board specific file containing:
 * - PCI Memory Mapping
 * - PCI IO Mapping
 * - PCI Interrupt Mapping
 */

/* PIP405 PCI INT Routing:
 *                      IRQ0  VECTOR
 * PIXX4 IDSEL  = AD16  INTA#  28 (Function 2 USB is INTD# = 31)
 * VGA   IDSEL  = AD17  INTB#  29
 * SCSI  IDSEL  = AD18  INTC#  30
 * PC104 IDSEL0 = AD20  INTA#  28
 * PC104 IDSEL1 = AD21  INTB#  29
 * PC104 IDSEL2 = AD22  INTC#  30
 * PC104 IDSEL3 = AD23  INTD#  31
 *
 * busdevfunc = EXXX XXXX BBBB BBBB DDDD DFFF RRRR RR00
 *              ^         ^         ^     ^   ^
 *             31        23        15    10   7
 * E = Enabled
 * B = Bussnumber
 * D = Devicenumber (Device0 = AD10)
 * F = Functionnumber
 * R = Registernumber
 *
 * Device = (busdevfunc>>11) + 10
 * Vector = devicenumber % 4 + 28
 *
 */
#define PCI_HIGHEST_ON_BOARD_ID	19
/*#define PCI_DEV_NUMBER(x)	(((x>>11) & 0x1f) + 10) */
#define PCI_IRQ_VECTOR(x)	((PCI_DEV(x) + 10) % 4) + 28


/* PCI Device List for PIP405 */

/* Mapping:
 * +-------------+------------+------------+--------------------------------+
 * | PCI MemAddr | PCI IOAddr | Local Addr | Device / Function              |
 * +-------------+------------+------------+--------------------------------+
 * |  0x00000000 |            | 0xA0000000 | ISA Memory (hard wired)        |
 * |  0x00FFFFFF |            | 0xA0FFFFFF |                                |
 * +-------------+------------+------------+--------------------------------+
 * |             | 0x00000000 | 0xE8000000 | ISA IO (hard wired)            |
 * |             | 0x0000FFFF | 0xE800FFFF |                                |
 * +-------------+------------+------------+--------------------------------+
 * |  0x80000000 |            | 0x80000000 | VGA Controller Memory          |
 * |  0x80FFFFFF |            | 0x80FFFFFF |                                |
 * +-------------+------------+------------+--------------------------------+
 * |  0x81000000 |            | 0x81000000 | SCSI Controller Memory         |
 * |  0x81FFFFFF |            | 0x81FFFFFF |                                |
 * +-------------+------------+------------+--------------------------------+
 */

struct pci_pip405_config_entry {
	int		index;	/* address */
	unsigned long	val;	/* value */
	int		width;	/* data size */
};

extern void pci_pip405_write_regs(struct pci_controller *,
				  pci_dev_t,
				  struct pci_config_table *);

/* PIIX4 ISA Bridge Function 0 */
static struct pci_pip405_config_entry piix4_isa_bridge_f0[] = {
	{PCI_CFG_PIIX4_SERIRQ,	0xD0,		1}, /* enable Continous SERIRQ Pin */
	{PCI_CFG_PIIX4_GENCFG,	0x00018041,	4}, /* enable SERIRQs, ISA, PNP, GPI11 */
	{PCI_CFG_PIIX4_TOM,	0xFE,		1}, /* Top of Memory		*/
	{PCI_CFG_PIIX4_XBCS,	0x02C4,		2}, /* disable all peri CS	*/
	{PCI_CFG_PIIX4_RTCCFG,	0x21,		1}, /* enable RTC		*/
#if defined(CONFIG_PIP405)
	{PCI_CFG_PIIX4_MBDMA,	0x82,		1}, /* set MBDMA0 to DMA 2	*/
	{PCI_CFG_PIIX4_MBDMA+1,	0x83,		1}, /* set MBDMA1 to DMA 3	*/
#endif
	{PCI_CFG_PIIX4_DLC,	0x0,		1}, /* disable passive release feature */
	{ }					    /* end of device table	*/
};

/* PIIX4 IDE Controller Function 1 */
static struct pci_pip405_config_entry piix4_ide_cntrl_f1[] = {
	{PCI_CFG_PIIX4_BMIBA,	0x0001000,	4}, /* set BMI to a valid address */
	{PCI_COMMAND,		0x0001,		2}, /* enable IO access	*/
#if !defined(CONFIG_MIP405T)
	{PCI_CFG_PIIX4_IDETIM,	0x80008000,	4}, /* enable Both IDE channels	*/
#else
	{PCI_CFG_PIIX4_IDETIM,	0x00008000,	4}, /* enable IDE channel0	*/
#endif
	{ }					    /* end of device table	*/
};

/* PIIX4 USB Controller Function 2 */
static struct pci_pip405_config_entry piix4_usb_cntrl_f2[] = {
#if !defined(CONFIG_MIP405T)
	{PCI_INTERRUPT_LINE,	31,		1}, /* Int vector = 31		*/
	{PCI_BASE_ADDRESS_4,	0x0000E001,	4}, /* Set IO Address to 0xe000 to 0xe01F */
	{PCI_LATENCY_TIMER,	0x80,		1}, /* Latency Timer 0x80	*/
	{0xC0,			0x2000,		2}, /* Legacy support		*/
	{PCI_COMMAND,		0x0005,		2}, /* enable IO access and Master */
#endif
	{ }					    /* end of device table	*/
};

/* PIIX4 Power Management Function 3 */
static struct pci_pip405_config_entry piix4_pmm_cntrl_f3[] = {
	{PCI_CFG_PIIX4_PMBA,	0x00004000,	4}, /* set PMBA to "valid" value */
	{PCI_CFG_PIIX4_SMBBA,	0x00005000,	4}, /* set SMBBA to "valid" value */
	{PCI_CFG_PIIX4_PMMISC,	0x01,		1}, /* enable PMBA IO access	*/
	{PCI_COMMAND,		0x0001,		2}, /* enable IO access	*/
	{ }					    /* end of device table	*/
};
/* PPC405 Dummy only used to prevent autosetup on this host bridge */
static struct pci_pip405_config_entry ppc405_dummy[] = {
	{ }					    /* end of device table	*/
};

void pci_405gp_setup_vga(struct pci_controller *hose, pci_dev_t dev,
			 struct pci_config_table *entry);


static struct pci_config_table pci_pip405_config_table[]={
	{PCI_VENDOR_ID_IBM,			/* 405 dummy */
	 PCI_DEVICE_ID_IBM_405GP,
	 PCI_ANY_ID,
	 PCI_ANY_ID, PCI_ANY_ID, 0,
	 pci_pip405_write_regs, {(unsigned long) ppc405_dummy}},

	{PCI_VENDOR_ID_INTEL,			/* PIIX4 ISA Bridge Function 0 */
	 PCI_DEVICE_ID_INTEL_82371AB_0,
	 PCI_ANY_ID,
	 PCI_ANY_ID, PCI_ANY_ID, 0,
	 pci_pip405_write_regs, {(unsigned long) piix4_isa_bridge_f0}},

	{PCI_VENDOR_ID_INTEL,			/* PIIX4 IDE Controller Function 1 */
	 PCI_DEVICE_ID_INTEL_82371AB,
	 PCI_ANY_ID,
	 PCI_ANY_ID, PCI_ANY_ID, 1,
	 pci_pip405_write_regs, {(unsigned long) piix4_ide_cntrl_f1}},

	{PCI_VENDOR_ID_INTEL,			/* PIIX4 USB Controller Function 2 */
	 PCI_DEVICE_ID_INTEL_82371AB_2,
	 PCI_ANY_ID,
	 PCI_ANY_ID, PCI_ANY_ID, 2,
	 pci_pip405_write_regs, {(unsigned long) piix4_usb_cntrl_f2}},

	{PCI_VENDOR_ID_INTEL,			/* PIIX4 USB Controller Function 3 */
	 PCI_DEVICE_ID_INTEL_82371AB_3,
	 PCI_ANY_ID,
	 PCI_ANY_ID, PCI_ANY_ID, 3,
	 pci_pip405_write_regs, {(unsigned long) piix4_pmm_cntrl_f3}},

	{PCI_ANY_ID,
	 PCI_ANY_ID,
	 PCI_CLASS_DISPLAY_VGA,
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 pci_405gp_setup_vga},

	{PCI_ANY_ID,
	 PCI_ANY_ID,
	 PCI_CLASS_NOT_DEFINED_VGA,
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 pci_405gp_setup_vga},

	{ }
};
#endif /* _PCI_PARTS_H_ */
