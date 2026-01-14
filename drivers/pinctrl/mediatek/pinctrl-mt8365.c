// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 BayLibre, SAS
 * Author: Julien Masson <jmasson@baylibre.com>
 */

#include <dm.h>

#include "pinctrl-mtk-common.h"

#define PIN_FIELD(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits) \
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits, 32, false)

#define PIN_FIELD_DEFAULT(_s_pin, _e_pin, _s_addr) \
	PIN_FIELD(_s_pin, _e_pin, _s_addr, 0x10, 0, 1)

#define PIN_FIELD_MODE(_s_pin, _e_pin, _s_addr) \
	PIN_FIELD(_s_pin, _e_pin, _s_addr, 0x10, 0, 3)

#define PIN_FIELD_IES_SMT(_s_pin, _e_pin, _s_addr, _s_bit) \
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, 0x10, _s_bit, 1, 32, true)

#define PIN_FIELD_DRV(_s_pin, _e_pin, _s_addr, _s_bit) \
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, 0x10, _s_bit, 4, 32, true)

#define PIN_FIELD_PUPD(_s_pin, _s_addr, _s_bit) \
	PIN_FIELD(_s_pin, _s_pin, _s_addr, 0x10, _s_bit, 1)

static const struct mtk_pin_field_calc mt8365_pin_mode_range[] = {
	PIN_FIELD_MODE(0,   9,   0x1e0),
	PIN_FIELD_MODE(10,  19,  0x1f0),
	PIN_FIELD_MODE(20,  29,  0x200),
	PIN_FIELD_MODE(30,  39,  0x210),
	PIN_FIELD_MODE(40,  49,  0x220),
	PIN_FIELD_MODE(50,  59,  0x230),
	PIN_FIELD_MODE(60,  69,  0x240),
	PIN_FIELD_MODE(70,  79,  0x250),
	PIN_FIELD_MODE(80,  89,  0x260),
	PIN_FIELD_MODE(90,  99,  0x270),
	PIN_FIELD_MODE(100, 109, 0x280),
	PIN_FIELD_MODE(110, 119, 0x290),
	PIN_FIELD_MODE(120, 129, 0x2a0),
	PIN_FIELD_MODE(130, 139, 0x2b0),
	PIN_FIELD_MODE(140, 144, 0x2c0),
};

static const struct mtk_pin_field_calc mt8365_pin_dir_range[] = {
	PIN_FIELD_DEFAULT(0, 144, 0x140),
};

static const struct mtk_pin_field_calc mt8365_pin_di_range[] = {
	PIN_FIELD_DEFAULT(0, 144, 0x0),
};

static const struct mtk_pin_field_calc mt8365_pin_do_range[] = {
	PIN_FIELD_DEFAULT(0, 144, 0xa0),
};

