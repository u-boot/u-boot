/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <ppc440.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/4xx_pcie.h>
#include <asm/gpio.h>

extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips */

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	u32 sdr0_cust0;
	u32 pvr = get_pvr();

	/*------------------------------------------------------------------+
	 * Setup the interrupt controller polarities, triggers, etc.
	 *------------------------------------------------------------------*/
	mtdcr(uic0sr, 0xffffffff);	/* clear all */
	mtdcr(uic0er, 0x00000000);	/* disable all */
	mtdcr(uic0cr, 0x00000005);	/* ATI & UIC1 crit are critical */
	mtdcr(uic0pr, 0xffffffff);	/* per ref-board manual */
	mtdcr(uic0tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic0vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic0sr, 0xffffffff);	/* clear all */

	mtdcr(uic1sr, 0xffffffff);	/* clear all */
	mtdcr(uic1er, 0x00000000);	/* disable all */
	mtdcr(uic1cr, 0x00000000);	/* all non-critical */
	mtdcr(uic1pr, 0xffffffff);	/* per ref-board manual */
	mtdcr(uic1tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic1vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic1sr, 0xffffffff);	/* clear all */

	mtdcr(uic2sr, 0xffffffff);	/* clear all */
	mtdcr(uic2er, 0x00000000);	/* disable all */
	mtdcr(uic2cr, 0x00000000);	/* all non-critical */
	mtdcr(uic2pr, 0xffffffff);	/* per ref-board manual */
	mtdcr(uic2tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic2vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic2sr, 0xffffffff);	/* clear all */

	mtdcr(uic3sr, 0xffffffff);	/* clear all */
	mtdcr(uic3er, 0x00000000);	/* disable all */
	mtdcr(uic3cr, 0x00000000);	/* all non-critical */
	mtdcr(uic3pr, 0xffffffff);	/* per ref-board manual */
	mtdcr(uic3tr, 0x00000000);	/* per ref-board manual */
	mtdcr(uic3vr, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(uic3sr, 0xffffffff);	/* clear all */

	/* SDR Setting - enable NDFC */
	mfsdr(SDR0_CUST0, sdr0_cust0);
	sdr0_cust0 = SDR0_CUST0_MUX_NDFC_SEL	|
		SDR0_CUST0_NDFC_ENABLE		|
		SDR0_CUST0_NDFC_BW_8_BIT	|
		SDR0_CUST0_NDFC_ARE_MASK	|
		SDR0_CUST0_NDFC_BAC_ENCODE(3)	|
		(0x80000000 >> (28 + CFG_NAND_CS));
	mtsdr(SDR0_CUST0, sdr0_cust0);

	/*
	 * Configure PFC (Pin Function Control) registers
	 * UART0: 4 pins
	 */
	mtsdr(SDR0_PFC1, 0x00040000);

	/* Enable PCI host functionality in SDR0_PCI0 */
	mtsdr(SDR0_PCI0, 0xe0000000);

	/* Enable ethernet and take out of reset */
	out_8((void *)CFG_BCSR_BASE + 6, 0);

	/* Remove NOR-FLASH, NAND-FLASH & EEPROM hardware write protection */
	out_8((void *)CFG_BCSR_BASE + 5, 0);

	/* Enable USB host & USB-OTG */
	out_8((void *)CFG_BCSR_BASE + 7, 0);

	mtsdr(SDR0_SRST1, 0);	/* Pull AHB out of reset default=1 */

	/* Setup PLB4-AHB bridge based on the system address map */
	mtdcr(AHB_TOP, 0x8000004B);
	mtdcr(AHB_BOT, 0x8000004B);

	if ((pvr == PVR_460EX_RA) || (pvr == PVR_460EX_SE_RA)) {
		/*
		 * Configure USB-STP pins as alternate and not GPIO
		 * It seems to be neccessary to configure the STP pins as GPIO
		 * input at powerup (perhaps while USB reset is asserted). So
		 * we configure those pins to their "real" function now.
		 */
		gpio_config(16, GPIO_OUT, GPIO_ALT1, GPIO_OUT_1);
		gpio_config(19, GPIO_OUT, GPIO_ALT1, GPIO_OUT_1);
	}

	return 0;
}

int checkboard (void)
{
	char *s = getenv("serial#");
	u32 pvr = get_pvr();

	if ((pvr == PVR_460GT_RA) || (pvr == PVR_460GT_SE_RA))
		printf("Board: Glacier - AMCC PPC460GT Evaluation Board");
	else
		printf("Board: Canyonlands - AMCC PPC460EX Evaluation Board");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}

/*
 * Override the default functions in cpu/ppc4xx/44x_spd_ddr2.c with
 * board specific values.
 */
