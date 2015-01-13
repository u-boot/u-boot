/*
 * Keystone2: DDR3 configuration
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>

#include <i2c.h>
#include <asm/arch/ddr3.h>
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

/* DDR3 PHY configuration data with 1600M rate, 8GB size */
struct ddr3_phy_config ddr3phy_1600_8g = {
	.pllcr          = 0x0001C000ul,
	.pgcr1_mask     = (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val      = ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0           = 0x42C21590ul,
	.ptr1           = 0xD05612C0ul,
	.ptr2           = 0, /* not set in gel */
	.ptr3           = 0x0D861A80ul,
	.ptr4           = 0x0C827100ul,
	.dcr_mask       = (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val        = ((1 << 10)),
	.dtpr0          = 0xA19DBB66ul,
	.dtpr1          = 0x32868300ul,
	.dtpr2          = 0x50035200ul,
	.mr0            = 0x00001C70ul,
	.mr1            = 0x00000006ul,
	.mr2            = 0x00000018ul,
	.dtcr           = 0x730035C7ul,
	.pgcr2          = 0x00F07A12ul,
	.zq0cr1         = 0x0000005Dul,
	.zq1cr1         = 0x0000005Bul,
	.zq2cr1         = 0x0000005Bul,
	.pir_v1         = 0x00000033ul,
	.pir_v2         = 0x0000FF81ul,
};

/* DDR3 EMIF configuration data with 1600M rate, 8GB size */
struct ddr3_emif_config ddr3_1600_8g = {
	.sdcfg          = 0x6200CE6Aul,
	.sdtim1         = 0x16709C55ul,
	.sdtim2         = 0x00001D4Aul,
	.sdtim3         = 0x435DFF54ul,
	.sdtim4         = 0x553F0CFFul,
	.zqcfg          = 0xF0073200ul,
	.sdrfc          = 0x00001869ul,
};

#ifdef CONFIG_K2HK_EVM
/* DDR3 PHY configuration data with 1333M rate, and 2GB size */
struct ddr3_phy_config ddr3phy_1333_2g = {
	.pllcr          = 0x0005C000ul,
	.pgcr1_mask     = (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val      = ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0           = 0x42C21590ul,
	.ptr1           = 0xD05612C0ul,
	.ptr2           = 0, /* not set in gel */
	.ptr3           = 0x0B4515C2ul,
	.ptr4           = 0x0A6E08B4ul,
	.dcr_mask       = (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val        = ((1 << 10)),
	.dtpr0          = 0x8558AA55ul,
	.dtpr1          = 0x32857280ul,
	.dtpr2          = 0x5002C200ul,
	.mr0            = 0x00001A60ul,
	.mr1            = 0x00000006ul,
	.mr2            = 0x00000010ul,
	.dtcr           = 0x710035C7ul,
	.pgcr2          = 0x00F065B8ul,
	.zq0cr1         = 0x0000005Dul,
	.zq1cr1         = 0x0000005Bul,
	.zq2cr1         = 0x0000005Bul,
	.pir_v1         = 0x00000033ul,
	.pir_v2         = 0x0000FF81ul,
};

/* DDR3 EMIF configuration data with 1333M rate, and 2GB size */
struct ddr3_emif_config ddr3_1333_2g = {
	.sdcfg          = 0x62008C62ul,
	.sdtim1         = 0x125C8044ul,
	.sdtim2         = 0x00001D29ul,
	.sdtim3         = 0x32CDFF43ul,
	.sdtim4         = 0x543F0ADFul,
	.zqcfg          = 0x70073200ul,
	.sdrfc          = 0x00001457ul,
};
#endif

#ifdef CONFIG_K2E_EVM
/* DDR3 PHY configuration data with 1600M rate, and 4GB size  */
struct ddr3_phy_config ddr3phy_1600_4g = {
	.pllcr          = 0x0001C000ul,
	.pgcr1_mask     = (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val      = ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0           = 0x42C21590ul,
	.ptr1           = 0xD05612C0ul,
	.ptr2           = 0, /* not set in gel */
	.ptr3           = 0x08861A80ul,
	.ptr4           = 0x0C827100ul,
	.dcr_mask       = (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val        = ((1 << 10)),
	.dtpr0          = 0x9D9CBB66ul,
	.dtpr1          = 0x12840300ul,
	.dtpr2          = 0x5002D200ul,
	.mr0            = 0x00001C70ul,
	.mr1            = 0x00000006ul,
	.mr2            = 0x00000018ul,
	.dtcr           = 0x710035C7ul,
	.pgcr2          = 0x00F07A12ul,
	.zq0cr1         = 0x0001005Dul,
	.zq1cr1         = 0x0001005Bul,
	.zq2cr1         = 0x0001005Bul,
	.pir_v1         = 0x00000033ul,
	.pir_v2         = 0x0000FF81ul,
};

/* DDR3 EMIF configuration data with 1600M rate, and 4GB size  */
struct ddr3_emif_config ddr3_1600_4g = {
	.sdcfg          = 0x6200CE62ul,
	.sdtim1         = 0x166C9855ul,
	.sdtim2         = 0x00001D4Aul,
	.sdtim3         = 0x421DFF53ul,
	.sdtim4         = 0x543F07FFul,
	.zqcfg          = 0x70073200ul,
	.sdrfc          = 0x00001869ul,
};
#endif

struct ddr3_phy_config ddr3phy_1600_2g = {
	.pllcr          = 0x0001C000ul,
	.pgcr1_mask     = (IODDRM_MASK | ZCKSEL_MASK),
	.pgcr1_val      = ((1 << 2) | (1 << 7) | (1 << 23)),
	.ptr0           = 0x42C21590ul,
	.ptr1           = 0xD05612C0ul,
	.ptr2           = 0, /* not set in gel */
	.ptr3           = 0x0D861A80ul,
	.ptr4           = 0x0C827100ul,
	.dcr_mask       = (PDQ_MASK | MPRDQ_MASK | BYTEMASK_MASK),
	.dcr_val        = ((1 << 10)),
	.dtpr0          = 0x9D5CBB66ul,
	.dtpr1          = 0x12868300ul,
	.dtpr2          = 0x5002D200ul,
	.mr0            = 0x00001C70ul,
	.mr1            = 0x00000006ul,
	.mr2            = 0x00000018ul,
	.dtcr           = 0x710035C7ul,
	.pgcr2          = 0x00F07A12ul,
	.zq0cr1         = 0x0001005Dul,
	.zq1cr1         = 0x0001005Bul,
	.zq2cr1         = 0x0001005Bul,
	.pir_v1         = 0x00000033ul,
	.pir_v2         = 0x0000FF81ul,
};

struct ddr3_emif_config ddr3_1600_2g = {
	.sdcfg          = 0x6200CE62ul,
	.sdtim1         = 0x166C9855ul,
	.sdtim2         = 0x00001D4Aul,
	.sdtim3         = 0x435DFF53ul,
	.sdtim4         = 0x543F0CFFul,
	.zqcfg          = 0x70073200ul,
	.sdrfc          = 0x00001869ul,
};

int ddr3_get_dimm_params(char *dimm_name)
{
	int ret;
	int old_bus;
	u8 spd_params[256];

	i2c_init(CONFIG_SYS_DAVINCI_I2C_SPEED, CONFIG_SYS_DAVINCI_I2C_SLAVE);

	old_bus = i2c_get_bus_num();
	i2c_set_bus_num(1);

	ret = i2c_read(0x53, 0, 1, spd_params, 256);

	i2c_set_bus_num(old_bus);

	dimm_name[0] = '\0';

	if (ret) {
		puts("Cannot read DIMM params\n");
		return 1;
	}

	/*
	 * We need to convert spd data to dimm parameters
	 * and to DDR3 EMIF and PHY regirsters values.
	 * For now we just return DIMM type string value.
	 * Caller may use this value to choose appropriate
	 * a pre-set DDR3 configuration
	 */

	strncpy(dimm_name, (char *)&spd_params[0x80], 18);
	dimm_name[18] = '\0';

	return 0;
}
