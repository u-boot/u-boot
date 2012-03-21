/*
 * (C) Copyright 2010
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include <common.h>
#include <asm/ppc4xx.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/cache.h>

#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR) || \
    defined(CONFIG_SDRAM_PPC4xx_IBM_DDR2)
#if defined(CONFIG_DDR_ECC) || defined(CONFIG_SDRAM_ECC)

#if defined(CONFIG_405EX)
/*
 * Currently only 405EX uses 16bit data bus width as an alternative
 * option to 32bit data width (SDRAM0_MCOPT1_WDTH)
 */
#define SDRAM_DATA_ALT_WIDTH	2
#else
#define SDRAM_DATA_ALT_WIDTH	8
#endif

#if defined(CONFIG_SYS_OCM_BASE)
#define CONFIG_FUNC_ISRAM_ADDR	CONFIG_SYS_OCM_BASE
#endif

#if defined(CONFIG_SYS_ISRAM_BASE)
#define CONFIG_FUNC_ISRAM_ADDR	CONFIG_SYS_ISRAM_BASE
#endif

#if !defined(CONFIG_FUNC_ISRAM_ADDR)
#error "No internal SRAM/OCM provided!"
#endif

#define force_inline inline __attribute__ ((always_inline))

static inline void machine_check_disable(void)
{
	mtmsr(mfmsr() & ~MSR_ME);
}

static inline void machine_check_enable(void)
{
	mtmsr(mfmsr() | MSR_ME);
}

/*
 * These helper functions need to be inlined, since they
 * are called from the functions running from internal SRAM.
 * SDRAM operation is forbidden at that time, so calling
 * functions in SDRAM has to be avoided.
 */
static force_inline void wait_ddr_idle(void)
{
	u32 val;

	do {
		mfsdram(SDRAM_MCSTAT, val);
	} while ((val & SDRAM_MCSTAT_IDLE_MASK) == SDRAM_MCSTAT_IDLE_NOT);
}

static force_inline void recalibrate_ddr(void)
{
	u32 val;

	/*
	 * Rewrite RQDC & RFDC to calibrate again. If this is not
	 * done, the SDRAM controller is working correctly after
	 * changing the MCOPT1_MCHK bits.
	 */
	mfsdram(SDRAM_RQDC, val);
	mtsdram(SDRAM_RQDC, val);
	mfsdram(SDRAM_RFDC, val);
	mtsdram(SDRAM_RFDC, val);
}

static force_inline void set_mcopt1_mchk(u32 bits)
{
	u32 val;

	wait_ddr_idle();
	mfsdram(SDRAM_MCOPT1, val);
	mtsdram(SDRAM_MCOPT1, (val & ~SDRAM_MCOPT1_MCHK_MASK) | bits);
	recalibrate_ddr();
}

/*
 * The next 2 functions are copied to internal SRAM/OCM and run
 * there. No function calls allowed here. No SDRAM acitivity should
 * be done here.
 */
static void inject_ecc_error(void *ptr, int par)
{
	u32 val;

	/*
	 * Taken from PPC460EX/EXr/GT users manual (Rev 1.21)
	 * 22.2.17.13 ECC Diagnostics
	 *
	 * Items 1 ... 5 are already done by now, running from RAM
	 * with ECC enabled
	 */

	out_be32(ptr, 0x00000000);
	val = in_be32(ptr);

	/* 6. Set memory controller to no error checking */
	set_mcopt1_mchk(SDRAM_MCOPT1_MCHK_NON);

	/* 7. Modify one or two bits for error simulation */
	if (par == 1)
		out_be32(ptr, in_be32(ptr) ^ 0x00000001);
	else
		out_be32(ptr, in_be32(ptr) ^ 0x00000003);

	/* 8. Wait for SDRAM idle */
	val = in_be32(ptr);
	set_mcopt1_mchk(SDRAM_MCOPT1_MCHK_CHK_REP);

	/* Wait for SDRAM idle */
	wait_ddr_idle();

	/* Continue with 9. in calling function... */
}

