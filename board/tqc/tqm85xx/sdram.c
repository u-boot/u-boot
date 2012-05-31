
/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/processor.h>
#include <asm/mmu.h>

struct sdram_conf_s {
	unsigned long size;
	unsigned long reg;
#ifdef CONFIG_TQM8548
	unsigned long refresh;
#endif /* CONFIG_TQM8548 */
};

typedef struct sdram_conf_s sdram_conf_t;

#ifdef CONFIG_TQM8548
#ifdef CONFIG_TQM8548_AG
sdram_conf_t ddr_cs_conf[] = {
	{(1024 << 20), 0x80044202, 0x0002D000},	/* 1024MB, 14x10(4)	*/
	{ (512 << 20), 0x80044102, 0x0001A000},	/*  512MB, 13x10(4)	*/
	{ (256 << 20), 0x80040102, 0x00014000},	/*  256MB, 13x10(4)	*/
	{ (128 << 20), 0x80040101, 0x0000C000},	/*  128MB, 13x9(4)	*/
};
#else /* !CONFIG_TQM8548_AG */
sdram_conf_t ddr_cs_conf[] = {
	{(512 << 20), 0x80044102, 0x0001A000},	/* 512MB, 13x10(4)	*/
	{(256 << 20), 0x80040102, 0x00014000},	/* 256MB, 13x10(4)	*/
	{(128 << 20), 0x80040101, 0x0000C000},	/* 128MB, 13x9(4)	*/
};
#endif /* CONFIG_TQM8548_AG */
#else /* !CONFIG_TQM8548 */
sdram_conf_t ddr_cs_conf[] = {
	{(512 << 20), 0x80000202},	/* 512MB, 14x10(4)	*/
	{(256 << 20), 0x80000102},	/* 256MB, 13x10(4)	*/
	{(128 << 20), 0x80000101},	/* 128MB, 13x9(4)	*/
	{( 64 << 20), 0x80000001},	/*  64MB, 12x9(4)	*/
};
#endif /* CONFIG_TQM8548 */

#define	N_DDR_CS_CONF (sizeof(ddr_cs_conf) / sizeof(ddr_cs_conf[0]))

int cas_latency (void);
static phys_size_t sdram_setup(int);

/*
 * Autodetect onboard DDR SDRAM on 85xx platforms
 *
 * NOTE: Some of the hardcoded values are hardware dependant,
 *       so this should be extended for other future boards
 *       using this routine!
 */
phys_size_t fixed_sdram(void)
{
	int casl = 0;
	phys_size_t dram_size = 0;

	casl = cas_latency();
	dram_size = sdram_setup(casl);
	if ((dram_size == 0) && (casl != CONFIG_DDR_DEFAULT_CL)) {
		/*
		 * Try again with default CAS latency
		 */
		printf("Problem with CAS lantency, using default CL %d/10!\n",
		       CONFIG_DDR_DEFAULT_CL);
		dram_size = sdram_setup(CONFIG_DDR_DEFAULT_CL);
		puts("       ");
	}
	return dram_size;
}

