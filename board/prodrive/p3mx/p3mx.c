/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Based on original work by
 *	Roel Loeffen, (C) Copyright 2006 Prodrive B.V.
 *	Josh Huber, (C) Copyright 2001 Mission Critical Linux, Inc.
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
 * modifications for the DB64360 eval board based by Ingo.Assmus@keymile.com
 * modifications for the cpci750 by reinhard.arlt@esd-electronics.com
 * modifications for the P3M750 by roel.loeffen@prodrive.nl
 */

/*
 * p3m750.c - main board support/init for the Prodrive p3m750/p3m7448.
 */

#include <common.h>
#include <74xx_7xx.h>
#include "../../Marvell/include/memory.h"
#include "../../Marvell/include/pci.h"
#include "../../Marvell/include/mv_gen_reg.h"
#include <net.h>
#include <i2c.h>

#include "eth.h"
#include "mpsc.h"
#include "64460.h"
#include "mv_regs.h"
#include "p3mx.h"

DECLARE_GLOBAL_DATA_PTR;

#undef	DEBUG
/*#define	DEBUG */

#ifdef CONFIG_PCI
#define	MAP_PCI
#endif /* of CONFIG_PCI */

#ifdef DEBUG
#define DP(x) x
#else
#define DP(x)
#endif

extern flash_info_t flash_info[];

/* ------------------------------------------------------------------------- */

/* this is the current GT register space location */
/* it starts at CONFIG_SYS_DFL_GT_REGS but moves later to CONFIG_SYS_GT_REGS */

/* Unfortunately, we cant change it while we are in flash, so we initialize it
 * to the "final" value. This means that any debug_led calls before
 * board_early_init_f wont work right (like in cpu_init_f).
 * See also my_remap_gt_regs below. (NTL)
 */

void board_prebootm_init (void);
unsigned int INTERNAL_REG_BASE_ADDR = CONFIG_SYS_GT_REGS;
int display_mem_map (void);
void set_led(int);

/* ------------------------------------------------------------------------- */

/*
 * This is a version of the GT register space remapping function that
 * doesn't touch globals (meaning, it's ok to run from flash.)
 *
 * Unfortunately, this has the side effect that a writable
 * INTERNAL_REG_BASE_ADDR is impossible. Oh well.
 */

void my_remap_gt_regs (u32 cur_loc, u32 new_loc)
{
	u32 temp;

	/* check and see if it's already moved */
	temp = in_le32 ((u32 *) (new_loc + INTERNAL_SPACE_DECODE));
	if ((temp & 0xffff) == new_loc >> 16)
		return;

	temp = (in_le32 ((u32 *) (cur_loc + INTERNAL_SPACE_DECODE)) &
		0xffff0000) | (new_loc >> 16);

	out_le32 ((u32 *) (cur_loc + INTERNAL_SPACE_DECODE), temp);

	while (GTREGREAD (INTERNAL_SPACE_DECODE) != temp);
}

#ifdef CONFIG_PCI

