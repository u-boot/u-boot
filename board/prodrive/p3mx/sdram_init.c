/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*************************************************************************
 * adaption for the Marvell DB64460 Board
 * Ingo Assmus (ingo.assmus@keymile.com)
 *************************************************************************/

/* sdram_init.c - automatic memory sizing */

#include <common.h>
#include <74xx_7xx.h>
#include "../../Marvell/include/memory.h"
#include "../../Marvell/include/pci.h"
#include "../../Marvell/include/mv_gen_reg.h"
#include <net.h>

#include "eth.h"
#include "mpsc.h"
#include "../../Marvell/common/i2c.h"
#include "64460.h"
#include "mv_regs.h"

DECLARE_GLOBAL_DATA_PTR;

#undef	DEBUG
#define MAP_PCI

#ifdef DEBUG
#define DP(x) x
#else
#define DP(x)
#endif

int set_dfcdlInit (void);	/* setup delay line of Mv64460 */
int mvDmaIsChannelActive (int);
int mvDmaSetMemorySpace (ulong, ulong, ulong, ulong, ulong);
int mvDmaTransfer (int, ulong, ulong, ulong, ulong);

#define D_CACHE_FLUSH_LINE(addr, offset)				\
	{								\
		__asm__ __volatile__ ("dcbf %0,%1" : : "r" (addr), "r" (offset)); \
	}

int memory_map_bank (unsigned int bankNo,
		     unsigned int bankBase, unsigned int bankLength)
{
#ifdef MAP_PCI
	PCI_HOST host;
#endif

#ifdef DEBUG
	if (bankLength > 0) {
		printf ("mapping bank %d at %08x - %08x\n",
			bankNo, bankBase, bankBase + bankLength - 1);
	} else {
		printf ("unmapping bank %d\n", bankNo);
	}
#endif

	memoryMapBank (bankNo, bankBase, bankLength);

#ifdef MAP_PCI
	for (host = PCI_HOST0; host <= PCI_HOST1; host++) {
		const int features =
			PREFETCH_ENABLE |
			DELAYED_READ_ENABLE |
			AGGRESSIVE_PREFETCH |
			READ_LINE_AGGRESSIVE_PREFETCH |
			READ_MULTI_AGGRESSIVE_PREFETCH |
			MAX_BURST_4 | PCI_NO_SWAP;

		pciMapMemoryBank (host, bankNo, bankBase, bankLength);

		pciSetRegionSnoopMode (host, bankNo, PCI_SNOOP_WB, bankBase,
				       bankLength);

		pciSetRegionFeatures (host, bankNo, features, bankBase,
				      bankLength);
	}
#endif

	return 0;
}

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */
long int dram_size (long int *base, long int maxsize)
{
	volatile long int *addr, *b = base;
	long int cnt, val, save1, save2;

#define STARTVAL (1<<20)	/* start test at 1M */
	for (cnt = STARTVAL / sizeof (long); cnt < maxsize / sizeof (long);
	     cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */

		save1 = *addr;	/* save contents of addr */
		save2 = *b;	/* save contents of base */

		*addr = cnt;	/* write cnt to addr */
		*b = 0;		/* put null at base */

		/* check at base address */
		if ((*b) != 0) {
			*addr = save1;	/* restore *addr */
			*b = save2;	/* restore *b */
			return (0);
		}
		val = *addr;	/* read *addr */
		val = *addr;	/* read *addr */

		*addr = save1;
		*b = save2;

		if (val != cnt) {
			DP (printf
			    ("Found %08x  at Address %08x (failure)\n",
			     (unsigned int) val, (unsigned int) addr));
			/* fix boundary condition.. STARTVAL means zero */
			if (cnt == STARTVAL / sizeof (long))
				cnt = 0;
			return (cnt * sizeof (long));
		}
	}

	return maxsize;
}

#define SDRAM_NORMAL			0x0
#define SDRAM_PRECHARGE_ALL		0x1
#define SDRAM_REFRESH_ALL		0x2
#define SDRAM_MODE_REG_SETUP		0x3
#define SDRAM_XTEN_MODE_REG_SETUP	0x4
#define SDRAM_NOP			0x5
#define SDRAM_SELF_REFRESH		0x7

