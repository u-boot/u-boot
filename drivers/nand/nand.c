/*
 * (C) Copyright 2005
 * 2N Telekomunikace, a.s. <www.2n.cz>
 * Ladislav Michl <michl@2n.cz>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#ifdef CFG_NAND_LEGACY
#error CFG_NAND_LEGACY defined in a file not using the legacy NAND support!
#endif

#if (CONFIG_COMMANDS & CFG_CMD_NAND)

#include <nand.h>

#ifndef CFG_NAND_BASE_LIST
#define CFG_NAND_BASE_LIST { CFG_NAND_BASE }
#endif

int nand_curr_device = -1;
nand_info_t nand_info[CFG_MAX_NAND_DEVICE];

static struct nand_chip nand_chip[CFG_MAX_NAND_DEVICE];
static ulong base_address[CFG_MAX_NAND_DEVICE] = CFG_NAND_BASE_LIST;

static const char default_nand_name[] = "nand";

extern void board_nand_init(struct nand_chip *nand);

static void nand_init_chip(struct mtd_info *mtd, struct nand_chip *nand,
			   ulong base_addr)
{
	mtd->priv = nand;

	nand->IO_ADDR_R = nand->IO_ADDR_W = (void  __iomem *)base_addr;
	board_nand_init(nand);

	if (nand_scan(mtd, 1) == 0) {
		if (!mtd->name)
			mtd->name = (char *)default_nand_name;
	} else
		mtd->name = NULL;

}

void nand_init(void)
{
	int i;
	unsigned int size = 0;
	for (i = 0; i < CFG_MAX_NAND_DEVICE; i++) {
		nand_init_chip(&nand_info[i], &nand_chip[i], base_address[i]);
		size += nand_info[i].size;
		if (nand_curr_device == -1)
			nand_curr_device = i;
}
	printf("%lu MiB\n", size / (1024 * 1024));
}

#endif
