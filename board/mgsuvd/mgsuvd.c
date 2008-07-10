/*
 * (C) Copyright 2007
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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

#if 0
#define DEBUG
#endif

#include <common.h>
#include <mpc8xx.h>

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

const uint sdram_table[] =
{
	0x0f07fc04, 0x0ffffc04, 0x00bdfc04, 0x0ff77c00,
	0x1ffffc05, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	/* 0x08 Burst Read */
	0x0f07fc04, 0x0ffffc04, 0x00bdfc04, 0x00fffc00,
	0x00fffc00, 0x00fffc00, 0x0ff77c00, 0x1ffffc05,
	/* 0x10 Load mode register */
	0x0ffffc34, 0x0ff57c04, 0x0ffffc04, 0x1ffffc05,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	/* 0x18 Single Write */
	0x0f07fc04, 0x0ffffc00, 0x00bd7c04, 0x0ffffc04,
	0x0ff77c04, 0x1ffffc05, 0xfffffc04, 0xfffffc04,
	/* 0x20 Burst Write */
	0x0f07fc04, 0x0ffffc00, 0x00bd7c00, 0x00fffc00,
	0x00fffc00, 0x00fffc04, 0x0ffffc04, 0x0ff77c04,
	0x1ffffc05, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	/* 0x30 Precharge all and Refresh */
	0x0ff77c04, 0x0ffffc04, 0x0ff5fc84, 0x0ffffc04,
	0x0ffffc04, 0x0ffffc84, 0x1ffffc05, 0xfffffc04,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	/* 0x3C Exception */
	0x7ffffc04, 0xfffffc07, 0xfffffc04, 0xfffffc04,
};

int checkboard (void)
{
	puts ("Board: Keymile mgsuvd\n");
	return (0);
}

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size;

	upmconfig (UPMB, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CFG_MPTPR;

	/*
	 * The following value is used as an address (i.e. opcode) for
	 * the LOAD MODE REGISTER COMMAND during SDRAM initialisation. If
	 * the port size is 32bit the SDRAM does NOT "see" the lower two
	 * address lines, i.e. mar=0x00000088 -> opcode=0x00000022 for
	 * MICRON SDRAMs:
	 * ->    0 00 010 0 010
	 *       |  |   | |   +- Burst Length = 4
	 *       |  |   | +----- Burst Type   = Sequential
	 *       |  |   +------- CAS Latency  = 2
	 *       |  +----------- Operating Mode = Standard
	 *       +-------------- Write Burst Mode = Programmed Burst Length
	 */
	memctl->memc_mar = CFG_MAR;

	/*
	 * Map controller banks 1 to the SDRAM banks 1 at
	 * preliminary addresses - these have to be modified after the
	 * SDRAM size has been determined.
	 */
	memctl->memc_or1 = CFG_OR1_PRELIM;
	memctl->memc_br1 = CFG_BR1_PRELIM;

	memctl->memc_mbmr = CFG_MBMR & (~(MBMR_PTBE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80802830;	/* SDRAM bank 0 */
	udelay (1);
	memctl->memc_mcr = 0x80802110;	/* SDRAM bank 0 - execute twice */
	udelay (1);

	memctl->memc_mbmr |= MBMR_PTBE;	/* enable refresh */

	udelay (1000);

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 */
	size =  get_ram_size(SDRAM_BASE1_PRELIM, SDRAM_MAX_SIZE);

	udelay (1000);

	debug ("SDRAM Bank 0: %ld MB\n", size >> 20);

	return (size);
}

/*
 * Early board initalization.
 */
int board_early_init_r(void)
{
	/* setup the UPIOx */
	*(char *)(CFG_PIGGY_BASE + 0x02) = 0xc0;
	*(char *)(CFG_PIGGY_BASE + 0x03) = 0x35;
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
/*
 * update "memory" property in the blob
 */
void ft_blob_update(void *blob, bd_t *bd)
{
	int ret, nodeoffset = 0;
	ulong brg_data[1] = {0};
	ulong memory_data[2] = {0};
	ulong flash_data[4] = {0};

	memory_data[0] = cpu_to_be32(bd->bi_memstart);
	memory_data[1] = cpu_to_be32(bd->bi_memsize);

		nodeoffset = fdt_path_offset (blob, "/memory");
		if (nodeoffset >= 0) {
			ret = fdt_setprop(blob, nodeoffset, "reg", memory_data,
						sizeof(memory_data));
		if (ret < 0)
			printf("ft_blob_update(): cannot set /memory/reg "
				"property err:%s\n", fdt_strerror(ret));
		}
		else {
			/* memory node is required in dts */
			printf("ft_blob_update(): cannot find /memory node "
			"err:%s\n", fdt_strerror(nodeoffset));
	}

	flash_data[2] = cpu_to_be32(bd->bi_flashstart);
	flash_data[3] = cpu_to_be32(bd->bi_flashsize);
	nodeoffset = fdt_path_offset (blob, "/localbus");
	if (nodeoffset >= 0) {
		ret = fdt_setprop(blob, nodeoffset, "ranges", flash_data,
					sizeof(flash_data));
	if (ret < 0)
		printf("ft_blob_update(): cannot set /localbus/ranges "
			"property err:%s\n", fdt_strerror(ret));
	}
	else {
		/* memory node is required in dts */
		printf("ft_blob_update(): cannot find /localbus node "
		"err:%s\n", fdt_strerror(nodeoffset));
	}
	/* BRG */
	brg_data[0] = cpu_to_be32(bd->bi_busfreq);
	nodeoffset = fdt_path_offset (blob, "/soc/cpm");
	if (nodeoffset >= 0) {
		ret = fdt_setprop(blob, nodeoffset, "brg-frequency", brg_data,
					sizeof(brg_data));
	if (ret < 0)
		printf("ft_blob_update(): cannot set /soc/cpm/brg-frequency "
			"property err:%s\n", fdt_strerror(ret));
	}
	else {
		/* memory node is required in dts */
		printf("ft_blob_update(): cannot find /soc/cpm node "
		"err:%s\n", fdt_strerror(nodeoffset));
	}
	/* MAC Adresse */
	nodeoffset = fdt_path_offset (blob, "/soc/cpm/ethernet");
	if (nodeoffset >= 0) {
		ret = fdt_setprop(blob, nodeoffset, "mac-address", bd->bi_enetaddr,
					sizeof(uchar) * 6);
	if (ret < 0)
		printf("ft_blob_update(): cannot set /soc/cpm/scc/mac-address "
			"property err:%s\n", fdt_strerror(ret));
	}
	else {
		/* memory node is required in dts */
		printf("ft_blob_update(): cannot find /soc/cpm/ethernet node "
		"err:%s\n", fdt_strerror(nodeoffset));
	}
}

void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup( blob, bd);
	ft_blob_update(blob, bd);
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
