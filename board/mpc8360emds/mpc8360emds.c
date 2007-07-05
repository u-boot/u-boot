/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 * based on board/mpc8349emds/mpc8349emds.c
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <i2c.h>
#include <spd.h>
#include <miiphy.h>
#include <command.h>
#if defined(CONFIG_PCI)
#include <pci.h>
#endif
#if defined(CONFIG_SPD_EEPROM)
#include <spd_sdram.h>
#else
#include <asm/mmu.h>
#endif
#if defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
#elif defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <libfdt_env.h>
#endif

const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* GETH1 */
	{0,  3, 1, 0, 1}, /* TxD0 */
	{0,  4, 1, 0, 1}, /* TxD1 */
	{0,  5, 1, 0, 1}, /* TxD2 */
	{0,  6, 1, 0, 1}, /* TxD3 */
	{1,  6, 1, 0, 3}, /* TxD4 */
	{1,  7, 1, 0, 1}, /* TxD5 */
	{1,  9, 1, 0, 2}, /* TxD6 */
	{1, 10, 1, 0, 2}, /* TxD7 */
	{0,  9, 2, 0, 1}, /* RxD0 */
	{0, 10, 2, 0, 1}, /* RxD1 */
	{0, 11, 2, 0, 1}, /* RxD2 */
	{0, 12, 2, 0, 1}, /* RxD3 */
	{0, 13, 2, 0, 1}, /* RxD4 */
	{1,  1, 2, 0, 2}, /* RxD5 */
	{1,  0, 2, 0, 2}, /* RxD6 */
	{1,  4, 2, 0, 2}, /* RxD7 */
	{0,  7, 1, 0, 1}, /* TX_EN */
	{0,  8, 1, 0, 1}, /* TX_ER */
	{0, 15, 2, 0, 1}, /* RX_DV */
	{0, 16, 2, 0, 1}, /* RX_ER */
	{0,  0, 2, 0, 1}, /* RX_CLK */
	{2,  9, 1, 0, 3}, /* GTX_CLK - CLK10 */
	{2,  8, 2, 0, 1}, /* GTX125 - CLK9 */
	/* GETH2 */
	{0, 17, 1, 0, 1}, /* TxD0 */
	{0, 18, 1, 0, 1}, /* TxD1 */
	{0, 19, 1, 0, 1}, /* TxD2 */
	{0, 20, 1, 0, 1}, /* TxD3 */
	{1,  2, 1, 0, 1}, /* TxD4 */
	{1,  3, 1, 0, 2}, /* TxD5 */
	{1,  5, 1, 0, 3}, /* TxD6 */
	{1,  8, 1, 0, 3}, /* TxD7 */
	{0, 23, 2, 0, 1}, /* RxD0 */
	{0, 24, 2, 0, 1}, /* RxD1 */
	{0, 25, 2, 0, 1}, /* RxD2 */
	{0, 26, 2, 0, 1}, /* RxD3 */
	{0, 27, 2, 0, 1}, /* RxD4 */
	{1, 12, 2, 0, 2}, /* RxD5 */
	{1, 13, 2, 0, 3}, /* RxD6 */
	{1, 11, 2, 0, 2}, /* RxD7 */
	{0, 21, 1, 0, 1}, /* TX_EN */
	{0, 22, 1, 0, 1}, /* TX_ER */
	{0, 29, 2, 0, 1}, /* RX_DV */
	{0, 30, 2, 0, 1}, /* RX_ER */
	{0, 31, 2, 0, 1}, /* RX_CLK */
	{2,  2, 1, 0, 2}, /* GTX_CLK = CLK10 */
	{2,  3, 2, 0, 1}, /* GTX125 - CLK4 */

	{0,  1, 3, 0, 2}, /* MDIO */
	{0,  2, 1, 0, 1}, /* MDC */

	{0,  0, 0, 0, QE_IOP_TAB_END}, /* END of table */
};

int board_early_init_f(void)
{

	u8 *bcsr = (u8 *)CFG_BCSR;
	const immap_t *immr = (immap_t *)CFG_IMMR;

	/* Enable flash write */
	bcsr[0xa] &= ~0x04;

	/* Disable G1TXCLK, G2TXCLK h/w buffers (rev.2 h/w bug workaround) */
	if (immr->sysconf.spridr == SPR_8360_REV20 ||
	    immr->sysconf.spridr == SPR_8360E_REV20 ||
	    immr->sysconf.spridr == SPR_8360_REV21 ||
	    immr->sysconf.spridr == SPR_8360E_REV21)
		bcsr[0xe] = 0x30;

	return 0;
}

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRC)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif
int fixed_sdram(void);
void sdram_init(void);