static const struct mtk_pin_field_calc mt8365_pin_ies_range[] = {
	PIN_FIELD_IES_SMT(0,   3,   0x410, 0),
	PIN_FIELD_IES_SMT(4,   7,   0x410, 1),
	PIN_FIELD_IES_SMT(8,   11,  0x410, 2),
	PIN_FIELD_IES_SMT(12,  15,  0x410, 3),
	PIN_FIELD_IES_SMT(16,  18,  0x410, 4),
	PIN_FIELD_IES_SMT(19,  19,  0x410, 5),
	PIN_FIELD_IES_SMT(20,  21,  0x410, 6),
	PIN_FIELD_IES_SMT(22,  22,  0x410, 7),
	PIN_FIELD_IES_SMT(23,  25,  0x410, 8),
	PIN_FIELD_IES_SMT(26,  29,  0x410, 9),
	PIN_FIELD_IES_SMT(30,  34,  0x410, 10),
	PIN_FIELD_IES_SMT(35,  40,  0x410, 11),
	PIN_FIELD_IES_SMT(41,  44,  0x410, 12),
	PIN_FIELD_IES_SMT(45,  48,  0x410, 13),
	PIN_FIELD_IES_SMT(49,  56,  0x410, 14),
	PIN_FIELD_IES_SMT(57,  58,  0x410, 15),
	PIN_FIELD_IES_SMT(59,  60,  0x410, 16),
	PIN_FIELD_IES_SMT(61,  62,  0x410, 17),
	PIN_FIELD_IES_SMT(63,  64,  0x410, 18),
	PIN_FIELD_IES_SMT(65,  70,  0x410, 19),
	PIN_FIELD_IES_SMT(71,  79,  0x410, 20),
	PIN_FIELD_IES_SMT(80,  80,  0x410, 21),
	PIN_FIELD_IES_SMT(81,  81,  0x410, 22),
	PIN_FIELD_IES_SMT(82,  82,  0x410, 23),
	PIN_FIELD_IES_SMT(83,  83,  0x410, 24),
	PIN_FIELD_IES_SMT(84,  84,  0x410, 25),
	PIN_FIELD_IES_SMT(85,  85,  0x410, 26),
	PIN_FIELD_IES_SMT(86,  86,  0x410, 27),
	PIN_FIELD_IES_SMT(87,  87,  0x410, 28),
	PIN_FIELD_IES_SMT(88,  88,  0x410, 29),
	PIN_FIELD_IES_SMT(89,  89,  0x410, 30),
	PIN_FIELD_IES_SMT(90,  90,  0x410, 31),
	PIN_FIELD_IES_SMT(91,  91,  0x420, 0),
	PIN_FIELD_IES_SMT(92,  92,  0x420, 1),
	PIN_FIELD_IES_SMT(93,  93,  0x420, 2),
	PIN_FIELD_IES_SMT(94,  94,  0x420, 3),
	PIN_FIELD_IES_SMT(95,  95,  0x420, 4),
	PIN_FIELD_IES_SMT(96,  96,  0x420, 5),
	PIN_FIELD_IES_SMT(97,  97,  0x420, 6),
	PIN_FIELD_IES_SMT(98,  98,  0x420, 7),
	PIN_FIELD_IES_SMT(99,  99,  0x420, 8),
	PIN_FIELD_IES_SMT(100, 100, 0x420, 9),
	PIN_FIELD_IES_SMT(101, 101, 0x420, 10),
	PIN_FIELD_IES_SMT(102, 102, 0x420, 11),
	PIN_FIELD_IES_SMT(103, 103, 0x420, 12),
	PIN_FIELD_IES_SMT(104, 104, 0x420, 13),
	PIN_FIELD_IES_SMT(105, 109, 0x420, 14),
	PIN_FIELD_IES_SMT(110, 113, 0x420, 15),
	PIN_FIELD_IES_SMT(114, 116, 0x420, 16),
	PIN_FIELD_IES_SMT(117, 119, 0x420, 17),
	PIN_FIELD_IES_SMT(120, 122, 0x420, 18),
	PIN_FIELD_IES_SMT(123, 125, 0x420, 19),
	PIN_FIELD_IES_SMT(126, 128, 0x420, 20),
	PIN_FIELD_IES_SMT(129, 135, 0x420, 21),
	PIN_FIELD_IES_SMT(136, 144, 0x420, 22),
};

static const struct mtk_pin_field_calc mt8365_pin_smt_range[] = {
	PIN_FIELD_IES_SMT(0,   3,   0x410, 0),
	PIN_FIELD_IES_SMT(4,   7,   0x410, 1),
	PIN_FIELD_IES_SMT(8,   11,  0x410, 2),
	PIN_FIELD_IES_SMT(12,  15,  0x410, 3),
	PIN_FIELD_IES_SMT(16,  18,  0x410, 4),
	PIN_FIELD_IES_SMT(19,  19,  0x410, 5),
	PIN_FIELD_IES_SMT(20,  21,  0x410, 6),
	PIN_FIELD_IES_SMT(22,  22,  0x410, 7),
	PIN_FIELD_IES_SMT(23,  25,  0x410, 8),
	PIN_FIELD_IES_SMT(26,  29,  0x410, 9),
	PIN_FIELD_IES_SMT(30,  34,  0x410, 10),
	PIN_FIELD_IES_SMT(35,  40,  0x410, 11),
	PIN_FIELD_IES_SMT(41,  44,  0x410, 12),
	PIN_FIELD_IES_SMT(45,  48,  0x410, 13),
	PIN_FIELD_IES_SMT(49,  56,  0x410, 14),
	PIN_FIELD_IES_SMT(57,  58,  0x410, 15),
	PIN_FIELD_IES_SMT(59,  60,  0x410, 16),
	PIN_FIELD_IES_SMT(61,  62,  0x410, 17),
	PIN_FIELD_IES_SMT(63,  64,  0x410, 18),
	PIN_FIELD_IES_SMT(65,  70,  0x410, 19),
	PIN_FIELD_IES_SMT(71,  79,  0x410, 20),
	PIN_FIELD_IES_SMT(80,  80,  0x410, 21),
	PIN_FIELD_IES_SMT(81,  81,  0x410, 22),
	PIN_FIELD_IES_SMT(82,  82,  0x410, 23),
	PIN_FIELD_IES_SMT(83,  83,  0x410, 24),
	PIN_FIELD_IES_SMT(84,  84,  0x410, 25),
	PIN_FIELD_IES_SMT(85,  85,  0x410, 26),
	PIN_FIELD_IES_SMT(86,  86,  0x410, 27),
	PIN_FIELD_IES_SMT(87,  87,  0x410, 28),
	PIN_FIELD_IES_SMT(88,  88,  0x410, 29),
	PIN_FIELD_IES_SMT(89,  89,  0x410, 30),
	PIN_FIELD_IES_SMT(90,  90,  0x410, 31),
	PIN_FIELD_IES_SMT(91,  91,  0x420, 0),
	PIN_FIELD_IES_SMT(92,  92,  0x420, 1),
	PIN_FIELD_IES_SMT(93,  93,  0x420, 2),
	PIN_FIELD_IES_SMT(94,  94,  0x420, 3),
	PIN_FIELD_IES_SMT(95,  95,  0x420, 4),
	PIN_FIELD_IES_SMT(96,  96,  0x420, 5),
	PIN_FIELD_IES_SMT(97,  97,  0x420, 6),
	PIN_FIELD_IES_SMT(98,  98,  0x420, 7),
	PIN_FIELD_IES_SMT(99,  99,  0x420, 8),
	PIN_FIELD_IES_SMT(100, 100, 0x420, 9),
	PIN_FIELD_IES_SMT(101, 101, 0x420, 10),
	PIN_FIELD_IES_SMT(102, 102, 0x420, 11),
	PIN_FIELD_IES_SMT(103, 103, 0x420, 12),
	PIN_FIELD_IES_SMT(104, 104, 0x420, 13),
	PIN_FIELD_IES_SMT(105, 109, 0x420, 14),
	PIN_FIELD_IES_SMT(110, 113, 0x420, 15),
	PIN_FIELD_IES_SMT(114, 116, 0x420, 16),
	PIN_FIELD_IES_SMT(117, 119, 0x420, 17),
	PIN_FIELD_IES_SMT(120, 122, 0x420, 18),
	PIN_FIELD_IES_SMT(123, 125, 0x420, 19),
	PIN_FIELD_IES_SMT(126, 128, 0x420, 20),
	PIN_FIELD_IES_SMT(129, 135, 0x420, 21),
	PIN_FIELD_IES_SMT(136, 144, 0x420, 22),
};

