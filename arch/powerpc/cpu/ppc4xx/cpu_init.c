/*
 * (C) Copyright 2000-2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <asm/ppc4xx-emac.h>
#include <asm/processor.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/ppc4xx.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_PLL_RECONFIG
#define CONFIG_SYS_PLL_RECONFIG	0
#endif

#if defined(CONFIG_440EPX) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
static void reset_with_rli(void)
{
	u32 reg;

	/*
	 * Set reload inhibit so configuration will persist across
	 * processor resets
	 */
	mfcpr(CPR0_ICFG, reg);
	reg |= CPR0_ICFG_RLI_MASK;
	mtcpr(CPR0_ICFG, reg);

	/* Reset processor if configuration changed */
	__asm__ __volatile__ ("sync; isync");
	mtspr(SPRN_DBCR0, 0x20000000);
}
#endif

void reconfigure_pll(u32 new_cpu_freq)
{
#if defined(CONFIG_440EPX)
	int	reset_needed = 0;
	u32	reg, temp;
	u32	prbdv0, target_prbdv0,				/* CLK_PRIMBD */
		fwdva, target_fwdva, fwdvb, target_fwdvb,	/* CLK_PLLD */
		fbdv, target_fbdv, lfbdv, target_lfbdv,
		perdv0,	target_perdv0,				/* CLK_PERD */
		spcid0,	target_spcid0;				/* CLK_SPCID */

	/* Reconfigure clocks if necessary.
	 * See PPC440EPx User's Manual, sections 8.2 and 14 */
	if (new_cpu_freq == 667) {
		target_prbdv0 = 2;
		target_fwdva = 2;
		target_fwdvb = 4;
		target_fbdv = 20;
		target_lfbdv = 1;
		target_perdv0 = 4;
		target_spcid0 = 4;

		mfcpr(CPR0_PRIMBD0, reg);
		temp = (reg & PRBDV_MASK) >> 24;
		prbdv0 = temp ? temp : 8;
		if (prbdv0 != target_prbdv0) {
			reg &= ~PRBDV_MASK;
			reg |= ((target_prbdv0 == 8 ? 0 : target_prbdv0) << 24);
			mtcpr(CPR0_PRIMBD0, reg);
			reset_needed = 1;
		}

		mfcpr(CPR0_PLLD, reg);

		temp = (reg & PLLD_FWDVA_MASK) >> 16;
		fwdva = temp ? temp : 16;

		temp = (reg & PLLD_FWDVB_MASK) >> 8;
		fwdvb = temp ? temp : 8;

		temp = (reg & PLLD_FBDV_MASK) >> 24;
		fbdv = temp ? temp : 32;

		temp = (reg & PLLD_LFBDV_MASK);
		lfbdv = temp ? temp : 64;

		if (fwdva != target_fwdva || fbdv != target_fbdv || lfbdv != target_lfbdv) {
			reg &= ~(PLLD_FWDVA_MASK | PLLD_FWDVB_MASK |
				 PLLD_FBDV_MASK | PLLD_LFBDV_MASK);
			reg |= ((target_fwdva == 16 ? 0 : target_fwdva) << 16) |
				((target_fwdvb == 8 ? 0 : target_fwdvb) << 8) |
				((target_fbdv == 32 ? 0 : target_fbdv) << 24) |
				(target_lfbdv == 64 ? 0 : target_lfbdv);
			mtcpr(CPR0_PLLD, reg);
			reset_needed = 1;
		}

		mfcpr(CPR0_PERD, reg);
		perdv0 = (reg & CPR0_PERD_PERDV0_MASK) >> 24;
		if (perdv0 != target_perdv0) {
			reg &= ~CPR0_PERD_PERDV0_MASK;
			reg |= (target_perdv0 << 24);
			mtcpr(CPR0_PERD, reg);
			reset_needed = 1;
		}

		mfcpr(CPR0_SPCID, reg);
		temp = (reg & CPR0_SPCID_SPCIDV0_MASK) >> 24;
		spcid0 = temp ? temp : 4;
		if (spcid0 != target_spcid0) {
			reg &= ~CPR0_SPCID_SPCIDV0_MASK;
			reg |= ((target_spcid0 == 4 ? 0 : target_spcid0) << 24);
			mtcpr(CPR0_SPCID, reg);
			reset_needed = 1;
		}
	}

	/* Get current value of FWDVA.*/
	mfcpr(CPR0_PLLD, reg);
	temp = (reg & PLLD_FWDVA_MASK) >> 16;

	/*
	 * Check to see if FWDVA has been set to value of 1. if it has we must
	 * modify it.
	 */
	if (temp == 1) {
		/*
		 * Load register that contains current boot strapping option.
		 */
		mfcpr(CPR0_ICFG, reg);
		/*
		 * Strapping option bits (ICS) are already in correct position,
		 * only masking needed.
		 */
		reg &= CPR0_ICFG_ICS_MASK;

		if ((reg == BOOT_STRAP_OPTION_A) || (reg == BOOT_STRAP_OPTION_B) ||
		    (reg == BOOT_STRAP_OPTION_D) || (reg == BOOT_STRAP_OPTION_E)) {
			mfcpr(CPR0_PLLD, reg);

			/* Get current value of fbdv.  */
			temp = (reg & PLLD_FBDV_MASK) >> 24;
			fbdv = temp ? temp : 32;

			/* Get current value of lfbdv. */
			temp = (reg & PLLD_LFBDV_MASK);
			lfbdv = temp ? temp : 64;

			/*
			 * Get current value of FWDVA. Assign current FWDVA to
			 * new FWDVB.
			 */
			mfcpr(CPR0_PLLD, reg);
			target_fwdvb = (reg & PLLD_FWDVA_MASK) >> 16;
			fwdvb = target_fwdvb ? target_fwdvb : 8;

			/*
			 * Get current value of FWDVB. Assign current FWDVB to
			 * new FWDVA.
			 */
			target_fwdva = (reg & PLLD_FWDVB_MASK) >> 8;
			fwdva = target_fwdva ? target_fwdva : 16;

			/*
			 * Update CPR0_PLLD with switched FWDVA and FWDVB.
			 */
			reg &= ~(PLLD_FWDVA_MASK | PLLD_FWDVB_MASK |
				PLLD_FBDV_MASK | PLLD_LFBDV_MASK);
			reg |= ((fwdva == 16 ? 0 : fwdva) << 16) |
				((fwdvb == 8 ? 0 : fwdvb) << 8) |
				((fbdv == 32 ? 0 : fbdv) << 24) |
				(lfbdv == 64 ? 0 : lfbdv);
			mtcpr(CPR0_PLLD, reg);

			/* Acknowledge that a reset is required. */
			reset_needed = 1;
		}
	}

	/* Now reset the CPU if needed */
	if (reset_needed)
		reset_with_rli();
#endif

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
	u32 reg;

	/*
	 * See "9.2.1.1 Booting with Option E" in the 460EX/GT
	 * users manual
	 */
	mfcpr(CPR0_PLLC, reg);
	if ((reg & (CPR0_PLLC_RST | CPR0_PLLC_ENG)) == CPR0_PLLC_RST) {
		/*
		 * Set engage bit
		 */
		reg = (reg & ~CPR0_PLLC_RST) | CPR0_PLLC_ENG;
		mtcpr(CPR0_PLLC, reg);

		/* Now reset the CPU */
		reset_with_rli();
	}
#endif
}