long int initdram(int board_type)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32) im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CFG_DDR_BASE & LAWBAR_BAR;
#if defined(CONFIG_SPD_EEPROM)
	msize = spd_sdram();
#else
	msize = fixed_sdram();
#endif

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRC)
	/*
	 * Initialize DDR ECC byte
	 */
	ddr_enable_ecc(msize * 1024 * 1024);
#endif
	/*
	 * Initialize SDRAM if it is on local bus.
	 */
	sdram_init();
	puts("   DDR RAM: ");
	/* return total bus SDRAM size(bytes)  -- DDR */
	return (msize * 1024 * 1024);
}

#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	msize = CFG_DDR_SIZE;
	for (ddr_size = msize << 20, ddr_size_log2 = 0;
	     (ddr_size > 1); ddr_size = ddr_size >> 1, ddr_size_log2++) {
		if (ddr_size & 1) {
			return -1;
		}
	}
	im->sysconf.ddrlaw[0].ar =
	    LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);
#if (CFG_DDR_SIZE != 256)
#warning Currenly any ddr size other than 256 is not supported
#endif
#ifdef CONFIG_DDR_II
	im->ddr.csbnds[0].csbnds = CFG_DDR_CS0_BNDS;
	im->ddr.cs_config[0] = CFG_DDR_CS0_CONFIG;
	im->ddr.timing_cfg_0 = CFG_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CFG_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CFG_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CFG_DDR_TIMING_3;
	im->ddr.sdram_cfg = CFG_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CFG_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CFG_DDR_MODE;
	im->ddr.sdram_mode2 = CFG_DDR_MODE2;
	im->ddr.sdram_interval = CFG_DDR_INTERVAL;
	im->ddr.sdram_clk_cntl = CFG_DDR_CLK_CNTL;
#else
	im->ddr.csbnds[0].csbnds = 0x00000007;
	im->ddr.csbnds[1].csbnds = 0x0008000f;

	im->ddr.cs_config[0] = CFG_DDR_CONFIG;
	im->ddr.cs_config[1] = CFG_DDR_CONFIG;

	im->ddr.timing_cfg_1 = CFG_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CFG_DDR_TIMING_2;
	im->ddr.sdram_cfg = CFG_DDR_CONTROL;

	im->ddr.sdram_mode = CFG_DDR_MODE;
	im->ddr.sdram_interval = CFG_DDR_INTERVAL;
#endif
	udelay(200);
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	return msize;
}
#endif				/*!CFG_SPD_EEPROM */

int checkboard(void)
{
	puts("Board: Freescale MPC8360EMDS\n");
	return 0;
}

/*
 * if MPC8360EMDS is soldered with SDRAM
 */
#if defined(CFG_BR2_PRELIM)  \
	&& defined(CFG_OR2_PRELIM) \
	&& defined(CFG_LBLAWBAR2_PRELIM) \
	&& defined(CFG_LBLAWAR2_PRELIM)
/*
 * Initialize SDRAM memory on the Local Bus.
 */

void sdram_init(void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile lbus83xx_t *lbc = &immap->lbus;
	uint *sdram_addr = (uint *) CFG_LBC_SDRAM_BASE;

	puts("\n   SDRAM on Local Bus: ");
	print_size(CFG_LBC_SDRAM_SIZE * 1024 * 1024, "\n");
	/*
	 * Setup SDRAM Base and Option Registers, already done in cpu_init.c
	 */
	/*setup mtrpt, lsrt and lbcr for LB bus */
	lbc->lbcr = CFG_LBC_LBCR;
	lbc->mrtpr = CFG_LBC_MRTPR;
	lbc->lsrt = CFG_LBC_LSRT;
	asm("sync");

	/*
	 * Configure the SDRAM controller Machine Mode Register.
	 */
	lbc->lsdmr = CFG_LBC_LSDMR_5;	/* Normal Operation */
	lbc->lsdmr = CFG_LBC_LSDMR_1;	/* Precharge All Banks */
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);

	/*
	 * We need do 8 times auto refresh operation.
	 */
	lbc->lsdmr = CFG_LBC_LSDMR_2;
	asm("sync");
	*sdram_addr = 0xff;	/* 1 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 2 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 3 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 4 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 5 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 6 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 7 times */
	udelay(100);
	*sdram_addr = 0xff;	/* 8 times */
	udelay(100);

	/* Mode register write operation */
	lbc->lsdmr = CFG_LBC_LSDMR_4;
	asm("sync");
	*(sdram_addr + 0xcc) = 0xff;
	udelay(100);

	/* Normal operation */
	lbc->lsdmr = CFG_LBC_LSDMR_5 | 0x40000000;
	asm("sync");
	*sdram_addr = 0xff;
	udelay(100);
}
#else
void sdram_init(void)
{
	puts("SDRAM on Local Bus is NOT available!\n");
}
#endif