static const struct mtk_pin_field_calc mt8365_pin_pullen_range[] = {
	PIN_FIELD_DEFAULT(0, 144, 0x860),
};

static const struct mtk_pin_field_calc mt8365_pin_pullsel_range[] = {
	PIN_FIELD_DEFAULT(0, 144, 0x900),
};

static const struct mtk_pin_field_calc mt8365_pin_drv_range[] = {
	PIN_FIELD_DRV(0,   3,   0x710, 0),
	PIN_FIELD_DRV(4,   7,   0x710, 4),
	PIN_FIELD_DRV(8,   11,  0x710, 8),
	PIN_FIELD_DRV(12,  15,  0x710, 12),
	PIN_FIELD_DRV(16,  18,  0x710, 16),
	PIN_FIELD_DRV(19,  19,  0x710, 20),
	PIN_FIELD_DRV(20,  21,  0x710, 24),
	PIN_FIELD_DRV(22,  22,  0x710, 28),
	PIN_FIELD_DRV(23,  25,  0x720, 0),
	PIN_FIELD_DRV(26,  29,  0x720, 4),
	PIN_FIELD_DRV(30,  34,  0x720, 8),
	PIN_FIELD_DRV(35,  40,  0x720, 12),
	PIN_FIELD_DRV(41,  44,  0x720, 16),
	PIN_FIELD_DRV(45,  48,  0x720, 20),
	PIN_FIELD_DRV(49,  56,  0x720, 24),
	PIN_FIELD_DRV(57,  58,  0x720, 28),
	PIN_FIELD_DRV(59,  60,  0x730, 0),
	PIN_FIELD_DRV(61,  62,  0x730, 4),
	PIN_FIELD_DRV(63,  64,  0x730, 8),
	PIN_FIELD_DRV(65,  70,  0x730, 12),
	PIN_FIELD_DRV(71,  79,  0x730, 16),
	PIN_FIELD_DRV(80,  80,  0x730, 20),
	PIN_FIELD_DRV(81,  81,  0x730, 24),
	PIN_FIELD_DRV(82,  85,  0x730, 28),
	PIN_FIELD_DRV(86,  86,  0x740, 12),
	PIN_FIELD_DRV(87,  87,  0x740, 16),
	PIN_FIELD_DRV(88,  88,  0x740, 20),
	PIN_FIELD_DRV(89,  92,  0x740, 24),
	PIN_FIELD_DRV(93,  96,  0x750, 8),
	PIN_FIELD_DRV(97,  97,  0x750, 24),
	PIN_FIELD_DRV(98,  98,  0x750, 28),
	PIN_FIELD_DRV(99,  99,  0x760, 0),
	PIN_FIELD_DRV(100, 103, 0x750, 8),
	PIN_FIELD_DRV(104, 104, 0x760, 20),
	PIN_FIELD_DRV(105, 109, 0x760, 24),
	PIN_FIELD_DRV(110, 113, 0x760, 28),
	PIN_FIELD_DRV(114, 116, 0x770, 0),
	PIN_FIELD_DRV(117, 119, 0x770, 4),
	PIN_FIELD_DRV(120, 122, 0x770, 8),
	PIN_FIELD_DRV(123, 125, 0x770, 12),
	PIN_FIELD_DRV(126, 128, 0x770, 16),
	PIN_FIELD_DRV(129, 135, 0x770, 20),
	PIN_FIELD_DRV(136, 144, 0x770, 24),
};