long int initdram (int board_type)
{
	int tmp;
	int start;
	ulong size;
	ulong memSpaceAttr;
	ulong dest;

	/* first disable all banks */
	memory_map_bank(0, 0, 0);
	memory_map_bank(1, 0, 0);
	memory_map_bank(2, 0, 0);
	memory_map_bank(3, 0, 0);

	/* calibrate delay lines */
	set_dfcdlInit();

	GT_REG_WRITE(MV64460_SDRAM_OPERATION, SDRAM_NOP);		/* 0x1418 */
	do {
		tmp = GTREGREAD(MV64460_SDRAM_OPERATION);
	} while(tmp != 0x0);

	/* SDRAM controller configuration */
#ifdef CONFIG_MV64460_ECC
	GT_REG_WRITE(MV64460_SDRAM_CONFIG,		0x58201400);	/* 0x1400 */
#else
	GT_REG_WRITE(MV64460_SDRAM_CONFIG,		0x58200400);	/* 0x1400 */
#endif
	GT_REG_WRITE(MV64460_D_UNIT_CONTROL_LOW,	0xC3000540);	/* 0x1404  */
	GT_REG_WRITE(MV64460_D_UNIT_CONTROL_HIGH,	0x0300F777);	/* 0x1424 */
	GT_REG_WRITE(MV64460_SDRAM_TIMING_CONTROL_LOW,	0x01712220);	/* 0x1408 */
	GT_REG_WRITE(MV64460_SDRAM_TIMING_CONTROL_HIGH, 0x0000005D);	/* 0x140C */
	GT_REG_WRITE(MV64460_SDRAM_ADDR_CONTROL,	0x00000012);	/* 0x1410 */
	GT_REG_WRITE(MV64460_SDRAM_OPEN_PAGES_CONTROL,	0x00000001);	/* 0x1414 */

	/* SDRAM drive strength */
	GT_REG_WRITE(MV64460_SDRAM_ADDR_CTRL_PADS_CALIBRATION, 0x80000000); /* 0x14C0 */
	GT_REG_WRITE(MV64460_SDRAM_ADDR_CTRL_PADS_CALIBRATION, 0x80000008); /* 0x14C0 */
	GT_REG_WRITE(MV64460_SDRAM_DATA_PADS_CALIBRATION, 0x80000000);	    /* 0x14C4 */
	GT_REG_WRITE(MV64460_SDRAM_DATA_PADS_CALIBRATION, 0x80000008);	    /* 0x14C4 */

	/* setup SDRAM device registers */

	/* precharge all */
	GT_REG_WRITE(MV64460_SDRAM_OPERATION, SDRAM_PRECHARGE_ALL);	/* 0x1418 */
	do {
		tmp = GTREGREAD(MV64460_SDRAM_OPERATION);
	} while(tmp != 0x0);

	/* enable DLL */
	GT_REG_WRITE(MV64460_EXTENDED_DRAM_MODE, 0x00000000);			/* 0x1420 */
	GT_REG_WRITE(MV64460_SDRAM_OPERATION, SDRAM_XTEN_MODE_REG_SETUP);	/* 0x1418 */
	do {
		tmp = GTREGREAD(MV64460_SDRAM_OPERATION);
	} while(tmp != 0x0);

	/* reset DLL */
	GT_REG_WRITE(MV64460_SDRAM_MODE, 0x00000132);	/* 0x141C */
	GT_REG_WRITE(MV64460_SDRAM_OPERATION, SDRAM_MODE_REG_SETUP);	/* 0x1418 */
	do {
		tmp = GTREGREAD(MV64460_SDRAM_OPERATION);
	} while(tmp != 0x0);

	/* precharge all */
	GT_REG_WRITE(MV64460_SDRAM_OPERATION, SDRAM_PRECHARGE_ALL);	/* 0x1418 */
	do {
		tmp = GTREGREAD(MV64460_SDRAM_OPERATION);
	} while(tmp != 0x0);

	/* wait for 2 auto refresh commands */
	udelay(20);

	/* un-reset DLL */
	GT_REG_WRITE(MV64460_SDRAM_MODE, 0x00000032);	/* 0x141C */
	GT_REG_WRITE(MV64460_SDRAM_OPERATION, SDRAM_MODE_REG_SETUP);	/* 0x1418 */
	do {
		tmp = GTREGREAD(MV64460_SDRAM_OPERATION);
	} while(tmp != 0x0);

	/* wait 200 cycles */
	udelay(2);  /* FIXME  make this dynamic for the system clock */

	/* SDRAM init done */
	memory_map_bank(0, CFG_SDRAM_BASE,  (256 << 20));
#ifdef CFG_SDRAM1_BASE
	memory_map_bank(1, CFG_SDRAM1_BASE, (256 << 20));
#endif

	/* DUNIT_MMASK: enable SnoopHitEn bit to avoid errata CPU-#4
	 */
	tmp = GTREGREAD(MV64460_D_UNIT_MMASK);				/* 0x14B0 */
	GT_REG_WRITE(MV64460_D_UNIT_MMASK, tmp | 0x2);

	start = (0 << 20);
#ifdef CONFIG_P3M750
	size = (512 << 20);
#elif defined (CONFIG_P3M7448)
	size = (128 << 20);
#endif

#ifdef CONFIG_MV64460_ECC
	memSpaceAttr = ((~(BIT0 << 0)) & 0xf) << 8;
	mvDmaSetMemorySpace (0, 0, memSpaceAttr, start, size);
	for (dest = start; dest < start + size; dest += _8M) {
		mvDmaTransfer (0, start, dest, _8M,
			       BIT8 /*DMA_DTL_128BYTES */  |
			       BIT3 /*DMA_HOLD_SOURCE_ADDR */ |
			       BIT11 /*DMA_BLOCK_TRANSFER_MODE */ );
		while (mvDmaIsChannelActive (0));
	}
#endif

	return (size);
}

