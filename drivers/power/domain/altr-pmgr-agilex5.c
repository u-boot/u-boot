// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <dm.h>
#include <power-domain-uclass.h>
#include <asm/io.h>
#include <asm/arch/handoff_soc64.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <time.h>

#define PSS_FWENCTL					0x44
#define PSS_PGENCTL					0x48
#define PSS_PGSTAT					0x4c

#define DATA_MASK					GENMASK(7, 0)
#define TIMEOUT_MS					1000

static int wait_verify_fsm(u16 timeout_ms, uintptr_t base_addr, u32 peripheral_handoff)
{
	u32 data = 0;
	u32 pgstat = 0;
	ulong start = get_timer(0);

	/* Wait FSM ready */
	do {
		data = FIELD_GET(DATA_MASK, readl(base_addr + PSS_PGSTAT));
		if (data != 0)
			break;
	} while (get_timer(start) < timeout_ms);

	if (get_timer(start) >= timeout_ms)
		return -ETIMEDOUT;

	/* Verify PSS SRAM power gated */
	pgstat = FIELD_GET(DATA_MASK, readl(base_addr + PSS_PGSTAT));
	if (pgstat != FIELD_GET(DATA_MASK, peripheral_handoff))
		return -EPERM;

	return 0;
}

static int pss_sram_power_off(uintptr_t base_addr, u32 *handoff_table)
{
	u32 peripheral_handoff;

	/* Get PSS SRAM handoff data */
	peripheral_handoff = handoff_table[0];

	/* Enable firewall for PSS SRAM */
	setbits_le32(base_addr + PSS_FWENCTL, peripheral_handoff);

	/* Wait */
	udelay(1);

	/* Power gating PSS SRAM */
	setbits_le32(base_addr + PSS_PGENCTL, peripheral_handoff);

	return wait_verify_fsm(TIMEOUT_MS, base_addr, peripheral_handoff);
}

static int altera_pmgr_off(struct power_domain *power_domain)
{
	fdt_addr_t base_addr = dev_read_addr(power_domain->dev);
	u32 handoff_table[SOC64_HANDOFF_PERI_LEN];
	int ret;

	/* Read handoff data for peripherals configuration */
	ret = socfpga_handoff_read((void *)SOC64_HANDOFF_PERI, handoff_table,
				   SOC64_HANDOFF_PERI_LEN);
	if (ret) {
		debug("%s: handoff data read failed. ret: %d\n", __func__, ret);
		return ret;
	}

	pss_sram_power_off(base_addr, handoff_table);

	return 0;
}

static int altera_pmgr_probe(struct udevice *dev)
{
	struct power_domain *power_domain = dev_get_priv(dev);

	if (!power_domain)
		return -EINVAL;

	power_domain->dev = dev;

	return altera_pmgr_off(power_domain);
}

static const struct udevice_id altera_pmgr_ids[] = {
	{ .compatible = "altr,pmgr-agilex5" },
	{ /* sentinel */ }
};

static struct power_domain_ops altera_pmgr_ops = {
	.off = altera_pmgr_off,
};

U_BOOT_DRIVER(altr_pmgr) = {
	.name = "altr_pmgr",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = altera_pmgr_ids,
	.ops = &altera_pmgr_ops,
	.probe = altera_pmgr_probe,
	.priv_auto = sizeof(struct power_domain),
};
