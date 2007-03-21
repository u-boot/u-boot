/*
 * (C) Copyright 2007
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
 */

#include <common.h>
#include <asm/processor.h>

#define CRAM_BANK0_BASE 		0x0
#define CRAM_DIDR			0x00100000
#define	MICRON_MT45W8MW16BGX_CRAM_ID	0x1b431b43
#define	MICRON_MT45W8MW16BGX_CRAM_ID2	0x13431343
#define	MICRON_DIDR_VENDOR_ID		0x00030003	/* 00011b */
#define	CRAM_DIDR_VENDOR_ID_MASK	0x001f001f	/* DIDR[4:0] */
#define	CRAM_DEVID_NOT_SUPPORTED	0x00000000

#define PSRAM_PASS	0x50415353	/* "PASS" */
#define PSRAM_FAIL	0x4641494C	/* "FAIL" */

static u32 is_cram_inited(void);
static u32 is_cram(void);
static long int cram_init(u32);
static void cram_bcr_write(u32);
void udelay (unsigned long);

void sdram_init(void)
{
	volatile unsigned long spr_reg;

	/*
	 * If CRAM not initialized or CRAM looks initialized because this
	 * is after a warm reboot then set SPRG7 to indicate CRAM needs
	 * initialization.  Note that CRAM is initialized by the SPI and
	 * NAND preloader.
	 */
	spr_reg = (volatile unsigned long) mfspr(SPRG6);
	if ((is_cram_inited() != 1) || (spr_reg != LOAK_SPL)) {
		mtspr(SPRG7, LOAK_NONE);	/* "NONE" */
	}
#if 1
	/*
	 * When running the NAND SPL, the normal EBC configuration is not
	 * done, so We need to enable EPLD access on EBC_CS_2 and the memory
	 * on EBC_CS_3
	 */

	/* Enable CPLD - Needed for PSRAM Access */


	/* Init SDRAM by setting EBC Bank 3 for PSRAM */
	mtebc(pb1ap, CFG_EBC_PB1AP);
	mtebc(pb1cr, CFG_EBC_PB1CR);

	mtebc(pb2ap, CFG_EBC_PB2AP);
	mtebc(pb2cr, CFG_EBC_PB2CR);

	/* pre-boot loader code: we are in OCM */
	mtspr(SPRG6, LOAK_SPL);	/* "SPL " */
	mtspr(SPRG7, LOAK_OCM);	/* "OCM " */
#endif
	return;
}

static void cram_bcr_write(u32 wr_val)
{
	u32 tmp_reg;
	u32 val;
	volatile u32 gpio_reg;

	/* # Program CRAM write */

	/*
	 * set CRAM_CRE = 0x1
	 * set wr_val = wr_val << 2
	 */
	gpio_reg = in32(GPIO1_OR);
	out32(GPIO1_OR,  gpio_reg | 0x00000400);
	wr_val = wr_val << 2;
	/* wr_val = 0x1c048; */

	/*
	 * # stop PLL clock before programming CRAM
	 * set EPLD0_MUX_CTL.OESPR3 = 1
	 * delay 2
	 */

	/*
	 * # CS1
	 * read 0x00200000
	 * #shift 2 bit left before write
	 * set val = wr_val + 0x00200000
	 * write dmem val 0
	 * read 0x00200000 val
	 * print val/8x
	 */
	tmp_reg = in32(0x00200000);
	val = wr_val + 0x00200000;
	/* val = 0x0021c048; */
	out32(val, 0x0000);
	udelay(100000);
	val = in32(0x00200000);

	debug("CRAM VAL: %x for CS1 ", val);

	/*
	 * # CS2
	 * read 0x02200000
	 * #shift 2 bit left before write
	 * set val = wr_val + 0x02200000
	 * write dmem val 0
	 * read 0x02200000 val
	 * print val/8x
	 */
	tmp_reg = in32(0x02200000);
	val = wr_val + 0x02200000;
	/* val = 0x0221c048; */
	out32(val, 0x0000);
	udelay(100000);
	val = in32(0x02200000);

	debug("CRAM VAL: %x for CS2 ", val);

	/*
	 * # Start PLL clock before programming CRAM
	 * set EPLD0_MUX_CTL.OESPR3 = 0
	 */

	/*
	 * set CRAMCR = 0x1
	 */
	gpio_reg = in32(GPIO1_OR);
	out32(GPIO1_OR,  gpio_reg | 0x00000400);

	/*
	 * # read CRAM config BCR ( bit19:18 = 10b )
	 * #read 0x00200000
	 * # 1001_1001_0001_1111 ( 991f ) =>
	 * #10_0110_0100_0111_1100  =>   2647c => 0022647c
	 * #0011_0010_0011_1110 (323e)
	 * #
	 */

	/*
	 * set EPLD0_MUX_CTL.CRAMCR = 0x0
	 */
	gpio_reg = in32(GPIO1_OR);
	out32(GPIO1_OR,  gpio_reg & 0xFFFFFBFF);
	return;
}

