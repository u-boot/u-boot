/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 *
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2002,2003, Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <ioports.h>
#include <flash.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/io.h>

#if defined(CFG_FPGA_BASE)
#include "upm_table.h"
#endif
DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[];	/* FLASH chips info */

void local_bus_init (void);
ulong flash_get_size (ulong base, int banknum);

int checkboard (void)
{
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
	char *src;
	int f;
	char *s = getenv("serial#");

	puts("Board: Socrates");
	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

#ifdef CONFIG_PCI
	/* Check the PCI_clk sel bit */
	if (in_be32(&gur->porpllsr) & (1<<15)) {
		src = "SYSCLK";
		f = CONFIG_SYS_CLK_FREQ;
	} else {
		src = "PCI_CLK";
		f = CONFIG_PCI_CLK_FREQ;
	}
	printf ("PCI1:  32 bit, %d MHz (%s)\n",	f/1000000, src);
#else
	printf ("PCI1:  disabled\n");
#endif

	/*
	 * Initialize local bus.
	 */
	local_bus_init ();
#if defined(CFG_FPGA_BASE)
	/* Init UPMA for FPGA access */
	upmconfig(UPMA, (uint *)UPMTableA, sizeof(UPMTableA)/sizeof(int));
#endif
	return 0;
}

int misc_init_r (void)
{
	volatile ccsr_lbc_t *memctl = (void *)(CFG_MPC85xx_LBC_ADDR);

	/*
	 * Adjust flash start and offset to detected values
	 */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Check if boot FLASH isn't max size
	 */
	if (gd->bd->bi_flashsize < (0 - CFG_FLASH0)) {
		memctl->or0 = gd->bd->bi_flashstart | (CFG_OR0_PRELIM & 0x00007fff);
		memctl->br0 = gd->bd->bi_flashstart | (CFG_BR0_PRELIM & 0x00007fff);

		/*
		 * Re-check to get correct base address
		 */
		flash_get_size(gd->bd->bi_flashstart, CFG_MAX_FLASH_BANKS - 1);
	}

	/*
	 * Check if only one FLASH bank is available
	 */
	if (gd->bd->bi_flashsize != CFG_MAX_FLASH_BANKS * (0 - CFG_FLASH0)) {
		memctl->or1 = 0;
		memctl->br1 = 0;

		/*
		 * Re-do flash protection upon new addresses
		 */
		flash_protect (FLAG_PROTECT_CLEAR,
			       gd->bd->bi_flashstart, 0xffffffff,
			       &flash_info[CFG_MAX_FLASH_BANKS - 1]);

		/* Monitor protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CFG_MONITOR_BASE, CFG_MONITOR_BASE + monitor_flash_len - 1,
			       &flash_info[CFG_MAX_FLASH_BANKS - 1]);

		/* Environment protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CFG_ENV_ADDR,
			       CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
			       &flash_info[CFG_MAX_FLASH_BANKS - 1]);

		/* Redundant environment protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CFG_ENV_ADDR_REDUND,
			       CFG_ENV_ADDR_REDUND + CFG_ENV_SIZE_REDUND - 1,
			       &flash_info[CFG_MAX_FLASH_BANKS - 1]);
	}

	return 0;
}

/*
 * Initialize Local Bus
 */
void local_bus_init (void)
{

	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);
	volatile ccsr_local_ecm_t *ecm = (void *)(CFG_MPC85xx_ECM_ADDR);

	lbc->ltesr = 0xffffffff;	/* Clear LBC error interrupts */
	lbc->lteir = 0xffffffff;	/* Enable LBC error interrupts */
	ecm->eedr = 0xffffffff;		/* Clear ecm errors */
	ecm->eeer = 0xffffffff;		/* Enable ecm errors */

}

#if defined(CONFIG_PCI)
/*
 * Initialize PCI Devices, report devices found.
 */

#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_mpc85xxads_config_table[] = {
	{PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 PCI_IDSEL_NUMBER, PCI_ANY_ID,
	 pci_cfgfunc_config_device, {PCI_ENET0_IOADDR,
				     PCI_ENET0_MEMADDR,
				     PCI_COMMAND_MEMORY |
				     PCI_COMMAND_MASTER}},
	{}
};
#endif


static struct pci_controller hose = {
#ifndef CONFIG_PCI_PNP
	config_table:pci_mpc85xxads_config_table,
#endif
};

#endif /* CONFIG_PCI */


void pci_init_board (void)
{
#ifdef CONFIG_PCI
	pci_mpc85xx_init (&hose);
#endif /* CONFIG_PCI */
}

#ifdef CONFIG_BOARD_EARLY_INIT_R
int board_early_init_r (void)
{
#ifdef CONFIG_PS2MULT
	ps2mult_early_init();
#endif /* CONFIG_PS2MULT */
	return (0);
}
#endif /* CONFIG_BOARD_EARLY_INIT_R */

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void
ft_board_setup(void *blob, bd_t *bd)
{
	u32 val[4];
	int rc;

	ft_cpu_setup(blob, bd);

	/* Fixup NOR mapping */
	val[0] = 0;				/* chip select number */
	val[1] = 0;				/* always 0 */
	val[2] = gd->bd->bi_flashstart;
	val[3] = gd->bd->bi_flashsize;

	rc = fdt_find_and_setprop(blob, "/localbus", "ranges",
				  val, sizeof(val), 1);
	if (rc)
		printf("Unable to update property NOR mapping, err=%s\n",
		       fdt_strerror(rc));

#if defined (CFG_FPGA_BASE)
	memset(val, 0, sizeof(val));
	val[0] = CFG_FPGA_BASE;
	rc = fdt_find_and_setprop(blob, "/localbus/fpga", "virtual-reg",
				  val, sizeof(val), 1);
	if (rc)
		printf("Unable to update property \"fpga\", err=%s\n",
		       fdt_strerror(rc));
#endif
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
