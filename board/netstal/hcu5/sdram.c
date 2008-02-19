/*
 * (C) Copyright 2007
 * Niklaus Giger (Niklaus.Giger@netstal.com)
 * (C) Copyright 2006
 * Sylvie Gohl,             AMCC/IBM, gohl.sylvie@fr.ibm.com
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Thierry Roman,           AMCC/IBM, thierry_roman@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 * Robert Snyder,           AMCC/IBM, rob.snyder@fr.ibm.com
 *
 * (C) Copyright 2006
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

/* define DEBUG for debug output */
#undef DEBUG

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <ppc440.h>

void hcu_led_set(u32 value);
void dcbz_area(u32 start_address, u32 num_bytes);
void dflush(void);

#define DDR_DCR_BASE 0x10
#define ddrcfga  (DDR_DCR_BASE+0x0)   /* DDR configuration address reg */
#define ddrcfgd  (DDR_DCR_BASE+0x1)   /* DDR configuration data reg    */

#define DDR0_01_INT_MASK_MASK             0x000000FF
#define DDR0_00_INT_ACK_ALL               0x7F000000
#define DDR0_01_INT_MASK_ALL_ON           0x000000FF
#define DDR0_01_INT_MASK_ALL_OFF          0x00000000

#define DDR0_17_DLLLOCKREG_MASK           0x00010000 /* Read only */
#define DDR0_17_DLLLOCKREG_UNLOCKED       0x00000000
#define DDR0_17_DLLLOCKREG_LOCKED         0x00010000

#define DDR0_22                         0x16
/* ECC */
#define DDR0_22_CTRL_RAW_MASK             0x03000000
#define DDR0_22_CTRL_RAW_ECC_DISABLE      0x00000000 /* ECC not enabled */
#define DDR0_22_CTRL_RAW_ECC_CHECK_ONLY   0x01000000 /* ECC no correction */
#define DDR0_22_CTRL_RAW_NO_ECC_RAM       0x02000000 /* Not a ECC RAM*/
#define DDR0_22_CTRL_RAW_ECC_ENABLE       0x03000000 /* ECC correcting on */
#define DDR0_03_CASLAT_DECODE(n)            ((((unsigned long)(n))>>16)&0x7)

#define ECC_RAM				0x03267F0B
#define NO_ECC_RAM			0x00267F0B

#define HCU_HW_SDRAM_CONFIG_MASK	0x7

#define MY_TLB_WORD2_I_ENABLE TLB_WORD2_I_ENABLE
	/* disable caching on DDR2 */

void board_add_ram_info(int use_default)
{
	PPC4xx_SYS_INFO board_cfg;
	u32 val;

	mfsdram(DDR0_22, val);
	val &= DDR0_22_CTRL_RAW_MASK;
	switch (val) {
	case DDR0_22_CTRL_RAW_ECC_DISABLE:
		puts(" (ECC disabled");
		break;
	case DDR0_22_CTRL_RAW_ECC_CHECK_ONLY:
		puts(" (ECC check only");
		break;
	case DDR0_22_CTRL_RAW_NO_ECC_RAM:
		puts(" (no ECC ram");
		break;
	case DDR0_22_CTRL_RAW_ECC_ENABLE:
		puts(" (ECC enabled");
		break;
	}

	get_sys_info(&board_cfg);
	printf(", %d MHz", (board_cfg.freqPLB * 2) / 1000000);

	mfsdram(DDR0_03, val);
	val = DDR0_03_CASLAT_DECODE(val);
	printf(", CL%d)", val);
}

/*--------------------------------------------------------------------
 * wait_for_dlllock.
 *--------------------------------------------------------------------*/
static int wait_for_dlllock(void)
{
	unsigned long val;
	int wait = 0;

	/* -----------------------------------------------------------+
	 * Wait for the DCC master delay line to finish calibration
	 * ----------------------------------------------------------*/
	mtdcr(ddrcfga, DDR0_17);
	val = DDR0_17_DLLLOCKREG_UNLOCKED;

	while (wait != 0xffff) {
		val = mfdcr(ddrcfgd);
		if ((val & DDR0_17_DLLLOCKREG_MASK) ==
		    DDR0_17_DLLLOCKREG_LOCKED)
			/* dlllockreg bit on */
			return 0;
		else
			wait++;
	}
	debug("0x%04x: DDR0_17 Value (dlllockreg bit): 0x%08x\n", wait, val);
	debug("Waiting for dlllockreg bit to raise\n");

	return -1;
}

/***********************************************************************
 *
 * sdram_panic -- Panic if we cannot configure the sdram correctly
 *
 ************************************************************************/
void sdram_panic(const char *reason)
{
	printf("\n%s: reason %s",  __FUNCTION__,  reason);
	hcu_led_set(0xff);
	while (1) {
	}
	/* Never return */
}

#ifdef CONFIG_DDR_ECC
static void blank_string(int size)
{
	int i;

	for (i=0; i<size; i++)
		putc('\b');
	for (i=0; i<size; i++)
		putc(' ');
	for (i=0; i<size; i++)
		putc('\b');
}
/*---------------------------------------------------------------------------+
 * program_ecc.
 *---------------------------------------------------------------------------*/