static const struct mtk_pin_field_calc mt8365_pin_pupd_range[] = {
	/* KeyPad */
	PIN_FIELD_PUPD(22, 0x70, 2),
	PIN_FIELD_PUPD(23, 0x70, 5),
	PIN_FIELD_PUPD(24, 0x70, 8),
	PIN_FIELD_PUPD(25, 0x70, 11),
	/* MSDC2 */
	PIN_FIELD_PUPD(80, 0x70, 14),
	PIN_FIELD_PUPD(81, 0x70, 17),
	PIN_FIELD_PUPD(82, 0x70, 20),
	PIN_FIELD_PUPD(83, 0x70, 23),
	PIN_FIELD_PUPD(84, 0x70, 26),
	PIN_FIELD_PUPD(85, 0x70, 29),
	PIN_FIELD_PUPD(86, 0x80, 2),
	/* MSDC1 */
	PIN_FIELD_PUPD(87, 0x80, 5),
	PIN_FIELD_PUPD(88, 0x80, 8),
	PIN_FIELD_PUPD(89, 0x80, 11),
	PIN_FIELD_PUPD(90, 0x80, 14),
	PIN_FIELD_PUPD(91, 0x80, 17),
	PIN_FIELD_PUPD(92, 0x80, 20),
	/* MSDC0 */
	PIN_FIELD_PUPD(93, 0x80, 23),
	PIN_FIELD_PUPD(94, 0x80, 26),
	PIN_FIELD_PUPD(95, 0x80, 29),
	PIN_FIELD_PUPD(96, 0x90, 2),
	PIN_FIELD_PUPD(97, 0x90, 5),
	PIN_FIELD_PUPD(98, 0x90, 8),
	PIN_FIELD_PUPD(99, 0x90, 11),
	PIN_FIELD_PUPD(100, 0x90, 14),
	PIN_FIELD_PUPD(101, 0x90, 17),
	PIN_FIELD_PUPD(102, 0x90, 20),
	PIN_FIELD_PUPD(103, 0x90, 23),
	PIN_FIELD_PUPD(104, 0x90, 26),
	/* NFI */
	PIN_FIELD_PUPD(105, 0x90, 29),
	PIN_FIELD_PUPD(106, 0xf0, 2),
	PIN_FIELD_PUPD(107, 0xf0, 5),
	PIN_FIELD_PUPD(108, 0xf0, 8),
	PIN_FIELD_PUPD(109, 0xf0, 11),
};

static const struct mtk_pin_field_calc mt8365_pin_r1_range[] = {
	/* KeyPad */
	PIN_FIELD_PUPD(22, 0x70, 1),
	PIN_FIELD_PUPD(23, 0x70, 4),
	PIN_FIELD_PUPD(24, 0x70, 7),
	PIN_FIELD_PUPD(25, 0x70, 10),
	/* MSDC2 */
	PIN_FIELD_PUPD(80, 0x70, 13),
	PIN_FIELD_PUPD(81, 0x70, 16),
	PIN_FIELD_PUPD(82, 0x70, 19),
	PIN_FIELD_PUPD(83, 0x70, 22),
	PIN_FIELD_PUPD(84, 0x70, 25),
	PIN_FIELD_PUPD(85, 0x70, 28),
	PIN_FIELD_PUPD(86, 0x80, 1),
	/* MSDC1 */
	PIN_FIELD_PUPD(87, 0x80, 4),
	PIN_FIELD_PUPD(88, 0x80, 7),
	PIN_FIELD_PUPD(89, 0x80, 10),
	PIN_FIELD_PUPD(90, 0x80, 13),
	PIN_FIELD_PUPD(91, 0x80, 16),
	PIN_FIELD_PUPD(92, 0x80, 19),
	/* MSDC0 */
	PIN_FIELD_PUPD(93, 0x80, 22),
	PIN_FIELD_PUPD(94, 0x80, 25),
	PIN_FIELD_PUPD(95, 0x80, 28),
	PIN_FIELD_PUPD(96, 0x90, 1),
	PIN_FIELD_PUPD(97, 0x90, 4),
	PIN_FIELD_PUPD(98, 0x90, 7),
	PIN_FIELD_PUPD(99, 0x90, 10),
	PIN_FIELD_PUPD(100, 0x90, 13),
	PIN_FIELD_PUPD(101, 0x90, 16),
	PIN_FIELD_PUPD(102, 0x90, 19),
	PIN_FIELD_PUPD(103, 0x90, 22),
	PIN_FIELD_PUPD(104, 0x90, 25),
	/* N */
	PIN_FIELD_PUPD(105, 0x90, 28),
	PIN_FIELD_PUPD(106, 0xf0, 1),
	PIN_FIELD_PUPD(107, 0xf0, 4),
	PIN_FIELD_PUPD(108, 0xf0, 7),
	PIN_FIELD_PUPD(109, 0xf0, 10),
};

