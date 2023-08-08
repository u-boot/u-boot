/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 * https://spdx.org/licenses
 */

#include <common.h>
#include <asm/arch-armada8k/cache_llc.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <mvebu/mvebu_chip_sar.h>
#include <dm/device.h>

#define CP_DEV_ID_STATUS_REG		(MVEBU_REGISTER(0x2400240))
#define DEVICE_ID_STATUS_MASK		0xffff
#define AP_DEV_ID_STATUS_REG		(SOC_REGS_PHY_BASE + 0x6F8240)
#define JTAG_DEV_ID_STATUS_REG		(SOC_REGS_PHY_BASE + 0x6F8244)
#define AP_DEV_ID_STATUS_MASK		0xfff
#define AP_DEV_REV_ID_STATUS_MASK	0xf0000000
#define SW_REV_STATUS_OFFSET		16
#define AP_REV_STATUS_OFFSET		28
#define SW_REV_STATUS_MASK		0xf

#define A8040_DEVICE_ID			0x8040
#define CN9130_DEVICE_ID		0x7025

#define AP807_ID			0x807

/* to differentiate differnet SOC with similar DEVICE_ID */
#define AP807_SHARED_DEVICE_ID_A0	0x7045
#define AP807_SHARED_DEVICE_ID_A1	0x6025

#define DEVICE_ID_SUB_REV		(MVEBU_REGISTER(0x2400230))
#define DEVICE_ID_SUB_REV_OFFSET	7
#define DEVICE_ID_SUB_REV_MASK		(0xffff << DEVICE_ID_SUB_REV_OFFSET)

#define NF_CLOCK_SEL_MASK		0x1
#define SOC_MUX_NAND_EN_MASK		0x1
#define CLOCK_1Mhz			1000000

struct mochi_module {
	u32 module_type;
	u32 module_rev;
};

struct soc_info {
	struct mochi_module soc;
	char *soc_name;
	struct mochi_module ap;
	struct mochi_module cp;
	u32 ap_num;
	u32 cp_num;
	u32 sub_rev;
};

static struct soc_info soc_info_table[] = {
	{ {0x7025, 0}, "cn9130-A1",	{0x807, 2}, {0x115, 0}, 1, 1, 0},
	{ {0x7025, 0}, "cn9131-A1",	{0x807, 2}, {0x115, 0}, 1, 2, 0},
	{ {0x7025, 0}, "cn9132-A1",	{0x807, 2}, {0x115, 0}, 1, 3, 0},
	{ {0x6025, 0}, "Armada3900-A1",	{0x807, 1}, {0x115, 0}, 1, 1, 0},
	{ {0x6025, 0}, "Armada3900-A3",	{0x807, 2}, {0x115, 0}, 1, 1, 0},
	{ {0x7045, 0}, "Armada3900-A0", {0x807, 0}, {0x115, 0}, 1, 1, 0},
	{ {0x7040, 1}, "Armada7040-A1", {0x806, 1}, {0x110, 1}, 1, 1, 0},
	{ {0x7040, 2}, "Armada7040-A2", {0x806, 1}, {0x110, 2}, 1, 1, 0},
	{ {0x7045, 0}, "Armada7040-B0", {0x806, 2}, {0x115, 0}, 1, 1, 0},
	{ {0x8040, 1}, "Armada8040-A1", {0x806, 1}, {0x110, 1}, 1, 2, 0},
	{ {0x8040, 2}, "Armada8040-A2", {0x806, 1}, {0x110, 2}, 1, 2, 0},
	{ {0x8045, 0}, "Armada8040-B0", {0x806, 2}, {0x115, 0}, 1, 2, 0},
};

int mvebu_dfx_smc(u32 subfid, u32 *reg, u32 addr, u32 val)
{
	struct pt_regs pregs = {0};

	pregs.regs[0] = MV_SIP_DFX;
	pregs.regs[1] = subfid;
	pregs.regs[2] = addr;
	pregs.regs[3] = val;

	smc_call(&pregs);

	if (pregs.regs[0] == 0 && reg)
		*reg = pregs.regs[1];

	debug("%s: sub-fid %d, reg_val 0x%x, addr 0x%x, val 0x%x, ret %ld\n",
	      __func__, subfid, reg ? *reg : -1, addr, val, pregs.regs[0]);

	return pregs.regs[0];
}

