/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0
 * https://spdx.org/licenses
 */

#include <asm/io.h>
#include <command.h>
#include <common.h>
#include <config.h>
#include <linux/ctype.h>
#include <mach/soc.h>
#include <stddef.h>
#include <stdlib.h>

/* GMAC_MIB Counters register definitions */
#define MV_MIB_GOOD_OCTETS_RECEIVED_LOW		0x0
#define MV_MIB_GOOD_OCTETS_RECEIVED_HIGH	0x4
#define MV_MIB_BAD_OCTETS_RECEIVED		0x8
#define MV_MIB_CRC_ERRORS_SENT			0xc
#define MV_MIB_UNICAST_FRAMES_RECEIVED		0x10
/* Reserved					0x14 */
#define MV_MIB_BROADCAST_FRAMES_RECEIVED	0x18
#define MV_MIB_MULTICAST_FRAMES_RECEIVED	0x1c
#define MV_MIB_FRAMES_64_OCTETS			0x20
#define MV_MIB_FRAMES_65_TO_127_OCTETS		0x24
#define MV_MIB_FRAMES_128_TO_255_OCTETS		0x28
#define MV_MIB_FRAMES_256_TO_511_OCTETS		0x2c
#define MV_MIB_FRAMES_512_TO_1023_OCTETS	0x30
#define MV_MIB_FRAMES_1024_TO_MAX_OCTETS	0x34
#define MV_MIB_GOOD_OCTETS_SENT_LOW		0x38
#define MV_MIB_GOOD_OCTETS_SENT_HIGH		0x3c
#define MV_MIB_UNICAST_FRAMES_SENT		0x40
/* Reserved					0x44 */
#define MV_MIB_MULTICAST_FRAMES_SENT		0x48
#define MV_MIB_BROADCAST_FRAMES_SENT		0x4c
/* Reserved					0x50 */
#define MV_MIB_FC_SENT				0x54
#define MV_MIB_FC_RECEIVED			0x58
#define MV_MIB_RX_FIFO_OVERRUN			0x5c
#define MV_MIB_UNDERSIZE_RECEIVED		0x60
#define MV_MIB_FRAGMENTS_RECEIVED		0x64
#define MV_MIB_OVERSIZE_RECEIVED		0x68
#define MV_MIB_JABBER_RECEIVED			0x6c
#define MV_MIB_MAC_RECEIVE_ERROR		0x70
#define MV_MIB_BAD_CRC_EVENT			0x74
#define MV_MIB_COLLISION			0x78
#define MV_MIB_LATE_COLLISION			0x7c

#define COUNTERS_BASE		0x129000
#define GOP_OFFSET		(((gop) + ((gop) != 0)) * 0x100)
#define COUNTERS_ADDRESS(cp, gop_p) \
	(COUNTERS_BASE + MVEBU_REGS_BASE_CP(0, (cp)) + GOP_OFFSET)
#define PORT_NUM_STR		(sizeof("mvpp2-") - 1)
#define IS_MVPP2		((memcmp(name, "mvpp2-", PORT_NUM_STR) == 0) &&\
	(isdigit(name[PORT_NUM_STR])))

u64 mv_gop110_mib_read64(u64 counters_address, unsigned int offset)
{
	u64 val, val2;

	val = readl(counters_address + offset);
	if (offset == MV_MIB_GOOD_OCTETS_RECEIVED_LOW ||
	    offset == MV_MIB_GOOD_OCTETS_SENT_LOW) {
		val2 = readl(counters_address + offset + 4);
		val += (val2 << 32);
	}

	return val;
}

static void mv_gop110_mib_print(u64 counters_address, u32 offset,
				char *mib_name)
{
	u64 val;

	val = mv_gop110_mib_read64(counters_address, offset);
	printf("  %-32s: 0x%02x = %lld\n", mib_name, offset, val);
}

