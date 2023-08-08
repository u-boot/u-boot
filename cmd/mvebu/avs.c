/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 * https://spdx.org/licenses
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <mach/soc.h>

#define MV_TO_AVS_VAL(mv)	((mv) * 1024 / 1189)
#define AVS_VAL_TO_MV(avs)	((avs) * 1189 / 1024)

#define MIN_AVS_VAL_MV		600
#define MAX_AVS_VAL_MV		1000

#define AVS_VAL_MASK		(0x3ff)
#define AVS_LOW_LIMIT_SHIFT	(3)
#define AVS_HIGH_LIMIT_SHIFT	(13)
#define AVS_ENABLE_MASK		BIT(0)
#define AVS_SOFT_RESET_MASK	BIT(2)
#define AVS_CFG_DEBUG_SELECT	BIT(29)

#define MVEBU_AR_RFU_BASE(ap)		(MVEBU_REGS_BASE_AP(ap) + 0x6F0000)
#define MVEBU_AVS_SRV_CTRL_REG(ap)	(MVEBU_AR_RFU_BASE(ap) + 0x8120)
#define MVEBU_AVS_CTRL_REG(ap)		(MVEBU_AR_RFU_BASE(ap) + 0x8130)
#define MVEBU_AVS_STAT_REG(ap)		(MVEBU_AR_RFU_BASE(ap) + 0x8134)

static void do_get_avs(void)
{
	int ap, ap_num, cp_num;

	soc_get_ap_cp_num(&ap_num, &cp_num);
	for (ap = 0; ap < ap_num; ap++) {
		u32 reg_val;

		/* Cause the AVS configuration to be
		 * reflected by AV status register
		 */
		writel(AVS_CFG_DEBUG_SELECT, MVEBU_AVS_SRV_CTRL_REG(ap));
		/* Read and format the result */
		reg_val = readl(MVEBU_AVS_STAT_REG(ap));
		reg_val &= AVS_VAL_MASK;
		printf("AP-%d: returned AVS value is %d mV\n",
		       ap, AVS_VAL_TO_MV(reg_val));
	}
}

static void do_set_avs(unsigned long val_mv)
{
	unsigned int avs_val = MV_TO_AVS_VAL(val_mv) & AVS_VAL_MASK;
	int ap, ap_num, cp_num;

	soc_get_ap_cp_num(&ap_num, &cp_num);
	for (ap = 0; ap < ap_num; ap++) {
		u32 reg_val;

		printf("AP-%d: setting the AVS to %ld mV (0x%x)\n",
		       ap, val_mv, avs_val);
		reg_val = avs_val << AVS_LOW_LIMIT_SHIFT;
		reg_val |= avs_val << AVS_HIGH_LIMIT_SHIFT;
		reg_val |= AVS_SOFT_RESET_MASK | AVS_ENABLE_MASK;
		writel(reg_val, MVEBU_AVS_CTRL_REG(ap));
	}
	mdelay(1000);
	do_get_avs();
}

int do_avs_cmd(cmd_tbl_t *cmdtp, int flag, int argc,
	       char * const argv[])
{
	if (strncmp(argv[1], "get", 3) == 0) {
		do_get_avs();
	} else if ((argc == 3) && (strncmp(argv[1], "set", 3) == 0)) {
		unsigned long val_mv = simple_strtoul(argv[2], NULL, 10);

		if ((val_mv < MIN_AVS_VAL_MV) || (val_mv > MAX_AVS_VAL_MV)) {
			printf("Requested value %ldmV is ",  val_mv);
			printf("out of the supported range\n");
			return 0;
		}
		do_set_avs(val_mv);
	} else {
		printf("Bad format. Use \"help avs\" for more info");
		printf("about this command\n");
	}

	return 0;
}

U_BOOT_CMD(
	avs,      3,     1,      do_avs_cmd,
	"Set/Get Adaptive Voltage Scaling (AVS) value\n",
	"[operation] [value]\n"
	"\t-operation     Either \"set\" or \"get\"\n"
	"\t-value         AVS value in mV. Valid only for \"set\" operation\n"
	"\t               Valid AVS range is 600mV - 1000mV\n"
);
