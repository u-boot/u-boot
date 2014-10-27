/*
 * Keystone2: DDR3 test commands
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <asm/arch/hardware.h>
#include <asm/arch/ddr3.h>
#include <common.h>
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;

#define DDR_MIN_ADDR		CONFIG_SYS_SDRAM_BASE

#define DDR_REMAP_ADDR		0x80000000
#define ECC_START_ADDR1		((DDR_MIN_ADDR - DDR_REMAP_ADDR) >> 17)

#define ECC_END_ADDR1		(((gd->start_addr_sp - DDR_REMAP_ADDR - \
				 CONFIG_STACKSIZE) >> 17) - 2)

#define DDR_TEST_BURST_SIZE	1024

static int ddr_memory_test(u32 start_address, u32 end_address, int quick)
{
	u32 index_start, value, index;

	index_start = start_address;

	while (1) {
		/* Write a pattern */
		for (index = index_start;
				index < index_start + DDR_TEST_BURST_SIZE;
				index += 4)
			__raw_writel(index, index);

		/* Read and check the pattern */
		for (index = index_start;
				index < index_start + DDR_TEST_BURST_SIZE;
				index += 4) {
			value = __raw_readl(index);
			if (value != index) {
				printf("ddr_memory_test: Failed at address index = 0x%x value = 0x%x *(index) = 0x%x\n",
				       index, value, __raw_readl(index));

				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if (index_start >= end_address)
			break;

		if (quick)
			continue;

		/* Write a pattern for complementary values */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 4)
			__raw_writel((u32)~index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 4) {
			value = __raw_readl(index);
			if (value != ~index) {
				printf("ddr_memory_test: Failed at address index = 0x%x value = 0x%x *(index) = 0x%x\n",
				       index, value, __raw_readl(index));

				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if (index_start >= end_address)
			break;

		/* Write a pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 2)
			__raw_writew((u16)index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 2) {
			value = __raw_readw(index);
			if (value != (u16)index) {
				printf("ddr_memory_test: Failed at address index = 0x%x value = 0x%x *(index) = 0x%x\n",
				       index, value, __raw_readw(index));

				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if (index_start >= end_address)
			break;

		/* Write a pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 1)
			__raw_writeb((u8)index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 1) {
			value = __raw_readb(index);
			if (value != (u8)index) {
				printf("ddr_memory_test: Failed at address index = 0x%x value = 0x%x *(index) = 0x%x\n",
				       index, value, __raw_readb(index));

				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if (index_start >= end_address)
			break;
	}

	puts("ddr memory test PASSED!\n");
	return 0;
}

static int ddr_memory_compare(u32 address1, u32 address2, u32 size)
{
	u32 index, value, index2, value2;

	for (index = address1, index2 = address2;
	     index < address1 + size;
	     index += 4, index2 += 4) {
		value = __raw_readl(index);
		value2 = __raw_readl(index2);

		if (value != value2) {
			printf("ddr_memory_test: Compare failed at address = 0x%x value = 0x%x, address2 = 0x%x value2 = 0x%x\n",
			       index, value, index2, value2);

			return -1;
		}
	}

	puts("ddr memory compare PASSED!\n");
	return 0;
}

static int ddr_memory_ecc_err(u32 base, u32 address, u32 ecc_err)
{
	u32 value1, value2, value3;

	puts("Disabling DDR ECC ...\n");
	ddr3_disable_ecc(base);

	value1 = __raw_readl(address);
	value2 = value1 ^ ecc_err;
	__raw_writel(value2, address);

	value3 = __raw_readl(address);
	printf("ECC err test, addr 0x%x, read data 0x%x, wrote data 0x%x, err pattern: 0x%x, read after write data 0x%x\n",
	       address, value1, value2, ecc_err, value3);

	__raw_writel(ECC_START_ADDR1 | (ECC_END_ADDR1 << 16),
		     base + KS2_DDR3_ECC_ADDR_RANGE1_OFFSET);

	puts("Enabling DDR ECC ...\n");
	ddr3_enable_ecc(base, 1);

	value1 = __raw_readl(address);
	printf("ECC err test, addr 0x%x, read data 0x%x\n", address, value1);

	ddr3_check_ecc_int(base);
	return 0;
}

static int do_ddr_test(cmd_tbl_t *cmdtp,
		       int flag, int argc, char * const argv[])
{
	u32 start_addr, end_addr, size, ecc_err;

	if ((argc == 4) && (strncmp(argv[1], "ecc_err", 8) == 0)) {
		if (!ddr3_ecc_support_rmw(KS2_DDR3A_EMIF_CTRL_BASE)) {
			puts("ECC RMW isn't supported for this SOC\n");
			return 1;
		}

		start_addr = simple_strtoul(argv[2], NULL, 16);
		ecc_err = simple_strtoul(argv[3], NULL, 16);

		if ((start_addr < CONFIG_SYS_SDRAM_BASE) ||
		    (start_addr > (CONFIG_SYS_SDRAM_BASE +
		     CONFIG_MAX_RAM_BANK_SIZE - 1))) {
			puts("Invalid address!\n");
			return cmd_usage(cmdtp);
		}

		ddr_memory_ecc_err(KS2_DDR3A_EMIF_CTRL_BASE,
				   start_addr, ecc_err);
		return 0;
	}

	if (!(((argc == 4) && (strncmp(argv[1], "test", 5) == 0)) ||
	      ((argc == 5) && (strncmp(argv[1], "compare", 8) == 0))))
		return cmd_usage(cmdtp);

	start_addr = simple_strtoul(argv[2], NULL, 16);
	end_addr = simple_strtoul(argv[3], NULL, 16);

	if ((start_addr < CONFIG_SYS_SDRAM_BASE) ||
	    (start_addr > (CONFIG_SYS_SDRAM_BASE +
	     CONFIG_MAX_RAM_BANK_SIZE - 1)) ||
	    (end_addr < CONFIG_SYS_SDRAM_BASE) ||
	    (end_addr > (CONFIG_SYS_SDRAM_BASE +
	     CONFIG_MAX_RAM_BANK_SIZE - 1)) || (start_addr >= end_addr)) {
		puts("Invalid start or end address!\n");
		return cmd_usage(cmdtp);
	}

	puts("Please wait ...\n");
	if (argc == 5) {
		size = simple_strtoul(argv[4], NULL, 16);
		ddr_memory_compare(start_addr, end_addr, size);
	} else {
		ddr_memory_test(start_addr, end_addr, 0);
	}

	return 0;
}

U_BOOT_CMD(ddr,	5, 1, do_ddr_test,
	   "DDR3 test",
	   "test <start_addr in hex> <end_addr in hex> - test DDR from start\n"
	   "	address to end address\n"
	   "ddr compare <start_addr in hex> <end_addr in hex> <size in hex> -\n"
	   "	compare DDR data of (size) bytes from start address to end\n"
	   "	address\n"
	   "ddr ecc_err <addr in hex> <bit_err in hex> - generate bit errors\n"
	   "	in DDR data at <addr>, the command will read a 32-bit data\n"
	   "	from <addr>, and write (data ^ bit_err) back to <addr>\n"
);
