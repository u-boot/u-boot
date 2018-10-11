// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Synopsys, Inc. All rights reserved.
 */

#include <common.h>
#include <dwmmc.h>
#include <malloc.h>

DECLARE_GLOBAL_DATA_PTR;

#define ARC_PERIPHERAL_BASE	0xF0000000
#define SDIO_BASE		(ARC_PERIPHERAL_BASE + 0x10000)

int board_mmc_init(bd_t *bis)
{
	struct dwmci_host *host = NULL;

	host = malloc(sizeof(struct dwmci_host));
	if (!host) {
		printf("dwmci_host malloc fail!\n");
		return 1;
	}

	memset(host, 0, sizeof(struct dwmci_host));
	host->name = "Synopsys Mobile storage";
	host->ioaddr = (void *)SDIO_BASE;
	host->buswidth = 4;
	host->dev_index = 0;
	host->bus_hz = 50000000;

	add_dwmci(host, host->bus_hz / 2, 400000);

	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct dwmci_host *host = mmc->priv;

	return !(dwmci_readl(host, DWMCI_CDETECT) & 1);
}

#define CREG_BASE		0xF0001000
#define CREG_BOOT_OFFSET	0
#define CREG_BOOT_WP_OFFSET	8

#define CGU_BASE		0xF0000000
#define CGU_IP_SW_RESET		0x0FF0

void reset_cpu(ulong addr)
{
	writel(1, (u32 *)(CGU_BASE + CGU_IP_SW_RESET));
	while (1)
		; /* loop forever till reset */
}

static int do_emdk_rom(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	u32 creg_boot = readl((u32 *)(CREG_BASE + CREG_BOOT_OFFSET));

	if (!strcmp(argv[1], "unlock"))
		creg_boot &= ~BIT(CREG_BOOT_WP_OFFSET);
	else if (!strcmp(argv[1], "lock"))
		creg_boot |= BIT(CREG_BOOT_WP_OFFSET);
	else
		return CMD_RET_USAGE;

	writel(creg_boot, (u32 *)(CREG_BASE + CREG_BOOT_OFFSET));

	return CMD_RET_SUCCESS;
}

cmd_tbl_t cmd_emdk[] = {
	U_BOOT_CMD_MKENT(rom, 2, 0, do_emdk_rom, "", ""),
};

static int do_emdk(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	cmd_tbl_t *c;

	c = find_cmd_tbl(argv[1], cmd_emdk, ARRAY_SIZE(cmd_emdk));

	/* Strip off leading 'emdk' command */
	argc--;
	argv++;

	if (c == NULL || argc > c->maxargs)
		return CMD_RET_USAGE;

	return c->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	emdk, CONFIG_SYS_MAXARGS, 0, do_emdk,
	"Synopsys EMDK specific commands",
	"rom unlock - Unlock non-volatile memory for writing\n"
	"emdk rom lock - Lock non-volatile memory to prevent writing\n"
);