static void gt_pci_config (void)
{
	unsigned int stat;
	unsigned int val = 0x00fff864;	/* DINK32: BusNum 23:16,  DevNum 15:11, */
					/* FuncNum 10:8, RegNum 7:2 */

	/*
	 * In PCIX mode devices provide their own bus and device numbers.
	 * We query the Discovery II's
	 * config registers by writing ones to the bus and device.
	 * We then update the Virtual register with the correct value for the
	 * bus and device.
	 */
	if ((GTREGREAD (PCI_0_MODE) & (BIT4 | BIT5)) != 0) {	/* if  PCI-X */
		GT_REG_WRITE (PCI_0_CONFIG_ADDR, BIT31 | val);

		GT_REG_READ (PCI_0_CONFIG_DATA_VIRTUAL_REG, &stat);

		GT_REG_WRITE (PCI_0_CONFIG_ADDR, BIT31 | val);
		GT_REG_WRITE (PCI_0_CONFIG_DATA_VIRTUAL_REG,
			      (stat & 0xffff0000) | CONFIG_SYS_PCI_IDSEL);

	}
	if ((GTREGREAD (PCI_1_MODE) & (BIT4 | BIT5)) != 0) {	/* if  PCI-X */
		GT_REG_WRITE (PCI_1_CONFIG_ADDR, BIT31 | val);
		GT_REG_READ (PCI_1_CONFIG_DATA_VIRTUAL_REG, &stat);

		GT_REG_WRITE (PCI_1_CONFIG_ADDR, BIT31 | val);
		GT_REG_WRITE (PCI_1_CONFIG_DATA_VIRTUAL_REG,
			      (stat & 0xffff0000) | CONFIG_SYS_PCI_IDSEL);
	}

	/* Enable master */
	PCI_MASTER_ENABLE (0, SELF);
	PCI_MASTER_ENABLE (1, SELF);

	/* Enable PCI0/1 Mem0 and IO 0 disable all others */
	GT_REG_READ (BASE_ADDR_ENABLE, &stat);
	stat |= (1 << 11) | (1 << 12) | (1 << 13) | (1 << 16) | (1 << 17) |
		(1 << 18);
	stat &= ~((1 << 9) | (1 << 10) | (1 << 14) | (1 << 15));
	GT_REG_WRITE (BASE_ADDR_ENABLE, stat);

	/* ronen:
	 * add write to pci remap registers for 64460.
	 * in 64360 when writing to pci base go and overide remap automaticaly,
	 * in 64460 it doesn't
	 */
	GT_REG_WRITE (PCI_0_IO_BASE_ADDR, CONFIG_SYS_PCI0_IO_SPACE >> 16);
	GT_REG_WRITE (PCI_0I_O_ADDRESS_REMAP, CONFIG_SYS_PCI0_IO_SPACE_PCI >> 16);
	GT_REG_WRITE (PCI_0_IO_SIZE, (CONFIG_SYS_PCI0_IO_SIZE - 1) >> 16);

	GT_REG_WRITE (PCI_0_MEMORY0_BASE_ADDR, CONFIG_SYS_PCI0_MEM_BASE >> 16);
	GT_REG_WRITE (PCI_0MEMORY0_ADDRESS_REMAP, CONFIG_SYS_PCI0_MEM_BASE >> 16);
	GT_REG_WRITE (PCI_0_MEMORY0_SIZE, (CONFIG_SYS_PCI0_MEM_SIZE - 1) >> 16);

	GT_REG_WRITE (PCI_1_IO_BASE_ADDR, CONFIG_SYS_PCI1_IO_SPACE >> 16);
	GT_REG_WRITE (PCI_1I_O_ADDRESS_REMAP, CONFIG_SYS_PCI1_IO_SPACE_PCI >> 16);
	GT_REG_WRITE (PCI_1_IO_SIZE, (CONFIG_SYS_PCI1_IO_SIZE - 1) >> 16);

	GT_REG_WRITE (PCI_1_MEMORY0_BASE_ADDR, CONFIG_SYS_PCI1_MEM_BASE >> 16);
	GT_REG_WRITE (PCI_1MEMORY0_ADDRESS_REMAP, CONFIG_SYS_PCI1_MEM_BASE >> 16);
	GT_REG_WRITE (PCI_1_MEMORY0_SIZE, (CONFIG_SYS_PCI1_MEM_SIZE - 1) >> 16);

	/* PCI interface settings */
	/* Timeout set to retry forever */
	GT_REG_WRITE (PCI_0TIMEOUT_RETRY, 0x0);
	GT_REG_WRITE (PCI_1TIMEOUT_RETRY, 0x0);

	/* ronen - enable only CS0 and Internal reg!! */
	GT_REG_WRITE (PCI_0BASE_ADDRESS_REGISTERS_ENABLE, 0xfffffdfe);
	GT_REG_WRITE (PCI_1BASE_ADDRESS_REGISTERS_ENABLE, 0xfffffdfe);

	/* ronen:
	 * update the pci internal registers base address.
	 */
#ifdef MAP_PCI
	for (stat = 0; stat <= PCI_HOST1; stat++)
		pciWriteConfigReg (stat,
				   PCI_INTERNAL_REGISTERS_MEMORY_MAPPED_BASE_ADDRESS,
				   SELF, CONFIG_SYS_GT_REGS);
#endif

}
#endif