void board_add_ram_info(int use_default)
{
	u32 val;

	puts(" (CL=");
	switch ((GTREGREAD(MV64460_SDRAM_MODE) >> 4) & 0x7) {
	case 0x2:
		puts("2");
		break;
	case 0x3:
		puts("3");
		break;
	case 0x5:
		puts("1.5");
		break;
	case 0x6:
		puts("2.5");
		break;
	}

	val = GTREGREAD(MV64460_SDRAM_CONFIG);

	puts(", ECC ");
	if (val & 0x00001000)
		puts("enabled)");
	else
		puts("not enabled)");
}

/*
 * mvDmaIsChannelActive - Check if IDMA channel is active
 *
 * channel	= IDMA channel number from 0 to 7
 */
int mvDmaIsChannelActive (int channel)
{
	ulong data;

	data = GTREGREAD (MV64460_DMA_CHANNEL0_CONTROL + 4 * channel);
	if (data & BIT14)	/* activity status */
		return 1;

	return 0;
}

/*
 * mvDmaSetMemorySpace - Set a DMA memory window for the DMA's address decoding
 *			 map.
 *
 * memSpace	= IDMA memory window number from 0 to 7
 * trg_if	= Target interface:
 *		  0x0 DRAM
 *		  0x1 Device Bus
 *		  0x2 Integrated SDRAM (or CPU bus 60x only)
 *		  0x3 PCI0
 *		  0x4 PCI1
 * attr		= IDMA attributes (see MV datasheet)
 * base_addr	= Sets up memory window for transfers
 *
 */
int mvDmaSetMemorySpace (ulong memSpace,
			 ulong trg_if,
			 ulong attr, ulong base_addr, ulong size)
{
	ulong temp;

	/* The base address must be aligned to the size.  */
	if (base_addr % size != 0)
		return 0;

	if (size >= 0x10000) {	 /* 64K */
		size &= 0xffff0000;
		base_addr = (base_addr & 0xffff0000);
		/* Set the new attributes */
		GT_REG_WRITE (MV64460_DMA_BASE_ADDR_REG0 + memSpace * 8,
			      (base_addr | trg_if | attr));
		GT_REG_WRITE ((MV64460_DMA_SIZE_REG0 + memSpace * 8),
			      (size - 1) & 0xffff0000);
		temp = GTREGREAD (MV64460_DMA_BASE_ADDR_ENABLE_REG);
		GT_REG_WRITE (DMA_BASE_ADDR_ENABLE_REG,
			      (temp & ~(BIT0 << memSpace)));
		return 1;
	}

	return 0;
}

