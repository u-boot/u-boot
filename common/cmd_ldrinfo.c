/*
 * U-boot - ldrinfo
 *
 * Copyright (c) 2010 Analog Devices Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * Licensed under the GPL-2 or later.
 */

#include <config.h>
#include <common.h>
#include <command.h>

#include <asm/blackfin.h>
#include <asm/mach-common/bits/bootrom.h>

static uint32_t ldrinfo_header(const void *addr)
{
	uint32_t skip = 0;

#if defined(__ADSPBF561__)
	/* BF56x has a 4 byte global header */
	uint32_t header, sign;
	static const char * const spi_speed[] = {
		"500K", "1M", "2M", "??",
	};

	memcpy(&header, addr, sizeof(header));

	sign = (header & GFLAG_56X_SIGN_MASK) >> GFLAG_56X_SIGN_SHIFT;
	printf("Header: %08X ( %s-bit-flash wait:%i hold:%i spi:%s %s)\n",
		header,
		(header & GFLAG_56X_16BIT_FLASH) ? "16" : "8",
		(header & GFLAG_56X_WAIT_MASK) >> GFLAG_56X_WAIT_SHIFT,
		(header & GFLAG_56X_HOLD_MASK) >> GFLAG_56X_HOLD_SHIFT,
		spi_speed[(header & GFLAG_56X_SPI_MASK) >> GFLAG_56X_SPI_SHIFT],
		sign == GFLAG_56X_SIGN_MAGIC ? "" : "!!hdrsign!! ");

	skip = 4;
#endif

	    /* |Block @ 12345678: 12345678 12345678 12345678 12345678 | */
#if defined(__ADSPBF531__) || defined(__ADSPBF532__) || defined(__ADSPBF533__) || \
    defined(__ADSPBF534__) || defined(__ADSPBF536__) || defined(__ADSPBF537__) || \
    defined(__ADSPBF538__) || defined(__ADSPBF539__) || defined(__ADSPBF561__)
	printf("                  Address  Count    Flags\n");
#else
	printf("                  BCode    Address  Count    Argument\n");
#endif

	return skip;
}

struct ldr_flag {
	uint16_t flag;
	const char *desc;
};

static uint32_t ldrinfo_block(const void *base_addr)
{
	uint32_t count;

	printf("Block @ %08X: ", (uint32_t)base_addr);

#if defined(__ADSPBF531__) || defined(__ADSPBF532__) || defined(__ADSPBF533__) || \
    defined(__ADSPBF534__) || defined(__ADSPBF536__) || defined(__ADSPBF537__) || \
    defined(__ADSPBF538__) || defined(__ADSPBF539__) || defined(__ADSPBF561__)

	uint32_t addr, pval;
	uint16_t flags;
	int i;
	static const struct ldr_flag ldr_flags[] = {
		{ BFLAG_53X_ZEROFILL,    "zerofill"  },
		{ BFLAG_53X_RESVECT,     "resvect"   },
		{ BFLAG_53X_INIT,        "init"      },
		{ BFLAG_53X_IGNORE,      "ignore"    },
		{ BFLAG_53X_COMPRESSED,  "compressed"},
		{ BFLAG_53X_FINAL,       "final"     },
	};

	memcpy(&addr, base_addr, sizeof(addr));
	memcpy(&count, base_addr+4, sizeof(count));
	memcpy(&flags, base_addr+8, sizeof(flags));

	printf("%08X %08X %04X ( ", addr, count, flags);

	for (i = 0; i < ARRAY_SIZE(ldr_flags); ++i)
		if (flags & ldr_flags[i].flag)
			printf("%s ", ldr_flags[i].desc);

	pval = (flags & BFLAG_53X_PFLAG_MASK) >> BFLAG_53X_PFLAG_SHIFT;
	if (pval)
		printf("gpio%i ", pval);
	pval = (flags & BFLAG_53X_PPORT_MASK) >> BFLAG_53X_PPORT_SHIFT;
	if (pval)
		printf("port%c ", 'e' + pval);

	if (flags & BFLAG_53X_ZEROFILL)
		count = 0;
	if (flags & BFLAG_53X_FINAL)
		count = 0;
	else
		count += sizeof(addr) + sizeof(count) + sizeof(flags);

#else

	const uint8_t *raw8 = base_addr;
	uint32_t bcode, addr, arg, sign, chk;
	int i;
	static const struct ldr_flag ldr_flags[] = {
		{ BFLAG_SAFE,        "safe"      },
		{ BFLAG_AUX,         "aux"       },
		{ BFLAG_FILL,        "fill"      },
		{ BFLAG_QUICKBOOT,   "quickboot" },
		{ BFLAG_CALLBACK,    "callback"  },
		{ BFLAG_INIT,        "init"      },
		{ BFLAG_IGNORE,      "ignore"    },
		{ BFLAG_INDIRECT,    "indirect"  },
		{ BFLAG_FIRST,       "first"     },
		{ BFLAG_FINAL,       "final"     },
	};

	memcpy(&bcode, base_addr, sizeof(bcode));
	memcpy(&addr, base_addr+4, sizeof(addr));
	memcpy(&count, base_addr+8, sizeof(count));
	memcpy(&arg, base_addr+12, sizeof(arg));

	printf("%08X %08X %08X %08X ( ", bcode, addr, count, arg);

	if (addr % 4)
		printf("!!addralgn!! ");
	if (count % 4)
		printf("!!cntalgn!! ");

	sign = (bcode & BFLAG_HDRSIGN_MASK) >> BFLAG_HDRSIGN_SHIFT;
	if (sign != BFLAG_HDRSIGN_MAGIC)
		printf("!!hdrsign!! ");

	chk = 0;
	for (i = 0; i < 16; ++i)
		chk ^= raw8[i];
	if (chk)
		printf("!!hdrchk!! ");

	printf("dma:%i ", bcode & BFLAG_DMACODE_MASK);

	for (i = 0; i < ARRAY_SIZE(ldr_flags); ++i)
		if (bcode & ldr_flags[i].flag)
			printf("%s ", ldr_flags[i].desc);

	if (bcode & BFLAG_FILL)
		count = 0;
	if (bcode & BFLAG_FINAL)
		count = 0;
	else
		count += sizeof(bcode) + sizeof(addr) + sizeof(count) + sizeof(arg);

#endif

	printf(")\n");

	return count;
}

static int do_ldrinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const void *addr;
	uint32_t skip;

	/* Get the address */
	if (argc < 2)
		addr = (void *)load_addr;
	else
		addr = (void *)simple_strtoul(argv[1], NULL, 16);

	/* Walk the LDR */
	addr += ldrinfo_header(addr);
	do {
		skip = ldrinfo_block(addr);
		addr += skip;
	} while (skip);

	return 0;
}

U_BOOT_CMD(
	ldrinfo, 2, 0, do_ldrinfo,
	"validate ldr image in memory",
	"[addr]\n"
);
