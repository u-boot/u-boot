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

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_ECC

/*
 * MEMORY ECC test
 *
 * This test performs the checks ECC facility of memory.
 */
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/io.h>
#include <asm/ppc440.h>

DECLARE_GLOBAL_DATA_PTR;

const static uint8_t syndrome_codes[] = {
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
#define ECC_PATTERN_UNCORR	0x61010101

inline static void disable_ecc(void)
{
	uint32_t value;

	sync(); /* Wait for any pending memory accesses to complete. */
	mfsdram(DDR0_22, value);
	mtsdram(DDR0_22, (value & ~DDR0_22_CTRL_RAW_MASK)
		| DDR0_22_CTRL_RAW_ECC_DISABLE);
}

inline static void clear_and_enable_ecc(void)
{
	uint32_t value;

	sync(); /* Wait for any pending memory accesses to complete. */
	mfsdram(DDR0_00, value);
	mtsdram(DDR0_00, value | DDR0_00_INT_ACK_ALL);
	mfsdram(DDR0_22, value);
	mtsdram(DDR0_22, (value & ~DDR0_22_CTRL_RAW_MASK)
		| DDR0_22_CTRL_RAW_ECC_ENABLE);
}

static uint32_t get_ecc_status(void)
{
	uint32_t int_status;
#if defined(DEBUG)
	uint8_t syndrome;
	uint32_t hdata, ldata, haddr, laddr;
	uint32_t value;
#endif

	mfsdram(DDR0_00, int_status);
	int_status &= DDR0_00_INT_STATUS_MASK;

#if defined(DEBUG)
	if (int_status & (DDR0_00_INT_STATUS_BIT0 | DDR0_00_INT_STATUS_BIT1)) {
		mfsdram(DDR0_32, laddr);
		mfsdram(DDR0_33, haddr);
		haddr &= 0x00000001;
		if (int_status & DDR0_00_INT_STATUS_BIT1)
			debug("Multiple accesses");
		else
			debug("A single access");

		debug(" outside the defined physical memory space detected\n"
		      "        addr = 0x%01x%08x\n", haddr, laddr);
	}
	if (int_status & (DDR0_00_INT_STATUS_BIT2 | DDR0_00_INT_STATUS_BIT3)) {
		unsigned int bit;

		mfsdram(DDR0_23, value);
		syndrome = (value >> 16) & 0xff;
		for (bit = 0; bit < sizeof(syndrome_codes); bit++)
			if (syndrome_codes[bit] == syndrome)
				break;

		mfsdram(DDR0_38, laddr);
		mfsdram(DDR0_39, haddr);
		haddr &= 0x00000001;
		mfsdram(DDR0_40, ldata);
		mfsdram(DDR0_41, hdata);
		if (int_status & DDR0_00_INT_STATUS_BIT3)
			debug("Multiple correctable ECC events");
		else
			debug("Single correctable ECC event");

		debug(" detected\n        0x%01x%08x - 0x%08x%08x, bit - %d\n",
		      haddr, laddr, hdata, ldata, bit);
	}
	if (int_status & (DDR0_00_INT_STATUS_BIT4 | DDR0_00_INT_STATUS_BIT5)) {
		mfsdram(DDR0_23, value);
		syndrome = (value >> 8) & 0xff;
		mfsdram(DDR0_34, laddr);
		mfsdram(DDR0_35, haddr);
		haddr &= 0x00000001;
		mfsdram(DDR0_36, ldata);
		mfsdram(DDR0_37, hdata);
		if (int_status & DDR0_00_INT_STATUS_BIT5)
			debug("Multiple uncorrectable ECC events");
		else
			debug("Single uncorrectable ECC event");

		debug(" detected\n        0x%01x%08x - 0x%08x%08x, "
		      "syndrome - 0x%02x\n",
		      haddr, laddr, hdata, ldata, syndrome);
	}
	if (int_status & DDR0_00_INT_STATUS_BIT6)
		debug("DRAM initialization complete\n");
#endif /* defined(DEBUG) */

	return int_status;
}

static int test_ecc(uint32_t ecc_addr)
{
	uint32_t value;
	volatile uint32_t *const ecc_mem = (volatile uint32_t *)ecc_addr;
	int ret = 0;

	WATCHDOG_RESET();

	debug("Entering test_ecc(0x%08x)\n", ecc_addr);
	/* Set up correct ECC in memory */
	disable_ecc();
	clear_and_enable_ecc();
	out_be32(ecc_mem, ECC_PATTERN);
	out_be32(ecc_mem + 1, ECC_PATTERN);
	ppcDcbf((u32)ecc_mem);

	/* Verify no ECC error reading back */
	value = in_be32(ecc_mem);
	disable_ecc();
	if (ECC_PATTERN != value) {
		debug("Data read error (no-error case): "
		      "expected 0x%08x, read 0x%08x\n", ECC_PATTERN, value);
		ret = 1;
	}
	value = get_ecc_status();
	if (0x00000000 != value) {
		/* Expected no ECC status reported */
		debug("get_ecc_status(): expected 0x%08x, got 0x%08x\n",
		      0x00000000, value);
		ret = 1;
	}

	/* Test for correctable error by creating a one-bit error */
	out_be32(ecc_mem, ECC_PATTERN_CORR);
	ppcDcbf((u32)ecc_mem);
	clear_and_enable_ecc();
	value = in_be32(ecc_mem);
	disable_ecc();
	/* Test that the corrected data was read */
	if (ECC_PATTERN != value) {
		debug("Data read error (correctable-error case): "
		      "expected 0x%08x, read 0x%08x\n", ECC_PATTERN, value);
		ret = 1;
	}
	value = get_ecc_status();
	if ((DDR0_00_INT_STATUS_BIT2 | DDR0_00_INT_STATUS_BIT7) != value) {
		/* Expected a single correctable error reported */
		debug("get_ecc_status(): expected 0x%08x, got 0x%08x\n",
		      DDR0_00_INT_STATUS_BIT2, value);
		ret = 1;
	}

	/* Test for uncorrectable error by creating a two-bit error */
	out_be32(ecc_mem, ECC_PATTERN_UNCORR);
	ppcDcbf((u32)ecc_mem);
	clear_and_enable_ecc();
	value = in_be32(ecc_mem);
	disable_ecc();
	/* Test that the corrected data was read */
	if (ECC_PATTERN_UNCORR != value) {
		debug("Data read error (uncorrectable-error case): "
		      "expected 0x%08x, read 0x%08x\n", ECC_PATTERN_UNCORR,
		      value);
		ret = 1;
	}
	value = get_ecc_status();
	if ((DDR0_00_INT_STATUS_BIT4 | DDR0_00_INT_STATUS_BIT7) != value) {
		/* Expected a single uncorrectable error reported */
		debug("get_ecc_status(): expected 0x%08x, got 0x%08x\n",
		      DDR0_00_INT_STATUS_BIT4, value);
		ret = 1;
	}

	/* Remove error from SDRAM and enable ECC. */
	out_be32(ecc_mem, ECC_PATTERN);
	ppcDcbf((u32)ecc_mem);
	clear_and_enable_ecc();

	return ret;
}

int ecc_post_test(int flags)
{
	int ret = 0;
	uint32_t value;
	uint32_t iaddr;

	mfsdram(DDR0_22, value);
	if (0x3 != DDR0_22_CTRL_RAW_DECODE(value)) {
		debug("SDRAM ECC not enabled, skipping ECC POST.\n");
		return 0;
	}

	/* Mask all interrupts. */
	mfsdram(DDR0_01, value);
	mtsdram(DDR0_01, (value & ~DDR0_01_INT_MASK_MASK)
		| DDR0_01_INT_MASK_ALL_OFF);

	for (iaddr = ECC_START_ADDR; iaddr <= ECC_STOP_ADDR; iaddr += iaddr) {
		ret = test_ecc(iaddr);
		if (ret)
			break;
	}
	/*
	 * Clear possible errors resulting from ECC testing.  (If not done, we
	 * we could get an interrupt later on when exceptions are enabled.)
	 */
	set_mcsr(get_mcsr());
	debug("ecc_post_test() returning %d\n", ret);
	return ret;
}
#endif /* CONFIG_POST & CONFIG_SYS_POST_ECC */
#endif /* defined(CONFIG_440EPX) || defined(CONFIG_440GRX) */
