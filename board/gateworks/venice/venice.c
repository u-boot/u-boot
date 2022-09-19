// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Gateworks Corporation
 */

#include <fdt_support.h>
#include <init.h>
#include <led.h>
#include <miiphy.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>

#include "eeprom.h"

int board_phys_sdram_size(phys_size_t *size)
{
	if (!size)
		return -EINVAL;

	*size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	int i  = 0;
	const char *dtb;
	static char init;
	char buf[32];

	do {
		dtb = eeprom_get_dtb_name(i++, buf, sizeof(buf));
		if (!strcmp(dtb, name)) {
			if (!init++)
				printf("DTB     : %s\n", name);
			return 0;
		}
	} while (dtb);

	return -1;
}

#if (IS_ENABLED(CONFIG_NET))
static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

#ifndef CONFIG_IMX8MP
	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);
#else
	/* Enable RGMII TX clk output */
	setbits_le32(&gpr->gpr[1], BIT(22));
#endif

	return 0;
}

static int setup_eqos(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* set INTF as RGMII, enable RGMII TXC clock */
	clrsetbits_le32(&gpr->gpr[1],
			IOMUXC_GPR_GPR1_GPR_ENET_QOS_INTF_SEL_MASK, BIT(16));
	setbits_le32(&gpr->gpr[1], BIT(19) | BIT(21));

	return set_clk_eqos(ENET_125MHZ);
}

int board_phy_config(struct phy_device *phydev)
{
	unsigned short val;
	ofnode node;

	switch (phydev->phy_id) {
	case 0x2000a231: /* TI DP83867 GbE PHY */
		puts("DP83867 ");
		/* LED configuration */
		val = 0;
		val |= 0x5 << 4; /* LED1(Amber;Speed)   : 1000BT link */
		val |= 0xb << 8; /* LED2(Green;Link/Act): blink for TX/RX act */
		phy_write(phydev, MDIO_DEVAD_NONE, 24, val);
		break;
	case 0xd565a401: /* MaxLinear GPY111 */
		puts("GPY111 ");
		node = phy_get_ofnode(phydev);
		if (ofnode_valid(node)) {
			u32 rx_delay, tx_delay;

			rx_delay = ofnode_read_u32_default(node, "rx-internal-delay-ps", 2000);
			tx_delay = ofnode_read_u32_default(node, "tx-internal-delay-ps", 2000);
			val = phy_read(phydev, MDIO_DEVAD_NONE, 0x17);
			val &= ~((0x7 << 12) | (0x7 << 8));
			val |= (rx_delay / 500) << 12;
			val |= (tx_delay / 500) << 8;
			phy_write(phydev, MDIO_DEVAD_NONE, 0x17, val);
		}
		break;
	}

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif // IS_ENABLED(CONFIG_NET)

int board_init(void)
{
	eeprom_init(1);

	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();
	if (IS_ENABLED(CONFIG_DWC_ETH_QOS))
		setup_eqos();

	return 0;
}

int board_late_init(void)
{
	const char *str;
	char env[32];
	int ret, i;
	u8 enetaddr[6];
	char fdt[64];

	/* Set board serial/model */
	if (!env_get("serial#"))
		env_set_ulong("serial#", eeprom_get_serial());
	env_set("model", eeprom_get_model());

	/* Set fdt_file vars */
	i = 0;
	do {
		str = eeprom_get_dtb_name(i, fdt, sizeof(fdt));
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
			ret = eeprom_getmac(i, enetaddr);
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

int ft_board_setup(void *fdt, struct bd_info *bd)
{
	const char *base_model = eeprom_get_baseboard_model();
	char pcbrev;
	int off;

	/* set board model dt prop */
	fdt_setprop_string(fdt, 0, "board", eeprom_get_model());

	/* update temp thresholds */
	off = fdt_path_offset(fdt, "/thermal-zones/cpu-thermal/trips");
	if (off >= 0) {
		int minc, maxc, prop;

		get_cpu_temp_grade(&minc, &maxc);
		fdt_for_each_subnode(prop, fdt, off) {
			const char *type = fdt_getprop(fdt, prop, "type", NULL);

			if (type && (!strcmp("critical", type)))
				fdt_setprop_u32(fdt, prop, "temperature", maxc * 1000);
			else if (type && (!strcmp("passive", type)))
				fdt_setprop_u32(fdt, prop, "temperature", (maxc - 10) * 1000);
		}
	}

	if (!strncmp(base_model, "GW73", 4)) {
		pcbrev = get_pcb_rev(base_model);

		if (pcbrev > 'B') {
			printf("adjusting dt for %s\n", base_model);

			/*
			 * revC replaced PCIe 5-port switch with 4-port
			 * which changed ethernet1 PCIe GbE
			 * from: pcie@0,0/pcie@1,0/pcie@2,4/pcie@6.0
			 *   to: pcie@0,0/pcie@1,0/pcie@2,3/pcie@5.0
			 */
			off = fdt_path_offset(fdt, "ethernet1");
			if (off > 0) {
				u32 reg[5];

				fdt_set_name(fdt, off, "pcie@5,0");
				off = fdt_parent_offset(fdt, off);
				fdt_set_name(fdt, off, "pcie@2,3");
				memset(reg, 0, sizeof(reg));
				reg[0] = cpu_to_fdt32(PCI_DEVFN(3, 0));
				fdt_setprop(fdt, off, "reg", reg, sizeof(reg));
			}
		}
	}

	return 0;
}
