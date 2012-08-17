/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
