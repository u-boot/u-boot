/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#ifdef CONFIG_PCI

#include <pci.h>
#include <asm/m8260_pci.h>

/*
 *   Local->PCI map (from CPU)                             controlled by
 *   MPC826x master window
 *
 *   0x80000000 - 0xBFFFFFFF    Total CPU2PCI space        PCIBR0
 *                       
 *   0x80000000 - 0x8FFFFFFF    PCI Mem with prefetch      (Outbound ATU #1)
 *   0x90000000 - 0x9FFFFFFF    PCI Mem w/o  prefetch      (Outbound ATU #2)
 *   0xA0000000 - 0xAFFFFFFF    32-bit PCI IO              (Outbound ATU #3)
 *                      
 *   PCI->Local map (from PCI)
 *   MPC826x slave window                                  controlled by
 *
 *   0x00000000 - 0x07FFFFFF    MPC826x local memory       (Inbound ATU #1)
 */

/* 
 * Slave window that allows PCI masters to access MPC826x local memory. 
 * This window is set up using the first set of Inbound ATU registers
 */

#define PCI_SLV_MEM_LOCAL	CFG_SDRAM_BASE		/* Local base */
#define PCI_SLV_MEM_BUS		0x00000000		/* PCI base */
#define PICMR0_MASK_ATTRIB	(PICMR_MASK_512MB | PICMR_ENABLE | \
                          	 PICMR_PREFETCH_EN)

/* 
 * This is the window that allows the CPU to access PCI address space.
 * It will be setup with the SIU PCIBR0 register. All three PCI master
 * windows, which allow the CPU to access PCI prefetch, non prefetch,
 * and IO space (see below), must all fit within this window. 
 */

#define PCI_MSTR_LOCAL		0x80000000		/* Local base */
#define PCIMSK0_MASK		PCIMSK_1GB		/* Size of window */

/* 
 * Master window that allows the CPU to access PCI Memory (prefetch).
 * This window will be setup with the first set of Outbound ATU registers
 * in the bridge.
 */

#define PCI_MSTR_MEM_LOCAL	0x80000000          /* Local base */
#define PCI_MSTR_MEM_BUS	0x80000000          /* PCI base   */
#define CPU_PCI_MEM_START	PCI_MSTR_MEM_LOCAL
#define PCI_MSTR_MEM_SIZE	0x10000000          /* 256MB */
#define POCMR0_MASK_ATTRIB	(POCMR_MASK_256MB | POCMR_ENABLE | POCMR_PREFETCH_EN)

/* 
 * Master window that allows the CPU to access PCI Memory (non-prefetch).
 * This window will be setup with the second set of Outbound ATU registers
 * in the bridge.
 */

#define PCI_MSTR_MEMIO_LOCAL    0x90000000          /* Local base */
#define PCI_MSTR_MEMIO_BUS      0x90000000          /* PCI base   */
#define CPU_PCI_MEMIO_START     PCI_MSTR_MEMIO_LOCAL
#define PCI_MSTR_MEMIO_SIZE     0x10000000          /* 256MB */
#define POCMR1_MASK_ATTRIB      (POCMR_MASK_256MB | POCMR_ENABLE)

/* 
 * Master window that allows the CPU to access PCI IO space.
 * This window will be setup with the third set of Outbound ATU registers
 * in the bridge.
 */

#define PCI_MSTR_IO_LOCAL       0xA0000000          /* Local base */
#ifdef CONFIG_ATC
#define PCI_MSTR_IO_BUS         0x00000000          /* PCI base   */
#else
#define PCI_MSTR_IO_BUS         0xA0000000          /* PCI base   */
#endif
#define CPU_PCI_IO_START        PCI_MSTR_IO_LOCAL
#define PCI_MSTR_IO_SIZE        0x10000000          /* 256MB */
#define POCMR2_MASK_ATTRIB      (POCMR_MASK_256MB | POCMR_ENABLE | POCMR_PCI_IO)

/* PCI bus configuration registers.
 */

#define PCI_CLASS_BRIDGE_CTLR	0x06


static inline void  pci_outl(u32 addr, u32 data)
{
    *(volatile u32 *) addr = cpu_to_le32(data);
}