static void rewrite_ecc_parity(void *ptr, int par)
{
	u32 current_address = (u32)ptr;
	u32 end_address;
	u32 address_increment;
	u32 mcopt1;
	u32 val;

	/*
	 * Fill ECC parity byte again. Otherwise further accesses to
	 * the failure address will result in exceptions.
	 */

	/* Wait for SDRAM idle */
	val = in_be32(0x00000000);
	set_mcopt1_mchk(SDRAM_MCOPT1_MCHK_GEN);

	/* ECC bit set method for non-cached memory */
	mfsdram(SDRAM_MCOPT1, mcopt1);
	if ((mcopt1 & SDRAM_MCOPT1_DMWD_MASK) == SDRAM_MCOPT1_DMWD_32)
		address_increment = 4;
	else
		address_increment = SDRAM_DATA_ALT_WIDTH;
	end_address = current_address + CONFIG_SYS_CACHELINE_SIZE;

	while (current_address < end_address) {
		*((unsigned long *)current_address) = 0;
		current_address += address_increment;
	}

	set_mcopt1_mchk(SDRAM_MCOPT1_MCHK_CHK_REP);

	/* Wait for SDRAM idle */
	wait_ddr_idle();
}

static int do_ecctest(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 old_val;
	u32 val;
	u32 *ptr;
	void (*sram_func)(u32 *, int);
	int error;

	if (argc < 3) {
		return cmd_usage(cmdtp);
	}

	ptr = (u32 *)simple_strtoul(argv[1], NULL, 16);
	error = simple_strtoul(argv[2], NULL, 16);
	if ((error < 1) || (error > 2)) {
		return cmd_usage(cmdtp);
	}

	printf("Using address %p for %d bit ECC error injection\n",
	       ptr, error);

	/*
	 * Save value to restore it later on
	 */
	old_val = in_be32(ptr);

	/*
	 * Copy ECC injection function into internal SRAM/OCM
	 */
	sram_func = (void *)CONFIG_FUNC_ISRAM_ADDR;
	memcpy((void *)CONFIG_FUNC_ISRAM_ADDR, inject_ecc_error, 0x10000);

	/*
	 * Disable interrupts and exceptions before calling this
	 * function in internal SRAM/OCM
	 */
	disable_interrupts();
	machine_check_disable();
	eieio();

	/*
	 * Jump to ECC simulation function in internal SRAM/OCM
	 */
	(*sram_func)(ptr, error);

	/* 10. Read the corresponding address */
	val = in_be32(ptr);

	/*
	 * Read and print ECC status register/info:
	 * The faulting address is only known upon uncorrectable ECC
	 * errors.
	 */
	mfsdram(SDRAM_ECCES, val);
	if (val & SDRAM_ECCES_CE)
		printf("ECC: Correctable error\n");
	if (val & SDRAM_ECCES_UE) {
		printf("ECC: Uncorrectable error at 0x%02x%08x\n",
		       mfdcr(SDRAM_ERRADDULL), mfdcr(SDRAM_ERRADDLLL));
	}

	/*
	 * Clear pending interrupts/exceptions
	 */
	mtsdram(SDRAM_ECCES, 0xffffffff);
	mtdcr(SDRAM_ERRSTATLL, 0xff000000);
	set_mcsr(get_mcsr());

	/* Now enable interrupts and exceptions again */
	eieio();
	machine_check_enable();
	enable_interrupts();

	/*
	 * The ECC parity byte need to be re-written for the
	 * corresponding address. Otherwise future accesses to it
	 * will result in exceptions.
	 *
	 * Jump to ECC parity generation function
	 */
	memcpy((void *)CONFIG_FUNC_ISRAM_ADDR, rewrite_ecc_parity, 0x10000);
	(*sram_func)(ptr, 0);

	/*
	 * Restore value in corresponding address
	 */
	out_be32(ptr, old_val);

	return 0;
}

U_BOOT_CMD(
	ecctest,	3,	0,	do_ecctest,
	"Test ECC by single and double error bit injection",
	"address 1/2"
);

#endif /* defined(CONFIG_DDR_ECC) || defined(CONFIG_SDRAM_ECC) */
#endif /* defined(CONFIG_SDRAM_PPC4xx_IBM_DDR)... */