static void program_ecc(unsigned long start_address, unsigned long num_bytes)
{
	u32 val;
	char str[] = "ECC generation -";
#if defined(CONFIG_PRAM)
	u32 *magicPtr;
	u32 magic;

	if ((mfspr(dbcr0) & 0x80000000) == 0) {
		/* only if no external debugger is alive!
		 * Check whether vxWorks is using EDR logging, if yes zero
		 * also PostMortem and user reserved memory
		 */
		magicPtr = (u32 *)(start_address + num_bytes -
				(CONFIG_PRAM*1024) + sizeof(u32));
		magic = in_be32(magicPtr);
		debug("%s:  CONFIG_PRAM %d kB magic 0x%x 0x%p\n",
		      __FUNCTION__, CONFIG_PRAM,
		      magicPtr, magic);
		if (magic == 0xbeefbabe) {
			printf("%s: preserving at %p\n", __FUNCTION__, magicPtr);
			num_bytes -= (CONFIG_PRAM*1024) - PM_RESERVED_MEM;
		}
	}
#endif

	sync();
	eieio();

	puts(str);

	/* ECC bit set method for cached memory */
	/* Fast method, no noticeable delay */
	dcbz_area(start_address, num_bytes);
	dflush();
	blank_string(strlen(str));

	/* Clear error status */
	mfsdram(DDR0_00, val);
	mtsdram(DDR0_00, val | DDR0_00_INT_ACK_ALL);

	/*
	 * Clear possible ECC errors
	 * If not done, then we could get an interrupt later on when
	 * exceptions are enabled.
	 */
	mtspr(mcsr, mfspr(mcsr));

	/* Set 'int_mask' parameter to functionnal value */
	mfsdram(DDR0_01, val);
	mtsdram(DDR0_01, ((val &~ DDR0_01_INT_MASK_MASK) |
			  DDR0_01_INT_MASK_ALL_OFF));

	return;
}
#endif


/***********************************************************************
 *
 * initdram -- 440EPx's DDR controller is a DENALI Core
 *
 ************************************************************************/
long int initdram (int board_type)
{
	unsigned int dram_size = 0;

	mtsdram(DDR0_02, 0x00000000);

	/* Values must be kept in sync with Excel-table <<A0001492.>> ! */
	mtsdram(DDR0_00, 0x0000190A);
	mtsdram(DDR0_01, 0x01000000);
	mtsdram(DDR0_03, 0x02030602);
	mtsdram(DDR0_04, 0x0A020200);
	mtsdram(DDR0_05, 0x02020307);
	switch (in_be16((u16 *)HCU_HW_VERSION_REGISTER) & HCU_HW_SDRAM_CONFIG_MASK) {
	case 1:
		dram_size = 256 * 1024 * 1024 ;
		mtsdram(DDR0_06, 0x0102C812);  /* 256MB RAM */
		mtsdram(DDR0_11, 0x0014C800);  /* 256MB RAM */
		mtsdram(DDR0_43, 0x030A0200);  /* 256MB RAM */
		break;
	case 0:
	default:
		dram_size = 128 * 1024 * 1024 ;
		mtsdram(DDR0_06, 0x0102C80D);  /* 128MB RAM */
		mtsdram(DDR0_11, 0x000FC800);  /* 128MB RAM */
		mtsdram(DDR0_43, 0x030A0300);  /* 128MB RAM */
		break;
	}
	mtsdram(DDR0_07, 0x00090100);

	/*
	 * TCPD=200 cycles of clock input is required to lock the DLL.
	 * CKE must be HIGH the entire time.mtsdram(DDR0_08, 0x02C80001);
	 */
	mtsdram(DDR0_08, 0x02C80001);
	mtsdram(DDR0_09, 0x00011D5F);
	mtsdram(DDR0_10, 0x00000100);
	mtsdram(DDR0_12, 0x00000003);
	mtsdram(DDR0_14, 0x00000000);
	mtsdram(DDR0_17, 0x1D000000);
	mtsdram(DDR0_18, 0x1D1D1D1D);
	mtsdram(DDR0_19, 0x1D1D1D1D);
	mtsdram(DDR0_20, 0x0B0B0B0B);
	mtsdram(DDR0_21, 0x0B0B0B0B);
#ifdef CONFIG_DDR_ECC
	mtsdram(DDR0_22, ECC_RAM);
#else
	mtsdram(DDR0_22, NO_ECC_RAM);
#endif

	mtsdram(DDR0_23, 0x00000000);
	mtsdram(DDR0_24, 0x01020001);
	mtsdram(DDR0_26, 0x2D930517);
	mtsdram(DDR0_27, 0x00008236);
	mtsdram(DDR0_28, 0x00000000);
	mtsdram(DDR0_31, 0x00000000);
	mtsdram(DDR0_42, 0x01000006);
	mtsdram(DDR0_44, 0x00000003);
	mtsdram(DDR0_02, 0x00000001);
	wait_for_dlllock();
	mtsdram(DDR0_00, 0x40000000);  /* Zero init bit */

	/*
	 * Program tlb entries for this size (dynamic)
	 */
	remove_tlb(CFG_SDRAM_BASE, 256 << 20);
	program_tlb(0, 0, dram_size, TLB_WORD2_W_ENABLE | TLB_WORD2_I_ENABLE);

	/*
	 * Setup 2nd TLB with same physical address but different virtual
	 * address with cache enabled. This is done for fast ECC generation.
	 */
	program_tlb(0, CFG_DDR_CACHED_ADDR, dram_size, 0);

#ifdef CONFIG_DDR_ECC
	/*
	 * If ECC is enabled, initialize the parity bits.
	 */
	program_ecc(CFG_DDR_CACHED_ADDR, dram_size);
#endif

	return (dram_size);
}