/* Setup CPU interface paramaters */
static void gt_cpu_config (void)
{
	cpu_t cpu = get_cpu_type ();
	ulong tmp;

	/* cpu configuration register */
	tmp = GTREGREAD (CPU_CONFIGURATION);
	/* set the SINGLE_CPU bit  see MV64460 */
#ifndef CONFIG_SYS_GT_DUAL_CPU		/* SINGLE_CPU seems to cause JTAG problems */
	tmp |= CPU_CONF_SINGLE_CPU;
#endif
	tmp &= ~CPU_CONF_AACK_DELAY_2;
	tmp |= CPU_CONF_DP_VALID;
	tmp |= CPU_CONF_AP_VALID;
	tmp |= CPU_CONF_PIPELINE;
	GT_REG_WRITE (CPU_CONFIGURATION, tmp);	/* Marvell (VXWorks) writes 0x20220FF */

	/* CPU master control register */
	tmp = GTREGREAD (CPU_MASTER_CONTROL);
	tmp |= CPU_MAST_CTL_ARB_EN;

	if ((cpu == CPU_7400) ||
	    (cpu == CPU_7410) || (cpu == CPU_7455) || (cpu == CPU_7450)) {

		tmp |= CPU_MAST_CTL_CLEAN_BLK;
		tmp |= CPU_MAST_CTL_FLUSH_BLK;

	} else {
		/* cleanblock must be cleared for CPUs
		 * that do not support this command (603e, 750)
		 * see Res#1 */
		tmp &= ~CPU_MAST_CTL_CLEAN_BLK;
		tmp &= ~CPU_MAST_CTL_FLUSH_BLK;
	}
	GT_REG_WRITE (CPU_MASTER_CONTROL, tmp);
}

/*
 * board_early_init_f.
 *
 * set up gal. device mappings, etc.
 */
int board_early_init_f (void)
{
	/* set up the GT the way the kernel wants it
	 * the call to move the GT register space will obviously
	 * fail if it has already been done, but we're going to assume
	 * that if it's not at the power-on location, it's where we put
	 * it last time. (huber)
	 */
	my_remap_gt_regs (CONFIG_SYS_DFL_GT_REGS, CONFIG_SYS_GT_REGS);

#ifdef CONFIG_PCI
	gt_pci_config ();
#endif
	/* mask all external interrupt sources */
	GT_REG_WRITE (CPU_INTERRUPT_MASK_REGISTER_LOW, 0);
	GT_REG_WRITE (CPU_INTERRUPT_MASK_REGISTER_HIGH, 0);
	/* new in >MV6436x */
	GT_REG_WRITE (CPU_INTERRUPT_1_MASK_REGISTER_LOW, 0);
	GT_REG_WRITE (CPU_INTERRUPT_1_MASK_REGISTER_HIGH, 0);
	/* --------------------- */
	GT_REG_WRITE (PCI_0INTERRUPT_CAUSE_MASK_REGISTER_LOW, 0);
	GT_REG_WRITE (PCI_0INTERRUPT_CAUSE_MASK_REGISTER_HIGH, 0);
	GT_REG_WRITE (PCI_1INTERRUPT_CAUSE_MASK_REGISTER_LOW, 0);
	GT_REG_WRITE (PCI_1INTERRUPT_CAUSE_MASK_REGISTER_HIGH, 0);

	/* Device and Boot bus settings
	 */
	memoryMapDeviceSpace(DEVICE0, 0, 0);
	GT_REG_WRITE(DEVICE_BANK0PARAMETERS, 0);
	memoryMapDeviceSpace(DEVICE1, 0, 0);
	GT_REG_WRITE(DEVICE_BANK1PARAMETERS, 0);
	memoryMapDeviceSpace(DEVICE2, 0, 0);
	GT_REG_WRITE(DEVICE_BANK2PARAMETERS, 0);
	memoryMapDeviceSpace(DEVICE3, 0, 0);
	GT_REG_WRITE(DEVICE_BANK3PARAMETERS, 0);

	GT_REG_WRITE(DEVICE_BOOT_BANK_PARAMETERS, CONFIG_SYS_BOOT_PAR);

	gt_cpu_config();

	/* MPP setup */
	GT_REG_WRITE (MPP_CONTROL0, CONFIG_SYS_MPP_CONTROL_0);
	GT_REG_WRITE (MPP_CONTROL1, CONFIG_SYS_MPP_CONTROL_1);
	GT_REG_WRITE (MPP_CONTROL2, CONFIG_SYS_MPP_CONTROL_2);
	GT_REG_WRITE (MPP_CONTROL3, CONFIG_SYS_MPP_CONTROL_3);

	GT_REG_WRITE (GPP_LEVEL_CONTROL, CONFIG_SYS_GPP_LEVEL_CONTROL);

	set_led(LED_RED);

	return 0;
}

