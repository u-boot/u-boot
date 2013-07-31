/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <faraday/ftsmc020.h>

struct ftsmc020_config {
	unsigned int	config;
	unsigned int	timing;
};

static void ftsmc020_setup_bank(unsigned int bank, struct ftsmc020_config *cfg)
{
	struct ftsmc020 *smc = (struct ftsmc020 *)CONFIG_FTSMC020_BASE;

	if (bank > 3) {
		printf("bank # %u invalid\n", bank);
		return;
	}

	writel(cfg->config, &smc->bank[bank].cr);
	writel(cfg->timing, &smc->bank[bank].tpr);
}

void ftsmc020_init(void)
{
	struct ftsmc020_config config[] = CONFIG_SYS_FTSMC020_CONFIGS;
	int i;

	for (i = 0; i < ARRAY_SIZE(config); i++)
		ftsmc020_setup_bank(i, &config[i]);
}