void pci_mpc8250_init(struct pci_controller *hose)
{
    u16 tempShort;
    u32 immr_addr = CFG_IMMR;
    volatile immap_t *immap = (immap_t *) CFG_IMMR;
    pci_dev_t host_devno = PCI_BDF(0, 0, 0);

    pci_setup_indirect(hose, CFG_IMMR + PCI_CFG_ADDR_REG,
	                         CFG_IMMR + PCI_CFG_DATA_REG);

    /* 
     * Setting required to enable IRQ1-IRQ7 (SIUMCR [DPPC]), 
     * and local bus for PCI (SIUMCR [LBPC]).
     */
    immap->im_siu_conf.sc_siumcr = 0x00640000;

    /* Make PCI lowest priority */
    /* Each 4 bits is a device bus request  and the MS 4bits 
       is highest priority */
    /* Bus               4bit value 
	   ---               ----------
       CPM high          0b0000
       CPM middle        0b0001
	   CPM low           0b0010
       PCI reguest       0b0011
       Reserved          0b0100
       Reserved          0b0101
       Internal Core     0b0110
       External Master 1 0b0111
       External Master 2 0b1000
       External Master 3 0b1001
       The rest are reserved */
    immap->im_siu_conf.sc_ppc_alrh = 0x61207893;

    /* Park bus on core while modifying PCI Bus accesses */
    immap->im_siu_conf.sc_ppc_acr = 0x6;

    /* 
     * Set up master window that allows the CPU to access PCI space. This 
     * window is set up using the first SIU PCIBR registers.
     */
    *(volatile unsigned long*)(immr_addr + M8265_PCIMSK0) = PCIMSK0_MASK;
    *(volatile unsigned long*)(immr_addr + M8265_PCIBR0) =
	    PCI_MSTR_LOCAL | PCIBR_ENABLE;

    /* Release PCI RST (by default the PCI RST signal is held low)  */
    pci_outl (immr_addr | PCI_GCR_REG, PCIGCR_PCI_BUS_EN);

    /* give it some time */
    udelay(1000);

    /* 
     * Set up master window that allows the CPU to access PCI Memory (prefetch) 
     * space. This window is set up using the first set of Outbound ATU registers.
     */
    pci_outl (immr_addr | POTAR_REG0, PCI_MSTR_MEM_BUS >> 12);      /* PCI base */
    pci_outl (immr_addr | POBAR_REG0, PCI_MSTR_MEM_LOCAL >> 12);    /* Local base */
    pci_outl (immr_addr | POCMR_REG0, POCMR0_MASK_ATTRIB);    /* Size & attribute */

    /* 
     * Set up master window that allows the CPU to access PCI Memory (non-prefetch) 
     * space. This window is set up using the second set of Outbound ATU registers.
     */
    pci_outl (immr_addr | POTAR_REG1, PCI_MSTR_MEMIO_BUS >> 12);    /* PCI base */
    pci_outl (immr_addr | POBAR_REG1, PCI_MSTR_MEMIO_LOCAL >> 12);  /* Local base */
    pci_outl (immr_addr | POCMR_REG1, POCMR1_MASK_ATTRIB);    /* Size & attribute */
    
    /* 
     * Set up master window that allows the CPU to access PCI IO space. This window
     * is set up using the third set of Outbound ATU registers.
     */
    pci_outl (immr_addr | POTAR_REG2, PCI_MSTR_IO_BUS >> 12);       /* PCI base */
    pci_outl (immr_addr | POBAR_REG2, PCI_MSTR_IO_LOCAL >> 12);     /* Local base */
    pci_outl (immr_addr | POCMR_REG2, POCMR2_MASK_ATTRIB);    /* Size & attribute */

    /* 
     * Set up slave window that allows PCI masters to access MPC826x local memory. 
     * This window is set up using the first set of Inbound ATU registers
     */
    pci_outl (immr_addr | PITAR_REG0, PCI_SLV_MEM_LOCAL >> 12);     /* Local base */
    pci_outl (immr_addr | PIBAR_REG0, PCI_SLV_MEM_BUS >> 12);       /* PCI base */
    pci_outl (immr_addr | PICMR_REG0, PICMR0_MASK_ATTRIB);    /* Size & attribute */

     /* See above for description - puts PCI request as highest priority */
    immap->im_siu_conf.sc_ppc_alrh = 0x03124567;

    /* Park the bus on the PCI */
    immap->im_siu_conf.sc_ppc_acr = PPC_ACR_BUS_PARK_PCI;

    /* Host mode - specify the bridge as a host-PCI bridge */

    pci_hose_write_config_byte(hose, host_devno, PCI_CLASS_CODE,
	                           PCI_CLASS_BRIDGE_CTLR);

    /* Enable the host bridge to be a master on the PCI bus, and to act as a PCI memory target */
    pci_hose_read_config_word(hose, host_devno, PCI_COMMAND, &tempShort);
    pci_hose_write_config_word(hose, host_devno, PCI_COMMAND,
		         tempShort | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);

    hose->first_busno = 0;
    hose->last_busno = 0xff;

    /* System memory space */
    pci_set_region(hose->regions + 0,
		       CFG_SDRAM_BASE,
		       CFG_SDRAM_BASE,
		       0x4000000,
		       PCI_REGION_MEM | PCI_REGION_MEMORY);

    /* PCI memory space */
    pci_set_region(hose->regions + 1,
		       PCI_MSTR_MEM_BUS,
		       PCI_MSTR_MEM_LOCAL,
		       PCI_MSTR_MEM_SIZE,
		       PCI_REGION_MEM);

    /* PCI I/O space */
    pci_set_region(hose->regions + 2,
		       PCI_MSTR_IO_BUS,
		       PCI_MSTR_IO_LOCAL,
		       PCI_MSTR_IO_SIZE,
		       PCI_REGION_IO);

    hose->region_count = 3;

    pci_register_hose(hose);

    hose->last_busno = pci_hose_scan(hose);
}

#endif	/* CONFIG_PCI */