static const struct mtk_pin_field_calc mt8365_pin_r0_range[] = {
	/* KeyPad */
	PIN_FIELD_PUPD(22, 0x70, 0),
	PIN_FIELD_PUPD(23, 0x70, 3),
	PIN_FIELD_PUPD(24, 0x70, 6),
	PIN_FIELD_PUPD(25, 0x70, 9),
	/* MSDC2 */
	PIN_FIELD_PUPD(80, 0x70, 12),
	PIN_FIELD_PUPD(81, 0x70, 15),
	PIN_FIELD_PUPD(82, 0x70, 18),
	PIN_FIELD_PUPD(83, 0x70, 21),
	PIN_FIELD_PUPD(84, 0x70, 24),
	PIN_FIELD_PUPD(85, 0x70, 27),
	PIN_FIELD_PUPD(86, 0x80, 0),
	/* MSDC1 */
	PIN_FIELD_PUPD(87, 0x80, 3),
	PIN_FIELD_PUPD(88, 0x80, 6),
	PIN_FIELD_PUPD(89, 0x80, 9),
	PIN_FIELD_PUPD(90, 0x80, 12),
	PIN_FIELD_PUPD(91, 0x80, 15),
	PIN_FIELD_PUPD(92, 0x80, 18),
	/* MSDC0 */
	PIN_FIELD_PUPD(93, 0x80, 21),
	PIN_FIELD_PUPD(94, 0x80, 24),
	PIN_FIELD_PUPD(95, 0x80, 27),
	PIN_FIELD_PUPD(96, 0x90, 0),
	PIN_FIELD_PUPD(97, 0x90, 3),
	PIN_FIELD_PUPD(98, 0x90, 6),
	PIN_FIELD_PUPD(99, 0x90, 9),
	PIN_FIELD_PUPD(100, 0x90, 12),
	PIN_FIELD_PUPD(101, 0x90, 15),
	PIN_FIELD_PUPD(102, 0x90, 18),
	PIN_FIELD_PUPD(103, 0x90, 21),
	PIN_FIELD_PUPD(104, 0x90, 24),
	/* NFI */
	PIN_FIELD_PUPD(105, 0x90, 27),
	PIN_FIELD_PUPD(106, 0xf0, 0),
	PIN_FIELD_PUPD(107, 0xf0, 3),
	PIN_FIELD_PUPD(108, 0xf0, 6),
	PIN_FIELD_PUPD(109, 0xf0, 9),
};

static const struct mtk_pin_reg_calc mt8365_reg_cals[PINCTRL_PIN_REG_MAX] = {
	[PINCTRL_PIN_REG_MODE]    = MTK_RANGE(mt8365_pin_mode_range),
	[PINCTRL_PIN_REG_DIR]     = MTK_RANGE(mt8365_pin_dir_range),
	[PINCTRL_PIN_REG_DI]      = MTK_RANGE(mt8365_pin_di_range),
	[PINCTRL_PIN_REG_DO]      = MTK_RANGE(mt8365_pin_do_range),
	[PINCTRL_PIN_REG_IES]     = MTK_RANGE(mt8365_pin_ies_range),
	[PINCTRL_PIN_REG_SMT]     = MTK_RANGE(mt8365_pin_smt_range),
	[PINCTRL_PIN_REG_PULLSEL] = MTK_RANGE(mt8365_pin_pullsel_range),
	[PINCTRL_PIN_REG_PULLEN]  = MTK_RANGE(mt8365_pin_pullen_range),
	[PINCTRL_PIN_REG_DRV]     = MTK_RANGE(mt8365_pin_drv_range),
	[PINCTRL_PIN_REG_PUPD]    = MTK_RANGE(mt8365_pin_pupd_range),
	[PINCTRL_PIN_REG_R1]      = MTK_RANGE(mt8365_pin_r1_range),
	[PINCTRL_PIN_REG_R0]      = MTK_RANGE(mt8365_pin_r0_range),
};

