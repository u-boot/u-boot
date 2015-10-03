/*
 * (C) Copyright 2012
 * Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/pmic_bus.h>
#include <axp_pmic.h>

static u8 axp209_mvolt_to_cfg(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

int axp_set_dcdc2(unsigned int mvolt)
{
	int rc;
	u8 cfg, current;

	cfg = axp209_mvolt_to_cfg(mvolt, 700, 2275, 25);

	/* Do we really need to be this gentle? It has built-in voltage slope */
	while ((rc = pmic_bus_read(AXP209_DCDC2_VOLTAGE, &current)) == 0 &&
	       current != cfg) {
		if (current < cfg)
			current++;
		else
			current--;

		rc = pmic_bus_write(AXP209_DCDC2_VOLTAGE, current);
		if (rc)
			break;
	}

	return rc;
}

int axp_set_dcdc3(unsigned int mvolt)
{
	u8 cfg = axp209_mvolt_to_cfg(mvolt, 700, 3500, 25);

	return pmic_bus_write(AXP209_DCDC3_VOLTAGE, cfg);
}

int axp_set_aldo2(unsigned int mvolt)
{
	int rc;
	u8 cfg, reg;

	cfg = axp209_mvolt_to_cfg(mvolt, 1800, 3300, 100);

	rc = pmic_bus_read(AXP209_LDO24_VOLTAGE, &reg);
	if (rc)
		return rc;

	/* LDO2 configuration is in upper 4 bits */
	reg = (reg & 0x0f) | (cfg << 4);
	return pmic_bus_write(AXP209_LDO24_VOLTAGE, reg);
}

int axp_set_aldo3(unsigned int mvolt)
{
	u8 cfg;

	if (mvolt == -1)
		cfg = 0x80;	/* determined by LDO3IN pin */
	else
		cfg = axp209_mvolt_to_cfg(mvolt, 700, 3500, 25);

	return pmic_bus_write(AXP209_LDO3_VOLTAGE, cfg);
}

int axp_set_aldo4(unsigned int mvolt)
{
	int rc;
	static const unsigned int vindex[] = {
		1250, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2500,
		2700, 2800, 3000, 3100, 3200, 3300
	};
	u8 cfg, reg;

	/* Translate mvolt to register cfg value, requested <= selected */
	for (cfg = 15; vindex[cfg] > mvolt && cfg > 0; cfg--);

	rc = pmic_bus_read(AXP209_LDO24_VOLTAGE, &reg);
	if (rc)
		return rc;

	/* LDO4 configuration is in lower 4 bits */
	reg = (reg & 0xf0) | (cfg << 0);
	return pmic_bus_write(AXP209_LDO24_VOLTAGE, reg);
}

int axp_init(void)
{
	u8 ver;
	int i, rc;

	rc = pmic_bus_init();
	if (rc)
		return rc;

	rc = pmic_bus_read(AXP209_CHIP_VERSION, &ver);
	if (rc)
		return rc;

	/* Low 4 bits is chip version */
	ver &= 0x0f;

	if (ver != 0x1)
		return -1;

	/* Mask all interrupts */
	for (i = AXP209_IRQ_ENABLE1; i <= AXP209_IRQ_ENABLE5; i++) {
		rc = pmic_bus_write(i, 0);
		if (rc)
			return rc;
	}

	return 0;
}