u32 ddr_wrdtr(u32 default_val) {
	return (SDRAM_WRDTR_LLWP_1_CYC | SDRAM_WRDTR_WTR_180_DEG_ADV | 0x823);
}

u32 ddr_clktr(u32 default_val) {
	return (SDRAM_CLKTR_CLKP_90_DEG_ADV);
}

#if defined(CONFIG_NAND_U_BOOT)
/*
 * NAND booting U-Boot version uses a fixed initialization, since the whole
 * I2C SPD DIMM autodetection/calibration doesn't fit into the 4k of boot
 * code.
 */
long int initdram(int board_type)
{
	return CFG_MBYTES_SDRAM << 20;
}
#endif

#if defined(CFG_DRAM_TEST)
int testdram(void)
{
	unsigned long *mem = (unsigned long *)0;
	const unsigned long kend = (1024 / sizeof(unsigned long));
	unsigned long k, n;

	mtmsr(0);

	for (k = 0; k < CFG_KBYTES_SDRAM;
	     ++k, mem += (1024 / sizeof(unsigned long))) {
		if ((k & 1023) == 0) {
			printf("%3d MB\r", k / 1024);
		}

		memset(mem, 0xaaaaaaaa, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0xaaaaaaaa) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}

		memset(mem, 0x55555555, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0x55555555) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}
	}
	printf("SDRAM test passes\n");
	return 0;
}
#endif

/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller * hose )
{
	/*-------------------------------------------------------------------+
	 * Disable everything
	 *-------------------------------------------------------------------*/
	out_le32((void *)PCIX0_PIM0SA, 0); /* disable */
	out_le32((void *)PCIX0_PIM1SA, 0); /* disable */
	out_le32((void *)PCIX0_PIM2SA, 0); /* disable */
	out_le32((void *)PCIX0_EROMBA, 0); /* disable expansion rom */

	/*-------------------------------------------------------------------+
	 * Map all of SDRAM to PCI address 0x0000_0000. Note that the 440
	 * strapping options to not support sizes such as 128/256 MB.
	 *-------------------------------------------------------------------*/
	out_le32((void *)PCIX0_PIM0LAL, CFG_SDRAM_BASE);
	out_le32((void *)PCIX0_PIM0LAH, 0);
	out_le32((void *)PCIX0_PIM0SA, ~(gd->ram_size - 1) | 1);
	out_le32((void *)PCIX0_BAR0, 0);

	/*-------------------------------------------------------------------+
	 * Program the board's subsystem id/vendor id
	 *-------------------------------------------------------------------*/
	out_le16((void *)PCIX0_SBSYSVID, CFG_PCI_SUBSYS_VENDORID);
	out_le16((void *)PCIX0_SBSYSID, CFG_PCI_SUBSYS_DEVICEID);

	out_le16((void *)PCIX0_CMD, in16r(PCIX0_CMD) | PCI_COMMAND_MEMORY);
}
#endif	/* defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT) */

#if defined(CONFIG_PCI)
/*
 * is_pci_host
 *
 * This routine is called to determine if a pci scan should be
 * performed. With various hardware environments (especially cPCI and
 * PPMC) it's insufficient to depend on the state of the arbiter enable
 * bit in the strap register, or generic host/adapter assumptions.
 *
 * Rather than hard-code a bad assumption in the general 440 code, the
 * 440 pci code requires the board to decide at runtime.
 *
 * Return 0 for adapter mode, non-zero for host (monarch) mode.
 */
int is_pci_host(struct pci_controller *hose)
{
	/* Board is always configured as host. */
	return (1);
}

static struct pci_controller pcie_hose[2] = {{0},{0}};

void pcie_setup_hoses(int busno)
{
	struct pci_controller *hose;
	int i, bus;
	int ret = 0;
	char *env;
	unsigned int delay;

	/*
	 * assume we're called after the PCIX hose is initialized, which takes
	 * bus ID 0 and therefore start numbering PCIe's from 1.
	 */
	bus = busno;
	for (i = 0; i <= 1; i++) {

		if (is_end_point(i))
			ret = ppc4xx_init_pcie_endport(i);
		else
			ret = ppc4xx_init_pcie_rootport(i);
		if (ret) {
			printf("PCIE%d: initialization as %s failed\n", i,
			       is_end_point(i) ? "endpoint" : "root-complex");
			continue;
		}

		hose = &pcie_hose[i];
		hose->first_busno = bus;
		hose->last_busno = bus;
		hose->current_busno = bus;

		/* setup mem resource */
		pci_set_region(hose->regions + 0,
			       CFG_PCIE_MEMBASE + i * CFG_PCIE_MEMSIZE,
			       CFG_PCIE_MEMBASE + i * CFG_PCIE_MEMSIZE,
			       CFG_PCIE_MEMSIZE,
			       PCI_REGION_MEM);
		hose->region_count = 1;
		pci_register_hose(hose);

		if (is_end_point(i)) {
			ppc4xx_setup_pcie_endpoint(hose, i);
			/*
			 * Reson for no scanning is endpoint can not generate
			 * upstream configuration accesses.
			 */
		} else {
			ppc4xx_setup_pcie_rootpoint(hose, i);
			env = getenv ("pciscandelay");
			if (env != NULL) {
				delay = simple_strtoul(env, NULL, 10);
				if (delay > 5)
					printf("Warning, expect noticable delay before "
					       "PCIe scan due to 'pciscandelay' value!\n");
				mdelay(delay * 1000);
			}

			/*
			 * Config access can only go down stream
			 */
			hose->last_busno = pci_hose_scan(hose);
			bus = hose->last_busno + 1;
		}
	}
}
#endif /* CONFIG_PCI */

