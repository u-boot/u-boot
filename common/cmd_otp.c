/*
 * cmd_otp.c - interface to Blackfin on-chip One-Time-Programmable memory
 *
 * Copyright (c) 2007-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

/* There are 512 128-bit "pages" (0x000 to 0x1FF).
 * The pages are accessable as 64-bit "halfpages" (an upper and lower half).
 * The pages are not part of the memory map.  There is an OTP controller which
 * handles scanning in/out of bits.  While access is done through OTP MMRs,
 * the bootrom provides C-callable helper functions to handle the interaction.
 */

#include <config.h>
#include <common.h>
#include <command.h>

#ifdef CONFIG_CMD_OTP

#include <asm/blackfin.h>
#include <asm/mach-common/bits/otp.h>

static const char *otp_strerror(uint32_t err)
{
	switch (err) {
	case 0:                   return "no error";
	case OTP_WRITE_ERROR:     return "OTP fuse write error";
	case OTP_READ_ERROR:      return "OTP fuse read error";
	case OTP_ACC_VIO_ERROR:   return "invalid OTP address";
	case OTP_DATA_MULT_ERROR: return "multiple bad bits detected";
	case OTP_ECC_MULT_ERROR:  return "error in ECC bits";
	case OTP_PREV_WR_ERROR:   return "space already written";
	case OTP_DATA_SB_WARN:    return "single bad bit in half page";
	case OTP_ECC_SB_WARN:     return "single bad bit in ECC";
	default:                  return "unknown error";
	}
}

#define lowup(x) ((x) % 2 ? "upper" : "lower")

int do_otp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	bool force = false;
	if (!strcmp(argv[1], "--force")) {
		force = true;
		argv[1] = argv[0];
		argv++;
		--argc;
	}

	uint32_t (*otp_func)(uint32_t page, uint32_t flags, uint64_t *page_content);
	if (!strcmp(argv[1], "read"))
		otp_func = otp_read;
	else if (!strcmp(argv[1], "write"))
		otp_func = otp_write;
	else {
 usage:
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	uint64_t *addr = (uint64_t *)simple_strtoul(argv[2], NULL, 16);
	uint32_t page = simple_strtoul(argv[3], NULL, 16);
	uint32_t flags, ret;
	size_t i, count;
	ulong half;

	if (argc > 4)
		count = simple_strtoul(argv[4], NULL, 16);
	else
		count = 2;

	if (argc > 5) {
		half = simple_strtoul(argv[5], NULL, 16);
		if (half != 0 && half != 1) {
			puts("Error: 'half' can only be '0' or '1'\n");
			goto usage;
		}
	} else
		half = 0;

	/* do to the nature of OTP, make sure users are sure */
	if (!force && otp_func == otp_write) {
		printf(
			"Writing one time programmable memory\n"
			"Make sure your operating voltages and temperature are within spec\n"
			"   source address:  0x%p\n"
			"   OTP destination: %s page 0x%03X - %s page 0x%03X\n"
			"   number to write: %ld halfpages\n"
			" type \"YES\" (no quotes) to confirm: ",
			addr,
			lowup(half), page,
			lowup(half + count - 1), page + (half + count - 1) / 2,
			half + count
		);

		i = 0;
		while (1) {
			if (tstc()) {
				const char exp_ans[] = "YES\r";
				char c;
				putc(c = getc());
				if (exp_ans[i++] != c) {
					printf(" Aborting\n");
					return 1;
				} else if (!exp_ans[i]) {
					puts("\n");
					break;
				}
			}
		}

		/* Only supported in newer silicon ... enable writing */
#if (0)
		otp_command(OTP_INIT, ...);
#else
		*pOTP_TIMING = 0x32149485;
#endif
	}

	printf("OTP memory %s: addr 0x%08lx  page 0x%03X  count %ld ... ",
		argv[1], addr, page, count);

	ret = 0;
	for (i = half; i < count + half; ++i) {
		flags = (i % 2) ? OTP_UPPER_HALF : OTP_LOWER_HALF;
		ret = otp_func(page, flags, addr);
		if (ret & 0x1)
			break;
		else if (ret)
			puts("W");
		else
			puts(".");
		++addr;
		if (i % 2)
			++page;
	}
	if (ret & 0x1)
		printf("\nERROR at page 0x%03X (%s-halfpage): 0x%03X: %s\n",
			page, lowup(i), ret, otp_strerror(ret));
	else
		puts(" done\n");

	if (otp_func == otp_write)
		/* Only supported in newer silicon ... disable writing */
#if (0)
		otp_command(OTP_INIT, ...);
#else
		*pOTP_TIMING = 0x1485;
#endif

	return ret;
}

U_BOOT_CMD(otp, 6, 0, do_otp,
	"otp - One-Time-Programmable sub-system\n",
	"read <addr> <page> [count] [half]\n"
	"otp write [--force] <addr> <page> [count] [half]\n"
	"    - read/write 'count' half-pages starting at page 'page' (offset 'half')\n");

#endif