static const struct mtk_pin_desc mt8365_pins[] = {
	MTK_PIN(0,   "GPIO0",          DRV_GRP4),
	MTK_PIN(1,   "GPIO1",          DRV_GRP4),
	MTK_PIN(2,   "GPIO2",          DRV_GRP4),
	MTK_PIN(3,   "GPIO3",          DRV_GRP4),
	MTK_PIN(4,   "GPIO4",          DRV_GRP4),
	MTK_PIN(5,   "GPIO5",          DRV_GRP4),
	MTK_PIN(6,   "GPIO6",          DRV_GRP4),
	MTK_PIN(7,   "GPIO7",          DRV_GRP4),
	MTK_PIN(8,   "GPIO8",          DRV_GRP4),
	MTK_PIN(9,   "GPIO9",          DRV_GRP4),
	MTK_PIN(10,  "GPIO10",         DRV_GRP4),
	MTK_PIN(11,  "GPIO11",         DRV_GRP4),
	MTK_PIN(12,  "GPIO12",         DRV_GRP4),
	MTK_PIN(13,  "GPIO13",         DRV_GRP4),
	MTK_PIN(14,  "GPIO14",         DRV_GRP4),
	MTK_PIN(15,  "GPIO15",         DRV_GRP4),
	MTK_PIN(16,  "GPIO16",         DRV_GRP4),
	MTK_PIN(17,  "GPIO17",         DRV_GRP4),
	MTK_PIN(18,  "GPIO18",         DRV_GRP4),
	MTK_PIN(19,  "DISP_PWM",       DRV_GRP4),
	MTK_PIN(20,  "LCM_RST",        DRV_GRP4),
	MTK_PIN(21,  "DSI_TE",         DRV_GRP4),
	MTK_PIN(22,  "KPROW0",         DRV_GRP4),
	MTK_PIN(23,  "KPROW1",         DRV_GRP4),
	MTK_PIN(24,  "KPCOL0",         DRV_GRP4),
	MTK_PIN(25,  "KPCOL1",         DRV_GRP4),
	MTK_PIN(26,  "SPI_CS",         DRV_GRP4),
	MTK_PIN(27,  "SPI_CK",         DRV_GRP4),
	MTK_PIN(28,  "SPI_MI",         DRV_GRP4),
	MTK_PIN(29,  "SPI_MO",         DRV_GRP4),
	MTK_PIN(30,  "JTMS",           DRV_GRP4),
	MTK_PIN(31,  "JTCK",           DRV_GRP4),
	MTK_PIN(32,  "JTDI",           DRV_GRP4),
	MTK_PIN(33,  "JTDO",           DRV_GRP4),
	MTK_PIN(34,  "JTRST",          DRV_GRP4),
	MTK_PIN(35,  "URXD0",          DRV_GRP4),
	MTK_PIN(36,  "UTXD0",          DRV_GRP4),
	MTK_PIN(37,  "URXD1",          DRV_GRP4),
	MTK_PIN(38,  "UTXD1",          DRV_GRP4),
	MTK_PIN(39,  "URXD2",          DRV_GRP4),
	MTK_PIN(40,  "UTXD2",          DRV_GRP4),
	MTK_PIN(41,  "PWRAP_SPI0_MI",  DRV_GRP4),
	MTK_PIN(42,  "PWRAP_SPI0_MO",  DRV_GRP4),
	MTK_PIN(43,  "PWRAP_SPI0_CK",  DRV_GRP4),
	MTK_PIN(44,  "PWRAP_SPI0_CSN", DRV_GRP4),
	MTK_PIN(45,  "RTC32K_CK",      DRV_GRP4),
	MTK_PIN(46,  "WATCHDOG",       DRV_GRP4),
	MTK_PIN(47,  "SRCLKENA0",      DRV_GRP4),
	MTK_PIN(48,  "SRCLKENA1",      DRV_GRP4),
	MTK_PIN(49,  "AUD_CLK_MOSI",   DRV_GRP4),
	MTK_PIN(50,  "AUD_SYNC_MOSI",  DRV_GRP4),
	MTK_PIN(51,  "AUD_DAT_MOSI0",  DRV_GRP4),
	MTK_PIN(52,  "AUD_DAT_MOSI1",  DRV_GRP4),
	MTK_PIN(53,  "AUD_CLK_MISO",   DRV_GRP4),
	MTK_PIN(54,  "AUD_SYNC_MISO",  DRV_GRP4),
	MTK_PIN(55,  "AUD_DAT_MISO0",  DRV_GRP4),
	MTK_PIN(56,  "AUD_DAT_MISO1",  DRV_GRP4),
	MTK_PIN(57,  "SDA0",           DRV_GRP4),
	MTK_PIN(58,  "SCL0",           DRV_GRP4),
	MTK_PIN(59,  "SDA1",           DRV_GRP4),
	MTK_PIN(60,  "SCL1",           DRV_GRP4),
	MTK_PIN(61,  "SDA2",           DRV_GRP4),
	MTK_PIN(62,  "SCL2",           DRV_GRP4),
	MTK_PIN(63,  "SDA3",           DRV_GRP4),
	MTK_PIN(64,  "SCL3",           DRV_GRP4),
	MTK_PIN(65,  "CMMCLK0",        DRV_GRP4),
	MTK_PIN(66,  "CMMCLK1",        DRV_GRP4),
	MTK_PIN(67,  "CMPCLK",         DRV_GRP4),
	MTK_PIN(68,  "CMDAT0",         DRV_GRP4),
	MTK_PIN(69,  "CMDAT1",         DRV_GRP4),
	MTK_PIN(70,  "CMDAT2",         DRV_GRP4),
	MTK_PIN(71,  "CMDAT3",         DRV_GRP4),
	MTK_PIN(72,  "CMDAT4",         DRV_GRP4),
	MTK_PIN(73,  "CMDAT5",         DRV_GRP4),
	MTK_PIN(74,  "CMDAT6",         DRV_GRP4),
	MTK_PIN(75,  "CMDAT7",         DRV_GRP4),
	MTK_PIN(76,  "CMDAT8",         DRV_GRP4),
	MTK_PIN(77,  "CMDAT9",         DRV_GRP4),
	MTK_PIN(78,  "CMHSYNC",        DRV_GRP4),
	MTK_PIN(79,  "CMVSYNC",        DRV_GRP4),
	MTK_PIN(80,  "MSDC2_CMD",      DRV_GRP4),
	MTK_PIN(81,  "MSDC2_CLK",      DRV_GRP4),
	MTK_PIN(82,  "MSDC2_DAT0",     DRV_GRP4),
	MTK_PIN(83,  "MSDC2_DAT1",     DRV_GRP4),
	MTK_PIN(84,  "MSDC2_DAT2",     DRV_GRP4),
	MTK_PIN(85,  "MSDC2_DAT3",     DRV_GRP4),
	MTK_PIN(86,  "MSDC2_DSL",      DRV_GRP4),
	MTK_PIN(87,  "MSDC1_CMD",      DRV_GRP4),
	MTK_PIN(88,  "MSDC1_CLK",      DRV_GRP4),
	MTK_PIN(89,  "MSDC1_DAT0",     DRV_GRP4),
	MTK_PIN(90,  "MSDC1_DAT1",     DRV_GRP4),
	MTK_PIN(91,  "MSDC1_DAT2",     DRV_GRP4),
	MTK_PIN(92,  "MSDC1_DAT3",     DRV_GRP4),
	MTK_PIN(93,  "MSDC0_DAT7",     DRV_GRP4),
	MTK_PIN(94,  "MSDC0_DAT6",     DRV_GRP4),
	MTK_PIN(95,  "MSDC0_DAT5",     DRV_GRP4),
	MTK_PIN(96,  "MSDC0_DAT4",     DRV_GRP4),
	MTK_PIN(97,  "MSDC0_RSTB",     DRV_GRP4),
	MTK_PIN(98,  "MSDC0_CMD",      DRV_GRP4),
	MTK_PIN(99,  "MSDC0_CLK",      DRV_GRP4),
	MTK_PIN(100, "MSDC0_DAT3",     DRV_GRP4),
	MTK_PIN(101, "MSDC0_DAT2",     DRV_GRP4),
	MTK_PIN(102, "MSDC0_DAT1",     DRV_GRP4),
	MTK_PIN(103, "MSDC0_DAT0",     DRV_GRP4),
	MTK_PIN(104, "MSDC0_DSL",      DRV_GRP4),
	MTK_PIN(105, "NCLE",           DRV_GRP4),
	MTK_PIN(106, "NCEB1",          DRV_GRP4),
	MTK_PIN(107, "NCEB0",          DRV_GRP4),
	MTK_PIN(108, "NALE",           DRV_GRP4),
	MTK_PIN(109, "NRNB",           DRV_GRP4),
	MTK_PIN(110, "PCM_CLK",        DRV_GRP4),
	MTK_PIN(111, "PCM_SYNC",       DRV_GRP4),
	MTK_PIN(112, "PCM_RX",         DRV_GRP4),
	MTK_PIN(113, "PCM_TX",         DRV_GRP4),
	MTK_PIN(114, "I2S_DATA_IN",    DRV_GRP4),
	MTK_PIN(115, "I2S_LRCK",       DRV_GRP4),
	MTK_PIN(116, "I2S_BCK",        DRV_GRP4),
	MTK_PIN(117, "DMIC0_CLK",      DRV_GRP4),
	MTK_PIN(118, "DMIC0_DAT0",     DRV_GRP4),
	MTK_PIN(119, "DMIC0_DAT1",     DRV_GRP4),
	MTK_PIN(120, "DMIC1_CLK",      DRV_GRP4),
	MTK_PIN(121, "DMIC1_DAT0",     DRV_GRP4),
	MTK_PIN(122, "DMIC1_DAT1",     DRV_GRP4),
	MTK_PIN(123, "DMIC2_CLK",      DRV_GRP4),
	MTK_PIN(124, "DMIC2_DAT0",     DRV_GRP4),
	MTK_PIN(125, "DMIC2_DAT1",     DRV_GRP4),
	MTK_PIN(126, "DMIC3_CLK",      DRV_GRP4),
	MTK_PIN(127, "DMIC3_DAT0",     DRV_GRP4),
	MTK_PIN(128, "DMIC3_DAT1",     DRV_GRP4),
	MTK_PIN(129, "TDM_TX_BCK",     DRV_GRP4),
	MTK_PIN(130, "TDM_TX_LRCK",    DRV_GRP4),
	MTK_PIN(131, "TDM_TX_MCK",     DRV_GRP4),
	MTK_PIN(132, "TDM_TX_DATA0",   DRV_GRP4),
	MTK_PIN(133, "TDM_TX_DATA1",   DRV_GRP4),
	MTK_PIN(134, "TDM_TX_DATA2",   DRV_GRP4),
	MTK_PIN(135, "TDM_TX_DATA3",   DRV_GRP4),
	MTK_PIN(136, "CONN_TOP_CLK",   DRV_GRP4),
	MTK_PIN(137, "CONN_TOP_DATA",  DRV_GRP4),
	MTK_PIN(138, "CONN_HRST_B",    DRV_GRP4),
	MTK_PIN(139, "CONN_WB_PTA",    DRV_GRP4),
	MTK_PIN(140, "CONN_BT_CLK",    DRV_GRP4),
	MTK_PIN(141, "CONN_BT_DATA",   DRV_GRP4),
	MTK_PIN(142, "CONN_WF_CTRL0",  DRV_GRP4),
	MTK_PIN(143, "CONN_WF_CTRL1",  DRV_GRP4),
	MTK_PIN(144, "CONN_WF_CTRL2",  DRV_GRP4),
};

