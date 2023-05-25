// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Collabora Ltd.
 * Copyright 2018-2020 Variscite Ltd.
 * Copyright 2023 DimOnOff Inc.
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <i2c_eeprom.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

/* Optional SOM features flags. */
#define VAR_EEPROM_F_WIFI		BIT(0)
#define VAR_EEPROM_F_ETH		BIT(1) /* Ethernet PHY on SOM. */
#define VAR_EEPROM_F_AUDIO		BIT(2)
#define VAR_EEPROM_F_MX8M_LVDS		BIT(3) /* i.MX8MM, i.MX8MN, i.MX8MQ only */
#define VAR_EEPROM_F_MX8Q_SOC_ID	BIT(3) /* 0 = i.MX8QM, 1 = i.MX8QP */
#define VAR_EEPROM_F_NAND		BIT(4)

#define VAR_IMX8_EEPROM_MAGIC	0x384D /* "8M" */

/* Number of DRAM adjustment tables. */
#define DRAM_TABLES_NUM 7

struct var_imx8_eeprom_info {
	u16 magic;
	u8 partnumber[3];         /* Part number */
	u8 assembly[10];          /* Assembly number */
	u8 date[9];               /* Build date */
	u8 mac[6];                /* MAC address */
	u8 somrev;
	u8 eeprom_version;
	u8 features;              /* SOM features */
	u8 dramsize;              /* DRAM size */
	u8 off[DRAM_TABLES_NUM + 1]; /* DRAM table offsets */
	u8 partnumber2[5];        /* Part number 2 */
} __packed;

static void setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

#if !defined(CONFIG_SPL_BUILD)

#if defined(CONFIG_DISPLAY_BOARDINFO)

static void display_som_infos(struct var_imx8_eeprom_info *info)
{
	char partnumber[sizeof(info->partnumber) +
			sizeof(info->partnumber2) + 1];
	char assembly[sizeof(info->assembly) + 1];
	char date[sizeof(info->date) + 1];

	/* Read first part of P/N. */
	memcpy(partnumber, info->partnumber, sizeof(info->partnumber));

	/* Read second part of P/N. */
	if (info->eeprom_version >= 3)
		memcpy(partnumber + sizeof(info->partnumber), info->partnumber2,
		       sizeof(info->partnumber2));

	memcpy(assembly, info->assembly, sizeof(info->assembly));
	memcpy(date, info->date, sizeof(info->date));

	/* Make sure strings are null terminated. */
	partnumber[sizeof(partnumber) - 1] = '\0';
	assembly[sizeof(assembly) - 1] = '\0';
	date[sizeof(date) - 1] = '\0';

	printf("SOM board: P/N: %s, Assy: %s, Date: %s\n"
	       "           Wifi: %s, EthPhy: %s, Rev: %d\n",
	       partnumber, assembly, date,
	       info->features & VAR_EEPROM_F_WIFI ? "yes" : "no",
	       info->features & VAR_EEPROM_F_ETH ? "yes" : "no",
	       info->somrev);
}

static int var_read_som_eeprom(struct var_imx8_eeprom_info *info)
{
	const char *path = "eeprom-som";
	struct udevice *dev;
	int ret, off;

	off = fdt_path_offset(gd->fdt_blob, path);
	if (off < 0) {
		pr_err("%s: fdt_path_offset() failed: %d\n", __func__, off);
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		pr_err("%s: uclass_get_device_by_of_offset() failed: %d\n",
		       __func__, ret);
		return ret;
	}

	ret = i2c_eeprom_read(dev, 0, (uint8_t *)info,
			      sizeof(struct var_imx8_eeprom_info));
	if (ret) {
		pr_err("%s: i2c_eeprom_read() failed: %d\n", __func__, ret);
		return ret;
	}

	if (htons(info->magic) != VAR_IMX8_EEPROM_MAGIC) {
		/* Do not fail if the content is invalid */
		pr_err("Board: Invalid board info magic: 0x%08x, expected 0x%08x\n",
		       htons(info->magic), VAR_IMX8_EEPROM_MAGIC);
	}

	return 0;
}

int checkboard(void)
{
	int rc;
	struct var_imx8_eeprom_info *info;

	info = malloc(sizeof(struct var_imx8_eeprom_info));
	if (!info)
		return -ENOMEM;

	rc = var_read_som_eeprom(info);
	if (rc)
		return rc;

	display_som_infos(info);

#if defined(CONFIG_BOARD_TYPES)
	gd->board_type = info->features;
#endif /* CONFIG_BOARD_TYPES */

	return 0;
}

#endif /* CONFIG_DISPLAY_BOARDINFO */

#endif /* CONFIG_SPL_BUILD */