void mv_print_counter_regs(char *port_name, u64 addr)
{
	printf("%s MiB Counters:\n", port_name);
	printf("\n[Rx]\n");
	mv_gop110_mib_print(addr, MV_MIB_GOOD_OCTETS_RECEIVED_LOW,
			    "GOOD_OCTETS_RECEIVED");
	mv_gop110_mib_print(addr, MV_MIB_BAD_OCTETS_RECEIVED,
			    "BAD_OCTETS_RECEIVED");

	mv_gop110_mib_print(addr, MV_MIB_UNICAST_FRAMES_RECEIVED,
			    "UNCAST_FRAMES_RECEIVED");
	mv_gop110_mib_print(addr, MV_MIB_BROADCAST_FRAMES_RECEIVED,
			    "BROADCAST_FRAMES_RECEIVED");
	mv_gop110_mib_print(addr, MV_MIB_MULTICAST_FRAMES_RECEIVED,
			    "MULTICAST_FRAMES_RECEIVED");

	printf("\n[RMON]\n");
	mv_gop110_mib_print(addr, MV_MIB_FRAMES_64_OCTETS,
			    "FRAMES_64_OCTETS");
	mv_gop110_mib_print(addr, MV_MIB_FRAMES_65_TO_127_OCTETS,
			    "FRAMES_65_TO_127_OCTETS");
	mv_gop110_mib_print(addr, MV_MIB_FRAMES_128_TO_255_OCTETS,
			    "FRAMES_128_TO_255_OCTETS");
	mv_gop110_mib_print(addr, MV_MIB_FRAMES_256_TO_511_OCTETS,
			    "FRAMES_256_TO_511_OCTETS");
	mv_gop110_mib_print(addr, MV_MIB_FRAMES_512_TO_1023_OCTETS,
			    "FRAMES_512_TO_1023_OCTETS");
	mv_gop110_mib_print(addr, MV_MIB_FRAMES_1024_TO_MAX_OCTETS,
			    "FRAMES_1024_TO_MAX_OCTETS");

	printf("\n[Tx]\n");
	mv_gop110_mib_print(addr, MV_MIB_GOOD_OCTETS_SENT_LOW,
			    "GOOD_OCTETS_SENT");
	mv_gop110_mib_print(addr, MV_MIB_UNICAST_FRAMES_SENT,
			    "UNICAST_FRAMES_SENT");
	mv_gop110_mib_print(addr, MV_MIB_MULTICAST_FRAMES_SENT,
			    "MULTICAST_FRAMES_SENT");
	mv_gop110_mib_print(addr, MV_MIB_BROADCAST_FRAMES_SENT,
			    "BROADCAST_FRAMES_SENT");
	mv_gop110_mib_print(addr, MV_MIB_CRC_ERRORS_SENT,
			    "CRC_ERRORS_SENT");

	printf("\n[FC control]\n");
	mv_gop110_mib_print(addr, MV_MIB_FC_RECEIVED,
			    "FC_RECEIVED");
	mv_gop110_mib_print(addr, MV_MIB_FC_SENT,
			    "FC_SENT");

	printf("\n[Errors]\n");
	mv_gop110_mib_print(addr, MV_MIB_RX_FIFO_OVERRUN,
			    "RX_FIFO_OVERRUN");
	mv_gop110_mib_print(addr, MV_MIB_UNDERSIZE_RECEIVED,
			    "UNDERSIZE_RECEIVED");
	mv_gop110_mib_print(addr, MV_MIB_FRAGMENTS_RECEIVED,
			    "FRAGMENTS_RECEIVED");
	mv_gop110_mib_print(addr, MV_MIB_OVERSIZE_RECEIVED,
			    "OVERSIZE_RECEIVED");
	mv_gop110_mib_print(addr, MV_MIB_JABBER_RECEIVED,
			    "JABBER_RECEIVED");
	mv_gop110_mib_print(addr, MV_MIB_MAC_RECEIVE_ERROR,
			    "MAC_RECEIVE_ERROR");
	mv_gop110_mib_print(addr, MV_MIB_BAD_CRC_EVENT,
			    "BAD_CRC_EVENT");
	mv_gop110_mib_print(addr, MV_MIB_COLLISION,
			    "COLLISION");
	/* This counter must be read last. Read it clear all the counters */
	mv_gop110_mib_print(addr, MV_MIB_LATE_COLLISION,
			    "LATE_COLLISION");
}

int mv_do_get_counters_cmd(cmd_tbl_t *cmdtp, int flag,
			   int argc, char *const argv[])
{
	char *name = NULL;

	if (argc == 2)
		name = argv[1];
	else
		name = env_get("ethact");
	if (name && IS_MVPP2) {
		u32 num = simple_strtoul(&name[PORT_NUM_STR],
					 NULL, 0);
		u32 cp_max;
		u32 ap_max;
		u32 cp = num / 3;
		u32 gop = num % 3;

		soc_get_ap_cp_num(&ap_max, &cp_max);
		if (cp_max <= cp) {
			pr_err("Error: Port: %s does not exist\n", name);
			return 0;
		}
		mv_print_counter_regs(name, COUNTERS_ADDRESS(cp, gop));
	} else {
		pr_err("Error: Bad port name: %s\n", name);
	}
	return 0;
}

U_BOOT_CMD(
	mv_get_counters, 2, 1, mv_do_get_counters_cmd,
	"Get gop counters",
	"mv_get_counters <port_name:default=ethact>\n");
