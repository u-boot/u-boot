// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/salvator-x/salvator-x.c
 *     This file is Salvator-X/Salvator-XS board support.
 *
 * Copyright (C) 2015-2017 Renesas Electronics Corporation
 * Copyright (C) 2015 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <asm/io.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/renesas.h>
#include <init.h>

#define HSUSB_MSTP704		BIT(4)	/* HSUSB */

/* HSUSB block registers */
#define HSUSB_REG_LPSTS			0xE6590102
#define HSUSB_REG_LPSTS_SUSPM_NORMAL	BIT(14)
#define HSUSB_REG_UGCTRL2		0xE6590184
#define HSUSB_REG_UGCTRL2_USB0SEL	0x30
#define HSUSB_REG_UGCTRL2_USB0SEL_EHCI	0x10

int board_init(void)
{
	/* USB1 pull-up */
	setbits_le32(PFC_PUEN6, PUEN_USB1_OVC | PUEN_USB1_PWEN);

	/* Configure the HSUSB block */
	mstp_clrbits_le32(SMSTPCR7, SMSTPCR7, HSUSB_MSTP704);
	/* Choice USB0SEL */
	clrsetbits_le32(HSUSB_REG_UGCTRL2, HSUSB_REG_UGCTRL2_USB0SEL,
			HSUSB_REG_UGCTRL2_USB0SEL_EHCI);
	/* low power status */
	setbits_le16(HSUSB_REG_LPSTS, HSUSB_REG_LPSTS_SUSPM_NORMAL);

	return 0;
}

#ifdef CONFIG_MULTI_DTB_FIT
int board_fit_config_name_match(const char *name)
{
	/* PRR driver is not available yet */
	u32 cpu_type = renesas_get_cpu_type();

	if ((cpu_type == RENESAS_CPU_TYPE_R8A7795) &&
	    !strcmp(name, "r8a77951-salvator-x"))
		return 0;

	if ((cpu_type == RENESAS_CPU_TYPE_R8A7796) &&
	    !strcmp(name, "r8a77960-salvator-x"))
		return 0;

	if ((cpu_type == RENESAS_CPU_TYPE_R8A77965) &&
	    !strcmp(name, "r8a77965-salvator-x"))
		return 0;

	return -1;
}
#endif
