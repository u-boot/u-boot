/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>

struct pmic *pfuze_common_init(unsigned char i2cbus)
{
	struct pmic *p;
	int ret;
	unsigned int reg;

	ret = power_pfuze100_init(i2cbus);
	if (ret)
		return NULL;

	p = pmic_get("PFUZE100");
	ret = pmic_probe(p);
	if (ret)
		return NULL;

	pmic_reg_read(p, PFUZE100_DEVICEID, &reg);
	printf("PMIC:  PFUZE100 ID=0x%02x\n", reg);

	/* Set SW1AB stanby volage to 0.975V */
	pmic_reg_read(p, PFUZE100_SW1ABSTBY, &reg);
	reg &= ~SW1x_STBY_MASK;
	reg |= SW1x_0_975V;
	pmic_reg_write(p, PFUZE100_SW1ABSTBY, reg);

	/* Set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
	pmic_reg_read(p, PUZE_100_SW1ABCONF, &reg);
	reg &= ~SW1xCONF_DVSSPEED_MASK;
	reg |= SW1xCONF_DVSSPEED_4US;
	pmic_reg_write(p, PUZE_100_SW1ABCONF, reg);

	/* Set SW1C standby voltage to 0.975V */
	pmic_reg_read(p, PFUZE100_SW1CSTBY, &reg);
	reg &= ~SW1x_STBY_MASK;
	reg |= SW1x_0_975V;
	pmic_reg_write(p, PFUZE100_SW1CSTBY, reg);

	/* Set SW1C/VDDSOC step ramp up time from 16us to 4us/25mV */
	pmic_reg_read(p, PFUZE100_SW1CCONF, &reg);
	reg &= ~SW1xCONF_DVSSPEED_MASK;
	reg |= SW1xCONF_DVSSPEED_4US;
	pmic_reg_write(p, PFUZE100_SW1CCONF, reg);

	return p;
}
