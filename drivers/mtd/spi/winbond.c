// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 * Copyright (C) 2016 Jagan Teki <jagan@openedev.com>
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <linux/sizes.h>
#include <linux/mtd/spi-nor.h>
#include "sf_internal.h"

static const struct flash_info winbond_nor_parts[] = {
	{ INFO("w25p80", 0xef2014, 0x0,	64 * 1024,    16, 0) },
	{ INFO("w25p16", 0xef2015, 0x0,	64 * 1024,    32, 0) },
	{ INFO("w25p32", 0xef2016, 0x0,	64 * 1024,    64, 0) },
	{ INFO("w25x05", 0xef3010, 0, 64 * 1024,  1,  SECT_4K) },
	{ INFO("w25x40", 0xef3013, 0, 64 * 1024,  8,  SECT_4K) },
	{ INFO("w25x16", 0xef3015, 0, 64 * 1024,  32, SECT_4K) },
	{ INFO("w25x32", 0xef3016, 0, 64 * 1024,  64, SECT_4K) },
	{ INFO("w25q20cl", 0xef4012, 0, 64 * 1024,  4, SECT_4K) },
	{ INFO("w25q20bw", 0xef5012, 0, 64 * 1024,  4, SECT_4K) },
	{ INFO("w25q20ew", 0xef6012, 0, 64 * 1024,  4, SECT_4K) },
	{
		INFO("w25q16dw", 0xef6015, 0, 64 * 1024,  32,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q32dw", 0xef6016, 0, 64 * 1024,  64,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q16jv", 0xef7015, 0, 64 * 1024,  32,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ)
	},
	{
		INFO("w25q32jv", 0xef7016, 0, 64 * 1024,  64,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q32jwm", 0xef8016, 0, 64 * 1024,  64,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q256jwm", 0xef8019, 0, 64 * 1024, 512,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{ INFO("w25x64", 0xef3017, 0, 64 * 1024, 128, SECT_4K) },
	{
		INFO("w25q64dw", 0xef6017, 0, 64 * 1024, 128,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q64jv", 0xef7017, 0, 64 * 1024, 128,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q128fw", 0xef6018, 0, 64 * 1024, 256,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q128jv", 0xef7018, 0, 64 * 1024, 256,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q128jw", 0xef8018, 0, 64 * 1024, 256,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q256fw", 0xef6019, 0, 64 * 1024, 512,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q256jw", 0xef7019, 0, 64 * 1024, 512,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q512nwq", 0xef6020, 0, 64 * 1024, 1024,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q512nwm", 0xef8020, 0, 64 * 1024, 1024,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q512jvq", 0xef4020, 0, 64 * 1024, 1024,
		     SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
		     SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q01jv", 0xef4021, 0, 64 * 1024, 2048,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q01jvfim", 0xef7021, 0, 64 * 1024, 2048,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{
		INFO("w25q02jv", 0xef7022, 0, 64 * 1024, 4096,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{ INFO("w25q80", 0xef5014, 0, 64 * 1024,  16, SECT_4K) },
	{ INFO("w25q80bl", 0xef4014, 0, 64 * 1024,  16, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{
		INFO("w25q16cl", 0xef4015, 0, 64 * 1024,  32,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{ INFO("w25q32", 0xef4016, 0, 64 * 1024,  64, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25q64cv", 0xef4017, 0, 64 * 1024,  128, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25q128", 0xef4018, 0, 64 * 1024, 256,
			SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ |
			SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
	},
	{ INFO("w25q256", 0xef4019, 0, 64 * 1024, 512, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25m512jw", 0xef6119, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25m512jv", 0xef7119, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25h02jv", 0xef9022, 0, 64 * 1024, 4096, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25h512nw-am", 0xefa020, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25h01nw-am", 0xefa021, 0, 64 * 1024, 2048, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25h02nw-am", 0xefa022, 0, 64 * 1024, 4096, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25q01nw-iq", 0xef6021, 0, 64 * 1024, 2048, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25q01nw-im", 0xef8021, 0, 64 * 1024, 2048, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w25q02nw-im", 0xef8022, 0, 64 * 1024, 4096, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ) },
	{ INFO("w77q51nw", 0xef8a1a, 0, 64 * 1024, 1024, SECT_4K | SPI_NOR_DUAL_READ | SPI_NOR_QUAD_READ | SPI_NOR_4B_OPCODES) },
};

#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY)

#define WINBOND_NOR_OP_SELDIE  0xc2    /* Select active die */

struct winbond_nor_priv {
	unsigned int n_dice;
};

/**
 * winbond_nor_select_die() - Set active die.
 * @nor:       pointer to 'struct spi_nor'.
 * @die:       die to set active.
 *
 * Certain Winbond chips feature more than a single die. This is mostly hidden
 * to the user, except that some chips may experience time deviation when
 * modifying the status bits between dies, which in some corner cases may
 * produce problematic races. Being able to explicitly select a die to check its
 * state in this case may be useful.
 *
 * Return: 0 on success, -errno otherwise.
 */
static int winbond_nor_select_die(struct spi_nor *nor, u8 die)
{
	struct spi_mem_op op =
		SPI_MEM_OP(SPI_MEM_OP_CMD(WINBOND_NOR_OP_SELDIE, 1),
			   SPI_MEM_OP_NO_ADDR,
			   SPI_MEM_OP_NO_DUMMY,
			   SPI_MEM_OP_DATA_OUT(1, &die, 0));

	int ret;

	spi_nor_setup_op(nor, &op, SNOR_PROTO_1_1_1);
	ret = spi_mem_exec_op(nor->spi, &op);
	if (ret)
		debug("Error %d selecting die %d\n", ret, die);

	return ret;
}

static int winbond_nor_multi_die_ready(struct spi_nor *nor)
{
	struct winbond_nor_priv *winbond_priv = nor->priv;
	unsigned int n_dice = winbond_priv ? winbond_priv->n_dice : 1;
	int ret, i;

	for (i = 0; i < n_dice; i++) {
		ret = winbond_nor_select_die(nor, i);
		if (ret)
			return ret;

		ret = spi_nor_sr_ready(nor);
		if (ret <= 0)
			return ret;
	}

	return 1;
}

static void winbond_nor_multi_die_post_sfdp_fixups(struct spi_nor *nor,
						   struct spi_nor_flash_parameter *param)
{
	struct winbond_nor_priv *winbond_priv;

	winbond_priv = kmalloc(sizeof(*winbond_priv), GFP_KERNEL);
	if (!winbond_priv)
		return;

	winbond_priv->n_dice = param->size / SZ_64M;

	/*
	 * SFDP supports dice numbers, but this information is only available in
	 * optional additional tables which are not provided by these chips.
	 * Dice number has an impact though, because these devices need extra
	 * care when reading the busy bit.
	 */
	nor->priv = winbond_priv;
	nor->ready = winbond_nor_multi_die_ready;
}

static struct spi_nor_fixups winbond_nor_multi_die_fixups = {
	.post_sfdp = winbond_nor_multi_die_post_sfdp_fixups,
};

static void winbond_set_fixups(struct spi_nor *nor)
{
	u8 multi_die_models[][2] = {
		{ 0x40, 0x21 }, /* W25Q01JV */
		{ 0x70, 0x22 }, /* W25Q02JV */
	};
	int i;

	for (i = 0; i < sizeof(multi_die_models) / 2; i++) {
		if (!memcmp(nor->info->id + 1, multi_die_models[i], 2)) {
			nor->fixups = &winbond_nor_multi_die_fixups;
			break;
		}
	}
}
#endif /* !CONFIG_IS_ENABLED(SPI_FLASH_TINY) */

const struct spi_nor_manufacturer spi_nor_winbond = {
	.parts = winbond_nor_parts,
	.nparts = ARRAY_SIZE(winbond_nor_parts),
#if !CONFIG_IS_ENABLED(SPI_FLASH_TINY)
	.set_fixups = winbond_set_fixups,
#endif
};
