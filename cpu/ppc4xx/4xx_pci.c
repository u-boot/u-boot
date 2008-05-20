/*-----------------------------------------------------------------------------+
 *
 *       This source code has been made available to you by IBM on an AS-IS
 *       basis.  Anyone receiving this source is licensed under IBM
 *       copyrights to use it in any way he or she deems fit, including
 *       copying it, modifying it, compiling it, and redistributing it either
 *       with or without modifications.  No license under IBM patents or
 *       patent applications is to be implied by the copyright license.
 *
 *       Any user of this software should understand that IBM cannot provide
 *       technical support for this software and will not be responsible for
 *       any consequences resulting from the use of this software.
 *
 *       Any person who transfers this source code or any derivative work
 *       must include the IBM copyright notice, this paragraph, and the
 *       preceding two paragraphs in the transferred software.
 *
 *       COPYRIGHT   I B M   CORPORATION 1995
 *       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 *-----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
 *
 *  File Name:   405gp_pci.c
 *
 *  Function:    Initialization code for the 405GP PCI Configuration regs.
 *
 *  Author:      Mark Game
 *
 *  Change Activity-
 *
 *  Date        Description of Change                                       BY
 *  ---------   ---------------------                                       ---
 *  09-Sep-98   Created                                                     MCG
 *  02-Nov-98   Removed External arbiter selected message                   JWB
 *  27-Nov-98   Zero out PTMBAR2 and disable in PTM2MS                      JWB
 *  04-Jan-99   Zero out other unused PMM and PTM regs. Change bus scan     MCG
 *              from (0 to n) to (1 to n).
 *  17-May-99   Port to Walnut                                              JWB
 *  17-Jun-99   Updated for VGA support                                     JWB
 *  21-Jun-99   Updated to allow SRAM region to be a target from PCI bus    JWB
 *  19-Jul-99   Updated for 405GP pass 1 errata #26 (Low PCI subsequent     MCG
 *              target latency timer values are not supported).
 *              Should be fixed in pass 2.
 *  09-Sep-99   Removed use of PTM2 since the SRAM region no longer needs   JWB
 *              to be a PCI target. Zero out PTMBAR2 and disable in PTM2MS.
 *  10-Dec-99   Updated PCI_Write_CFG_Reg for pass2 errata #6               JWB
 *  11-Jan-00   Ensure PMMxMAs disabled before setting PMMxLAs. This is not
 *              really required after a reset since PMMxMAs are already
 *	        disabled but is a good practice nonetheless.                JWB
 *  12-Jun-01   stefan.roese@esd-electronics.com
 *              - PCI host/adapter handling reworked
 *  09-Jul-01   stefan.roese@esd-electronics.com
 *              - PCI host now configures from device 0 (not 1) to max_dev,
 *                (host configures itself)
 *              - On CPCI-405 pci base address and size is generated from
 *                SDRAM and FLASH size (CFG regs not used anymore)
 *              - Some minor changes for CPCI-405-A (adapter version)
 *  14-Sep-01   stefan.roese@esd-electronics.com
 *              - CONFIG_PCI_SCAN_SHOW added to print pci devices upon startup
 *  28-Sep-01   stefan.roese@esd-electronics.com
 *              - Changed pci master configuration for linux compatibility
 *                (no need for bios_fixup() anymore)
 *  26-Feb-02   stefan.roese@esd-electronics.com
 *              - Bug fixed in pci configuration (Andrew May)
 *              - Removed pci class code init for CPCI405 board
 *  15-May-02   stefan.roese@esd-electronics.com
 *              - New vga device handling
 *  29-May-02   stefan.roese@esd-electronics.com
 *              - PCI class code init added (if defined)
 *----------------------------------------------------------------------------*/

#include <common.h>
#include <command.h>
#if !defined(CONFIG_440)
#include <asm/4xx_pci.h>
#endif
#include <asm/processor.h>
#include <pci.h>

#ifdef CONFIG_PCI

DECLARE_GLOBAL_DATA_PTR;

/*
 * Board-specific pci initialization
 * Platform code can reimplement pci_pre_init() if needed
 */
int __pci_pre_init(struct pci_controller *hose)
{
	return 1;
}
int pci_pre_init(struct pci_controller *hose) __attribute__((weak, alias("__pci_pre_init")));