#ifdef CONFIG_SYS_4xx_CHIP_21_ERRATA
void
chip_21_errata(void)
{
	/*
	 * See rev 1.09 of the 405EX/405EXr errata.  CHIP_21 says that
	 * sometimes reading the PVR and/or SDR0_ECID results in incorrect
	 * values.  Since the rev-D chip uses the SDR0_ECID bits to control
	 * internal features, that means the second PCIe or ethernet of an EX
	 * variant could fail to work.  Also, security features of both EX and
	 * EXr might be incorrectly disabled.
	 *
	 * The suggested workaround is as follows (covering rev-C and rev-D):
	 *
	 * 1.Read the PVR and SDR0_ECID3.
	 *
	 * 2.If the PVR matches an expected Revision C PVR value AND if
	 * SDR0_ECID3[12:15] is different from PVR[28:31], then processor is
	 * Revision C: continue executing the initialization code (no reset
	 * required).  else go to step 3.
	 *
	 * 3.If the PVR matches an expected Revision D PVR value AND if
	 * SDR0_ECID3[10:11] matches its expected value, then continue
	 * executing initialization code, no reset required.  else write
	 * DBCR0[RST] = 0b11 to generate a SysReset.
	 */

	u32 pvr;
	u32 pvr_28_31;
	u32 ecid3;
	u32 ecid3_10_11;
	u32 ecid3_12_15;

	/* Step 1: */
	pvr = get_pvr();
	mfsdr(SDR0_ECID3, ecid3);

	/* Step 2: */
	pvr_28_31 = pvr & 0xf;
	ecid3_10_11 = (ecid3 >> 20) & 0x3;
	ecid3_12_15 = (ecid3 >> 16) & 0xf;
	if ((pvr == CONFIG_405EX_CHIP21_PVR_REV_C) &&
			(pvr_28_31 != ecid3_12_15)) {
		/* No reset required. */
		return;
	}

	/* Step 3: */
	if ((pvr == CONFIG_405EX_CHIP21_PVR_REV_D) &&
			(ecid3_10_11 == CONFIG_405EX_CHIP21_ECID3_REV_D)) {
		/* No reset required. */
		return;
	}

	/* Reset required. */
	__asm__ __volatile__ ("sync; isync");
	mtspr(SPRN_DBCR0, 0x30000000);
}
#endif