/* various things to do after relocation */

int misc_init_r ()
{
	u8 val;

	icache_enable ();
#ifdef CONFIG_SYS_L2
	l2cache_enable ();
#endif
#ifdef CONFIG_MPSC
	mpsc_sdma_init ();
	mpsc_init2 ();
#endif

	/*
	 * Enable trickle changing in RTC upon powerup
	 * No diode, 250 ohm series resistor
	 */
	val = 0xa5;
	i2c_write(CONFIG_SYS_I2C_RTC_ADDR, 8, 1, &val, 1);

	return 0;
}

void after_reloc (ulong dest_addr, gd_t * gd)
{
	memoryMapDeviceSpace (BOOT_DEVICE, CONFIG_SYS_BOOT_SPACE, CONFIG_SYS_BOOT_SIZE);

/*	display_mem_map(); */

	/* now, jump to the main U-Boot board init code */
	set_led(LED_GREEN);
	board_init_r (gd, dest_addr);
	/* NOTREACHED */
}

/*
 * Check Board Identity:
 * right now, assume borad type. (there is just one...after all)
 */

int checkboard (void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: %s", CONFIG_SYS_BOARD_NAME);

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return (0);
}

void set_led(int col)
{
	int tmp;
	int on_pin;
	int off_pin;

	/* Program Mpp[22] as Gpp[22]
	 * Program Mpp[23] as Gpp[23]
	 */
	tmp = GTREGREAD(MPP_CONTROL2);
	tmp &= 0x00ffffff;
	GT_REG_WRITE(MPP_CONTROL2,tmp);

	/* Program Gpp[22] and Gpp[23] as output
	 */
	tmp = GTREGREAD(GPP_IO_CONTROL);
	tmp |= 0x00C00000;
	GT_REG_WRITE(GPP_IO_CONTROL, tmp);

	/* Program Gpp[22] and Gpp[23] as active high
	 */
	tmp = GTREGREAD(GPP_LEVEL_CONTROL);
	tmp &= 0xff3fffff;
	GT_REG_WRITE(GPP_LEVEL_CONTROL, tmp);

	switch(col) {
	default:
	case LED_OFF :
		on_pin  = 0;
		off_pin = ((1 << 23) | (1 << 22));
		break;
	case LED_RED :
		on_pin  = (1 << 23);
		off_pin = (1 << 22);
		break;
	case LED_GREEN :
		on_pin  = (1 << 22);
		off_pin = (1 << 23);
		break;
	case LED_ORANGE :
		on_pin  = ((1 << 23) | (1 << 22));
		off_pin = 0;
		break;
	}

	/* Set output Gpp[22] and Gpp[23]
	 */
	tmp = GTREGREAD(GPP_VALUE);
	tmp |= on_pin;
	tmp &= ~off_pin;
	GT_REG_WRITE(GPP_VALUE, tmp);
}