/* DISP PWM */
static const char *const mt8365_disp_pwm_groups[] = { "disp_pwm" };

static const int mt8365_disp_pwm_pins[] = { 19 };
static const int mt8365_disp_pwm_funcs[] = { 1 };

/* DPI */
static const int mt8365_dpi_enable_pins[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static const int mt8365_dpi_enable_funcs[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static const int mt8365_dpi_sleep_pins[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static const int mt8365_dpi_sleep_funcs[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const char *const mt8365_dpi_groups[] = { "dpi_enable", "dpi_sleep" };

/* I2C */
static const int mt8365_i2c0_pins[] = { 57, 58, };
static const int mt8365_i2c0_funcs[] = { 1, 1, };
static const int mt8365_i2c1_pins[] = { 59, 60, };
static const int mt8365_i2c1_funcs[] = { 1, 1, };
static const int mt8365_i2c2_pins[] = { 61, 62, };
static const int mt8365_i2c2_funcs[] = { 1, 1, };
static const int mt8365_i2c3_pins[] = { 63, 64, };
static const int mt8365_i2c3_funcs[] = { 1, 1, };

static const char *const mt8365_i2c_groups[] = { "i2c0", "i2c1", "i2c2", "i2c3" };

static const struct mtk_group_desc mt8365_groups[] = {
	PINCTRL_PIN_GROUP("disp_pwm",	mt8365_disp_pwm),
	PINCTRL_PIN_GROUP("dpi_enable", mt8365_dpi_enable),
	PINCTRL_PIN_GROUP("dpi_sleep",	mt8365_dpi_sleep),
	PINCTRL_PIN_GROUP("i2c0",       mt8365_i2c0),
	PINCTRL_PIN_GROUP("i2c1",	mt8365_i2c1),
	PINCTRL_PIN_GROUP("i2c2",       mt8365_i2c2),
	PINCTRL_PIN_GROUP("i2c3",       mt8365_i2c3),
};

static const struct mtk_function_desc mt8365_functions[] = {
	{"disp", mt8365_disp_pwm_groups, ARRAY_SIZE(mt8365_disp_pwm_groups)},
	{"dpi", mt8365_dpi_groups, ARRAY_SIZE(mt8365_dpi_groups)},
	{"i2c", mt8365_i2c_groups, ARRAY_SIZE(mt8365_i2c_groups)},
};

static const struct mtk_pinctrl_soc mt8365_data = {
	.name      = "mt8365_pinctrl",
	.reg_cal   = mt8365_reg_cals,
	.pins      = mt8365_pins,
	.npins     = ARRAY_SIZE(mt8365_pins),
	.grps      = mt8365_groups,
	.ngrps     = ARRAY_SIZE(mt8365_groups),
	.funcs     = mt8365_functions,
	.nfuncs    = ARRAY_SIZE(mt8365_functions),
	.gpio_mode = 0,
	.rev       = MTK_PINCTRL_V1,
};

static int mtk_pinctrl_mt8365_probe(struct udevice *dev)
{
	return mtk_pinctrl_common_probe(dev, &mt8365_data);
}

static const struct udevice_id mt8365_pctrl_match[] = {
	{ .compatible = "mediatek,mt8365-pinctrl" },
	{ }
};

U_BOOT_DRIVER(mt8365_pinctrl) = {
	.name      = "mt8365_pinctrl",
	.id        = UCLASS_PINCTRL,
	.of_match  = mt8365_pctrl_match,
	.ops       = &mtk_pinctrl_ops,
	.probe     = mtk_pinctrl_mt8365_probe,
	.priv_auto = sizeof(struct mtk_pinctrl_priv),
};
