/*
 * cmd_otp.c - interface to Blackfin on-chip One-Time-Programmable memory
 *
 * Copyright (c) 2007-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

/* There are 512 128-bit "pages" (0x000 through 0x1FF).
 * The pages are accessable as 64-bit "halfpages" (an upper and lower half).
 * The pages are not part of the memory map.  There is an OTP controller which
 * handles scanning in/out of bits.  While access is done through OTP MMRs,
 * the bootrom provides C-callable helper functions to handle the interaction.
 */

#include <config.h>
#include <common.h>
#include <command.h>

#include <asm/blackfin.h>
#include <asm/clock.h>
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

static int check_voltage(void)
{
	/* Make sure voltage limits are within datasheet spec */
	uint16_t vr_ctl = bfin_read_VR_CTL();

#ifdef __ADSPBF54x__
	/* 0.9V <= VDDINT <= 1.1V */
	if ((vr_ctl & 0xc) && (vr_ctl & 0xc0) == 0xc0)
		return 1;
#else
	/* for the parts w/out qualification yet */
	(void)vr_ctl;
#endif

	return 0;
}

static void set_otp_timing(bool write)
{
	static uint32_t timing;
	if (!timing) {
		uint32_t tp1, tp2, tp3;
		/* OTP_TP1 = 1000 / sclk_period (in nanoseconds)
		 * OTP_TP1 = 1000 / (1 / get_sclk() * 10^9)
		 * OTP_TP1 = (1000 * get_sclk()) / 10^9
		 * OTP_TP1 = get_sclk() / 10^6
		 */
		tp1 = get_sclk() / 1000000;
		/* OTP_TP2 = 400 / (2 * sclk_period)
		 * OTP_TP2 = 400 / (2 * 1 / get_sclk() * 10^9)
		 * OTP_TP2 = (400 * get_sclk()) / (2 * 10^9)
		 * OTP_TP2 = (2 * get_sclk()) / 10^7
		 */
		tp2 = (2 * get_sclk() / 10000000) << 8;
		/* OTP_TP3 = magic constant */
		tp3 = (0x1401) << 15;
		timing = tp1 | tp2 | tp3;
	}

	bfrom_OtpCommand(OTP_INIT, write ? timing : timing & ~(-1 << 15));
}

int do_otp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *cmd;
	uint32_t ret, base_flags;
	bool prompt_user, force_read;
	uint32_t (*otp_func)(uint32_t page, uint32_t flags, uint64_t *page_content);

	if (argc < 4) {
 usage:
		return CMD_RET_USAGE;
	}

	prompt_user = false;
	base_flags = 0;
	cmd = argv[1];
	if (!strcmp(cmd, "read"))
		otp_func = bfrom_OtpRead;
	else if (!strcmp(cmd, "dump")) {
		otp_func = bfrom_OtpRead;
		force_read = true;
	} else if (!strcmp(cmd, "write")) {
		otp_func = bfrom_OtpWrite;
		base_flags = OTP_CHECK_FOR_PREV_WRITE;
		if (!strcmp(argv[2], "--force")) {
			argv++;
			--argc;
		} else
			prompt_user = false;
	} else if (!strcmp(cmd, "lock")) {
		if (argc != 4)
			goto usage;
		otp_func = bfrom_OtpWrite;
		base_flags = OTP_LOCK;
	} else
		goto usage;

	uint64_t *addr = (uint64_t *)simple_strtoul(argv[2], NULL, 16);
	uint32_t page = simple_strtoul(argv[3], NULL, 16);
	uint32_t flags;
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

	/* "otp lock" has slightly different semantics */
	if (base_flags & OTP_LOCK) {
		count = page;
		page = (uint32_t)addr;
		addr = NULL;
	}

	/* do to the nature of OTP, make sure users are sure */
	if (prompt_user) {
		printf(
			"Writing one time programmable memory\n"
			"Make sure your operating voltages and temperature are within spec\n"
			"   source address:  0x%p\n"
			"   OTP destination: %s page 0x%03X - %s page 0x%03lX\n"
			"   number to write: %lu halfpages\n"
			" type \"YES\" (no quotes) to confirm: ",
			addr,
			lowup(half), page,
			lowup(half + count - 1), page + (half + count - 1) / 2,
			half + count
		);
		if (!confirm_yesno()) {
			printf(" Aborting\n");
			return 1;
		}
	}

	printf("OTP memory %s: addr 0x%p  page 0x%03X  count %zu ... ",
		cmd, addr, page, count);

	set_otp_timing(otp_func == bfrom_OtpWrite);
	if (otp_func == bfrom_OtpWrite && check_voltage()) {
		puts("ERROR: VDDINT voltage is out of spec for writing\n");
		return -1;
	}

	/* Do the actual reading/writing stuff */
	ret = 0;
	for (i = half; i < count + half; ++i) {
		flags = base_flags | (i % 2 ? OTP_UPPER_HALF : OTP_LOWER_HALF);
 try_again:
		ret = otp_func(page, flags, addr);
		if (ret & OTP_MASTER_ERROR) {
			if (force_read) {
				if (flags & OTP_NO_ECC)
					break;
				else
					flags |= OTP_NO_ECC;
				puts("E");
				goto try_again;
			} else
				break;
		} else if (ret)
			puts("W");
		else
			puts(".");
		if (!(base_flags & OTP_LOCK)) {
			++addr;
			if (i % 2)
				++page;
		} else
			++page;
	}
	if (ret & 0x1)
		printf("\nERROR at page 0x%03X (%s-halfpage): 0x%03X: %s\n",
			page, lowup(i), ret, otp_strerror(ret));
	else
		puts(" done\n");

	/* Make sure we disable writing */
	set_otp_timing(false);
	bfrom_OtpCommand(OTP_CLOSE, 0);

	return ret;
}

U_BOOT_CMD(
	otp, 7, 0, do_otp,
	"One-Time-Programmable sub-system",
	"read <addr> <page> [count] [half]\n"
	" - read 'count' half-pages starting at 'page' (offset 'half') to 'addr'\n"
	"otp dump <addr> <page> [count] [half]\n"
	" - like 'otp read', but skip read errors\n"
	"otp write [--force] <addr> <page> [count] [half]\n"
	" - write 'count' half-pages starting at 'page' (offset 'half') from 'addr'\n"
	"otp lock <page> <count>\n"
	" - lock 'count' pages starting at 'page'"
);
