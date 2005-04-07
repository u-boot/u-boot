/*
 * (C) Copyright 2005 2N TELEKOMUNIKACE, Ladislav Michl
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

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	*((volatile unsigned char *) VOICEBLUE_LED_REG) = 0xaa;

	/* arch number of VoiceBlue board */
	/* TODO: use define from asm/mach-types.h */
	gd->bd->bi_arch_number = 218;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x10000100;

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	*((volatile unsigned short *) VOICEBLUE_LED_REG) = 0xff;

 	/* Take the Ethernet controller out of reset and wait
 	 * for the EEPROM load to complete. */
	*((volatile unsigned short *) GPIO_DATA_OUTPUT_REG) |= 0x80;
	udelay(10);	/* doesn't work before interrupt_init call */
	*((volatile unsigned short *) GPIO_DATA_OUTPUT_REG) &= ~0x80;
	udelay(500);

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

#ifndef VOICEBLUE_SMALL_FLASH

#include <jffs2/jffs2.h>

extern flash_info_t flash_info[];
static struct part_info partinfo;
static int current_part = -1;

/* Partition table (Linux MTD see it this way)
 *
 * 0 - U-Boot
 * 1 - env
 * 2 - redundant env
 * 3 - data1 (jffs2)
 * 4 - data2 (jffs2)
 */

static struct {
	ulong offset;
	ulong size;
} part[5];

static void partition_flash(flash_info_t *info)
{
	char mtdparts[128];
	int i, n, size, psize;
	const ulong plen[3] = { CFG_MONITOR_LEN, CFG_ENV_SIZE, CFG_ENV_SIZE };

	size = n = 0;
	for (i = 0; i < 4; i++) {
		part[i].offset = info->start[n];
		psize = i < 3 ? plen[i] : (info->size - size) / 2;
		while (part[i].size < psize) {
			if (++n > info->sector_count) {
				printf("Partitioning error. System halted.\n");
				while (1) ;
			}
			part[i].size += info->start[n] - info->start[n - 1];
		}
		size += part[i].size;
	}
	part[4].offset = info->start[n];
	part[4].size = info->start[info->sector_count - 1] - info->start[n];

	sprintf(mtdparts, "omapflash.0:"
			"%dk(U-Boot)ro,%dk(env),%dk(r_env),%dk(data1),-(data2)",
			part[0].size >> 10, part[1].size >> 10,
			part[2].size >> 10, part[3].size >> 10);
	setenv ("mtdparts", mtdparts);
}

struct part_info* jffs2_part_info(int part_num)
{
	void *jffs2_priv_saved = partinfo.jffs2_priv;

	if (part_num != 3 && part_num != 4)
		return NULL;

	if (current_part != part_num) {
		memset(&partinfo, 0, sizeof(partinfo));
		current_part = part_num;
		partinfo.offset = (char*) part[part_num].offset;
		partinfo.size = part[part_num].size;
		partinfo.usr_priv = &current_part;
		partinfo.jffs2_priv = jffs2_priv_saved;
	}

	return &partinfo;
}

#endif

int misc_init_r(void)
{
	*((volatile unsigned short *) VOICEBLUE_LED_REG) = 0x55;

#ifndef VOICEBLUE_SMALL_FLASH
	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf("Unknown flash. System halted.\n");
		while (1) ;
	}
	partition_flash(&flash_info[0]);
#endif

	return 0;
}

int board_late_init(void)
{
	*((volatile unsigned char *) VOICEBLUE_LED_REG) = 0x00;

	return 0;
}
