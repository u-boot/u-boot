// SPDX-License-Identifier: GPL-2.0+
/*
 * NBX Freebox Serial Info Support
 *
 * Copyright (C) 2025 Free Mobile, Freebox
 *
 * Reads device serial number and MAC address from eMMC.
 * The serial info is stored at a fixed offset in the eMMC user area.
 *
 * Serial format: TTTT-VV-M-(YY)WW-NN-NNNNN / FLAGS
 * Where:
 *   TTTT  = Device type (e.g., 9018)
 *   VV    = Board version
 *   M     = Manufacturer code (ASCII)
 *   YY    = Year (BCD)
 *   WW    = Week (1-53)
 *   NNNNN = Serial number
 *   FLAGS = Feature flags
 */

#include <command.h>
#include <dm/device.h>
#include <env.h>
#include <event.h>
#include <mmc.h>
#include <malloc.h>
#include <memalign.h>
#include <vsprintf.h>
#include <u-boot/crc.h>
#include <asm/byteorder.h>
#include <linux/ctype.h>
#include <linux/errno.h>
#include "nbx_fbxserial.h"

/* Partition offset defined in Kconfig (CONFIG_NBX_MMC_PART_SERIAL_OFFSET) */

/*
 * Validate serial info structure
 */
static int nbx_fbx_check_serial(struct nbx_fbx_serial *fs)
{
	unsigned int sum, len;

	/* Check magic first */
	if (be32_to_cpu(fs->magic) != NBX_FBXSERIAL_MAGIC) {
		printf("Invalid magic for serial info (%08x != %08x)!\n",
		       be32_to_cpu(fs->magic), NBX_FBXSERIAL_MAGIC);
		return -EINVAL;
	}

	/* Check struct version */
	if (be32_to_cpu(fs->struct_version) > NBX_FBXSERIAL_VERSION) {
		printf("Version too big for fbxserial info (0x%08x)!\n",
		       be32_to_cpu(fs->struct_version));
		return -EINVAL;
	}

	/* Check for silly len */
	len = be32_to_cpu(fs->len);
	if (len > NBX_FBXSERIAL_MAX_SIZE) {
		printf("Silly len for serial info (%d)\n", len);
		return -EINVAL;
	}

	/* Validate CRC (crc32_no_comp: no one's complement) */
	sum = crc32_no_comp(0, (void *)fs + 4, len - 4);
	if (be32_to_cpu(fs->crc32) != sum) {
		printf("Invalid checksum for serial info (%08x != %08x)\n",
		       sum, be32_to_cpu(fs->crc32));
		return -EINVAL;
	}

	return 0;
}

int nbx_fbx_read_serial(int dev_num, unsigned long offset,
			struct nbx_fbx_serial *fs)
{
	struct mmc *mmc;
	struct blk_desc *bd;
	uint blk_start, blk_cnt;
	uint n;

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, ALIGN(sizeof(*fs), 512));
	mmc = find_mmc_device(dev_num);
	if (!mmc) {
		printf("No MMC device %d found\n", dev_num);
		nbx_fbxserial_set_default(fs);
		return -ENODEV;
	}

	if (mmc_init(mmc)) {
		puts("MMC init failed\n");
		nbx_fbxserial_set_default(fs);
		return -EIO;
	}

	/* Switch to partition 0 (user data area) */
	if (blk_select_hwpart_devnum(UCLASS_MMC, dev_num, 0)) {
		puts("MMC partition switch failed\n");
		nbx_fbxserial_set_default(fs);
		return -EIO;
	}

	bd = mmc_get_blk_desc(mmc);
	if (!bd) {
		puts("Failed to get MMC block descriptor\n");
		nbx_fbxserial_set_default(fs);
		return -EIO;
	}

	blk_start = ALIGN(offset, bd->blksz) / bd->blksz;
	blk_cnt = ALIGN(sizeof(*fs), bd->blksz) / bd->blksz;

	memset(fs, 0x42, sizeof(*fs));

	n = blk_dread(bd, blk_start, blk_cnt, buf);
	if (n != blk_cnt) {
		printf("Failed to read serial info from MMC\n");
		nbx_fbxserial_set_default(fs);
		return -EIO;
	}

	memcpy(fs, buf, sizeof(*fs));

	if (nbx_fbx_check_serial(fs) != 0) {
		nbx_fbxserial_set_default(fs);
		return -EINVAL;
	}

	return 0;
}