int board_early_init_r (void)
{
	/*
	 * Canyonlands has 64MBytes of NOR FLASH (Spansion 29GL512), but the
	 * boot EBC mapping only supports a maximum of 16MBytes
	 * (4.ff00.0000 - 4.ffff.ffff).
	 * To solve this problem, the FLASH has to get remapped to another
	 * EBC address which accepts bigger regions:
	 *
	 * 0xfc00.0000 -> 4.cc00.0000
	 */

	/* Remap the NOR FLASH to 0xcc00.0000 ... 0xcfff.ffff */
#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
	mtebc(pb3cr, CFG_FLASH_BASE_PHYS_L | 0xda000);
#else
	mtebc(pb0cr, CFG_FLASH_BASE_PHYS_L | 0xda000);
#endif

	/* Remove TLB entry of boot EBC mapping */
	remove_tlb(CFG_BOOT_BASE_ADDR, 16 << 20);

	/* Add TLB entry for 0xfc00.0000 -> 0x4.cc00.0000 */
	program_tlb(CFG_FLASH_BASE_PHYS, CFG_FLASH_BASE, CFG_FLASH_SIZE,
		    TLB_WORD2_I_ENABLE);

	/*
	 * Now accessing of the whole 64Mbytes of NOR FLASH at virtual address
	 * 0xfc00.0000 is possible
	 */

	/*
	 * Clear potential errors resulting from auto-calibration.
	 * If not done, then we could get an interrupt later on when
	 * exceptions are enabled.
	 */
	set_mcsr(get_mcsr());

	return 0;
}

int misc_init_r(void)
{
	u32 sdr0_srst1 = 0;
	u32 eth_cfg;
	u32 pvr = get_pvr();

	/*
	 * Set EMAC mode/configuration (GMII, SGMII, RGMII...).
	 * This is board specific, so let's do it here.
	 */
	mfsdr(SDR0_ETH_CFG, eth_cfg);
	/* disable SGMII mode */
	eth_cfg &= ~(SDR0_ETH_CFG_SGMII2_ENABLE |
		     SDR0_ETH_CFG_SGMII1_ENABLE |
		     SDR0_ETH_CFG_SGMII0_ENABLE);
	/* Set the for 2 RGMII mode */
	/* GMC0 EMAC4_0, GMC0 EMAC4_1, RGMII Bridge 0 */
	eth_cfg &= ~SDR0_ETH_CFG_GMC0_BRIDGE_SEL;
	if ((pvr == PVR_460EX_RA) || (pvr == PVR_460EX_SE_RA))
		eth_cfg |= SDR0_ETH_CFG_GMC1_BRIDGE_SEL;
	else
		eth_cfg &= ~SDR0_ETH_CFG_GMC1_BRIDGE_SEL;
	mtsdr(SDR0_ETH_CFG, eth_cfg);

	/*
	 * The AHB Bridge core is held in reset after power-on or reset
	 * so enable it now
	 */
	mfsdr(SDR0_SRST1, sdr0_srst1);
	sdr0_srst1 &= ~SDR0_SRST1_AHB;
	mtsdr(SDR0_SRST1, sdr0_srst1);

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	u32 val[4];
	int rc;

	ft_cpu_setup(blob, bd);

	/* Fixup NOR mapping */
	val[0] = 0;				/* chip select number */
	val[1] = 0;				/* always 0 */
	val[2] = gd->bd->bi_flashstart;
	val[3] = gd->bd->bi_flashsize;
	rc = fdt_find_and_setprop(blob, "/plb/opb/ebc", "ranges",
				  val, sizeof(val), 1);
	if (rc)
		printf("Unable to update property NOR mapping, err=%s\n",
		       fdt_strerror(rc));
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