static int get_soc_type_rev(u32 *type, u32 *rev)
{
	*type = readl(CP_DEV_ID_STATUS_REG) & DEVICE_ID_STATUS_MASK;
	*rev = (readl(CP_DEV_ID_STATUS_REG) >> SW_REV_STATUS_OFFSET) &
		SW_REV_STATUS_MASK;

	return 0;
}

static int get_ap_soc_type(u32 *type)
{
	int ret;

	/* Try read it with firmware use, if fails try legacy */
	ret = mvebu_dfx_sread(type, AP_DEV_ID_STATUS_REG);
	if (ret != SMCCC_RET_SUCCESS)
		*type = readl(AP_DEV_ID_STATUS_REG);

	*type &= AP_DEV_ID_STATUS_MASK;

	return 0;
}

static int get_ap_soc_rev(u32 *rev)
{
	int ret;

	/* Try read it with firmware use, if fails try legacy */
	ret = mvebu_dfx_sread(rev, JTAG_DEV_ID_STATUS_REG);
	if (ret != SMCCC_RET_SUCCESS)
		*rev = readl(JTAG_DEV_ID_STATUS_REG);

	*rev = (*rev & AP_DEV_REV_ID_STATUS_MASK) >> AP_REV_STATUS_OFFSET;
	return 0;
}

static int get_soc_sub_rev(u32 *sub_rev)
{
	u32 soc_type, rev, ap_type;

	get_soc_type_rev(&soc_type, &rev);
	get_ap_soc_type(&ap_type);

	if (ap_type == AP807_ID) {
		*sub_rev = readl(DEVICE_ID_SUB_REV) & DEVICE_ID_SUB_REV_MASK;
		*sub_rev >>= DEVICE_ID_SUB_REV_OFFSET;
		return 0;
	}

	*sub_rev = 0;

	return -1;
}

static int get_soc_table_index(u32 *index)
{
	u32 soc_type;
	u32 rev, i, ret = 1;
	u32 ap_type, sub_rev, ap_rev;

	*index = 0;
	get_soc_type_rev(&soc_type, &rev);
	get_ap_soc_type(&ap_type);
	get_ap_soc_rev(&ap_rev);

	/* specific checks needed for 9131,9132, */
	/* since their soc+ap characteristics same as 9130*/
	if ((of_machine_is_compatible("marvell,cn9131-db"))) {
		for (i = 0; i < ARRAY_SIZE(soc_info_table) && ret != 0; i++) {
			if (strcmp(soc_info_table[i].soc_name,
				   "cn9131-A1") != 0)
				continue;
			*index = i;
			ret = 0;
		}
	} else if ((of_machine_is_compatible("marvell,cn9132-db"))) {
		for (i = 0; i < ARRAY_SIZE(soc_info_table) && ret != 0; i++) {
			if (strcmp(soc_info_table[i].soc_name,
				   "cn9132-A1") != 0)
				continue;
			*index = i;
			ret = 0;
		}
	} else	{
		for (i = 0; i < ARRAY_SIZE(soc_info_table) && ret != 0; i++) {
			if ((soc_type != soc_info_table[i].soc.module_type) ||
			    (rev != soc_info_table[i].soc.module_rev) ||
				ap_type != soc_info_table[i].ap.module_type ||
				(ap_rev != soc_info_table[i].ap.module_rev))
				continue;

			if (!get_soc_sub_rev(&sub_rev) &&
			    (sub_rev != soc_info_table[i].sub_rev))
				continue;

			*index = i;
			ret = 0;
		}
	}

	if (ret)
		pr_err("using default SoC info: %s\n",
		       soc_info_table[*index].soc_name);

	return ret;
}

static int get_soc_name(char **soc_name)
{
	u32 index;

	get_soc_table_index(&index);
	*soc_name = soc_info_table[index].soc_name;

	return 0;
}