#if defined(CONFIG_405GP) || defined(CONFIG_405EP)

#if defined(CONFIG_PMC405)
ushort pmc405_pci_subsys_deviceid(void);
#endif

/*#define DEBUG*/

/*-----------------------------------------------------------------------------+
 * pci_init.  Initializes the 405GP PCI Configuration regs.
 *-----------------------------------------------------------------------------*/
void pci_405gp_init(struct pci_controller *hose)
{
	int i, reg_num = 0;
	bd_t *bd = gd->bd;

	unsigned short temp_short;
	unsigned long ptmpcila[2] = {CFG_PCI_PTM1PCI, CFG_PCI_PTM2PCI};
#if defined(CONFIG_CPCI405) || defined(CONFIG_PMC405)
	char *ptmla_str, *ptmms_str;
#endif
	unsigned long ptmla[2]    = {CFG_PCI_PTM1LA, CFG_PCI_PTM2LA};
	unsigned long ptmms[2]    = {CFG_PCI_PTM1MS, CFG_PCI_PTM2MS};
#if defined(CONFIG_PIP405) || defined (CONFIG_MIP405)
	unsigned long pmmla[3]    = {0x80000000, 0xA0000000, 0};
	unsigned long pmmma[3]    = {0xE0000001, 0xE0000001, 0};
	unsigned long pmmpcila[3] = {0x80000000, 0x00000000, 0};
	unsigned long pmmpciha[3] = {0x00000000, 0x00000000, 0};
#else
	unsigned long pmmla[3]    = {0x80000000, 0,0};
	unsigned long pmmma[3]    = {0xC0000001, 0,0};
	unsigned long pmmpcila[3] = {0x80000000, 0,0};
	unsigned long pmmpciha[3] = {0x00000000, 0,0};
#endif
#ifdef CONFIG_PCI_PNP
#if (CONFIG_PCI_HOST == PCI_HOST_AUTO)
	char *s;
#endif
#endif

#if defined(CONFIG_CPCI405) || defined(CONFIG_PMC405)
	ptmla_str = getenv("ptm1la");
	ptmms_str = getenv("ptm1ms");
	if(NULL != ptmla_str && NULL != ptmms_str ) {
	        ptmla[0] = simple_strtoul (ptmla_str, NULL, 16);
		ptmms[0] = simple_strtoul (ptmms_str, NULL, 16);
	}

	ptmla_str = getenv("ptm2la");
	ptmms_str = getenv("ptm2ms");
	if(NULL != ptmla_str && NULL != ptmms_str ) {
	        ptmla[1] = simple_strtoul (ptmla_str, NULL, 16);
		ptmms[1] = simple_strtoul (ptmms_str, NULL, 16);
	}
#endif

	/*
	 * Register the hose
	 */
	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* ISA/PCI I/O space */
	pci_set_region(hose->regions + reg_num++,
		       MIN_PCI_PCI_IOADDR,
		       MIN_PLB_PCI_IOADDR,
		       0x10000,
		       PCI_REGION_IO);

	/* PCI I/O space */
	pci_set_region(hose->regions + reg_num++,
		       0x00800000,
		       0xe8800000,
		       0x03800000,
		       PCI_REGION_IO);

	reg_num = 2;

	/* Memory spaces */
	for (i=0; i<2; i++)
		if (ptmms[i] & 1)
		{
			if (!i) hose->pci_fb = hose->regions + reg_num;

			pci_set_region(hose->regions + reg_num++,
				       ptmpcila[i], ptmla[i],
				       ~(ptmms[i] & 0xfffff000) + 1,
				       PCI_REGION_MEM |
				       PCI_REGION_MEMORY);
		}

	/* PCI memory spaces */
	for (i=0; i<3; i++)
		if (pmmma[i] & 1)
		{
			pci_set_region(hose->regions + reg_num++,
				       pmmpcila[i], pmmla[i],
				       ~(pmmma[i] & 0xfffff000) + 1,
				       PCI_REGION_MEM);
		}

	hose->region_count = reg_num;

	pci_setup_indirect(hose,
			   PCICFGADR,
			   PCICFGDATA);

	if (hose->pci_fb)
		pciauto_region_init(hose->pci_fb);

	/* Let board change/modify hose & do initial checks */
	if (pci_pre_init (hose) == 0) {
		printf("PCI: Board-specific initialization failed.\n");
		printf("PCI: Configuration aborted.\n");
		return;
	}

	pci_register_hose(hose);

	/*--------------------------------------------------------------------------+
	 * 405GP PCI Master configuration.
	 * Map one 512 MB range of PLB/processor addresses to PCI memory space.
	 * PLB address 0x80000000-0xBFFFFFFF ==> PCI address 0x80000000-0xBFFFFFFF
	 * Use byte reversed out routines to handle endianess.
	 *--------------------------------------------------------------------------*/
	out32r(PMM0MA,    (pmmma[0]&~0x1)); /* disable, configure PMMxLA, PMMxPCILA first */
	out32r(PMM0LA,    pmmla[0]);
	out32r(PMM0PCILA, pmmpcila[0]);
	out32r(PMM0PCIHA, pmmpciha[0]);
	out32r(PMM0MA,    pmmma[0]);

	/*--------------------------------------------------------------------------+
	 * PMM1 is not used.  Initialize them to zero.
	 *--------------------------------------------------------------------------*/
	out32r(PMM1MA,    (pmmma[1]&~0x1));
	out32r(PMM1LA,    pmmla[1]);
	out32r(PMM1PCILA, pmmpcila[1]);
	out32r(PMM1PCIHA, pmmpciha[1]);
	out32r(PMM1MA,    pmmma[1]);

	/*--------------------------------------------------------------------------+
	 * PMM2 is not used.  Initialize them to zero.
	 *--------------------------------------------------------------------------*/
	out32r(PMM2MA,    (pmmma[2]&~0x1));
	out32r(PMM2LA,    pmmla[2]);
	out32r(PMM2PCILA, pmmpcila[2]);
	out32r(PMM2PCIHA, pmmpciha[2]);
	out32r(PMM2MA,    pmmma[2]);

	/*--------------------------------------------------------------------------+
	 * 405GP PCI Target configuration.  (PTM1)
	 * Note: PTM1MS is hardwire enabled but we set the enable bit anyway.
	 *--------------------------------------------------------------------------*/
	out32r(PTM1LA,    ptmla[0]);         /* insert address                     */
	out32r(PTM1MS,    ptmms[0]);         /* insert size, enable bit is 1       */
	pci_write_config_dword(PCIDEVID_405GP, PCI_BASE_ADDRESS_1, ptmpcila[0]);

	/*--------------------------------------------------------------------------+
	 * 405GP PCI Target configuration.  (PTM2)
	 *--------------------------------------------------------------------------*/
	out32r(PTM2LA, ptmla[1]);            /* insert address                     */
	pci_write_config_dword(PCIDEVID_405GP, PCI_BASE_ADDRESS_2, ptmpcila[1]);

	if (ptmms[1] == 0)
	{
		out32r(PTM2MS,    0x00000001);   /* set enable bit                     */
		pci_write_config_dword(PCIDEVID_405GP, PCI_BASE_ADDRESS_2, 0x00000000);
		out32r(PTM2MS,    0x00000000);   /* disable                            */
	}
	else
	{
		out32r(PTM2MS, ptmms[1]);        /* insert size, enable bit is 1       */
	}

	/*
	 * Insert Subsystem Vendor and Device ID
	 */
	pci_write_config_word(PCIDEVID_405GP, PCI_SUBSYSTEM_VENDOR_ID, CFG_PCI_SUBSYS_VENDORID);
#ifdef CONFIG_CPCI405
	if (mfdcr(strap) & PSR_PCI_ARBIT_EN)
		pci_write_config_word(PCIDEVID_405GP, PCI_SUBSYSTEM_ID, CFG_PCI_SUBSYS_DEVICEID);
	else
		pci_write_config_word(PCIDEVID_405GP, PCI_SUBSYSTEM_ID, CFG_PCI_SUBSYS_DEVICEID2);
#else
	pci_write_config_word(PCIDEVID_405GP, PCI_SUBSYSTEM_ID, CFG_PCI_SUBSYS_DEVICEID);
#endif

	/*
	 * Insert Class-code
	 */
#ifdef CFG_PCI_CLASSCODE
	pci_write_config_word(PCIDEVID_405GP, PCI_CLASS_SUB_CODE, CFG_PCI_CLASSCODE);
#endif /* CFG_PCI_CLASSCODE */

	/*--------------------------------------------------------------------------+
	 * If PCI speed = 66Mhz, set 66Mhz capable bit.
	 *--------------------------------------------------------------------------*/
	if (bd->bi_pci_busfreq >= 66000000) {
		pci_read_config_word(PCIDEVID_405GP, PCI_STATUS, &temp_short);
		pci_write_config_word(PCIDEVID_405GP,PCI_STATUS,(temp_short|PCI_STATUS_66MHZ));
	}

#if (CONFIG_PCI_HOST != PCI_HOST_ADAPTER)
#if (CONFIG_PCI_HOST == PCI_HOST_AUTO)
	if ((mfdcr(strap) & PSR_PCI_ARBIT_EN) ||
	    (((s = getenv("pciscan")) != NULL) && (strcmp(s, "yes") == 0)))
#endif
	{
		/*--------------------------------------------------------------------------+
		 * Write the 405GP PCI Configuration regs.
		 * Enable 405GP to be a master on the PCI bus (PMM).
		 * Enable 405GP to act as a PCI memory target (PTM).
		 *--------------------------------------------------------------------------*/
		pci_read_config_word(PCIDEVID_405GP, PCI_COMMAND, &temp_short);
		pci_write_config_word(PCIDEVID_405GP, PCI_COMMAND, temp_short |
				      PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);
	}
#endif

#if defined(CONFIG_405EP) /* on ppc405ep vendor id is not set */
	pci_write_config_word(PCIDEVID_405GP, PCI_VENDOR_ID, 0x1014); /* IBM */
#endif

	/*
	 * Set HCE bit (Host Configuration Enabled)
	 */
	pci_read_config_word(PCIDEVID_405GP, PCIBRDGOPT2, &temp_short);
	pci_write_config_word(PCIDEVID_405GP, PCIBRDGOPT2, (temp_short | 0x0001));

#ifdef CONFIG_PCI_PNP
	/*--------------------------------------------------------------------------+
	 * Scan the PCI bus and configure devices found.
	 *--------------------------------------------------------------------------*/
#if (CONFIG_PCI_HOST == PCI_HOST_AUTO)
	if ((mfdcr(strap) & PSR_PCI_ARBIT_EN) ||
	    (((s = getenv("pciscan")) != NULL) && (strcmp(s, "yes") == 0)))
#endif
	{
#ifdef CONFIG_PCI_SCAN_SHOW
		printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif
		hose->last_busno = pci_hose_scan(hose);
	}
#endif  /* CONFIG_PCI_PNP */

}