/*
 * mvDmaTransfer - Transfer data from src_addr to dst_addr on one of the 4
 *		   DMA channels.
 *
 * channel	= IDMA channel number from 0 to 3
 * destAddr	= Destination address
 * sourceAddr	= Source address
 * size		= Size in bytes
 * command	= See MV datasheet
 *
 */
int mvDmaTransfer (int channel, ulong sourceAddr,
		   ulong destAddr, ulong size, ulong command)
{
	ulong engOffReg = 0;	/* Engine Offset Register */

	if (size > 0xffff)
		command = command | BIT31;	/* DMA_16M_DESCRIPTOR_MODE */
	command = command | ((command >> 6) & 0x7);
	engOffReg = channel * 4;
	GT_REG_WRITE (MV64460_DMA_CHANNEL0_BYTE_COUNT + engOffReg, size);
	GT_REG_WRITE (MV64460_DMA_CHANNEL0_SOURCE_ADDR + engOffReg, sourceAddr);
	GT_REG_WRITE (MV64460_DMA_CHANNEL0_DESTINATION_ADDR + engOffReg, destAddr);
	command = command |
		BIT12	|			/* DMA_CHANNEL_ENABLE */
		BIT9;				/* DMA_NON_CHAIN_MODE */
	/* Activate DMA channel By writting to mvDmaControlRegister */
	GT_REG_WRITE (MV64460_DMA_CHANNEL0_CONTROL + engOffReg, command);
	return 1;
}

/****************************************************************************************
 *			       SDRAM INIT						*
 *  This procedure detect all Sdram types: 64, 128, 256, 512 Mbit, 1Gbit and 2Gb	*
 *		 This procedure fits only the Atlantis					*
 *											*
 ***************************************************************************************/

/****************************************************************************************
 *			       DFCDL initialize MV643xx Design Considerations		*
 *											*
 ***************************************************************************************/
int set_dfcdlInit (void)
{
	int i;

	/* Values from MV64460 User Manual */
	unsigned int dfcdl_tbl[] = { 0x00000000, 0x00000001, 0x00000042, 0x00000083,
				     0x000000c4, 0x00000105, 0x00000146, 0x00000187,
				     0x000001c8, 0x00000209, 0x0000024a, 0x0000028b,
				     0x000002cc, 0x0000030d, 0x0000034e, 0x0000038f,
				     0x000003d0, 0x00000411, 0x00000452, 0x00000493,
				     0x000004d4, 0x00000515, 0x00000556, 0x00000597,
				     0x000005d8, 0x00000619, 0x0000065a, 0x0000069b,
				     0x000006dc, 0x0000071d, 0x0000075e, 0x0000079f,
				     0x000007e0, 0x00000821, 0x00000862, 0x000008a3,
				     0x000008e4, 0x00000925, 0x00000966, 0x000009a7,
				     0x000009e8, 0x00000a29, 0x00000a6a, 0x00000aab,
				     0x00000aec, 0x00000b2d, 0x00000b6e, 0x00000baf,
				     0x00000bf0, 0x00000c31, 0x00000c72, 0x00000cb3,
				     0x00000cf4, 0x00000d35, 0x00000d76, 0x00000db7,
				     0x00000df8, 0x00000e39, 0x00000e7a, 0x00000ebb,
				     0x00000efc, 0x00000f3d, 0x00000f7e, 0x00000fbf };

	for (i = 0; i < 64; i++)
		GT_REG_WRITE (SRAM_DATA0, dfcdl_tbl[i]);
	GT_REG_WRITE (DFCDL_CONFIG0, 0x00300000);	/* enable dynamic delay line updating */

	return (0);
}