int soc_get_ap_cp_num(void *ap_num, void *cp_num)
{
	u32 index;

	get_soc_table_index(&index);
	*((u32 *)ap_num) = soc_info_table[index].ap_num;
	*((u32 *)cp_num) = soc_info_table[index].cp_num;

	return 0;
}

/* Get SoC's Application Processor (AP) module type and revision */
static int get_ap_type_rev(u32 *type, u32 *rev)
{
	u32 index;

	get_soc_table_index(&index);
	*type = soc_info_table[index].ap.module_type;
	*rev = soc_info_table[index].ap.module_rev;

	return 0;
}

/* Get SoC's Communication Processor (CP) module type and revision */
static int get_cp_type_rev(u32 *type, u32 *rev)
{
	u32 index;

	get_soc_table_index(&index);
	*type = soc_info_table[index].cp.module_type;
	*rev = soc_info_table[index].cp.module_rev;

	return 0;
}

/* Print device's SoC name and AP & CP information */
void soc_print_device_info(void)
{
	u32 ap_num, cp_num, ap_type, ap_rev, cp_type, cp_rev;
	char *soc_name = NULL;

	soc_get_ap_cp_num(&ap_num, &cp_num);

	get_soc_name(&soc_name);
	get_ap_type_rev(&ap_type, &ap_rev);
	get_cp_type_rev(&cp_type, &cp_rev);

	if (ap_rev > 1)
		printf("SoC: %s; AP%x-B0; ", soc_name, ap_type);
	else
		printf("SoC: %s; AP%x-A%d; ", soc_name, ap_type, ap_rev);

	/* more than one cp module */
	if (cp_num > 1)
		printf("%dxCP%x-A%d\n", cp_num, cp_type, cp_rev);
	else
		printf("CP%x-A%d\n", cp_type, cp_rev);
}

/* Print System cache (LLC) status and mode */
void soc_print_system_cache_info(void)
{
	u32 val;
	int llc_en = 0, excl = 0;

	val = readl(MVEBU_LLC_BASE + LLC_CTRL_REG_OFFSET);
	if (val & LLC_EN) {
		llc_en = 1;
		if (val & LLC_EXCL_EN)
			excl = 1;
	}

	printf("LLC %s%s\n", llc_en ? "Enabled" : "Disabled",
	       excl ? " (Exclusive Mode)" : "");
}

#ifdef CONFIG_NAND_PXA3XX
/* Return NAND clock in Hz */
u32 mvebu_get_nand_clock(void __iomem *nand_flash_clk_ctrl_reg)
{
	u32 reg;

	if (!nand_flash_clk_ctrl_reg)
		return 0;

	reg = readl(nand_flash_clk_ctrl_reg);
	if (reg & NF_CLOCK_SEL_MASK)
		return 400 * CLOCK_1Mhz;
	else
		return 250 * CLOCK_1Mhz;
}

/* Select NAND in the device bus multiplexer */
void mvebu_nand_select(void __iomem *soc_dev_multiplex_reg)
{
	if (!soc_dev_multiplex_reg)
		return;

	setbits_le32(soc_dev_multiplex_reg, SOC_MUX_NAND_EN_MASK);
}
#endif

int soc_early_init_f(void)
{
#ifdef CONFIG_MVEBU_SAR
	/* Sample at reset register init */
	mvebu_sar_init();
#endif

	return 0;
}

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	u32 type, rev;

	get_soc_type_rev(&type, &rev);

	/* A8040 A1/A2 doesn't support linux kernel cpuidle feautre,
	 * so U-boot needs to update Linux bootargs according
	 * to the device id:
	 *
	 * Device	Device_ID
	 * -------------------------------
	 * A8040 A1	0x18040
	 * A8040 A2	0x28040
	 * A8040 B0	0x08045
	 *
	 * So we need to check if 16 LSB bits are 0x8040.
	 * The variable 'type', which is returned by
	 * get_soc_type_rev() holds these bits.
	 */
	if (type == A8040_DEVICE_ID)
		env_set("cpuidle", "cpuidle.off=1");

	return 0;
}
#endif