/*
 * drivers/pci/pci.c skips every host bridge but the 405GP since it could
 * be set as an Adapter.
 *
 * I (Andrew May) don't know what we should do here, but I don't want
 * the auto setup of a PCI device disabling what is done pci_405gp_init
 * as has happened before.
 */
void pci_405gp_setup_bridge(struct pci_controller *hose, pci_dev_t dev,
			    struct pci_config_table *entry)
{
#ifdef DEBUG
	printf("405gp_setup_bridge\n");
#endif
}

/*
 *
 */

void pci_405gp_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char int_line = 0xff;

	/*
	 * Write pci interrupt line register (cpci405 specific)
	 */
	switch (PCI_DEV(dev) & 0x03)
	{
	case 0:
		int_line = 27 + 2;
		break;
	case 1:
		int_line = 27 + 3;
		break;
	case 2:
		int_line = 27 + 0;
		break;
	case 3:
		int_line = 27 + 1;
		break;
	}

	pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, int_line);
}

void pci_405gp_setup_vga(struct pci_controller *hose, pci_dev_t dev,
			 struct pci_config_table *entry)
{
	unsigned int cmdstat = 0;

	pciauto_setup_device(hose, dev, 6, hose->pci_mem, hose->pci_prefetch, hose->pci_io);

	/* always enable io space on vga boards */
	pci_hose_read_config_dword(hose, dev, PCI_COMMAND, &cmdstat);
	cmdstat |= PCI_COMMAND_IO;
	pci_hose_write_config_dword(hose, dev, PCI_COMMAND, cmdstat);
}

