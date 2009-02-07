/*
 * (C) Copyright 2001
 * Stefan Roese, esd gmbh germany, stefan.roese@esd-electronics.com
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
#include <ppc4xx.h>
#include <asm/processor.h>
#include <pci.h>


u_long pci9054_iobase;


#define PCI_PRIMARY_CAR	(0x500000dc) /* PCI config address reg */
#define PCI_PRIMARY_CDR	(0x80000000) /* PCI config data    reg */


/*-----------------------------------------------------------------------------+
|  Subroutine:  pci9054_read_config_dword
|  Description: Read a PCI configuration register
|  Inputs:
|               hose            PCI Controller
|               dev             PCI Bus+Device+Function number
|               offset          Configuration register number
|               value           Address of the configuration register value
|  Return value:
|               0               Successful
+-----------------------------------------------------------------------------*/
int pci9054_read_config_dword(struct pci_controller *hose,
			      pci_dev_t dev, int offset, u32* value)
{
  unsigned long      conAdrVal;
  unsigned long      val;

  /* generate coded value for CON_ADR register */
  conAdrVal = dev | (offset & 0xfc) | 0x80000000;

  /* Load the CON_ADR (CAR) value first, then read from CON_DATA (CDR) */
  *(unsigned long *)PCI_PRIMARY_CAR = conAdrVal;

  /* Note: *pResult comes back as -1 if machine check happened */
  val = in32r(PCI_PRIMARY_CDR);

  *value = (unsigned long) val;

  out32r(PCI_PRIMARY_CAR, 0);

  if ((*(unsigned long *)0x50000304) & 0x60000000)
    {
      /* clear pci master/target abort bits */
      *(unsigned long *)0x50000304 = *(unsigned long *)0x50000304;
    }

  return 0;
}

/*-----------------------------------------------------------------------------+
|  Subroutine:  pci9054_write_config_dword
|  Description: Write a PCI configuration register.
|  Inputs:
|               hose            PCI Controller
|               dev             PCI Bus+Device+Function number
|               offset          Configuration register number
|               Value           Configuration register value
|  Return value:
|               0               Successful
| Updated for pass2 errata #6. Need to disable interrupts and clear the
| PCICFGADR reg after writing the PCICFGDATA reg.
+-----------------------------------------------------------------------------*/
int pci9054_write_config_dword(struct pci_controller *hose,
			       pci_dev_t dev, int offset, u32 value)
{
  unsigned long      conAdrVal;

  conAdrVal = dev | (offset & 0xfc) | 0x80000000;

  *(unsigned long *)PCI_PRIMARY_CAR = conAdrVal;

  out32r(PCI_PRIMARY_CDR, value);

  out32r(PCI_PRIMARY_CAR, 0);

  /* clear pci master/target abort bits */
  *(unsigned long *)0x50000304 = *(unsigned long *)0x50000304;

  return (0);
}

/*-----------------------------------------------------------------------
 */

#ifdef CONFIG_DASA_SIM
static void pci_dasa_sim_config_pci9054(struct pci_controller *hose, pci_dev_t dev,
					struct pci_config_table *_)
{
  unsigned int iobase;
  unsigned short status = 0;
  unsigned char timer;

  /*
   * Configure PLX PCI9054
   */
  pci_read_config_word(CONFIG_SYS_PCI9054_DEV_FN, PCI_COMMAND, &status);
  status |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
  pci_write_config_word(CONFIG_SYS_PCI9054_DEV_FN, PCI_COMMAND, status);

  /* Check the latency timer for values >= 0x60.
   */
  pci_read_config_byte(CONFIG_SYS_PCI9054_DEV_FN, PCI_LATENCY_TIMER, &timer);
  if (timer < 0x60)
    {
      pci_write_config_byte(CONFIG_SYS_PCI9054_DEV_FN, PCI_LATENCY_TIMER, 0x60);
    }

  /* Set I/O base register.
   */
  pci_write_config_dword(CONFIG_SYS_PCI9054_DEV_FN, PCI_BASE_ADDRESS_0, CONFIG_SYS_PCI9054_IOBASE);
  pci_read_config_dword(CONFIG_SYS_PCI9054_DEV_FN, PCI_BASE_ADDRESS_0, &iobase);

  pci9054_iobase = pci_mem_to_phys(CONFIG_SYS_PCI9054_DEV_FN, iobase & PCI_BASE_ADDRESS_MEM_MASK);

  if (pci9054_iobase == 0xffffffff)
    {
      printf("Error: Can not set I/O base register.\n");
      return;
    }
}
#endif

static struct pci_config_table pci9054_config_table[] = {
#ifndef CONFIG_PCI_PNP
  { PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
    PCI_BUS(CONFIG_SYS_ETH_DEV_FN), PCI_DEV(CONFIG_SYS_ETH_DEV_FN), PCI_FUNC(CONFIG_SYS_ETH_DEV_FN),
    pci_cfgfunc_config_device, { CONFIG_SYS_ETH_IOBASE,
				 CONFIG_SYS_ETH_IOBASE,
				 PCI_COMMAND_IO | PCI_COMMAND_MASTER }},
#ifdef CONFIG_DASA_SIM
  { PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
    PCI_BUS(CONFIG_SYS_PCI9054_DEV_FN), PCI_DEV(CONFIG_SYS_PCI9054_DEV_FN), PCI_FUNC(CONFIG_SYS_PCI9054_DEV_FN),
    pci_dasa_sim_config_pci9054 },
#endif
#endif
  { }
};

static struct pci_controller pci9054_hose = {
  config_table: pci9054_config_table,
};

void pci_init(void)
{
  struct pci_controller *hose = &pci9054_hose;

  /*
   * Register the hose
   */
  hose->first_busno = 0;
  hose->last_busno = 0xff;

  /* System memory space */
  pci_set_region(hose->regions + 0,
		 0x00000000, 0x00000000, 0x01000000,
		 PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

  /* PCI Memory space */
  pci_set_region(hose->regions + 1,
		 0x00000000, 0xc0000000, 0x10000000,
		 PCI_REGION_MEM);

  pci_set_ops(hose,
	      pci_hose_read_config_byte_via_dword,
	      pci_hose_read_config_word_via_dword,
	      pci9054_read_config_dword,
	      pci_hose_write_config_byte_via_dword,
	      pci_hose_write_config_word_via_dword,
	      pci9054_write_config_dword);

  hose->region_count = 2;

  pci_register_hose(hose);

  hose->last_busno = pci_hose_scan(hose);
}
