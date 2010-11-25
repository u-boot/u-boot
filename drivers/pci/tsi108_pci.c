/*
 * (C) Copyright 2004 Tundra Semiconductor Corp.
 * Alex Bounine <alexandreb@tundra.com>
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
 * PCI initialisation for the Tsi108 EMU board.
 */

#include <config.h>

#include <common.h>
#include <pci.h>
#include <asm/io.h>
#include <tsi108.h>
#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <fdt_support.h>
#endif

struct pci_controller local_hose;

void tsi108_clear_pci_error (void)
{
	u32 err_stat, err_addr, pci_stat;

	/*
	 * Quietly clear errors signalled as result of PCI/X configuration read
	 * requests.
	 */
	/* Read PB Error Log Registers */
	err_stat = *(volatile u32 *)(CONFIG_SYS_TSI108_CSR_BASE +
				     TSI108_PB_REG_OFFSET + PB_ERRCS);
	err_addr = *(volatile u32 *)(CONFIG_SYS_TSI108_CSR_BASE +
				     TSI108_PB_REG_OFFSET + PB_AERR);
	if (err_stat & PB_ERRCS_ES) {
		/* Clear PCI/X bus errors if applicable */
		if ((err_addr & 0xFF000000) == CONFIG_SYS_PCI_CFG_BASE) {
			/* Clear error flag */
			*(u32 *) (CONFIG_SYS_TSI108_CSR_BASE +
				  TSI108_PB_REG_OFFSET + PB_ERRCS) =
			    PB_ERRCS_ES;

			/* Clear read error reported in PB_ISR */
			*(u32 *) (CONFIG_SYS_TSI108_CSR_BASE +
				  TSI108_PB_REG_OFFSET + PB_ISR) =
			    PB_ISR_PBS_RD_ERR;

		/* Clear errors reported by PCI CSR (Normally Master Abort) */
			pci_stat = *(volatile u32 *)(CONFIG_SYS_TSI108_CSR_BASE +
						     TSI108_PCI_REG_OFFSET +
						     PCI_CSR);
			*(volatile u32 *)(CONFIG_SYS_TSI108_CSR_BASE +
					  TSI108_PCI_REG_OFFSET + PCI_CSR) =
			    pci_stat;

			*(volatile u32 *)(CONFIG_SYS_TSI108_CSR_BASE +
					  TSI108_PCI_REG_OFFSET +
					  PCI_IRP_STAT) = PCI_IRP_STAT_P_CSR;
		}
	}

	return;
}

unsigned int __get_pci_config_dword (u32 addr)
{
	unsigned int retval;

	__asm__ __volatile__ ("       lwbrx %0,0,%1\n"
			     "1:     eieio\n"
			     "2:\n"
			     ".section .fixup,\"ax\"\n"
			     "3:     li %0,-1\n"
			     "       b 2b\n"
			     ".section __ex_table,\"a\"\n"
			     "       .align 2\n"
			     "       .long 1b,3b\n"
			     ".section .text.__get_pci_config_dword"
				: "=r"(retval) : "r"(addr));

	return (retval);
}

static int tsi108_read_config_dword (struct pci_controller *hose,
				    pci_dev_t dev, int offset, u32 * value)
{
	dev &= (CONFIG_SYS_PCI_CFG_SIZE - 1);
	dev |= (CONFIG_SYS_PCI_CFG_BASE | (offset & 0xfc));
	*value = __get_pci_config_dword(dev);
	if (0xFFFFFFFF == *value)
		tsi108_clear_pci_error ();
	return 0;
}

static int tsi108_write_config_dword (struct pci_controller *hose,
				     pci_dev_t dev, int offset, u32 value)
{
	dev &= (CONFIG_SYS_PCI_CFG_SIZE - 1);
	dev |= (CONFIG_SYS_PCI_CFG_BASE | (offset & 0xfc));

	out_le32 ((volatile unsigned *)dev, value);

	return 0;
}

void pci_init_board (void)
{
	struct pci_controller *hose = (struct pci_controller *)&local_hose;

	hose->first_busno = 0;
	hose->last_busno = 0xff;

	pci_set_region (hose->regions + 0,
		       CONFIG_SYS_PCI_MEMORY_BUS,
		       CONFIG_SYS_PCI_MEMORY_PHYS,
		       CONFIG_SYS_PCI_MEMORY_SIZE, PCI_REGION_MEM | PCI_REGION_SYS_MEMORY);

	/* PCI memory space */
	pci_set_region (hose->regions + 1,
		       CONFIG_SYS_PCI_MEM_BUS,
		       CONFIG_SYS_PCI_MEM_PHYS, CONFIG_SYS_PCI_MEM_SIZE, PCI_REGION_MEM);

	/* PCI I/O space */
	pci_set_region (hose->regions + 2,
		       CONFIG_SYS_PCI_IO_BUS,
		       CONFIG_SYS_PCI_IO_PHYS, CONFIG_SYS_PCI_IO_SIZE, PCI_REGION_IO);

	hose->region_count = 3;

	pci_set_ops (hose,
		    pci_hose_read_config_byte_via_dword,
		    pci_hose_read_config_word_via_dword,
		    tsi108_read_config_dword,
		    pci_hose_write_config_byte_via_dword,
		    pci_hose_write_config_word_via_dword,
		    tsi108_write_config_dword);

	pci_register_hose (hose);

	hose->last_busno = pci_hose_scan (hose);

	debug ("Done PCI initialization\n");
	return;
}

#if defined(CONFIG_OF_LIBFDT)
void ft_pci_setup(void *blob, bd_t *bd)
{
	int nodeoffset;
	int tmp[2];
	const char *path;

	nodeoffset = fdt_path_offset(blob, "/aliases");
	if (nodeoffset >= 0) {
		path = fdt_getprop(blob, nodeoffset, "pci", NULL);
		if (path) {
			tmp[0] = cpu_to_be32(local_hose.first_busno);
			tmp[1] = cpu_to_be32(local_hose.last_busno);
			do_fixup_by_path(blob, path, "bus-range",
				&tmp, sizeof(tmp), 1);
		}
	}
}
#endif /* CONFIG_OF_LIBFDT */