#if defined(CONFIG_DDR_ECC) && defined(CONFIG_DDR_ECC_CMD)
/*
 * ECC user commands
 */
void ecc_print_status(void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ddr83xx_t *ddr = &immap->ddr;

	printf("\nECC mode: %s\n\n",
	       (ddr->sdram_cfg & SDRAM_CFG_ECC_EN) ? "ON" : "OFF");

	/* Interrupts */
	printf("Memory Error Interrupt Enable:\n");
	printf("  Multiple-Bit Error Interrupt Enable: %d\n",
	       (ddr->err_int_en & ECC_ERR_INT_EN_MBEE) ? 1 : 0);
	printf("  Single-Bit Error Interrupt Enable: %d\n",
	       (ddr->err_int_en & ECC_ERR_INT_EN_SBEE) ? 1 : 0);
	printf("  Memory Select Error Interrupt Enable: %d\n\n",
	       (ddr->err_int_en & ECC_ERR_INT_EN_MSEE) ? 1 : 0);

	/* Error disable */
	printf("Memory Error Disable:\n");
	printf("  Multiple-Bit Error Disable: %d\n",
	       (ddr->err_disable & ECC_ERROR_DISABLE_MBED) ? 1 : 0);
	printf("  Sinle-Bit Error Disable: %d\n",
	       (ddr->err_disable & ECC_ERROR_DISABLE_SBED) ? 1 : 0);
	printf("  Memory Select Error Disable: %d\n\n",
	       (ddr->err_disable & ECC_ERROR_DISABLE_MSED) ? 1 : 0);

	/* Error injection */
	printf("Memory Data Path Error Injection Mask High/Low: %08lx %08lx\n",
	       ddr->data_err_inject_hi, ddr->data_err_inject_lo);

	printf("Memory Data Path Error Injection Mask ECC:\n");
	printf("  ECC Mirror Byte: %d\n",
	       (ddr->ecc_err_inject & ECC_ERR_INJECT_EMB) ? 1 : 0);
	printf("  ECC Injection Enable: %d\n",
	       (ddr->ecc_err_inject & ECC_ERR_INJECT_EIEN) ? 1 : 0);
	printf("  ECC Error Injection Mask: 0x%02x\n\n",
	       ddr->ecc_err_inject & ECC_ERR_INJECT_EEIM);

	/* SBE counter/threshold */
	printf("Memory Single-Bit Error Management (0..255):\n");
	printf("  Single-Bit Error Threshold: %d\n",
	       (ddr->err_sbe & ECC_ERROR_MAN_SBET) >> ECC_ERROR_MAN_SBET_SHIFT);
	printf("  Single-Bit Error Counter: %d\n\n",
	       (ddr->err_sbe & ECC_ERROR_MAN_SBEC) >> ECC_ERROR_MAN_SBEC_SHIFT);

	/* Error detect */
	printf("Memory Error Detect:\n");
	printf("  Multiple Memory Errors: %d\n",
	       (ddr->err_detect & ECC_ERROR_DETECT_MME) ? 1 : 0);
	printf("  Multiple-Bit Error: %d\n",
	       (ddr->err_detect & ECC_ERROR_DETECT_MBE) ? 1 : 0);
	printf("  Single-Bit Error: %d\n",
	       (ddr->err_detect & ECC_ERROR_DETECT_SBE) ? 1 : 0);
	printf("  Memory Select Error: %d\n\n",
	       (ddr->err_detect & ECC_ERROR_DETECT_MSE) ? 1 : 0);

	/* Capture data */
	printf("Memory Error Address Capture: 0x%08lx\n", ddr->capture_address);
	printf("Memory Data Path Read Capture High/Low: %08lx %08lx\n",
	       ddr->capture_data_hi, ddr->capture_data_lo);
	printf("Memory Data Path Read Capture ECC: 0x%02x\n\n",
	       ddr->capture_ecc & CAPTURE_ECC_ECE);

	printf("Memory Error Attributes Capture:\n");
	printf(" Data Beat Number: %d\n",
	       (ddr->capture_attributes & ECC_CAPT_ATTR_BNUM) >>
	       ECC_CAPT_ATTR_BNUM_SHIFT);
	printf("  Transaction Size: %d\n",
	       (ddr->capture_attributes & ECC_CAPT_ATTR_TSIZ) >>
	       ECC_CAPT_ATTR_TSIZ_SHIFT);
	printf("  Transaction Source: %d\n",
	       (ddr->capture_attributes & ECC_CAPT_ATTR_TSRC) >>
	       ECC_CAPT_ATTR_TSRC_SHIFT);
	printf("  Transaction Type: %d\n",
	       (ddr->capture_attributes & ECC_CAPT_ATTR_TTYP) >>
	       ECC_CAPT_ATTR_TTYP_SHIFT);
	printf("  Error Information Valid: %d\n\n",
	       ddr->capture_attributes & ECC_CAPT_ATTR_VLD);
}