#if !(defined(CONFIG_PIP405) || defined (CONFIG_MIP405)) && !(defined (CONFIG_SC3))

/*
 *As is these functs get called out of flash Not a horrible
 *thing, but something to keep in mind. (no statics?)
 */
static struct pci_config_table pci_405gp_config_table[] = {
/*if VendID is 0 it terminates the table search (ie Walnut)*/
#ifdef CFG_PCI_SUBSYS_VENDORID
	{CFG_PCI_SUBSYS_VENDORID, PCI_ANY_ID, PCI_CLASS_BRIDGE_HOST,
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, pci_405gp_setup_bridge},
#endif
	{PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_DISPLAY_VGA,
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, pci_405gp_setup_vga},

	{PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_NOT_DEFINED_VGA,
	 PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, pci_405gp_setup_vga},

	{ }
};

static struct pci_controller hose = {
	fixup_irq: pci_405gp_fixup_irq,
	config_table: pci_405gp_config_table,
};

void pci_init_board(void)
{
	/*we want the ptrs to RAM not flash (ie don't use init list)*/
	hose.fixup_irq    = pci_405gp_fixup_irq;
	hose.config_table = pci_405gp_config_table;
	pci_405gp_init(&hose);
}

#endif

#endif /* CONFIG_405GP */

