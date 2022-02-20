// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Gateworks Corporation
 */

#include <common.h>
#include <init.h>
#include <led.h>
#include <linux/delay.h>
#include <miiphy.h>
#include <netdev.h>

#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/unaligned.h>

#include "gsc.h"

DECLARE_GLOBAL_DATA_PTR;

int board_phys_sdram_size(phys_size_t *size)
{
	const fdt64_t *val;
	int offset;
	int len;

	/* get size from dt which SPL updated per EEPROM config */
	offset = fdt_path_offset(gd->fdt_blob, "/memory");
	if (offset < 0)
		return -EINVAL;

	val = fdt_getprop(gd->fdt_blob, offset, "reg", &len);
	if (len < sizeof(*val) * 2)
		return -EINVAL;
	*size = get_unaligned_be64(&val[1]);

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	int i  = 0;
	const char *dtb;
	static char init;
	char buf[32];

	do {
		dtb = gsc_get_dtb_name(i++, buf, sizeof(buf));
		if (!strcmp(dtb, name)) {
			if (!init++)
				printf("DTB     : %s\n", name);
			return 0;
		}
	} while (dtb);

	return -1;
}

#if (IS_ENABLED(CONFIG_FEC_MXC))
static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	unsigned short val;

	switch (phydev->phy_id) {
	case 0x2000a231: /* TI DP83867 GbE PHY */
		puts("DP83867 ");
		/* LED configuration */
		val = 0;
		val |= 0x5 << 4; /* LED1(Amber;Speed)   : 1000BT link */
		val |= 0xb << 8; /* LED2(Green;Link/Act): blink for TX/RX act */
		phy_write(phydev, MDIO_DEVAD_NONE, 24, val);
		break;
	}

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif // IS_ENABLED(CONFIG_FEC_MXC)

int board_init(void)
{
	gsc_init(1);

	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	gsc_hwmon();

	return 0;
}

int board_late_init(void)
{
	const char *str;
	char env[32];
	int ret, i;
	u8 enetaddr[6];
	char fdt[64];

	led_default_state();

	/* Set board serial/model */
	if (!env_get("serial#"))
		env_set_ulong("serial#", gsc_get_serial());
	env_set("model", gsc_get_model());

	/* Set fdt_file vars */
	i = 0;
	do {
		str = gsc_get_dtb_name(i, fdt, sizeof(fdt));
		if (str) {
			sprintf(env, "fdt_file%d", i + 1);
			strcat(fdt, ".dtb");
			env_set(env, fdt);
		}
		i++;
	} while (str);

	/* Set mac addrs */
	i = 0;
	do {
		if (i)
			sprintf(env, "eth%daddr", i);
		else
			sprintf(env, "ethaddr");
		str = env_get(env);
		if (!str) {
			ret = gsc_getmac(i, enetaddr);
			if (!ret)
				eth_env_set_enetaddr(env, enetaddr);
		}
		i++;
	} while (!ret);

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int off;

	/* set board model dt prop */
	fdt_setprop_string(blob, 0, "board", gsc_get_model());

	/* update temp thresholds */
	off = fdt_path_offset(blob, "/thermal-zones/cpu-thermal/trips");
	if (off >= 0) {
		int minc, maxc, prop;

		get_cpu_temp_grade(&minc, &maxc);
		fdt_for_each_subnode(prop, blob, off) {
			const char *type = fdt_getprop(blob, prop, "type", NULL);

			if (type && (!strcmp("critical", type)))
				fdt_setprop_u32(blob, prop, "temperature", maxc * 1000);
			else if (type && (!strcmp("passive", type)))
				fdt_setprop_u32(blob, prop, "temperature", (maxc - 10) * 1000);
		}
	}

	return 0;
}