void nbx_fbx_dump_serial(struct nbx_fbx_serial *fs)
{
	int i;

	printf("Serial: %04u-%02u-%c-(%02u)%02u-%02u-%05u / %08x\n",
	       ntohs(fs->type),
	       fs->version,
	       isprint(fs->manufacturer) ? fs->manufacturer : '?',
	       ntohs(fs->year) / 100,
	       ntohs(fs->year) % 100,
	       fs->week,
	       ntohl(fs->number),
	       ntohl(fs->flags));

	printf("Mac:    %02X:%02X:%02X:%02X:%02X:%02X\n",
	       fs->mac_addr_base[0],
	       fs->mac_addr_base[1],
	       fs->mac_addr_base[2],
	       fs->mac_addr_base[3],
	       fs->mac_addr_base[4],
	       fs->mac_addr_base[5]);

	/* Show bundle info */
	for (i = 0; i < be32_to_cpu(fs->extinfo_count); i++) {
		struct nbx_serial_extinfo *p;

		if (i >= NBX_EXTINFO_MAX_COUNT)
			break;

		p = &fs->extinfos[i];
		if (be32_to_cpu(p->type) == NBX_EXTINFO_TYPE_EXTDEV &&
		    be32_to_cpu(p->u.extdev.type) == NBX_EXTDEV_TYPE_BUNDLE) {
			/* Ensure null termination */
			p->u.extdev.serial[sizeof(p->u.extdev.serial) - 1] = 0;
			printf("Bundle: %s\n", p->u.extdev.serial);
		}
	}

	printf("\n");
}

int nbx_fbx_init_ethaddr(int dev_num, unsigned long offset)
{
	struct nbx_fbx_serial fs;
	char mac[32];
	int ret;

	ret = nbx_fbx_read_serial(dev_num, offset, &fs);

	/* Even on error, fs has default values set */
	snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
		 fs.mac_addr_base[0], fs.mac_addr_base[1],
		 fs.mac_addr_base[2], fs.mac_addr_base[3],
		 fs.mac_addr_base[4], fs.mac_addr_base[5]);

	nbx_fbx_dump_serial(&fs);

	env_set("ethaddr", mac);
	env_set("eth1addr", mac);
	env_set("eth2addr", mac);

	return ret;
}

/*
 * fbxserial show - display serial info from eMMC
 */
static int do_fbxserial_show(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	struct nbx_fbx_serial fs;
	int dev = 0;
	unsigned long offset = CONFIG_NBX_MMC_PART_SERIAL_OFFSET;

	if (argc >= 1)
		dev = dectoul(argv[0], NULL);

	if (argc >= 2)
		offset = hextoul(argv[1], NULL);

	if (nbx_fbx_read_serial(dev, offset, &fs) != 0)
		printf("Warning: Using default serial info\n");

	nbx_fbx_dump_serial(&fs);

	return CMD_RET_SUCCESS;
}

/*
 * fbxserial init - initialize ethaddr from serial info
 */
static int do_fbxserial_init(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	int dev = 0;
	unsigned long offset = CONFIG_NBX_MMC_PART_SERIAL_OFFSET;

	if (argc >= 1)
		dev = dectoul(argv[0], NULL);

	if (argc >= 2)
		offset = hextoul(argv[1], NULL);

	return nbx_fbx_init_ethaddr(dev, offset);
}

static struct cmd_tbl cmd_fbxserial_sub[] = {
	U_BOOT_CMD_MKENT(show, 3, 0, do_fbxserial_show, "", ""),
	U_BOOT_CMD_MKENT(init, 3, 0, do_fbxserial_init, "", ""),
};

static int do_fbxserial(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct cmd_tbl *cp;

	/* Default to 'show' if no subcommand */
	if (argc < 2)
		return do_fbxserial_show(cmdtp, flag, 0, NULL);

	cp = find_cmd_tbl(argv[1], cmd_fbxserial_sub,
			  ARRAY_SIZE(cmd_fbxserial_sub));

	if (!cp)
		return CMD_RET_USAGE;

	return cp->cmd(cmdtp, flag, argc - 2, argv + 2);
}

U_BOOT_CMD(
	fbxserial, 5, 0, do_fbxserial,
	"NBX serial info and MAC address initialization",
	"show [dev] [offset] - display serial info from eMMC\n"
	"fbxserial init [dev] [offset] - initialize ethaddr from serial info\n"
	"    dev    - MMC device number (default 0)\n"
	"    offset - offset in eMMC in hex (default from Kconfig)"
);

/*
 * Early init hook: Set MAC address from eMMC serial info before
 * network driver probes. EVT_SETTINGS_R is triggered after MMC
 * is available but before initr_net().
 */
static int nbx_fbx_settings_r(void)
{
	if (!of_machine_is_compatible("nbx,armada8040"))
		return 0;

	nbx_fbx_init_ethaddr(0, CONFIG_NBX_MMC_PART_SERIAL_OFFSET);
	return 0;
}

EVENT_SPY_SIMPLE(EVT_SETTINGS_R, nbx_fbx_settings_r);