/*
 * Breath some life into the CPU...
 *
 * Reconfigure PLL if necessary,
 * set up the memory map,
 * initialize a bunch of registers
 */
void
cpu_init_f (void)
{
#if defined(CONFIG_WATCHDOG) || defined(CONFIG_440GX) || defined(CONFIG_460EX)
	u32 val;
#endif

#ifdef CONFIG_SYS_4xx_CHIP_21_ERRATA
	chip_21_errata();
#endif

	reconfigure_pll(CONFIG_SYS_PLL_RECONFIG);

#if (defined(CONFIG_405EP) || defined (CONFIG_405EX)) && \
    !defined(CONFIG_SYS_4xx_GPIO_TABLE)
	/*
	 * GPIO0 setup (select GPIO or alternate function)
	 */
#if defined(CONFIG_SYS_GPIO0_OR)
	out32(GPIO0_OR, CONFIG_SYS_GPIO0_OR);		/* set initial state of output pins	*/
#endif
#if defined(CONFIG_SYS_GPIO0_ODR)
	out32(GPIO0_ODR, CONFIG_SYS_GPIO0_ODR);	/* open-drain select			*/
#endif
	out32(GPIO0_OSRH, CONFIG_SYS_GPIO0_OSRH);	/* output select			*/
	out32(GPIO0_OSRL, CONFIG_SYS_GPIO0_OSRL);
	out32(GPIO0_ISR1H, CONFIG_SYS_GPIO0_ISR1H);	/* input select				*/
	out32(GPIO0_ISR1L, CONFIG_SYS_GPIO0_ISR1L);
	out32(GPIO0_TSRH, CONFIG_SYS_GPIO0_TSRH);	/* three-state select			*/
	out32(GPIO0_TSRL, CONFIG_SYS_GPIO0_TSRL);
#if defined(CONFIG_SYS_GPIO0_ISR2H)
	out32(GPIO0_ISR2H, CONFIG_SYS_GPIO0_ISR2H);
	out32(GPIO0_ISR2L, CONFIG_SYS_GPIO0_ISR2L);
#endif
#if defined (CONFIG_SYS_GPIO0_TCR)
	out32(GPIO0_TCR, CONFIG_SYS_GPIO0_TCR);	/* enable output driver for outputs	*/
#endif
#endif /* CONFIG_405EP ... && !CONFIG_SYS_4xx_GPIO_TABLE */

#if defined (CONFIG_405EP)
	/*
	 * Set EMAC noise filter bits
	 */
	mtdcr(CPC0_EPCTL, CPC0_EPCTL_E0NFE | CPC0_EPCTL_E1NFE);
#endif /* CONFIG_405EP */

#if defined(CONFIG_SYS_4xx_GPIO_TABLE)
	gpio_set_chip_configuration();
#endif /* CONFIG_SYS_4xx_GPIO_TABLE */

	/*
	 * External Bus Controller (EBC) Setup
	 */
#if (defined(CONFIG_SYS_EBC_PB0AP) && defined(CONFIG_SYS_EBC_PB0CR))
#if (defined(CONFIG_405GP) || \
     defined(CONFIG_405EP) || defined(CONFIG_405EZ) || \
     defined(CONFIG_405EX) || defined(CONFIG_405))
	/*
	 * Move the next instructions into icache, since these modify the flash
	 * we are running from!
	 */
	asm volatile("	bl	0f"		::: "lr");
	asm volatile("0:	mflr	3"		::: "r3");
	asm volatile("	addi	4, 0, 14"	::: "r4");
	asm volatile("	mtctr	4"		::: "ctr");
	asm volatile("1:	icbt	0, 3");
	asm volatile("	addi	3, 3, 32"	::: "r3");
	asm volatile("	bdnz	1b"		::: "ctr", "cr0");
	asm volatile("	addis	3, 0, 0x0"	::: "r3");
	asm volatile("	ori	3, 3, 0xA000"	::: "r3");
	asm volatile("	mtctr	3"		::: "ctr");
	asm volatile("2:	bdnz	2b"		::: "ctr", "cr0");