int display_mem_map (void)
{
	int i;
	unsigned int base, size, width;
#ifdef CONFIG_PCI
	int j;
#endif

	/* SDRAM */
	printf ("SD (DDR) RAM\n");
	for (i = 0; i <= BANK3; i++) {
		base = memoryGetBankBaseAddress (i);
		size = memoryGetBankSize (i);
		if (size != 0)
			printf ("BANK%d: base - 0x%08x\tsize - %dM bytes\n",
				i, base, size >> 20);
	}
#ifdef CONFIG_PCI
	/* CPU's PCI windows */
	for (i = 0; i <= PCI_HOST1; i++) {
		printf ("\nCPU's PCI %d windows\n", i);
		base = pciGetSpaceBase (i, PCI_IO);
		size = pciGetSpaceSize (i, PCI_IO);
		printf ("      IO: base - 0x%08x\tsize - %dM bytes\n", base,
			size >> 20);
		/* ronen currently only first PCI MEM is used 3 */
		for (j = 0; j <= PCI_REGION0; j++) {
			base = pciGetSpaceBase (i, j);
			size = pciGetSpaceSize (i, j);
			printf ("MEMORY %d: base - 0x%08x\tsize - %dM bytes\n",
				j, base, size >> 20);
		}
	}
#endif /* of CONFIG_PCI */

	/* Bootrom */
	base = memoryGetDeviceBaseAddress (BOOT_DEVICE);	/* Boot */
	size = memoryGetDeviceSize (BOOT_DEVICE);
	width = memoryGetDeviceWidth (BOOT_DEVICE) * 8;
	printf (" BOOT:  base - 0x%08x  size - %dM bytes\twidth - %d bits\t- FLASH\n",
		base, size >> 20, width);

	return (0);
}

/* DRAM check routines copied from gw8260 */

#if defined (CONFIG_SYS_DRAM_TEST)

/*********************************************************************/
/* NAME:  move64() -  moves a double word (64-bit)		     */
/*								     */
/* DESCRIPTION:							     */
/*   this function performs a double word move from the data at	     */
/*   the source pointer to the location at the destination pointer.  */
/*								     */
/* INPUTS:							     */
/*   unsigned long long *src  - pointer to data to move		     */
/*								     */
/* OUTPUTS:							     */
/*   unsigned long long *dest - pointer to locate to move data	     */
/*								     */
/* RETURNS:							     */
/*   None							     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*   May cloober fr0.						     */
/*								     */
/*********************************************************************/
static void move64 (unsigned long long *src, unsigned long long *dest)
{
	asm ("lfd  0, 0(3)\n\t"	/* fpr0   =  *scr       */
	     "stfd 0, 0(4)"	/* *dest  =  fpr0       */
	     : : : "fr0");		/* Clobbers fr0         */
	return;
}


#if defined (CONFIG_SYS_DRAM_TEST_DATA)

unsigned long long pattern[] = {
	0xaaaaaaaaaaaaaaaaULL,
	0xccccccccccccccccULL,
	0xf0f0f0f0f0f0f0f0ULL,
	0xff00ff00ff00ff00ULL,
	0xffff0000ffff0000ULL,
	0xffffffff00000000ULL,
	0x00000000ffffffffULL,
	0x0000ffff0000ffffULL,
	0x00ff00ff00ff00ffULL,
	0x0f0f0f0f0f0f0f0fULL,
	0x3333333333333333ULL,
	0x5555555555555555ULL
};