static u32 is_cram_inited()
{
	volatile unsigned long spr_reg;

	/*
 	 * If CRAM is initialized already, then don't reinitialize it again.
	 * In the case of NAND boot and SPI boot, CRAM will already be
	 * initialized by the pre-loader
	 */
	spr_reg = (volatile unsigned long) mfspr(SPRG7);
	if (spr_reg == LOAK_CRAM) {
		return 1;
	} else {
		return 0;
	}
}

/******
 * return 0 if not CRAM
 * return 1 if CRAM and it's already inited by preloader
 * else return cram_id (CRAM Device Identification Register)
 ******/
static u32 is_cram(void)
{
	u32 gpio_TCR, gpio_OSRL, gpio_OR, gpio_ISR1L;
	volatile u32 gpio_reg;
	volatile u32 cram_id = 0;

	if (is_cram_inited() == 1) {
		/* this is CRAM and it is already inited (by preloader) */
		cram_id = 1;
	} else {
		/*
		 * # CRAM CLOCK
		 * set GPIO0_TCR.G8 = 1
		 * set GPIO0_OSRL.G8 = 0
		 * set GPIO0_OR.G8 = 0
		 */
		gpio_reg = in32(GPIO0_TCR);
		gpio_TCR = gpio_reg;
		out32(GPIO0_TCR, gpio_reg | 0x00800000);
		gpio_reg = in32(GPIO0_OSRL);
		gpio_OSRL = gpio_reg;
		out32(GPIO0_OSRL, gpio_reg & 0xffffbfff);
		gpio_reg = in32(GPIO0_OR);
		gpio_OR = gpio_reg;
		out32(GPIO0_OR, gpio_reg & 0xff7fffff);

		/*
		 * # CRAM Addreaa Valid
		 * set GPIO0_TCR.G10 = 1
		 * set GPIO0_OSRL.G10 = 0
		 * set GPIO0_OR.G10 = 0
		 */
		gpio_reg = in32(GPIO0_TCR);
		out32(GPIO0_TCR, gpio_reg | 0x00200000);
		gpio_reg = in32(GPIO0_OSRL);
		out32(GPIO0_OSRL, gpio_reg & 0xfffffbff);
		gpio_reg = in32(GPIO0_OR);
		out32(GPIO0_OR, gpio_reg & 0xffdfffff);

		/*
		 * # config input (EBC_WAIT)
		 * set GPIO0_ISR1L.G9 = 1
		 * set GPIO0_TCR.G9 = 0
		 */
		gpio_reg = in32(GPIO0_ISR1L);
		gpio_ISR1L = gpio_reg;
		out32(GPIO0_ISR1L, gpio_reg | 0x00001000);
		gpio_reg = in32(GPIO0_TCR);
		out32(GPIO0_TCR, gpio_reg & 0xffbfffff);

		/*
		 * Enable CRE to read Registers
		 * set GPIO0_TCR.21 = 1
		 * set GPIO1_OR.21 = 1
		 */
		gpio_reg = in32(GPIO1_TCR);
		out32(GPIO1_TCR, gpio_reg | 0x00000400);

		gpio_reg = in32(GPIO1_OR);
		out32(GPIO1_OR,  gpio_reg | 0x00000400);

		/* Read Version ID */
		cram_id = (volatile u32) in32(CRAM_BANK0_BASE+CRAM_DIDR);
		udelay(100000);

		asm volatile("	sync");
		asm volatile("	eieio");

		debug("Cram ID: %X ", cram_id);

		switch (cram_id) {
		case MICRON_MT45W8MW16BGX_CRAM_ID:
		case MICRON_MT45W8MW16BGX_CRAM_ID2:
			/* supported CRAM vendor/part */
			break;
		case CRAM_DEVID_NOT_SUPPORTED:
		default:
			/* check for DIDR Vendor ID of Micron */
			if ((cram_id & CRAM_DIDR_VENDOR_ID_MASK) ==
						MICRON_DIDR_VENDOR_ID)
			{
				/* supported CRAM vendor */
				break;
			}
			/* this is not CRAM or not supported CRAM vendor/part */
			cram_id = 0;
			/*
			 * reset the GPIO registers to the values that were
			 * there before this routine
			 */
			out32(GPIO0_TCR, gpio_TCR);
			out32(GPIO0_OSRL, gpio_OSRL);
			out32(GPIO0_OR, gpio_OR);
			out32(GPIO0_ISR1L, gpio_ISR1L);
			break;
		}
	}

	return cram_id;
}