#endif

	mtebc(PB0AP, CONFIG_SYS_EBC_PB0AP);
	mtebc(PB0CR, CONFIG_SYS_EBC_PB0CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB1AP) && defined(CONFIG_SYS_EBC_PB1CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 1))
	mtebc(PB1AP, CONFIG_SYS_EBC_PB1AP);
	mtebc(PB1CR, CONFIG_SYS_EBC_PB1CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB2AP) && defined(CONFIG_SYS_EBC_PB2CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 2))
	mtebc(PB2AP, CONFIG_SYS_EBC_PB2AP);
	mtebc(PB2CR, CONFIG_SYS_EBC_PB2CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB3AP) && defined(CONFIG_SYS_EBC_PB3CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 3))
	mtebc(PB3AP, CONFIG_SYS_EBC_PB3AP);
	mtebc(PB3CR, CONFIG_SYS_EBC_PB3CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB4AP) && defined(CONFIG_SYS_EBC_PB4CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 4))
	mtebc(PB4AP, CONFIG_SYS_EBC_PB4AP);
	mtebc(PB4CR, CONFIG_SYS_EBC_PB4CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB5AP) && defined(CONFIG_SYS_EBC_PB5CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 5))
	mtebc(PB5AP, CONFIG_SYS_EBC_PB5AP);
	mtebc(PB5CR, CONFIG_SYS_EBC_PB5CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB6AP) && defined(CONFIG_SYS_EBC_PB6CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 6))
	mtebc(PB6AP, CONFIG_SYS_EBC_PB6AP);
	mtebc(PB6CR, CONFIG_SYS_EBC_PB6CR);
#endif

#if (defined(CONFIG_SYS_EBC_PB7AP) && defined(CONFIG_SYS_EBC_PB7CR) && !(CONFIG_SYS_INIT_DCACHE_CS == 7))
	mtebc(PB7AP, CONFIG_SYS_EBC_PB7AP);
	mtebc(PB7CR, CONFIG_SYS_EBC_PB7CR);
#endif

#if defined (CONFIG_SYS_EBC_CFG)
	mtebc(EBC0_CFG, CONFIG_SYS_EBC_CFG);
#endif

#if defined(CONFIG_WATCHDOG)
	val = mfspr(SPRN_TCR);
#if defined(CONFIG_440EP) || defined(CONFIG_440GR)
	val |= 0xb8000000;      /* generate system reset after 1.34 seconds */
#elif defined(CONFIG_440EPX)
	val |= 0xb0000000;      /* generate system reset after 1.34 seconds */
#else
	val |= 0xf0000000;      /* generate system reset after 2.684 seconds */
#endif
#if defined(CONFIG_SYS_4xx_RESET_TYPE)
	val &= ~0x30000000;			/* clear WRC bits */
	val |= CONFIG_SYS_4xx_RESET_TYPE << 28;	/* set board specific WRC type */
#endif
	mtspr(SPRN_TCR, val);

	val = mfspr(SPRN_TSR);
	val |= 0x80000000;      /* enable watchdog timer */
	mtspr(SPRN_TSR, val);

	reset_4xx_watchdog();
#endif /* CONFIG_WATCHDOG */

#if defined(CONFIG_440GX)
	/* Take the GX out of compatibility mode
	 * Travis Sawyer, 9 Mar 2004
	 * NOTE: 440gx user manual inconsistency here
	 *       Compatibility mode and Ethernet Clock select are not
	 *       correct in the manual
	 */
	mfsdr(SDR0_MFR, val);
	val &= ~0x10000000;
	mtsdr(SDR0_MFR,val);
#endif /* CONFIG_440GX */