/*********************************************************************/
/* NAME:  mem_test_data() -  test data lines for shorts and opens    */
/*								     */
/* DESCRIPTION:							     */
/*   Tests data lines for shorts and opens by forcing adjacent data  */
/*   to opposite states. Because the data lines could be routed in   */
/*   an arbitrary manner the must ensure test patterns ensure that   */
/*   every case is tested. By using the following series of binary   */
/*   patterns every combination of adjacent bits is test regardless  */
/*   of routing.						     */
/*								     */
/*     ...101010101010101010101010				     */
/*     ...110011001100110011001100				     */
/*     ...111100001111000011110000				     */
/*     ...111111110000000011111111				     */
/*								     */
/*   Carrying this out, gives us six hex patterns as follows:	     */
/*								     */
/*     0xaaaaaaaaaaaaaaaa					     */
/*     0xcccccccccccccccc					     */
/*     0xf0f0f0f0f0f0f0f0					     */
/*     0xff00ff00ff00ff00					     */
/*     0xffff0000ffff0000					     */
/*     0xffffffff00000000					     */
/*								     */
/*   The number test patterns will always be given by:		     */
/*								     */
/*   log(base 2)(number data bits) = log2 (64) = 6		     */
/*								     */
/*   To test for short and opens to other signals on our boards. we  */
/*   simply							     */
/*   test with the 1's complemnt of the paterns as well.	     */
/*								     */
/* OUTPUTS:							     */
/*   Displays failing test pattern				     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*  Assumes only one one SDRAM bank				     */
/*								     */
/*********************************************************************/
int mem_test_data (void)
{
	unsigned long long *pmem = (unsigned long long *) CONFIG_SYS_MEMTEST_START;
	unsigned long long temp64 = 0;
	int num_patterns = sizeof (pattern) / sizeof (pattern[0]);
	int i;
	unsigned int hi, lo;

	for (i = 0; i < num_patterns; i++) {
		move64 (&(pattern[i]), pmem);
		move64 (pmem, &temp64);

		/* hi = (temp64>>32) & 0xffffffff;          */
		/* lo = temp64 & 0xffffffff;                */
		/* printf("\ntemp64 = 0x%08x%08x", hi, lo); */

		hi = (pattern[i] >> 32) & 0xffffffff;
		lo = pattern[i] & 0xffffffff;
		/* printf("\npattern[%d] = 0x%08x%08x", i, hi, lo);  */

		if (temp64 != pattern[i]) {
			printf ("\n   Data Test Failed, pattern 0x%08x%08x",
				hi, lo);
			return 1;
		}
	}

	return 0;
}
#endif /* CONFIG_SYS_DRAM_TEST_DATA */

#if defined (CONFIG_SYS_DRAM_TEST_ADDRESS)
/*********************************************************************/
/* NAME:  mem_test_address() -	test address lines		     */
/*								     */
/* DESCRIPTION:							     */
/*   This function performs a test to verify that each word im	     */
/*   memory is uniquly addressable. The test sequence is as follows: */
/*								     */
/*   1) write the address of each word to each word.		     */
/*   2) verify that each location equals its address		     */
/*								     */
/* OUTPUTS:							     */
/*   Displays failing test pattern and address			     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
int mem_test_address (void)
{
	volatile unsigned int *pmem =
		(volatile unsigned int *) CONFIG_SYS_MEMTEST_START;
	const unsigned int size = (CONFIG_SYS_MEMTEST_END - CONFIG_SYS_MEMTEST_START) / 4;
	unsigned int i;

	/* write address to each location */
	for (i = 0; i < size; i++)
		pmem[i] = i;

	/* verify each loaction */
	for (i = 0; i < size; i++) {
		if (pmem[i] != i) {
			printf ("\n   Address Test Failed at 0x%x", i);
			return 1;
		}
	}
	return 0;
}
#endif /* CONFIG_SYS_DRAM_TEST_ADDRESS */