static long int cram_init(u32 already_inited)
{
	volatile u32 tmp_reg;
	u32 cram_wr_val;

	if (already_inited == 0) return 0;

	/*
	 * If CRAM is initialized already, then don't reinitialize it again.
	 * In the case of NAND boot and SPI boot, CRAM will already be
	 * initialized by the pre-loader
	 */
	if (already_inited != 1) {
		/*
		 * #o CRAM Card
		 * #  - CRAMCRE @reg16 = 1; for CRAM to use
		 * #  - CRAMCRE @reg16 = 0; for CRAM to program
		 *
		 * # enable CRAM SEL, move from setEPLD.cmd
		 * set EPLD0_MUX_CTL.OECRAM = 0
		 * set EPLD0_MUX_CTL.CRAMCR = 1
		 * set EPLD0_ETHRSTBOOT.SLCRAM = 0
		 * #end
		 */

		/*
		 * #1. EBC need to program READY, CLK, ADV for ASync mode
		 * # config output
		 */

		/*
		 * # CRAM CLOCK
		 * set GPIO0_TCR.G8 = 1
		 * set GPIO0_OSRL.G8 = 0
		 * set GPIO0_OR.G8 = 0
		 */
		tmp_reg = in32(GPIO0_TCR);
		out32(GPIO0_TCR, tmp_reg | 0x00800000);
		tmp_reg = in32(GPIO0_OSRL);
		out32(GPIO0_OSRL, tmp_reg & 0xffffbfff);
		tmp_reg = in32(GPIO0_OR);
		out32(GPIO0_OR, tmp_reg & 0xff7fffff);

		/*
		 * # CRAM Addreaa Valid
		 * set GPIO0_TCR.G10 = 1
		 * set GPIO0_OSRL.G10 = 0
		 * set GPIO0_OR.G10 = 0
		 */
		tmp_reg = in32(GPIO0_TCR);
		out32(GPIO0_TCR, tmp_reg | 0x00200000);
		tmp_reg = in32(GPIO0_OSRL);
		out32(GPIO0_OSRL, tmp_reg & 0xfffffbff);
		tmp_reg = in32(GPIO0_OR);
		out32(GPIO0_OR, tmp_reg & 0xffdfffff);

		/*
		 * # config input (EBC_WAIT)
		 * set GPIO0_ISR1L.G9 = 1
		 * set GPIO0_TCR.G9 = 0
		 */
		tmp_reg = in32(GPIO0_ISR1L);
		out32(GPIO0_ISR1L, tmp_reg | 0x00001000);
		tmp_reg = in32(GPIO0_TCR);
		out32(GPIO0_TCR, tmp_reg & 0xffbfffff);

		/*
		 * # config CS4 from GPIO
		 * set GPIO0_TCR.G0 = 1
		 * set GPIO0_OSRL.G0 = 1
		 */
		tmp_reg = in32(GPIO0_TCR);
		out32(GPIO0_TCR, tmp_reg | 0x80000000);
		tmp_reg = in32(GPIO0_OSRL);
		out32(GPIO0_OSRL, tmp_reg | 0x40000000);

		/*
		 * #2. EBC in Async mode
		 * # set EBC0_PB1AP = 0x078f0ec0
		 * set EBC0_PB1AP = 0x078f1ec0
		 * set EBC0_PB2AP = 0x078f1ec0
		 */
		mtebc(pb1ap, 0x078F1EC0);
		mtebc(pb2ap, 0x078F1EC0);

		/*
		 * #set EBC0_PB1CR = 0x000bc000
		 * #enable CS2 for CRAM
		 * set EBC0_PB2CR = 0x020bc000
		 */
		mtebc(pb1cr, 0x000BC000);
		mtebc(pb2cr, 0x020BC000);

		/*
		 * #3. set CRAM in Sync mode
		 * #exec cm_bcr_write.cmd { 0x701f }
		 * #3. set CRAM in Sync mode (full drv strength)
		 * exec cm_bcr_write.cmd { 0x701F }
		 */
		cram_wr_val = 0x7012;	/* CRAM burst setting */
		cram_bcr_write(cram_wr_val);

		/*
		 * #4. EBC in Sync mode
		 * #set EBC0_PB1AP = 0x9f800fc0
		 * #set EBC0_PB1AP = 0x900001c0
		 * set EBC0_PB2AP = 0x9C0201c0
		 * set EBC0_PB2AP = 0x9C0201c0
		 */
		mtebc(pb1ap, 0x9C0201C0);
		mtebc(pb2ap, 0x9C0201C0);

		/*
		 * #5. EBC need to program READY, CLK, ADV for Sync mode
		 * # config output
		 * set GPIO0_TCR.G8 = 1
		 * set GPIO0_OSRL.G8 = 1
		 * set GPIO0_TCR.G10 = 1
		 * set GPIO0_OSRL.G10 = 1
		 */
		tmp_reg = in32(GPIO0_TCR);
		out32(GPIO0_TCR, tmp_reg | 0x00800000);
		tmp_reg = in32(GPIO0_OSRL);
		out32(GPIO0_OSRL, tmp_reg | 0x00004000);
		tmp_reg = in32(GPIO0_TCR);
		out32(GPIO0_TCR, tmp_reg | 0x00200000);
		tmp_reg = in32(GPIO0_OSRL);
		out32(GPIO0_OSRL, tmp_reg | 0x00000400);

		/*
		 * # config input
		 * set GPIO0_ISR1L.G9 = 1
		 * set GPIO0_TCR.G9 = 0
		 */
		tmp_reg = in32(GPIO0_ISR1L);
		out32(GPIO0_ISR1L, tmp_reg | 0x00001000);
		tmp_reg = in32(GPIO0_TCR);
		out32(GPIO0_TCR, tmp_reg & 0xffbfffff);

		/*
		 * # config EBC to use RDY
		 * set SDR0_ULTRA0.EBCREN = 1
		 */
		mfsdr(sdrultra0, tmp_reg);
		mtsdr(sdrultra0, tmp_reg | 0x04000000);

		/*
		 * set EPLD0_MUX_CTL.OESPR3 = 0
		 */
		mtspr(SPRG7, LOAK_CRAM);	/* "CRAM" */
	} /* if (already_inited != 1) */

	return (64 * 1024 * 1024);
}