#if defined(CONFIG_460EX)
	/*
	 * Set SDR0_AHB_CFG[A2P_INCR4] (bit 24) and
	 * clear SDR0_AHB_CFG[A2P_PROT2] (bit 25) for a new 460EX errata
	 * regarding concurrent use of AHB USB OTG, USB 2.0 host and SATA
	 */
	mfsdr(SDR0_AHB_CFG, val);
	val |= 0x80;
	val &= ~0x40;
	mtsdr(SDR0_AHB_CFG, val);
	mfsdr(SDR0_USB2HOST_CFG, val);
	val &= ~0xf00;
	val |= 0x400;
	mtsdr(SDR0_USB2HOST_CFG, val);
#endif /* CONFIG_460EX */

#if defined(CONFIG_405EX) || \
    defined(CONFIG_440SP) || defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)  || \
    defined(CONFIG_460SX)
	/*
	 * Set PLB4 arbiter (Segment 0 and 1) to 4 deep pipeline read
	 */
	mtdcr(PLB4A0_ACR, (mfdcr(PLB4A0_ACR) & ~PLB4Ax_ACR_RDP_MASK) |
	      PLB4Ax_ACR_RDP_4DEEP);
	mtdcr(PLB4A1_ACR, (mfdcr(PLB4A1_ACR) & ~PLB4Ax_ACR_RDP_MASK) |
	      PLB4Ax_ACR_RDP_4DEEP);
#endif /* CONFIG_440SP/SPE || CONFIG_460EX/GT || CONFIG_405EX */

	gd = (gd_t *)(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_GBL_DATA_OFFSET);

	/* Clear initial global data */
	memset((void *)gd, 0, sizeof(gd_t));
}

/*
 * initialize higher level parts of CPU like time base and timers
 */
int cpu_init_r (void)
{
#if defined(CONFIG_405GP)
	uint pvr = get_pvr();

	/*
	 * Set edge conditioning circuitry on PPC405GPr
	 * for compatibility to existing PPC405GP designs.
	 */
	if ((pvr & 0xfffffff0) == (PVR_405GPR_RB & 0xfffffff0)) {
		mtdcr(CPC0_ECR, 0x60606000);
	}
#endif  /* defined(CONFIG_405GP) */

	return 0;
}

#if defined(CONFIG_PCI) && \
	(defined(CONFIG_440EP) || defined(CONFIG_440EPX) || \
	 defined(CONFIG_440GR) || defined(CONFIG_440GRX))
/*
 * 440EP(x)/GR(x) PCI async/sync clocking restriction:
 *
 * In asynchronous PCI mode, the synchronous PCI clock must meet
 * certain requirements. The following equation describes the
 * relationship that must be maintained between the asynchronous PCI
 * clock and synchronous PCI clock. Select an appropriate PCI:PLB
 * ratio to maintain the relationship:
 *
 * AsyncPCIClk - 1MHz <= SyncPCIclock <= (2 * AsyncPCIClk) - 1MHz
 */
static int ppc4xx_pci_sync_clock_ok(u32 sync, u32 async)
{
	if (((async - 1000000) > sync) || (sync > ((2 * async) - 1000000)))
		return 0;
	else
		return 1;
}

int ppc4xx_pci_sync_clock_config(u32 async)
{
	sys_info_t sys_info;
	u32 sync;
	int div;
	u32 reg;
	u32 spcid_val[] = {
		CPR0_SPCID_SPCIDV0_DIV1, CPR0_SPCID_SPCIDV0_DIV2,
		CPR0_SPCID_SPCIDV0_DIV3, CPR0_SPCID_SPCIDV0_DIV4 };

	get_sys_info(&sys_info);
	sync = sys_info.freqPCI;

	/*
	 * First check if the equation above is met
	 */
	if (!ppc4xx_pci_sync_clock_ok(sync, async)) {
		/*
		 * Reconfigure PCI sync clock to meet the equation.
		 * Start with highest possible PCI sync frequency
		 * (divider 1).
		 */
		for (div = 1; div <= 4; div++) {
			sync = sys_info.freqPLB / div;
			if (ppc4xx_pci_sync_clock_ok(sync, async))
			    break;
		}

		if (div <= 4) {
			mtcpr(CPR0_SPCID, spcid_val[div]);

			mfcpr(CPR0_ICFG, reg);
			reg |= CPR0_ICFG_RLI_MASK;
			mtcpr(CPR0_ICFG, reg);

			/* do chip reset */
			mtspr(SPRN_DBCR0, 0x20000000);
		} else {
			/* Impossible to configure the PCI sync clock */
			return -1;
		}
	}

	return 0;
}
#endif
