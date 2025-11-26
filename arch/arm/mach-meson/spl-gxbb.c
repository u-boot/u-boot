// SPDX-License-Identifier: GPL-2.0+
/*
 * Portions Copyright (C) 2015, Amlogic, Inc. All rights reserved.
 * Copyright (C) 2023, Ferass El Hafidi <funderscore@postmarketos.org>
 */
#include <image.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/gx.h>
#include <linux/delay.h>

#if CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS)
/*
 * This is only needed for GXBB because on GXL SCP firmware loading
 * has been moved to BL31.
 */
inline void send_scp(void *addr, size_t size, const uint8_t *sha2,
	uint32_t sha2_length)
{
	int i;

	puts("Trying to send the SCP firmware\n");
	writel(size, GX_MB_SRAM_BASE);

	udelay(500);

	writel(GX_MB_CMD_DATA_LEN, GX_SEC_HIU_MAILBOX_SET_0 + 3 * 3 * 4);
	while (readl(GX_SEC_HIU_MAILBOX_SET_0 + 3 * 3 * 4))
		;
	udelay(500);

	memcpy((void *)GX_MB_SRAM_BASE, (const void *)sha2, sha2_length);
	writel(GX_MB_CMD_SHA, GX_SEC_HIU_MAILBOX_SET_0 + 3 * 3 * 4);
	while (readl(GX_SEC_HIU_MAILBOX_STAT_0 + 3 * 3 * 4))
		;
	udelay(500);

	for (i = 0; i < size; i += 1024) {
		if (size >= i + 1024)
			memcpy((void *)GX_MB_SRAM_BASE,
			       (const void *)(unsigned long)(addr + i), 1024);
		else if (size > i)
			memcpy((void *)GX_MB_SRAM_BASE,
			       (const void *)(unsigned long)(addr + i), (size - i));

		writel(GX_MB_CMD_DATA, GX_SEC_HIU_MAILBOX_SET_0 + 3 * 3 * 4);
		while (readl(GX_SEC_HIU_MAILBOX_STAT_0 + 3 * 3 * 4))
			;
	}
	writel(GX_MB_CMD_OP_SHA, GX_SEC_HIU_MAILBOX_SET_0 + 3 * 3 * 4);

	while (readl(GX_SEC_HIU_MAILBOX_STAT_0 + 3 * 3 * 4))
		;
	udelay(500);

	/* We transferred all of the SCP firmware. Running it */
	writel(GX_MB_CMD_END, GX_SEC_HIU_MAILBOX_SET_0 + 3 * 3 * 4);
}

void board_fit_image_post_process(const void *fit, int node, void **image, size_t *size)
{
	const char *name = fit_get_name(fit, node, NULL);
	int noffset = 0, value_len;
	u8 *value;

	if (strcmp("scp", name) && strcmp("bl301", name))
		return;

	fdt_for_each_subnode(noffset, fit, node) {
		if (strncmp(fit_get_name(fit, noffset, NULL),
			    FIT_HASH_NODENAME, strlen(FIT_HASH_NODENAME)))
			continue;

		if (fit_image_hash_get_value(fit, noffset, &value, &value_len))
			continue;

		/* Send the SCP firmware to the SCP */
		send_scp(*image, *size, value, value_len);
		break;
	}
}
#endif

void meson_power_init(void)
{
	/* TODO: Support more voltages */

	/* Init PWM B */
	clrsetbits_32(GX_PWM_MISC_REG_AB, 0x7f << 16, (1 << 23) | (1 << 1));

	/* Set voltage */
	if (CONFIG_IS_ENABLED(MESON_GX_VCCK_1120MV))
		writel(0x02001a, GX_PWM_PWM_B);
	else if (CONFIG_IS_ENABLED(MESON_GX_VCCK_1100MV))
		writel(0x040018, GX_PWM_PWM_B);
	else if (CONFIG_IS_ENABLED(MESON_GX_VCCK_1000MV))
		writel(0x0e000e, GX_PWM_PWM_B);

	clrbits_32(GX_PIN_MUX_REG7, 1 << 22);
	clrsetbits_32(GX_PIN_MUX_REG3, 1 << 22, 1 << 21);

	/* Init PWM D */
	clrsetbits_32(GX_PWM_MISC_REG_CD, 0x7f << 16, (1 << 23) | (1 << 1));

	/* Set voltage */
	if (CONFIG_IS_ENABLED(MESON_GX_VDDEE_1100MV))
		writel(0x040018, GX_PWM_PWM_B);
	else if (CONFIG_IS_ENABLED(MESON_GX_VDDEE_1000MV))
		writel(0x0e000e, GX_PWM_PWM_B);

	clrbits_32(GX_PIN_MUX_REG7, 1 << 23);
	setbits_32(GX_PIN_MUX_REG3, 1 << 20);
}