/*-----------------------------------------------------------------------------+
 * CONFIG_440
 *-----------------------------------------------------------------------------*/
#if defined(CONFIG_440)

static struct pci_controller ppc440_hose = {0};


int pci_440_init (struct pci_controller *hose)
{
	int reg_num = 0;

#ifndef CONFIG_DISABLE_PISE_TEST
	/*--------------------------------------------------------------------------+
	 * The PCI initialization sequence enable bit must be set ... if not abort
	 * pci setup since updating the bit requires chip reset.
	 *--------------------------------------------------------------------------*/
#if defined(CONFIG_440GX) || defined(CONFIG_440SP) || defined(CONFIG_440SPE)
	unsigned long strap;

	mfsdr(sdr_sdstp1,strap);
	if ((strap & SDR0_SDSTP1_PISE_MASK) == 0) {
		printf("PCI: SDR0_STRP1[PISE] not set.\n");
		printf("PCI: Configuration aborted.\n");
		return -1;
	}
#elif defined(CONFIG_440GP)
	unsigned long strap;

	strap = mfdcr(cpc0_strp1);
	if ((strap & CPC0_STRP1_PISE_MASK) == 0) {
		printf("PCI: CPC0_STRP1[PISE] not set.\n");
		printf("PCI: Configuration aborted.\n");
		return -1;
	}
#endif
#endif /* CONFIG_DISABLE_PISE_TEST */

	/*--------------------------------------------------------------------------+
	 * PCI controller init
	 *--------------------------------------------------------------------------*/
	hose->first_busno = 0;
	hose->last_busno = 0;

	/* PCI I/O space */
	pci_set_region(hose->regions + reg_num++,
		       0x00000000,
		       PCIX0_IOBASE,
		       0x10000,
		       PCI_REGION_IO);

	/* PCI memory space */
	pci_set_region(hose->regions + reg_num++,
		       CFG_PCI_TARGBASE,
		       CFG_PCI_MEMBASE,
#ifdef CFG_PCI_MEMSIZE
		       CFG_PCI_MEMSIZE,
#else
		       0x10000000,
#endif
		       PCI_REGION_MEM );

#if defined(CONFIG_PCI_SYS_MEM_BUS) && defined(CONFIG_PCI_SYS_MEM_PHYS) && \
	defined(CONFIG_PCI_SYS_MEM_SIZE)
	/* System memory space */
	pci_set_region(hose->regions + reg_num++,
		       CONFIG_PCI_SYS_MEM_BUS,
		       CONFIG_PCI_SYS_MEM_PHYS,
		       CONFIG_PCI_SYS_MEM_SIZE,
		       PCI_REGION_MEM | PCI_REGION_MEMORY );
#endif

	hose->region_count = reg_num;

	pci_setup_indirect(hose, PCIX0_CFGADR, PCIX0_CFGDATA);

	/* Let board change/modify hose & do initial checks */
	if (pci_pre_init (hose) == 0) {
		printf("PCI: Board-specific initialization failed.\n");
		printf("PCI: Configuration aborted.\n");
		return -1;
	}

	pci_register_hose( hose );

	/*--------------------------------------------------------------------------+
	 * PCI target init
	 *--------------------------------------------------------------------------*/
#if defined(CFG_PCI_TARGET_INIT)
	pci_target_init(hose);                /* Let board setup pci target */
#else
	out16r( PCIX0_SBSYSVID, CFG_PCI_SUBSYS_VENDORID );
	out16r( PCIX0_SBSYSID, CFG_PCI_SUBSYS_ID );
	out16r( PCIX0_CLS, 0x00060000 ); /* Bridge, host bridge */
#endif

#if defined(CONFIG_440GX) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
	out32r( PCIX0_BRDGOPT1, 0x04000060 );               /* PLB Rq pri highest   */
	out32r( PCIX0_BRDGOPT2, in32(PCIX0_BRDGOPT2) | 0x83 ); /* Enable host config, clear Timeout, ensure int src1  */
#elif defined(PCIX0_BRDGOPT1)
	out32r( PCIX0_BRDGOPT1, 0x10000060 );               /* PLB Rq pri highest   */
	out32r( PCIX0_BRDGOPT2, in32(PCIX0_BRDGOPT2) | 1 ); /* Enable host config   */
#endif

	/*--------------------------------------------------------------------------+
	 * PCI master init: default is one 256MB region for PCI memory:
	 * 0x3_00000000 - 0x3_0FFFFFFF  ==> CFG_PCI_MEMBASE
	 *--------------------------------------------------------------------------*/
#if defined(CFG_PCI_MASTER_INIT)
	pci_master_init(hose);          /* Let board setup pci master */
#else
	out32r( PCIX0_POM0SA, 0 ); /* disable */
	out32r( PCIX0_POM1SA, 0 ); /* disable */
	out32r( PCIX0_POM2SA, 0 ); /* disable */
#if defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
	out32r( PCIX0_POM0LAL, 0x10000000 );
	out32r( PCIX0_POM0LAH, 0x0000000c );
#else
	out32r( PCIX0_POM0LAL, 0x00000000 );
	out32r( PCIX0_POM0LAH, 0x00000003 );
#endif
	out32r( PCIX0_POM0PCIAL, CFG_PCI_MEMBASE );
	out32r( PCIX0_POM0PCIAH, 0x00000000 );
	out32r( PCIX0_POM0SA, 0xf0000001 ); /* 256MB, enabled */
	out32r( PCIX0_STS, in32r( PCIX0_STS ) & ~0x0000fff8 );
#endif

	/*--------------------------------------------------------------------------+
	 * PCI host configuration -- we don't make any assumptions here ... the
	 * _board_must_indicate_ what to do -- there's just too many runtime
	 * scenarios in environments like cPCI, PPMC, etc. to make a determination
	 * based on hard-coded values or state of arbiter enable.
	 *--------------------------------------------------------------------------*/
	if (is_pci_host(hose)) {
#ifdef CONFIG_PCI_SCAN_SHOW
		printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif
#if !defined(CONFIG_440EP) && !defined(CONFIG_440GR) && \
    !defined(CONFIG_440EPX) && !defined(CONFIG_440GRX)
		out16r( PCIX0_CMD, in16r( PCIX0_CMD ) | PCI_COMMAND_MASTER);
#endif
		hose->last_busno = pci_hose_scan(hose);
	}
	return hose->last_busno;
}

void pci_init_board(void)
{
	int busno;

	busno = pci_440_init (&ppc440_hose);
#if defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
	pcie_setup_hoses(busno + 1);
#endif
}

#endif /* CONFIG_440 */

#if defined(CONFIG_405EX)
void pci_init_board(void)
{
#ifdef CONFIG_PCI_SCAN_SHOW
	printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif
	pcie_setup_hoses(0);
}
#endif /* CONFIG_405EX */

#endif /* CONFIG_PCI */