#if defined (CONFIG_SYS_DRAM_TEST_WALK)
/*********************************************************************/
/* NAME:   mem_march() -  memory march				     */
/*								     */
/* DESCRIPTION:							     */
/*   Marches up through memory. At each location verifies rmask if   */
/*   read = 1. At each location write wmask if	write = 1. Displays  */
/*   failing address and pattern.				     */
/*								     */
/* INPUTS:							     */
/*   volatile unsigned long long * base - start address of test	     */
/*   unsigned int size - number of dwords(64-bit) to test	     */
/*   unsigned long long rmask - read verify mask		     */
/*   unsigned long long wmask - wrtie verify mask		     */
/*   short read - verifies rmask if read = 1			     */
/*   short write  - writes wmask if write = 1			     */
/*								     */
/* OUTPUTS:							     */
/*   Displays failing test pattern and address			     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
int mem_march (volatile unsigned long long *base,
	       unsigned int size,
	       unsigned long long rmask,
	       unsigned long long wmask, short read, short write)
{
	unsigned int i;
	unsigned long long temp = 0;
	unsigned int hitemp, lotemp, himask, lomask;

	for (i = 0; i < size; i++) {
		if (read != 0) {
			/* temp = base[i]; */
			move64 ((unsigned long long *) &(base[i]), &temp);
			if (rmask != temp) {
				hitemp = (temp >> 32) & 0xffffffff;
				lotemp = temp & 0xffffffff;
				himask = (rmask >> 32) & 0xffffffff;
				lomask = rmask & 0xffffffff;

				printf ("\n Walking one's test failed: address = 0x%08x," "\n\texpected 0x%08x%08x, found 0x%08x%08x", i << 3, himask, lomask, hitemp, lotemp);
				return 1;
			}
		}
		if (write != 0) {
			/*  base[i] = wmask; */
			move64 (&wmask, (unsigned long long *) &(base[i]));
		}
	}
	return 0;
}
#endif /* CONFIG_SYS_DRAM_TEST_WALK */

/*********************************************************************/
/* NAME:   mem_test_walk() -  a simple walking ones test	     */
/*								     */
/* DESCRIPTION:							     */
/*   Performs a walking ones through entire physical memory. The     */
/*   test uses as series of memory marches, mem_march(), to verify   */
/*   and write the test patterns to memory. The test sequence is as  */
/*   follows:							     */
/*     1) march writing 0000...0001				     */
/*     2) march verifying 0000...0001  , writing  0000...0010	     */
/*     3) repeat step 2 shifting masks left 1 bit each time unitl    */
/*	   the write mask equals 1000...0000			     */
/*     4) march verifying 1000...0000				     */
/*   The test fails if any of the memory marches return a failure.   */
/*								     */
/* OUTPUTS:							     */
/*   Displays which pass on the memory test is executing	     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
int mem_test_walk (void)
{
	unsigned long long mask;
	volatile unsigned long long *pmem =
		(volatile unsigned long long *) CONFIG_SYS_MEMTEST_START;
	const unsigned long size = (CONFIG_SYS_MEMTEST_END - CONFIG_SYS_MEMTEST_START) / 8;

	unsigned int i;

	mask = 0x01;

	printf ("Initial Pass");
	mem_march (pmem, size, 0x0, 0x1, 0, 1);

	printf ("\b\b\b\b\b\b\b\b\b\b\b\b");
	printf ("		");
	printf ("         ");
	printf ("\b\b\b\b\b\b\b\b\b\b\b\b");

	for (i = 0; i < 63; i++) {
		printf ("Pass %2d", i + 2);
		if (mem_march (pmem, size, mask, mask << 1, 1, 1) != 0) {
			/*printf("mask: 0x%x, pass: %d, ", mask, i); */
			return 1;
		}
		mask = mask << 1;
		printf ("\b\b\b\b\b\b\b");
	}

	printf ("Last Pass");
	if (mem_march (pmem, size, 0, mask, 0, 1) != 0) {
		/* printf("mask: 0x%x", mask); */
		return 1;
	}
	printf ("\b\b\b\b\b\b\b\b\b");
	printf ("	     ");
	printf ("\b\b\b\b\b\b\b\b\b");

	return 0;
}

