// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Marek Vasut <marek.vasut+renesas@mailbox.org>
 */

#include <dm.h>
#include <i2c.h>
#include <malloc.h>
#include <net-common.h>

#define S4SK_FPGA_I2C_BUS			"i2c5"
#define S4SK_FPGA_I2C_DEV_ADDR			0x70
#define S4SK_FPGA_I2C_DEV_WIDTH			2
#define S4SK_FPGA_I2C_MAC_COUNT			4
#define S4SK_FPGA_I2C_MAC_OFFSET		0x58
#define S4SK_FPGA_I2C_MAC_WIDTH			8

int board_late_init(void)
{
	/*
	 * Extract AVB and TSN0,1,2 MAC addresses from FPGA via I2C.
	 *
	 * In case a matching ethaddr/ethNaddr environment variable
	 * is not set, set it, otherwise do not override it. This
	 * allows users to set their own MAC addresses via ethaddr
	 * and ethNaddr environment variables.
	 *
	 * The ethaddr/ethNaddr mapping follows Linux kernel DT aliases
	 * ethernetN property assignment:
	 * - ethaddr ..... TSN0 (IC104 connector)
	 * - eth1addr .... TSN1 (IC101 connector)
	 * - eth2addr .... TSN2 (Expansion connector)
	 * - eth3addr .... AVB (CN1 connector)
	 */
	ofnode i2c_node = ofnode_path(S4SK_FPGA_I2C_BUS);
	struct udevice *bus, *dev;
	unsigned char enetaddr[6];
	unsigned char macs[32];	/* Four MAC addresses in FPGA in total. */
	int i, idx, j, ret;

	ret = uclass_get_device_by_ofnode(UCLASS_I2C, i2c_node, &bus);
	if (ret < 0) {
		printf("s4sk: cannot find i2c bus (%d)\n", ret);
		return 0;
	}

	ret = i2c_get_chip(bus, S4SK_FPGA_I2C_DEV_ADDR,
			   S4SK_FPGA_I2C_DEV_WIDTH, &dev);
	if (ret < 0) {
		printf("s4sk: cannot find i2c chip (%d)\n", ret);
		return 0;
	}

	ret = dm_i2c_read(dev, S4SK_FPGA_I2C_MAC_OFFSET, macs, sizeof(macs));
	if (ret < 0) {
		printf("s4sk: failed to read MAC addresses via i2c (%d)\n", ret);
		return 0;
	}

	for (i = 0; i < S4SK_FPGA_I2C_MAC_COUNT; i++) {
		/*
		 * Remap TSN0,1,2 to ethaddr,eth1addr,eth2addr and
		 * AVB to eth3addr to match Linux /aliases ethernetN
		 * assignment, which starts with ethernet0 for TSN.
		 */
		idx = (i + 3) % 4;
		ret = eth_env_get_enetaddr_by_index("eth", idx, enetaddr);
		if (ret)	/* ethaddr is already set */
			continue;

		/* Byte-wise reverse the MAC address */
		for (j = 0; j < sizeof(enetaddr); j++)
			enetaddr[j] = macs[i * S4SK_FPGA_I2C_MAC_WIDTH + (5 - j)];

		if (!is_valid_ethaddr(enetaddr)) {
			printf("s4sk: MAC address %d in FPGA not valid (%pM)\n",
			       i, enetaddr);
			continue;
		}

		eth_env_set_enetaddr_by_index("eth", idx, enetaddr);
	}

	return 0;
}