/******
 * return 0 if not PSRAM
 * return 1 if is PSRAM
 ******/
static int is_psram(u32 addr)
{
	u32 test_pattern = 0xdeadbeef;
	volatile u32 readback;

	if (addr == CFG_SDRAM_BASE) {
		/* This is to temp enable OE for PSRAM */
		out16(EPLD_BASE+EPLD_MUXOE, 0x7f0f);
		udelay(10000);
	}

	out32(addr, test_pattern);
	asm volatile("	sync");
	asm volatile("	eieio");

	readback = (volatile u32) in32(addr);
	asm volatile("	sync");
	asm volatile("	eieio");
	if (readback == test_pattern) {
		return 1;
	} else {
		return 0;
	}
}

static long int psram_init(void)
{
	u32 readback;
	long psramsize = 0;
	int i;

	/* This is to temp enable OE for PSRAM */
	out16(EPLD_BASE+EPLD_MUXOE, 0x7f0f);
	udelay(10000);

	/*
	 * PSRAM bank 1: read then write to address 0x00000000
	 */
	for (i = 0; i < 100; i++) {
		if (is_psram(CFG_SDRAM_BASE + (i*256)) == 1) {
			readback = PSRAM_PASS;
		} else {
			readback = PSRAM_FAIL;
			break;
		}
	}
	if (readback == PSRAM_PASS) {
		debug("psram_init(bank0): pass\n");
		psramsize = (16 * 1024 * 1024);
	} else {
		debug("psram_init(bank0): fail\n");
		return 0;
	}

#if 0
	/*
	 * PSRAM bank 1: read then write to address 0x01000000
	 */
	for (i = 0; i < 100; i++) {
		if (is_psram((1 << 24) + (i*256)) == 1) {
			readback = PSRAM_PASS;
		} else {
			readback = PSRAM_FAIL;
			break;
		}
	}
	if (readback == PSRAM_PASS) {
		debug("psram_init(bank1): pass\n");
		psramsize = psramsize + (16 * 1024 * 1024);
	}
#endif

	mtspr(SPRG7, LOAK_PSRAM);	/* "PSRA" - PSRAM */

	return psramsize;
}

long int initdram(int board_type)
{
	long int sram_size;
	u32 cram_inited;

	/* Determine Attached Memory Expansion Card*/
	cram_inited = is_cram();
	if (cram_inited != 0) {					/* CRAM */
		debug("CRAM Expansion Card attached\n");
		sram_size = cram_init(cram_inited);
	} else if (is_psram(CFG_SDRAM_BASE+4) == 1) {		/* PSRAM */
		debug("PSRAM Expansion Card attached\n");
		sram_size = psram_init();
	} else { 						/* no SRAM */
		debug("No Memory Card Attached!!\n");
		sram_size = 0;
	}

	return sram_size;
}

int testdram(void)
{
	return (0);
}
