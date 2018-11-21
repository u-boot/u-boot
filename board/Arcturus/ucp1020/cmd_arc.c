// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Command for accessing Arcturus factory environment.
 *
 * Copyright 2013-2015 Arcturus Networks Inc.
 *           http://www.arcturusnetworks.com/products/ucp1020/
 *           by Oleksandr G Zhadan et al.
 *
 */

#include <common.h>
#include <div64.h>
#include <malloc.h>
#include <spi_flash.h>

#include <asm/io.h>

#ifndef CONFIG_SF_DEFAULT_SPEED
#   define CONFIG_SF_DEFAULT_SPEED	1000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
#   define CONFIG_SF_DEFAULT_MODE	SPI_MODE0
#endif
#ifndef CONFIG_SF_DEFAULT_CS
#   define CONFIG_SF_DEFAULT_CS		0
#endif
#ifndef CONFIG_SF_DEFAULT_BUS
#   define CONFIG_SF_DEFAULT_BUS	0
#endif

#define MAX_SERIAL_SIZE 15
#define MAX_HWADDR_SIZE 17

#define FIRM_ADDR1 (0x200 - sizeof(smac))
#define FIRM_ADDR2 (0x400 - sizeof(smac))
#define FIRM_ADDR3 (CONFIG_ENV_SECT_SIZE + 0x200 - sizeof(smac))
#define FIRM_ADDR4 (CONFIG_ENV_SECT_SIZE + 0x400 - sizeof(smac))

static struct spi_flash *flash;
char smac[4][18];

static int ishwaddr(char *hwaddr)
{
	if (strlen(hwaddr) == MAX_HWADDR_SIZE)
		if (hwaddr[2] == ':' &&
		    hwaddr[5] == ':' &&
		    hwaddr[8] == ':' &&
		    hwaddr[11] == ':' &&
		    hwaddr[14] == ':')
			return 0;
	return -1;
}

static int set_arc_product(int argc, char *const argv[])
{
	int err = 0;
	char *mystrerr = "ERROR: Failed to save factory info in spi location";

	if (argc != 5)
		return -1;

	/* Check serial number */
	if (strlen(argv[1]) != MAX_SERIAL_SIZE)
		return -1;

	/* Check HWaddrs */
	if (ishwaddr(argv[2]) || ishwaddr(argv[3]) || ishwaddr(argv[4]))
		return -1;

	strcpy(smac[3], argv[1]);
	strcpy(smac[2], argv[2]);
	strcpy(smac[1], argv[3]);
	strcpy(smac[0], argv[4]);

	flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);

	/*
	 * Save factory defaults
	 */

	if (spi_flash_write(flash, FIRM_ADDR1, sizeof(smac), smac)) {
		printf("%s: %s [1]\n", __func__, mystrerr);
		err++;
	}
	if (spi_flash_write(flash, FIRM_ADDR2, sizeof(smac), smac)) {
		printf("%s: %s [2]\n", __func__, mystrerr);
		err++;
	}

	if (spi_flash_write(flash, FIRM_ADDR3, sizeof(smac), smac)) {
		printf("%s: %s [3]\n", __func__, mystrerr);
		err++;
	}

	if (spi_flash_write(flash, FIRM_ADDR4, sizeof(smac), smac)) {
		printf("%s: %s [4]\n", __func__, mystrerr);
		err++;
	}

	if (err == 4) {
		printf("%s: %s [ALL]\n", __func__, mystrerr);
		return -2;
	}

	return 0;
}

int get_arc_info(void)
{
	int location = 1;
	char *myerr = "ERROR: Failed to read all 4 factory info spi locations";

	flash = spi_flash_probe(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);

	if (spi_flash_read(flash, FIRM_ADDR1, sizeof(smac), smac)) {
		location++;
		if (spi_flash_read(flash, FIRM_ADDR2, sizeof(smac), smac)) {
			location++;
			if (spi_flash_read(flash, FIRM_ADDR3, sizeof(smac),
					   smac)) {
				location++;
				if (spi_flash_read(flash, FIRM_ADDR4,
						   sizeof(smac), smac)) {
					printf("%s: %s\n", __func__, myerr);
					return -2;
				}
			}
		}
	}
	if (smac[3][0] != 0) {
		if (location > 1)
			printf("Using region %d\n", location);
		printf("SERIAL: ");
		if (smac[3][0] == 0xFF) {
			printf("\t<not found>\n");
		} else {
			printf("\t%s\n", smac[3]);
			env_set("SERIAL", smac[3]);
		}
	}

	if (strcmp(smac[2], "00:00:00:00:00:00") == 0)
		return 0;

	printf("HWADDR0:");
	if (smac[2][0] == 0xFF) {
		printf("\t<not found>\n");
	} else {
		char *ret = env_get("ethaddr");

		if (strcmp(ret, __stringify(CONFIG_ETHADDR)) == 0) {
			env_set("ethaddr", smac[2]);
			printf("\t%s (factory)\n", smac[2]);
		} else {
			printf("\t%s\n", ret);
		}
	}

	if (strcmp(smac[1], "00:00:00:00:00:00") == 0) {
		env_set("eth1addr", smac[2]);
		env_set("eth2addr", smac[2]);
		return 0;
	}

	printf("HWADDR1:");
	if (smac[1][0] == 0xFF) {
		printf("\t<not found>\n");
	} else {
		char *ret = env_get("eth1addr");

		if (strcmp(ret, __stringify(CONFIG_ETH1ADDR)) == 0) {
			env_set("eth1addr", smac[1]);
			printf("\t%s (factory)\n", smac[1]);
		} else {
			printf("\t%s\n", ret);
		}
	}

	if (strcmp(smac[0], "00:00:00:00:00:00") == 0) {
		env_set("eth2addr", smac[1]);
		return 0;
	}

	printf("HWADDR2:");
	if (smac[0][0] == 0xFF) {
		printf("\t<not found>\n");
	} else {
		char *ret = env_get("eth2addr");

		if (strcmp(ret, __stringify(CONFIG_ETH2ADDR)) == 0) {
			env_set("eth2addr", smac[0]);
			printf("\t%s (factory)\n", smac[0]);
		} else {
			printf("\t%s\n", ret);
		}
	}

	return 0;
}

static int do_arc_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	const char *cmd;
	int ret = -1;

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "product") == 0) {
		ret = set_arc_product(argc, argv);
		goto done;
	}
	if (strcmp(cmd, "info") == 0) {
		ret = get_arc_info();
		goto done;
	}
done:
	if (ret == -1)
		return CMD_RET_USAGE;

	return ret;
}

U_BOOT_CMD(arc, 6, 1, do_arc_cmd,
	   "Arcturus product command sub-system",
	   "product serial hwaddr0 hwaddr1 hwaddr2    - save Arcturus factory env\n"
	   "info                                      - show Arcturus factory env\n\n");