/*********************************************************************/
/* NAME:    testdram() -  calls any enabled memory tests	     */
/*								     */
/* DESCRIPTION:							     */
/*   Runs memory tests if the environment test variables are set to  */
/*   'y'.							     */
/*								     */
/* INPUTS:							     */
/*   testdramdata    - If set to 'y', data test is run.		     */
/*   testdramaddress - If set to 'y', address test is run.	     */
/*   testdramwalk    - If set to 'y', walking ones test is run	     */
/*								     */
/* OUTPUTS:							     */
/*   None							     */
/*								     */
/* RETURNS:							     */
/*   0 -  Passed test						     */
/*   1 -  Failed test						     */
/*								     */
/* RESTRICTIONS/LIMITATIONS:					     */
/*								     */
/*								     */
/*********************************************************************/
int testdram (void)
{
	char *s;
	int rundata    = 0;
	int runaddress = 0;
	int runwalk    = 0;

#ifdef CONFIG_SYS_DRAM_TEST_DATA
	s = getenv ("testdramdata");
	rundata = (s && (*s == 'y')) ? 1 : 0;
#endif
#ifdef CONFIG_SYS_DRAM_TEST_ADDRESS
	s = getenv ("testdramaddress");
	runaddress = (s && (*s == 'y')) ? 1 : 0;
#endif
#ifdef CONFIG_SYS_DRAM_TEST_WALK
	s = getenv ("testdramwalk");
	runwalk = (s && (*s == 'y')) ? 1 : 0;
#endif

	if ((rundata == 1) || (runaddress == 1) || (runwalk == 1))
		printf ("Testing RAM from 0x%08x to 0x%08x ...  "
			"(don't panic... that will take a moment !!!!)\n",
			CONFIG_SYS_MEMTEST_START, CONFIG_SYS_MEMTEST_END);
#ifdef CONFIG_SYS_DRAM_TEST_DATA
	if (rundata == 1) {
		printf ("Test DATA ...  ");
		if (mem_test_data () == 1) {
			printf ("failed \n");
			return 1;
		} else
			printf ("ok \n");
	}
#endif
#ifdef CONFIG_SYS_DRAM_TEST_ADDRESS
	if (runaddress == 1) {
		printf ("Test ADDRESS ...  ");
		if (mem_test_address () == 1) {
			printf ("failed \n");
			return 1;
		} else
			printf ("ok \n");
	}
#endif
#ifdef CONFIG_SYS_DRAM_TEST_WALK
	if (runwalk == 1) {
		printf ("Test WALKING ONEs ...  ");
		if (mem_test_walk () == 1) {
			printf ("failed \n");
			return 1;
		} else
			printf ("ok \n");
	}
#endif
	if ((rundata == 1) || (runaddress == 1) || (runwalk == 1))
		printf ("passed\n");
	return 0;

}
#endif /* CONFIG_SYS_DRAM_TEST */

/* ronen - the below functions are used by the bootm function           */
/*  - we map the base register to fbe00000 (same mapping as in the LSP) */
/*  - we turn off the RX gig dmas - to prevent the dma from overunning  */
/*    the kernel data areas.                                            */
/*  - we diable and invalidate the icache and dcache.                   */
void my_remap_gt_regs_bootm (u32 cur_loc, u32 new_loc)
{
	u32 temp;

	temp = in_le32 ((u32 *) (new_loc + INTERNAL_SPACE_DECODE));
	if ((temp & 0xffff) == new_loc >> 16)
		return;

	temp = (in_le32 ((u32 *) (cur_loc + INTERNAL_SPACE_DECODE)) &
		0xffff0000) | (new_loc >> 16);

	out_le32 ((u32 *) (cur_loc + INTERNAL_SPACE_DECODE), temp);

	while ((WORD_SWAP (*((volatile unsigned int *) (NONE_CACHEABLE |
							new_loc |
							(INTERNAL_SPACE_DECODE)))))
	       != temp);

}

int board_eth_init(bd_t *bis)
{
	return mv6446x_eth_initialize(bis);
}
