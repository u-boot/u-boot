// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>

u32 get_cpu_rev(void)
{
	return (MXC_CPU_IMX8ULP << 12) | CHIP_REV_1_0;
}

enum bt_mode get_boot_mode(void)
{
	u32 bt0_cfg = 0;

	bt0_cfg = readl(CMC0_RBASE + 0x80);
	bt0_cfg &= (BT0CFG_LPBOOT_MASK | BT0CFG_DUALBOOT_MASK);

	if (!(bt0_cfg & BT0CFG_LPBOOT_MASK)) {
		/* No low power boot */
		if (bt0_cfg & BT0CFG_DUALBOOT_MASK)
			return DUAL_BOOT;
		else
			return SINGLE_BOOT;
	}

	return LOW_POWER_BOOT;
}

#define CMC_SRS_TAMPER                    BIT(31)
#define CMC_SRS_SECURITY                  BIT(30)
#define CMC_SRS_TZWDG                     BIT(29)
#define CMC_SRS_JTAG_RST                  BIT(28)
#define CMC_SRS_CORE1                     BIT(16)
#define CMC_SRS_LOCKUP                    BIT(15)
#define CMC_SRS_SW                        BIT(14)
#define CMC_SRS_WDG                       BIT(13)
#define CMC_SRS_PIN_RESET                 BIT(8)
#define CMC_SRS_WARM                      BIT(4)
#define CMC_SRS_HVD                       BIT(3)
#define CMC_SRS_LVD                       BIT(2)
#define CMC_SRS_POR                       BIT(1)
#define CMC_SRS_WUP                       BIT(0)

static u32 reset_cause = -1;

static char *get_reset_cause(char *ret)
{
	u32 cause1, cause = 0, srs = 0;
	void __iomem *reg_ssrs = (void __iomem *)(SRC_BASE_ADDR + 0x88);
	void __iomem *reg_srs = (void __iomem *)(SRC_BASE_ADDR + 0x80);

	if (!ret)
		return "null";

	srs = readl(reg_srs);
	cause1 = readl(reg_ssrs);

	reset_cause = cause1;

	cause = cause1 & (CMC_SRS_POR | CMC_SRS_WUP | CMC_SRS_WARM);

	switch (cause) {
	case CMC_SRS_POR:
		sprintf(ret, "%s", "POR");
		break;
	case CMC_SRS_WUP:
		sprintf(ret, "%s", "WUP");
		break;
	case CMC_SRS_WARM:
		cause = cause1 & (CMC_SRS_WDG | CMC_SRS_SW |
			CMC_SRS_JTAG_RST);
		switch (cause) {
		case CMC_SRS_WDG:
			sprintf(ret, "%s", "WARM-WDG");
			break;
		case CMC_SRS_SW:
			sprintf(ret, "%s", "WARM-SW");
			break;
		case CMC_SRS_JTAG_RST:
			sprintf(ret, "%s", "WARM-JTAG");
			break;
		default:
			sprintf(ret, "%s", "WARM-UNKN");
			break;
		}
		break;
	default:
		sprintf(ret, "%s-%X", "UNKN", cause1);
		break;
	}

	debug("[%X] SRS[%X] %X - ", cause1, srs, srs ^ cause1);
	return ret;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
const char *get_imx_type(u32 imxtype)
{
	return "8ULP";
}

int print_cpuinfo(void)
{
	u32 cpurev;
	char cause[18];

	cpurev = get_cpu_rev();

	printf("CPU:   Freescale i.MX%s rev%d.%d at %d MHz\n",
	       get_imx_type((cpurev & 0xFF000) >> 12),
	       (cpurev & 0x000F0) >> 4, (cpurev & 0x0000F) >> 0,
	       mxc_get_clock(MXC_ARM_CLK) / 1000000);

	printf("Reset cause: %s\n", get_reset_cause(cause));

	printf("Boot mode: ");
	switch (get_boot_mode()) {
	case LOW_POWER_BOOT:
		printf("Low power boot\n");
		break;
	case DUAL_BOOT:
		printf("Dual boot\n");
		break;
	case SINGLE_BOOT:
	default:
		printf("Single boot\n");
		break;
	}

	return 0;
}
#endif
