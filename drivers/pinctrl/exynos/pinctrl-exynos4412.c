// SPDX-License-Identifier: GPL-2.0+
/*
 * Exynos4412 pinctrl driver.
 * refer to pinctrl_exynos7420 implementation
 *
 * Copyright (C) 2018 Xinlu Wang <wangkartx@gmail.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <dm/pinctrl.h>
#include <dm/root.h>
#include <fdtdec.h>
#include <asm/arch/pinmux.h>
#include "pinctrl-exynos.h"

static struct pinctrl_ops exynos4412_pinctrl_ops = {
    .set_state  = exynos_pinctrl_set_state,
};

static const struct samsung_pin_bank_data exynos4412_pin_bank0[] = {
    EXYNOS_PIN_BANK(8, 0x180, "gpf0"),
    EXYNOS_PIN_BANK(8, 0x1A0, "gpf1"),
    EXYNOS_PIN_BANK(8, 0x1C0, "gpf2"),
    EXYNOS_PIN_BANK(6, 0x1E0, "gpf3"),
};

const struct samsung_pin_ctrl exynos4412_pin_ctrl[] = {
    {
        /* pin-controller instance LCD data */
        .pin_banks  = exynos4412_pin_bank0,
        .nr_banks   = ARRAY_SIZE(exynos4412_pin_bank0),
    },
};

static const struct udevice_id exynos4412_pinctrl_ids[] = {
    { .compatible = "samsung,exynos4412-pinctrl",
        .data = (ulong)exynos4412_pin_ctrl },
    { }
};

U_BOOT_DRIVER(pinctrl_exynos4412) = {
    .name       = "pinctrl_exynos4412",
    .id         = UCLASS_PINCTRL,
    .of_match   = exynos4412_pinctrl_ids,
    .priv_auto_alloc_size = sizeof(struct exynos_pinctrl_priv),
    .ops        = &exynos4412_pinctrl_ops,
    .probe      = exynos_pinctrl_probe,
    .flags      = DM_FLAG_PRE_RELOC
};