int do_ecc(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ddr83xx_t *ddr = &immap->ddr;
	volatile u32 val;
	u64 *addr;
	u32 count;
	register u64 *i;
	u32 ret[2];
	u32 pattern[2];
	u32 writeback[2];

	/* The pattern is written into memory to generate error */
	pattern[0] = 0xfedcba98UL;
	pattern[1] = 0x76543210UL;

	/* After injecting error, re-initialize the memory with the value */
	writeback[0] = 0x01234567UL;
	writeback[1] = 0x89abcdefUL;

	if (argc > 4) {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if (argc == 2) {
		if (strcmp(argv[1], "status") == 0) {
			ecc_print_status();
			return 0;
		} else if (strcmp(argv[1], "captureclear") == 0) {
			ddr->capture_address = 0;
			ddr->capture_data_hi = 0;
			ddr->capture_data_lo = 0;
			ddr->capture_ecc = 0;
			ddr->capture_attributes = 0;
			return 0;
		}
	}
	if (argc == 3) {
		if (strcmp(argv[1], "sbecnt") == 0) {
			val = simple_strtoul(argv[2], NULL, 10);
			if (val > 255) {
				printf("Incorrect Counter value, "
				       "should be 0..255\n");
				return 1;
			}

			val = (val << ECC_ERROR_MAN_SBEC_SHIFT);
			val |= (ddr->err_sbe & ECC_ERROR_MAN_SBET);

			ddr->err_sbe = val;
			return 0;
		} else if (strcmp(argv[1], "sbethr") == 0) {
			val = simple_strtoul(argv[2], NULL, 10);
			if (val > 255) {
				printf("Incorrect Counter value, "
				       "should be 0..255\n");
				return 1;
			}

			val = (val << ECC_ERROR_MAN_SBET_SHIFT);
			val |= (ddr->err_sbe & ECC_ERROR_MAN_SBEC);

			ddr->err_sbe = val;
			return 0;
		} else if (strcmp(argv[1], "errdisable") == 0) {
			val = ddr->err_disable;

			if (strcmp(argv[2], "+sbe") == 0) {
				val |= ECC_ERROR_DISABLE_SBED;
			} else if (strcmp(argv[2], "+mbe") == 0) {
				val |= ECC_ERROR_DISABLE_MBED;
			} else if (strcmp(argv[2], "+mse") == 0) {
				val |= ECC_ERROR_DISABLE_MSED;
			} else if (strcmp(argv[2], "+all") == 0) {
				val |= (ECC_ERROR_DISABLE_SBED |
					ECC_ERROR_DISABLE_MBED |
					ECC_ERROR_DISABLE_MSED);
			} else if (strcmp(argv[2], "-sbe") == 0) {
				val &= ~ECC_ERROR_DISABLE_SBED;
			} else if (strcmp(argv[2], "-mbe") == 0) {
				val &= ~ECC_ERROR_DISABLE_MBED;
			} else if (strcmp(argv[2], "-mse") == 0) {
				val &= ~ECC_ERROR_DISABLE_MSED;
			} else if (strcmp(argv[2], "-all") == 0) {
				val &= ~(ECC_ERROR_DISABLE_SBED |
					 ECC_ERROR_DISABLE_MBED |
					 ECC_ERROR_DISABLE_MSED);
			} else {
				printf("Incorrect err_disable field\n");
				return 1;
			}

			ddr->err_disable = val;
			__asm__ __volatile__("sync");
			__asm__ __volatile__("isync");
			return 0;
		} else if (strcmp(argv[1], "errdetectclr") == 0) {
			val = ddr->err_detect;

			if (strcmp(argv[2], "mme") == 0) {
				val |= ECC_ERROR_DETECT_MME;
			} else if (strcmp(argv[2], "sbe") == 0) {
				val |= ECC_ERROR_DETECT_SBE;
			} else if (strcmp(argv[2], "mbe") == 0) {
				val |= ECC_ERROR_DETECT_MBE;
			} else if (strcmp(argv[2], "mse") == 0) {
				val |= ECC_ERROR_DETECT_MSE;
			} else if (strcmp(argv[2], "all") == 0) {
				val |= (ECC_ERROR_DETECT_MME |
					ECC_ERROR_DETECT_MBE |
					ECC_ERROR_DETECT_SBE |
					ECC_ERROR_DETECT_MSE);
			} else {
				printf("Incorrect err_detect field\n");
				return 1;
			}

			ddr->err_detect = val;
			return 0;
		} else if (strcmp(argv[1], "injectdatahi") == 0) {
			val = simple_strtoul(argv[2], NULL, 16);

			ddr->data_err_inject_hi = val;
			return 0;
		} else if (strcmp(argv[1], "injectdatalo") == 0) {
			val = simple_strtoul(argv[2], NULL, 16);

			ddr->data_err_inject_lo = val;
			return 0;
		} else if (strcmp(argv[1], "injectecc") == 0) {
			val = simple_strtoul(argv[2], NULL, 16);
			if (val > 0xff) {
				printf("Incorrect ECC inject mask, "
				       "should be 0x00..0xff\n");
				return 1;
			}
			val |= (ddr->ecc_err_inject & ~ECC_ERR_INJECT_EEIM);

			ddr->ecc_err_inject = val;
			return 0;
		} else if (strcmp(argv[1], "inject") == 0) {
			val = ddr->ecc_err_inject;

			if (strcmp(argv[2], "en") == 0)
				val |= ECC_ERR_INJECT_EIEN;
			else if (strcmp(argv[2], "dis") == 0)
				val &= ~ECC_ERR_INJECT_EIEN;
			else
				printf("Incorrect command\n");

			ddr->ecc_err_inject = val;
			__asm__ __volatile__("sync");
			__asm__ __volatile__("isync");
			return 0;
		} else if (strcmp(argv[1], "mirror") == 0) {
			val = ddr->ecc_err_inject;

			if (strcmp(argv[2], "en") == 0)
				val |= ECC_ERR_INJECT_EMB;
			else if (strcmp(argv[2], "dis") == 0)
				val &= ~ECC_ERR_INJECT_EMB;
			else
				printf("Incorrect command\n");

			ddr->ecc_err_inject = val;
			return 0;
		}
	}
	if (argc == 4) {
		if (strcmp(argv[1], "testdw") == 0) {
			addr = (u64 *) simple_strtoul(argv[2], NULL, 16);
			count = simple_strtoul(argv[3], NULL, 16);

			if ((u32) addr % 8) {
				printf("Address not alligned on "
				       "double word boundary\n");
				return 1;
			}
			disable_interrupts();

			for (i = addr; i < addr + count; i++) {

				/* enable injects */
				ddr->ecc_err_inject |= ECC_ERR_INJECT_EIEN;
				__asm__ __volatile__("sync");
				__asm__ __volatile__("isync");

				/* write memory location injecting errors */
				ppcDWstore((u32 *) i, pattern);
				__asm__ __volatile__("sync");

				/* disable injects */
				ddr->ecc_err_inject &= ~ECC_ERR_INJECT_EIEN;
				__asm__ __volatile__("sync");
				__asm__ __volatile__("isync");

				/* read data, this generates ECC error */
				ppcDWload((u32 *) i, ret);
				__asm__ __volatile__("sync");

				/* re-initialize memory, double word write the location again,
				 * generates new ECC code this time */
				ppcDWstore((u32 *) i, writeback);
				__asm__ __volatile__("sync");
			}
			enable_interrupts();
			return 0;
		}
		if (strcmp(argv[1], "testword") == 0) {
			addr = (u64 *) simple_strtoul(argv[2], NULL, 16);
			count = simple_strtoul(argv[3], NULL, 16);

			if ((u32) addr % 8) {
				printf("Address not alligned on "
				       "double word boundary\n");
				return 1;
			}
			disable_interrupts();

			for (i = addr; i < addr + count; i++) {

				/* enable injects */
				ddr->ecc_err_inject |= ECC_ERR_INJECT_EIEN;
				__asm__ __volatile__("sync");
				__asm__ __volatile__("isync");

				/* write memory location injecting errors */
				*(u32 *) i = 0xfedcba98UL;
				__asm__ __volatile__("sync");

				/* sub double word write,
				 * bus will read-modify-write,
				 * generates ECC error */
				*((u32 *) i + 1) = 0x76543210UL;
				__asm__ __volatile__("sync");

				/* disable injects */
				ddr->ecc_err_inject &= ~ECC_ERR_INJECT_EIEN;
				__asm__ __volatile__("sync");
				__asm__ __volatile__("isync");

				/* re-initialize memory,
				 * double word write the location again,
				 * generates new ECC code this time */
				ppcDWstore((u32 *) i, writeback);
				__asm__ __volatile__("sync");
			}
			enable_interrupts();
			return 0;
		}
	}
	printf("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(ecc, 4, 0, do_ecc,
	   "ecc     - support for DDR ECC features\n",
	   "status              - print out status info\n"
	   "ecc captureclear        - clear capture regs data\n"
	   "ecc sbecnt <val>        - set Single-Bit Error counter\n"
	   "ecc sbethr <val>        - set Single-Bit Threshold\n"
	   "ecc errdisable <flag>   - clear/set disable Memory Error Disable, flag:\n"
	   "  [-|+]sbe - Single-Bit Error\n"
	   "  [-|+]mbe - Multiple-Bit Error\n"
	   "  [-|+]mse - Memory Select Error\n"
	   "  [-|+]all - all errors\n"
	   "ecc errdetectclr <flag> - clear Memory Error Detect, flag:\n"
	   "  mme - Multiple Memory Errors\n"
	   "  sbe - Single-Bit Error\n"
	   "  mbe - Multiple-Bit Error\n"
	   "  mse - Memory Select Error\n"
	   "  all - all errors\n"
	   "ecc injectdatahi <hi>  - set Memory Data Path Error Injection Mask High\n"
	   "ecc injectdatalo <lo>  - set Memory Data Path Error Injection Mask Low\n"
	   "ecc injectecc <ecc>    - set ECC Error Injection Mask\n"
	   "ecc inject <en|dis>    - enable/disable error injection\n"
	   "ecc mirror <en|dis>    - enable/disable mirror byte\n"
	   "ecc testdw <addr> <cnt>  - test mem region with double word access:\n"
	   "  - enables injects\n"
	   "  - writes pattern injecting errors with double word access\n"
	   "  - disables injects\n"
	   "  - reads pattern back with double word access, generates error\n"
	   "  - re-inits memory\n"
	   "ecc testword <addr> <cnt>  - test mem region with word access:\n"
	   "  - enables injects\n"
	   "  - writes pattern injecting errors with word access\n"
	   "  - writes pattern with word access, generates error\n"
	   "  - disables injects\n" "  - re-inits memory");
#endif				/* if defined(CONFIG_DDR_ECC) && defined(CONFIG_DDR_ECC_CMD) */

#if (defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)) \
     && defined(CONFIG_OF_BOARD_SETUP)

/*
 * Prototypes of functions that we use.
 */
void ft_cpu_setup(void *blob, bd_t *bd);

#ifdef CONFIG_PCI
void ft_pci_setup(void *blob, bd_t *bd);
#endif

void
ft_board_setup(void *blob, bd_t *bd)
{
#if defined(CONFIG_OF_LIBFDT)
	int nodeoffset;
	int tmp[2];

	nodeoffset = fdt_find_node_by_path(fdt, "/memory");
	if (nodeoffset >= 0) {
		tmp[0] = cpu_to_be32(bd->bi_memstart);
		tmp[1] = cpu_to_be32(bd->bi_memsize);
		fdt_setprop(fdt, nodeoffset, "reg", tmp, sizeof(tmp));
	}
#else
	u32 *p;
	int len;

	p = ft_get_prop(blob, "/memory/reg", &len);
	if (p != NULL) {
		*p++ = cpu_to_be32(bd->bi_memstart);
		*p = cpu_to_be32(bd->bi_memsize);
	}
#endif

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif
	ft_cpu_setup(blob, bd);
}
#endif /* CONFIG_OF_x */
