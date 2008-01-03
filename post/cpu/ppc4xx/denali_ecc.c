/*
 * (C) Copyright 2007
 * Developed for DENX Software Engineering GmbH.
 *
 * Author: Pavel Kolesnikov <concord@emcraft.com>
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

/* define DEBUG for debugging output (obviously ;-)) */
#if 0
#define DEBUG
#endif

#include <common.h>
#include <watchdog.h>

#if defined(CONFIG_POST) && (defined(CONFIG_440EPX) || defined(CONFIG_440GRX))

#include <post.h>

#if CONFIG_POST & CFG_POST_ECC

/*
 * MEMORY ECC test
 *
 * This test performs the checks ECC facility of memory.
 */
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/io.h>
#include <ppc440.h>

DECLARE_GLOBAL_DATA_PTR;

const static unsigned char syndrome_codes[] = {
	0xF4, 0XF1, 0XEC, 0XEA, 0XE9, 0XE6, 0XE5, 0XE3,
	0XDC, 0XDA, 0XD9, 0XD6, 0XD5, 0XD3, 0XCE, 0XCB,
	0xB5, 0XB0, 0XAD, 0XAB, 0XA8, 0XA7, 0XA4, 0XA2,
	0X9D, 0X9B, 0X98, 0X97, 0X94, 0X92, 0X8F, 0X8A,
	0x75, 0x70, 0X6D, 0X6B, 0X68, 0X67, 0X64, 0X62,
	0X5E, 0X5B, 0X58, 0X57, 0X54, 0X52, 0X4F, 0X4A,
	0x34, 0x31, 0X2C, 0X2A, 0X29, 0X26, 0X25, 0X23,
	0X1C, 0X1A, 0X19, 0X16, 0X15, 0X13, 0X0E, 0X0B,
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

#define ECC_START_ADDR		0x10
#define ECC_STOP_ADDR		0x2000
#define ECC_PATTERN		0x01010101
#define ECC_PATTERN_CORR	0x11010101
#define ECC_PATTERN_UNCORR	0xF1010101

static int test_ecc_error(void)
{
	unsigned long value;
	unsigned long hdata, ldata, haddr, laddr;
	unsigned int bit;

	int ret = 0;

	mfsdram(DDR0_23, value);

	for (bit = 0; bit < sizeof(syndrome_codes); bit++)
		if (syndrome_codes[bit] == ((value >> 16) & 0xff))
			break;

	mfsdram(DDR0_00, value);

	if (value & DDR0_00_INT_STATUS_BIT0) {
		debug("Bit0. A single access outside the defined PHYSICAL"
		      " memory space detected\n");
		mfsdram(DDR0_32, laddr);
		mfsdram(DDR0_33, haddr);
		debug("        addr = 0x%08x%08x\n", haddr, laddr);
		ret = 1;
	}
	if (value & DDR0_00_INT_STATUS_BIT1) {
		debug("Bit1. Multiple accesses outside the defined PHYSICAL"
		      " memory space detected\n");
		ret = 2;
	}
	if (value & DDR0_00_INT_STATUS_BIT2) {
		debug("Bit2. Single correctable ECC event detected\n");
		mfsdram(DDR0_38, laddr);
		mfsdram(DDR0_39, haddr);
		mfsdram(DDR0_40, ldata);
		mfsdram(DDR0_41, hdata);
		debug("        0x%08x - 0x%08x%08x, bit - %d\n",
		      laddr, hdata, ldata, bit);
		ret = 3;
	}
	if (value & DDR0_00_INT_STATUS_BIT3) {
		debug("Bit3. Multiple correctable ECC events detected\n");
		mfsdram(DDR0_38, laddr);
		mfsdram(DDR0_39, haddr);
		mfsdram(DDR0_40, ldata);
		mfsdram(DDR0_41, hdata);
		debug("        0x%08x - 0x%08x%08x, bit - %d\n",
		      laddr, hdata, ldata, bit);
		ret = 4;
	}
	if (value & DDR0_00_INT_STATUS_BIT4) {
		debug("Bit4. Single uncorrectable ECC event detected\n");
		mfsdram(DDR0_34, laddr);
		mfsdram(DDR0_35, haddr);
		mfsdram(DDR0_36, ldata);
		mfsdram(DDR0_37, hdata);
		debug("        0x%08x - 0x%08x%08x, bit - %d\n",
		      laddr, hdata, ldata, bit);
		ret = 5;
	}
	if (value & DDR0_00_INT_STATUS_BIT5) {
		debug("Bit5. Multiple uncorrectable ECC events detected\n");
		mfsdram(DDR0_34, laddr);
		mfsdram(DDR0_35, haddr);
		mfsdram(DDR0_36, ldata);
		mfsdram(DDR0_37, hdata);
		debug("        0x%08x - 0x%08x%08x, bit - %d\n",
		      laddr, hdata, ldata, bit);
		ret = 6;
	}
	if (value & DDR0_00_INT_STATUS_BIT6) {
		debug("Bit6. DRAM initialization complete\n");
		ret = 7;
	}

	/* error status cleared */
	mfsdram(DDR0_00, value);
	mtsdram(DDR0_00, value | DDR0_00_INT_ACK_ALL);

	return ret;
}

static int test_ecc(unsigned long ecc_addr)
{
	unsigned long value;
	volatile unsigned *const ecc_mem = (volatile unsigned *) ecc_addr;
	int pret;
	int ret = 0;

	sync();
	eieio();
	WATCHDOG_RESET();

	debug("Entering test_ecc(0x%08lX)\n", ecc_addr);
	out_be32(ecc_mem, ECC_PATTERN);
	out_be32(ecc_mem + 1, ECC_PATTERN);
	in_be32(ecc_mem);
	pret = test_ecc_error();
	if (pret != 0) {
		debug("pret: expected 0, got %d\n", pret);
		ret = 1;
	}
	/* test for correctable error */
	/* disconnect from ecc storage */
	mfsdram(DDR0_22, value);
	mtsdram(DDR0_22, (value & ~DDR0_22_CTRL_RAW_MASK)
		| DDR0_22_CTRL_RAW_ECC_DISABLE);

	/* creating (correctable) single-bit error */
	out_be32(ecc_mem, ECC_PATTERN_CORR);

	/* enable ecc */
	mfsdram(DDR0_22, value);
	mtsdram(DDR0_22, (value & ~DDR0_22_CTRL_RAW_MASK)
		| DDR0_22_CTRL_RAW_ECC_ENABLE);
	sync();
	eieio();

	in_be32(ecc_mem);
	pret = test_ecc_error();
	/* if read data ok, 1 correctable error must be fixed */
	if (pret != 3) {
		debug("pret: expected 3, got %d\n", pret);
		ret = 1;
	}
	/* test for uncorrectable error */
	/* disconnect from ecc storage */
	mfsdram(DDR0_22, value);
	mtsdram(DDR0_22, (value & ~DDR0_22_CTRL_RAW_MASK)
		| DDR0_22_CTRL_RAW_NO_ECC_RAM);

	/* creating (uncorrectable) multiple-bit error */
	out_be32(ecc_mem, ECC_PATTERN_UNCORR);

	/* enable ecc */
	mfsdram(DDR0_22, value);
	mtsdram(DDR0_22, (value & ~DDR0_22_CTRL_RAW_MASK)
		| DDR0_22_CTRL_RAW_ECC_ENABLE);
	sync();
	eieio();

	in_be32(ecc_mem);
	pret = test_ecc_error();
	/* info about uncorrectable error must appear */
	if (pret != 5) {
		debug("pret: expected 5, got %d\n", pret);
		ret = 1;
	}
	/* remove error from SDRAM */
	out_be32(ecc_mem, ECC_PATTERN);
	/* clear error caused by read-modify-write */
	mfsdram(DDR0_00, value);
	mtsdram(DDR0_00, value | DDR0_00_INT_ACK_ALL);

	sync();
	eieio();
	return ret;
}

int ecc_post_test (int flags)
{
	int ret = 0;
	unsigned long value;
	unsigned long iaddr;

	sync();
	eieio();

	mfsdram(DDR0_22, value);
	if (0x3 != DDR0_22_CTRL_RAW_DECODE(value)) {
		debug("SDRAM ECC not enabled, skipping ECC POST.\n");
		return 0;
	}

	/* mask all int */
	mfsdram(DDR0_01, value);
	mtsdram(DDR0_01, (value & ~DDR0_01_INT_MASK_MASK)
		| DDR0_01_INT_MASK_ALL_OFF);

	/* clear error status */
	mfsdram(DDR0_00, value);
	mtsdram(DDR0_00, value | DDR0_00_INT_ACK_ALL);

	for (iaddr = ECC_START_ADDR; iaddr <= ECC_STOP_ADDR; iaddr += iaddr) {
		ret = test_ecc(iaddr);
		if (ret)
			break;
	}
	/*
	 * Clear possible errors resulting from ECC testing.
	 * If not done, then we could get an interrupt later on when
	 * exceptions are enabled.
	 */
	set_mcsr(get_mcsr());
	return ret;

}
#endif /* CONFIG_POST & CFG_POST_ECC */
#endif /* defined(CONFIG_POST) && ... */
