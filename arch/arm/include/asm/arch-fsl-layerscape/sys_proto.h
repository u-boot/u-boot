/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Layerscape mirror of the i.MX <asm/mach-imx/sys_proto.h>: declares
 * the SoC-personality helpers consumed by generic drivers that work on
 * both i.MX and QorIQ/Layerscape parts (e.g. drivers/thermal/imx_tmu.c
 * for the QorIQ TMU variant).
 */

#ifndef _ASM_ARCH_FSL_LAYERSCAPE_SYS_PROTO_H
#define _ASM_ARCH_FSL_LAYERSCAPE_SYS_PROTO_H

#include <linux/types.h>

/*
 * Per LX2160A Reference Manual, Rev. 1 (10/2021):
 *  - section 1.12.1: "NXP specs max power at 105 degC junction" for
 *    commercial / embedded operating conditions.
 *  - section 28.1:   TMU "Accuracy within +/- 3 degC".
 *
 * Layerscape SoCs do not expose an OCOTP-style "CPU temp grade" fuse,
 * so the implementation returns the documented junction-temperature
 * limit from the data sheet (-40 .. 105 degC commercial range). The
 * thermal driver subtracts 10 degC for its alert threshold, which
 * comfortably clears the +/- 3 degC TMU accuracy in both directions.
 */
u32 get_cpu_temp_grade(int *minc, int *maxc);

#endif /* _ASM_ARCH_FSL_LAYERSCAPE_SYS_PROTO_H */