static phys_size_t sdram_setup(int casl)
{
	int i;
	volatile ccsr_ddr_t *ddr = (void *)(CONFIG_SYS_MPC85xx_DDR_ADDR);
#ifdef CONFIG_TQM8548
	volatile ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#if defined(CONFIG_TQM8548_AG) || defined(CONFIG_TQM8548_BE)
	volatile ccsr_local_ecm_t *ecm = (void *)(CONFIG_SYS_MPC85xx_ECM_ADDR);
#endif
#else /* !CONFIG_TQM8548 */
	unsigned long cfg_ddr_timing1;
	unsigned long cfg_ddr_mode;
#endif /* CONFIG_TQM8548 */

	/*
	 * Disable memory controller.
	 */
	ddr->cs0_config = 0;
	ddr->sdram_cfg = 0;

#ifdef CONFIG_TQM8548
	/* Timing and refresh settings for DDR2-533 and below */

	ddr->cs0_bnds = (ddr_cs_conf[0].size - 1) >> 24;
	ddr->cs0_config = ddr_cs_conf[0].reg;
	ddr->timing_cfg_3 = 0x00020000;

	/* TIMING CFG 1, 533MHz
	 * PRETOACT: 4 Clocks
	 * ACTTOPRE: 12 Clocks
	 * ACTTORW:  4 Clocks
	 * CASLAT:   4 Clocks
	 * REFREC:   EXT_REFREC:REFREC 53 Clocks
	 * WRREC:    4 Clocks
	 * ACTTOACT: 3 Clocks
	 * WRTORD:   2 Clocks
	 */
	ddr->timing_cfg_1 = 0x4C47D432;

	/* TIMING CFG 2, 533MHz
	 * ADD_LAT:       3 Clocks
	 * CPO:           READLAT + 1
	 * WR_LAT:        3 Clocks
	 * RD_TO_PRE:     2 Clocks
	 * WR_DATA_DELAY: 1/2 Clock
	 * CKE_PLS:       3 Clock
	 * FOUR_ACT:      14 Clocks
	 */
	ddr->timing_cfg_2 = 0x331848CE;

	/* DDR SDRAM Mode, 533MHz
	 * MRS:          Extended Mode Register
	 * OUT:          Outputs enabled
	 * RDQS:         no
	 * DQS:          enabled
	 * OCD:          default state
	 * RTT:          75 Ohms
	 * Posted CAS:   3 Clocks
	 * ODS:          reduced strength
	 * DLL:          enabled
	 * MR:           Mode Register
	 * PD:           fast exit
	 * WR:           4 Clocks
	 * DLL:          no DLL reset
	 * TM:           normal
	 * CAS latency:  4 Clocks
	 * BT:           sequential
	 * Burst length: 4
	 */
	ddr->sdram_mode = 0x439E0642;

	/* DDR SDRAM Interval, 533MHz
	 * REFINT:  1040 Clocks
	 * BSTOPRE: 256
	 */
	ddr->sdram_interval = (1040 << 16) | 0x100;

	/*
	 * Workaround for erratum DDR19 according to MPC8548 Device Errata
	 * document, Rev. 1: DDR IO receiver must be set to an acceptable
	 * bias point by modifying a hidden register.
	 */
	if (SVR_REV (get_svr ()) < 0x21)
		gur->ddrioovcr = 0x90000000;	/* enable, VSEL 1.8V */

	/* DDR SDRAM CFG 2
	 * FRC_SR:      normal mode
	 * SR_IE:       no self-refresh interrupt
	 * DLL_RST_DIS: don't care, leave at reset value
	 * DQS_CFG:     differential DQS signals
	 * ODT_CFG:     assert ODT to internal IOs only during reads to DRAM
	 * LVWx_CFG:    don't care, leave at reset value
	 * NUM_PR:      1 refresh will be issued at a time
	 * DM_CFG:      don't care, leave at reset value
	 * D_INIT:      no data initialization
	 */
	ddr->sdram_cfg_2 = 0x04401000;

	/* DDR SDRAM MODE 2
	 * MRS: Extended Mode Register 2
	 */
	ddr->sdram_mode_2 = 0x8000C000;

	/* DDR SDRAM CLK CNTL
	 * CLK_ADJUST: 1/2 Clock 0x02000000
	 * CLK_ADJUST: 5/8 Clock 0x02800000
	 */
	ddr->sdram_clk_cntl = 0x02800000;

	/* wait for clock stabilization */
	asm ("sync;isync;msync");
	udelay (1000);

#if defined(CONFIG_TQM8548_AG) || defined(CONFIG_TQM8548_BE)
	/*
	 * Workaround for erratum DDR20 according to MPC8548 Device Errata
	 * document, Rev. 1: "CKE signal may not function correctly after
	 * assertion of HRESET"
	 */

	/* 1. Configure DDR register as is done in normal DDR configuration.
	 *    Do not set DDR_SDRAM_CFG[MEM_EN].
	 *
	 * 2. Set reserved bit EEBACR[3] at offset 0x1000
	 */
	ecm->eebacr |= 0x10000000;

	/*
	 * 3. Before DDR_SDRAM_CFG[MEM_EN] is set, write DDR_SDRAM_CFG_2[D_INIT]
	 *
	 * DDR_SDRAM_CFG_2:
	 * FRC_SR:      normal mode
	 * SR_IE:       no self-refresh interrupt
	 * DLL_RST_DIS: don't care, leave at reset value
	 * DQS_CFG:     differential DQS signals
	 * ODT_CFG:     assert ODT to internal IOs only during reads to DRAM
	 * LVWx_CFG:    don't care, leave at reset value
	 * NUM_PR:      1 refresh will be issued at a time
	 * DM_CFG:      don't care, leave at reset value
	 * D_INIT:      enable data initialization
	 */
	ddr->sdram_cfg_2 |= 0x00000010;

	/*
	 * 4. Before DDR_SDRAM_CFG[MEM_EN] set, write D3[21] to disable data
	 *    training
	 */
	ddr->debug[2] |= 0x00000400;

	/*
	 * 5. Wait 200 micro-seconds
	 */
	udelay (200);

	/*
	 * 6. Set DDR_SDRAM_CFG[MEM_EN]
	 *
	 * BTW, initialize DDR_SDRAM_CFG:
	 * MEM_EN:       enabled
	 * SREN:         don't care, leave at reset value
	 * ECC_EN:       no error report
	 * RD_EN:        no registered DIMMs
	 * SDRAM_TYPE:   DDR2
	 * DYN_PWR:      no power management
	 * 32_BE:        don't care, leave at reset value
	 * 8_BE:         4 beat burst
	 * NCAP:         don't care, leave at reset value
	 * 2T_EN:        1T Timing
	 * BA_INTLV_CTL: no interleaving
	 * x32_EN:       x16 organization
	 * PCHB8:        MA[10] for auto-precharge
	 * HSE:          half strength for single and 2-layer stacks
	 *               (full strength for 3- and 4-layer stacks not
	 *               yet considered)
	 * MEM_HALT:     no halt
	 * BI:           automatic initialization
	 */
	ddr->sdram_cfg = 0x83000008;

	/*
	 * 7. Poll DDR_SDRAM_CFG_2[D_INIT] until it is cleared by hardware
	 */
	asm ("sync;isync;msync");
	while (ddr->sdram_cfg_2 & 0x00000010)
		asm ("eieio");

	/*
	 * 8. Clear D3[21] to re-enable data training
	 */
	ddr->debug[2] &= ~0x00000400;

	/*
	 * 9. Set D2(21) to force data training to run
	 */
	ddr->debug[1] |= 0x00000400;

	/*
	 * 10. Poll on D2[21] until it is cleared by hardware
	 */
	asm ("sync;isync;msync");
	while (ddr->debug[1] & 0x00000400)
		asm ("eieio");

	/*
	 * 11. Clear reserved bit EEBACR[3] at offset 0x1000
	 */
	ecm->eebacr &= ~0x10000000;

#else /* !(CONFIG_TQM8548_AG || CONFIG_TQM8548_BE) */

	/* DDR SDRAM CLK CNTL
	 * MEM_EN:       enabled
	 * SREN:         don't care, leave at reset value
	 * ECC_EN:       no error report
	 * RD_EN:        no register DIMMs
	 * SDRAM_TYPE:   DDR2
	 * DYN_PWR:      no power management
	 * 32_BE:        don't care, leave at reset value
	 * 8_BE:         4 beat burst
	 * NCAP:         don't care, leave at reset value
	 * 2T_EN:        1T Timing
	 * BA_INTLV_CTL: no interleaving
	 * x32_EN:       x16 organization
	 * PCHB8:        MA[10] for auto-precharge
	 * HSE:          half strength for single and 2-layer stacks
	 * (full strength for 3- and 4-layer stacks no yet considered)
	 * MEM_HALT:     no halt
	 * BI:           automatic initialization
	 */
	ddr->sdram_cfg = 0x83000008;

#endif /* CONFIG_TQM8548_AG || CONFIG_TQM8548_BE */

	asm ("sync; isync; msync");
	udelay (1000);
#else /* !CONFIG_TQM8548 */
	switch (casl) {
	case 20:
		cfg_ddr_timing1 = 0x47405331 | (3 << 16);
		cfg_ddr_mode = 0x40020002 | (2 << 4);
		break;

	case 25:
		cfg_ddr_timing1 = 0x47405331 | (4 << 16);
		cfg_ddr_mode = 0x40020002 | (6 << 4);
		break;

	case 30:
	default:
		cfg_ddr_timing1 = 0x47405331 | (5 << 16);
		cfg_ddr_mode = 0x40020002 | (3 << 4);
		break;
	}

	ddr->cs0_bnds = (ddr_cs_conf[0].size - 1) >> 24;
	ddr->cs0_config = ddr_cs_conf[0].reg;
	ddr->timing_cfg_1 = cfg_ddr_timing1;
	ddr->timing_cfg_2 = 0x00000800;		/* P9-45,may need tuning */
	ddr->sdram_mode = cfg_ddr_mode;
	ddr->sdram_interval = 0x05160100;	/* autocharge,no open page */
	ddr->err_disable = 0x0000000D;

	asm ("sync; isync; msync");
	udelay (1000);

	ddr->sdram_cfg = 0xc2000000;		/* unbuffered,no DYN_PWR */
	asm ("sync; isync; msync");
	udelay (1000);
#endif /* CONFIG_TQM8548 */

	for (i = 0; i < N_DDR_CS_CONF; i++) {
		ddr->cs0_config = ddr_cs_conf[i].reg;

		if (get_ram_size (0, ddr_cs_conf[i].size) ==
		    ddr_cs_conf[i].size) {
			/*
			 * size detected -> set Chip Select Bounds Register
			 */
			ddr->cs0_bnds = (ddr_cs_conf[i].size - 1) >> 24;

			break;
		}
	}

#ifdef CONFIG_TQM8548
	if (i < N_DDR_CS_CONF) {
		/* Adjust refresh rate for DDR2 */

		ddr->timing_cfg_3 = ddr_cs_conf[i].refresh & 0x00070000;

		ddr->timing_cfg_1 = (ddr->timing_cfg_1 & 0xFFFF0FFF) |
		    (ddr_cs_conf[i].refresh & 0x0000F000);

		return ddr_cs_conf[i].size;
	}
#endif /* CONFIG_TQM8548 */

	/* return size if detected, else return 0 */
	return (i < N_DDR_CS_CONF) ? ddr_cs_conf[i].size : 0;
}

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	printf ("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf ("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf ("SDRAM test passed.\n");
	return 0;
}
#endif
