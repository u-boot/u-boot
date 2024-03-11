// SPDX-License-Identifier: GPL-2.0-only

#include <dm.h>
#include "pinctrl-mtk-common.h"


#define PIN_FIELD(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 32, false)

#define PINS_FIELD(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 32, true)

static const struct mtk_pin_field_calc mt6735_pin_mode_range[] = {
	PIN_FIELD(0, 169, 0x300, 0x10, 0, 3),
	PIN_FIELD(170, 170, 0x410, 0x10, 0, 3),
	PIN_FIELD(171, 171, 0x410, 0x10, 3, 3),
	PIN_FIELD(172, 172, 0x410, 0x10, 6, 3),
	PIN_FIELD(173, 173, 0x410, 0x10, 9, 3),
	PIN_FIELD(174, 174, 0x410, 0x10, 12, 3),
	PIN_FIELD(175, 175, 0x410, 0x10, 16, 3),
	PIN_FIELD(176, 176, 0x410, 0x10, 19, 3),
	PIN_FIELD(177, 177, 0x410, 0x10, 22, 3),
	PIN_FIELD(178, 178, 0x410, 0x10, 25, 3),
	PIN_FIELD(179, 179, 0x410, 0x10, 28, 3),
	PIN_FIELD(180, 180, 0x420, 0x10, 0, 3),
	PIN_FIELD(181, 181, 0x420, 0x10, 3, 3),
	PIN_FIELD(182, 182, 0x420, 0x10, 9, 3),
	PIN_FIELD(183, 183, 0x420, 0x10, 12, 3),
	PIN_FIELD(184, 184, 0x420, 0x10, 12, 3),
	PIN_FIELD(185, 185, 0x420, 0x10, 16, 3),
	PIN_FIELD(186, 186, 0x420, 0x10, 19, 3),
	PIN_FIELD(187, 187, 0x420, 0x10, 22, 3),
	PIN_FIELD(188, 188, 0x420, 0x10, 25, 3),
	PIN_FIELD(189, 189, 0x420, 0x10, 28, 3),
	PIN_FIELD(190, 203, 0x430, 0x10, 0, 3),
};

static const struct mtk_pin_field_calc mt6735_pin_dir_range[] = {
	PIN_FIELD(0, 203, 0x0, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_di_range[] = {
	PIN_FIELD(0, 203, 0x200, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_do_range[] = {
	PIN_FIELD(0, 203, 0x100, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_ies_range[] = {
	PINS_FIELD(0, 4, 0x900, 0x10, 10, 1),
	PINS_FIELD(5, 8, 0xa00, 0x10, 3, 1),
	PINS_FIELD(9, 12, 0xa00, 0x10, 6, 1),
	PINS_FIELD(13, 18, 0x800, 0x10, 0, 1),
	PINS_FIELD(19, 21, 0x800, 0x10, 1, 1),
	PINS_FIELD(42, 44, 0x900, 0x10, 0, 1),
	PINS_FIELD(45, 46, 0x900, 0x10, 1, 1),
	PINS_FIELD(47, 48, 0x900, 0x10, 2, 1),
	PINS_FIELD(49, 50, 0x900, 0x10, 3, 1),
	PINS_FIELD(51, 52, 0x900, 0x10, 4, 1),
	PINS_FIELD(53, 54, 0x900, 0x10, 5, 1),
	PIN_FIELD(55, 55, 0x900, 0x10, 6, 1),
	PIN_FIELD(56, 56, 0x900, 0x10, 7, 1),
	PINS_FIELD(57, 60, 0x900, 0x10, 8, 1),
	PINS_FIELD(61, 64, 0x900, 0x10, 9, 1),
	PINS_FIELD(65, 68, 0xa00, 0x10, 0, 1),
	PIN_FIELD(69, 69,  0xa00, 0x10, 1, 1),
	PINS_FIELD(70, 73, 0xa00, 0x10, 2, 1),
	PINS_FIELD(74, 77, 0xa00, 0x10, 4, 1),
	PINS_FIELD(78, 80, 0xa00, 0x10, 5, 1),
	PINS_FIELD(81, 86, 0xa00, 0x10, 7, 1),
	PINS_FIELD(87, 103, 0xa00, 0x10, 8, 1),
	PINS_FIELD(104, 114, 0xa00, 0x10, 9, 1),
	PINS_FIELD(118, 136, 0xb00, 0x10, 0, 1),
	PIN_FIELD(137, 137, 0xb00, 0x10, 1, 1),
	PINS_FIELD(138, 142, 0xb00, 0x10, 2, 1),
	PINS_FIELD(143, 145, 0xb00, 0x10, 3, 1),
	PINS_FIELD(146, 147, 0xb00, 0x10, 4, 1),
	PIN_FIELD(148, 148, 0xb00, 0x10, 5, 1),
	PIN_FIELD(149, 149,  0xb00, 0x10, 1, 1),
	PINS_FIELD(160, 162,  0xc00, 0x10, 0, 1),
	PINS_FIELD(163, 165, 0xc00, 0x10, 1, 1),
	PIN_FIELD(166, 166, 0xc00, 0x10, 2, 1),
	PIN_FIELD(167, 167,  0xc00, 0x10, 3, 1),
	PINS_FIELD(168, 171,  0xc00, 0x10, 4, 1),
	PIN_FIELD(172, 172, 0xd00, 0x10, 0, 1),
	PIN_FIELD(173, 173, 0xd00, 0x10, 1, 1),
	PIN_FIELD(174, 174,  0xd00, 0x10, 2, 1),
	PINS_FIELD(175, 182, 0xd00, 0x10, 3, 1),
	PIN_FIELD(183, 183, 0xd00, 0x10, 4, 1),
	PINS_FIELD(184, 185, 0xd00, 0x10, 5, 1),
	PINS_FIELD(186, 189, 0xd00, 0x10, 6, 1),
	PIN_FIELD(198, 198, 0x800, 0x10, 2, 1),
	PIN_FIELD(199, 199,  0x800, 0x10, 3, 1),
	PIN_FIELD(200, 203,  0x800, 0x10, 4, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_smt_range[] = {
	PINS_FIELD(0, 4, 0x910, 0x10, 10, 1),
	PINS_FIELD(5, 8, 0xa10, 0x10, 3, 1),
	PINS_FIELD(9, 12, 0xa10, 0x10, 6, 1),
	PINS_FIELD(13, 18, 0x810, 0x10, 0, 1),
	PINS_FIELD(19, 21, 0x810, 0x10, 1, 1),
	PINS_FIELD(42, 44, 0x910, 0x10, 0, 1),
	PINS_FIELD(45, 46, 0x910, 0x10, 1, 1),
	PINS_FIELD(47, 48, 0x910, 0x10, 2, 1),
	PINS_FIELD(49, 50, 0x910, 0x10, 3, 1),
	PINS_FIELD(51, 52, 0x910, 0x10, 4, 1),
	PINS_FIELD(53, 54, 0x910, 0x10, 5, 1),
	PIN_FIELD(55, 55, 0x910, 0x10, 6, 1),
	PIN_FIELD(56, 56, 0x910, 0x10, 7, 1),
	PINS_FIELD(57, 60, 0x910, 0x10, 8, 1),
	PINS_FIELD(61, 64, 0x910, 0x10, 9, 1),
	PINS_FIELD(65, 68, 0xa10, 0x10, 0, 1),
	PIN_FIELD(69, 69,  0xa10, 0x10, 1, 1),
	PINS_FIELD(70, 73, 0xa10, 0x10, 2, 1),
	PINS_FIELD(74, 77, 0xa10, 0x10, 4, 1),
	PINS_FIELD(78, 80, 0xa10, 0x10, 5, 1),
	PINS_FIELD(81, 86, 0xa10, 0x10, 7, 1),
	PINS_FIELD(87, 103, 0xa10, 0x10, 8, 1),
	PINS_FIELD(104, 114, 0xa10, 0x10, 9, 1),
	PINS_FIELD(118, 136, 0xb10, 0x10, 0, 1),
	PIN_FIELD(137, 137, 0xb10, 0x10, 1, 1),
	PINS_FIELD(138, 142, 0xb10, 0x10, 2, 1),
	PINS_FIELD(143, 145, 0xb10, 0x10, 3, 1),
	PINS_FIELD(146, 147, 0xb10, 0x10, 4, 1),
	PIN_FIELD(148, 148, 0xb10, 0x10, 5, 1),
	PIN_FIELD(149, 149,  0xb10, 0x10, 1, 1),
	PINS_FIELD(160, 162,  0xc10, 0x10, 0, 1),
	PINS_FIELD(163, 165, 0xc10, 0x10, 1, 1),
	PIN_FIELD(166, 166, 0xc10, 0x10, 2, 1),
	PIN_FIELD(167, 167,  0xc10, 0x10, 3, 1),
	PINS_FIELD(168, 171,  0xc10, 0x10, 4, 1),
	PIN_FIELD(172, 172, 0xd10, 0x10, 0, 1),
	PIN_FIELD(173, 173, 0xd10, 0x10, 1, 1),
	PIN_FIELD(174, 174,  0xd10, 0x10, 2, 1),
	PINS_FIELD(175, 182, 0xd10, 0x10, 3, 1),
	PIN_FIELD(183, 183, 0xd10, 0x10, 4, 1),
	PINS_FIELD(184, 185, 0xd10, 0x10, 5, 1),
	PINS_FIELD(186, 189, 0xd10, 0x10, 6, 1),
	PIN_FIELD(198, 198, 0x810, 0x10, 2, 1),
	PIN_FIELD(199, 199,  0x810, 0x10, 3, 1),
	PIN_FIELD(200, 203,  0x810, 0x10, 4, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_pullen_range[] = {
    PIN_FIELD(0, 0, 0x930, 0x10, 23, 1),
	PIN_FIELD(1, 1, 0x930, 0x10, 24, 1),
	PIN_FIELD(2, 2, 0x930, 0x10, 25, 1),
	PIN_FIELD(3, 3, 0x930, 0x10, 26, 1),
	PIN_FIELD(4, 4, 0x930, 0x10, 27, 1),
	PIN_FIELD(5, 5, 0xa30, 0x10, 9, 1),
	PIN_FIELD(6, 6, 0xa30, 0x10, 10, 1),
	PIN_FIELD(7, 7, 0xa30, 0x10, 11, 1),
	PIN_FIELD(8, 8, 0xa30, 0x10, 12, 1),
	PIN_FIELD(9, 9, 0xa30, 0x10, 20, 1),
	PIN_FIELD(10, 10, 0xa30, 0x10, 21, 1),
	PIN_FIELD(11, 11, 0xa30, 0x10, 22, 1),
	PIN_FIELD(12, 12, 0xa30, 0x10, 23, 1),
	PIN_FIELD(13, 13, 0x830, 0x10, 0, 1),
	PIN_FIELD(14, 14, 0x830, 0x10, 1, 1),
	PIN_FIELD(15, 15, 0x830, 0x10, 2, 1),
	PIN_FIELD(16, 16, 0x830, 0x10, 3, 1),
	PIN_FIELD(17, 17, 0x830, 0x10, 4, 1),
	PIN_FIELD(18, 18, 0x830, 0x10, 5, 1),
	PIN_FIELD(19, 19, 0x830, 0x10, 6, 1),
	PIN_FIELD(20, 20, 0x830, 0x10, 7, 1),
	PIN_FIELD(21, 21, 0x830, 0x10, 8, 1),
	PIN_FIELD(42, 42, 0x930, 0x10, 0, 1),
	PIN_FIELD(43, 43, 0x930, 0x10, 1, 1),
	PIN_FIELD(44, 44, 0x930, 0x10, 2, 1),
	PIN_FIELD(47, 47, 0x930, 0x10, 5, 1),
	PIN_FIELD(48, 48, 0x930, 0x10, 6, 1),
	PIN_FIELD(49, 49, 0x930, 0x10, 7, 1),
	PIN_FIELD(50, 50, 0x930, 0x10, 8, 1),
	PIN_FIELD(51, 51, 0x930, 0x10, 9, 1),
	PIN_FIELD(52, 52, 0x930, 0x10, 10, 1),
	PIN_FIELD(53, 53, 0x930, 0x10, 11, 1),
	PIN_FIELD(54, 54, 0x930, 0x10, 12, 1),
	PIN_FIELD(55, 55, 0x930, 0x10, 13, 1),
	PIN_FIELD(56, 56, 0x930, 0x10, 14, 1),
	PIN_FIELD(57, 57, 0x930, 0x10, 15, 1),
	PIN_FIELD(58, 58, 0x930, 0x10, 16, 1),
	PIN_FIELD(59, 59, 0x930, 0x10, 17, 1),
	PIN_FIELD(60, 60, 0x930, 0x10, 18, 1),
	PIN_FIELD(61, 61, 0x930, 0x10, 19, 1),
	PIN_FIELD(62, 62, 0x930, 0x10, 20, 1),
	PIN_FIELD(63, 63, 0x930, 0x10, 21, 1),
	PIN_FIELD(64, 64, 0x930, 0x10, 22, 1),
	PIN_FIELD(65, 65, 0xa30, 0x10, 0, 1),
	PIN_FIELD(66, 66, 0xa30, 0x10, 1, 1),
	PIN_FIELD(67, 67, 0xa30, 0x10, 2, 1),
	PIN_FIELD(68, 68, 0xa30, 0x10, 3, 1),
	PIN_FIELD(69, 69, 0xa30, 0x10, 4, 1),
	PIN_FIELD(70, 70, 0xa30, 0x10, 5, 1),
	PIN_FIELD(71, 71, 0xa30, 0x10, 6, 1),
	PIN_FIELD(72, 72, 0xa30, 0x10, 7, 1),
	PIN_FIELD(73, 73, 0xa30, 0x10, 8, 1),
	PIN_FIELD(74, 74, 0xa30, 0x10, 13, 1),
	PIN_FIELD(75, 75, 0xa30, 0x10, 14, 1),
	PIN_FIELD(76, 76, 0xa30, 0x10, 15, 1),
	PIN_FIELD(77, 77, 0xa30, 0x10, 16, 1),
	PIN_FIELD(78, 78, 0xa30, 0x10, 17, 1),
	PIN_FIELD(79, 79, 0xa30, 0x10, 18, 1),
	PIN_FIELD(80, 80, 0xa30, 0x10, 19, 1),
	PIN_FIELD(87, 87, 0xa30, 0x10, 30, 1),
	PIN_FIELD(88, 88, 0xa30, 0x10, 31, 1),
	PIN_FIELD(89, 89, 0xa40, 0x10, 0, 1),
	PIN_FIELD(90, 90, 0xa40, 0x10, 1, 1),
	PIN_FIELD(91, 91, 0xa40, 0x10, 2, 1),
	PIN_FIELD(92, 92, 0xa40, 0x10, 3, 1),
	PIN_FIELD(93, 93, 0xa40, 0x10, 4, 1),
	PIN_FIELD(94, 94, 0xa40, 0x10, 5, 1),
	PIN_FIELD(95, 95, 0xa40, 0x10, 6, 1),
	PIN_FIELD(96, 96, 0xa40, 0x10, 7, 1),
	PIN_FIELD(97, 97, 0xa40, 0x10, 8, 1),
	PIN_FIELD(98, 98, 0xa40, 0x10, 9, 1),
	PIN_FIELD(99, 99, 0xa40, 0x10, 10, 1),
	PIN_FIELD(100, 100, 0xa40, 0x10, 11, 1),
	PIN_FIELD(101, 101, 0xa40, 0x10, 12, 1),
	PIN_FIELD(102, 102, 0xa40, 0x10, 13, 1),
	PIN_FIELD(103, 103, 0xa40, 0x10, 14, 1),
	PIN_FIELD(104, 104, 0xa40, 0x10, 15, 1),
	PIN_FIELD(105, 105, 0xa40, 0x10, 16, 1),
	PIN_FIELD(106, 106, 0xa40, 0x10, 17, 1),
	PIN_FIELD(107, 107, 0xa40, 0x10, 18, 1),
	PIN_FIELD(108, 108, 0xa40, 0x10, 19, 1),
	PIN_FIELD(109, 109, 0xa40, 0x10, 20, 1),
	PIN_FIELD(110, 110, 0xa40, 0x10, 21, 1),
	PIN_FIELD(111, 111, 0xa40, 0x10, 22, 1),
	PIN_FIELD(112, 112, 0xa40, 0x10, 23, 1),
	PIN_FIELD(113, 113, 0xa40, 0x10, 24, 1),
	PIN_FIELD(114, 114, 0xa40, 0x10, 25, 1),
	PIN_FIELD(118, 118, 0xb30, 0x10, 0, 1),
	PIN_FIELD(119, 119, 0xb30, 0x10, 1, 1),
	PIN_FIELD(120, 120, 0xb30, 0x10, 2, 1),
	PIN_FIELD(121, 121, 0xb30, 0x10, 3, 1),
	PIN_FIELD(122, 122, 0xb30, 0x10, 4, 1),
	PIN_FIELD(123, 123, 0xb30, 0x10, 5, 1),
	PIN_FIELD(124, 124, 0xb30, 0x10, 6, 1),
	PIN_FIELD(125, 125, 0xb30, 0x10, 7, 1),
	PIN_FIELD(126, 126, 0xb30, 0x10, 8, 1),
	PIN_FIELD(127, 127, 0xb30, 0x10, 9, 1),
	PIN_FIELD(128, 128, 0xb30, 0x10, 10, 1),
	PIN_FIELD(129, 129, 0xb30, 0x10, 11, 1),
	PIN_FIELD(130, 130, 0xb30, 0x10, 12, 1),
	PIN_FIELD(131, 131, 0xb30, 0x10, 13, 1),
	PIN_FIELD(132, 132, 0xb30, 0x10, 14, 1),
	PIN_FIELD(133, 133, 0xb30, 0x10, 15, 1),
	PIN_FIELD(134, 134, 0xb30, 0x10, 16, 1),
	PIN_FIELD(135, 135, 0xb30, 0x10, 17, 1),
	PIN_FIELD(136, 136, 0xb30, 0x10, 18, 1),
	PIN_FIELD(137, 137, 0xb30, 0x10, 19, 1),
	PIN_FIELD(138, 138, 0xb30, 0x10, 20, 1),
	PIN_FIELD(139, 139, 0xb30, 0x10, 21, 1),
	PIN_FIELD(140, 140, 0xb30, 0x10, 22, 1),
	PIN_FIELD(141, 141, 0xb30, 0x10, 23, 1),
	PIN_FIELD(142, 142, 0xb30, 0x10, 24, 1),
	PIN_FIELD(143, 143, 0xb30, 0x10, 25, 1),
	PIN_FIELD(144, 144, 0xb30, 0x10, 26, 1),
	PIN_FIELD(145, 145, 0xb30, 0x10, 27, 1),
    PIN_FIELD(146, 146, 0xb30, 0x10, 28, 1),
    PIN_FIELD(147, 147, 0xb30, 0x10, 29, 1),
    PIN_FIELD(148, 148, 0xb30, 0x10, 30, 1),
    PIN_FIELD(149, 149, 0xb30, 0x10, 31, 1),
    PIN_FIELD(184, 184, 0xd30, 0x10, 12, 1),
    PIN_FIELD(185, 185, 0xd30, 0x10, 13, 1),
    PIN_FIELD(186, 186, 0xd30, 0x10, 14, 1),
    PIN_FIELD(187, 187, 0xd30, 0x10, 15, 1),
    PIN_FIELD(188, 188, 0xd30, 0x10, 16, 1),
    PIN_FIELD(189, 189, 0xd30, 0x10, 17, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_pullsel_range[] = {
	PIN_FIELD(0, 0, 0x950, 0x10, 23, 1),
	PIN_FIELD(1, 1, 0x950, 0x10, 24, 1),
	PIN_FIELD(2, 2, 0x950, 0x10, 25, 1),
	PIN_FIELD(3, 3, 0x950, 0x10, 26, 1),
	PIN_FIELD(4, 4, 0x950, 0x10, 27, 1),
	PIN_FIELD(5, 5, 0xa50, 0x10, 9, 1),
	PIN_FIELD(6, 6, 0xa50, 0x10, 10, 1),
	PIN_FIELD(7, 7, 0xa50, 0x10, 11, 1),
	PIN_FIELD(8, 8, 0xa50, 0x10, 12, 1),
	PIN_FIELD(9, 9, 0xa50, 0x10, 20, 1),
	PIN_FIELD(10, 10, 0xa50, 0x10, 21, 1),
	PIN_FIELD(11, 11, 0xa50, 0x10, 22, 1),
	PIN_FIELD(12, 12, 0xa50, 0x10, 23, 1),
	PIN_FIELD(13, 13, 0x850, 0x10, 0, 1),
	PIN_FIELD(14, 14, 0x850, 0x10, 1, 1),
	PIN_FIELD(15, 15, 0x850, 0x10, 2, 1),
	PIN_FIELD(16, 16, 0x850, 0x10, 3, 1),
	PIN_FIELD(17, 17, 0x850, 0x10, 4, 1),
	PIN_FIELD(18, 18, 0x850, 0x10, 5, 1),
	PIN_FIELD(19, 19, 0x850, 0x10, 6, 1),
	PIN_FIELD(20, 20, 0x850, 0x10, 7, 1),
	PIN_FIELD(21, 21, 0x850, 0x10, 8, 1),
	PIN_FIELD(42, 42, 0x950, 0x10, 0, 1),
	PIN_FIELD(43, 43, 0x950, 0x10, 1, 1),
	PIN_FIELD(44, 44, 0x950, 0x10, 2, 1),
	PIN_FIELD(47, 47, 0x950, 0x10, 5, 1),
	PIN_FIELD(48, 48, 0x950, 0x10, 6, 1),
	PIN_FIELD(49, 49, 0x950, 0x10, 7, 1),
	PIN_FIELD(50, 50, 0x950, 0x10, 8, 1),
	PIN_FIELD(51, 51, 0x950, 0x10, 9, 1),
	PIN_FIELD(52, 52, 0x950, 0x10, 10, 1),
	PIN_FIELD(53, 53, 0x950, 0x10, 11, 1),
	PIN_FIELD(54, 54, 0x950, 0x10, 12, 1),
	PIN_FIELD(55, 55, 0x950, 0x10, 13, 1),
	PIN_FIELD(56, 56, 0x950, 0x10, 14, 1),
	PIN_FIELD(57, 57, 0x950, 0x10, 15, 1),
	PIN_FIELD(58, 58, 0x950, 0x10, 16, 1),
	PIN_FIELD(59, 59, 0x950, 0x10, 17, 1),
	PIN_FIELD(60, 60, 0x950, 0x10, 18, 1),
	PIN_FIELD(61, 61, 0x950, 0x10, 19, 1),
	PIN_FIELD(62, 62, 0x950, 0x10, 20, 1),
	PIN_FIELD(63, 63, 0x950, 0x10, 21, 1),
	PIN_FIELD(64, 64, 0x950, 0x10, 22, 1),
	PIN_FIELD(65, 65, 0xa50, 0x10, 0, 1),
	PIN_FIELD(66, 66, 0xa50, 0x10, 1, 1),
	PIN_FIELD(67, 67, 0xa50, 0x10, 2, 1),
	PIN_FIELD(68, 68, 0xa50, 0x10, 3, 1),
	PIN_FIELD(69, 69, 0xa50, 0x10, 4, 1),
	PIN_FIELD(70, 70, 0xa50, 0x10, 5, 1),
	PIN_FIELD(71, 71, 0xa50, 0x10, 6, 1),
	PIN_FIELD(72, 72, 0xa50, 0x10, 7, 1),
	PIN_FIELD(73, 73, 0xa50, 0x10, 8, 1),
	PIN_FIELD(74, 74, 0xa50, 0x10, 13, 1),
	PIN_FIELD(75, 75, 0xa50, 0x10, 14, 1),
	PIN_FIELD(76, 76, 0xa50, 0x10, 15, 1),
	PIN_FIELD(77, 77, 0xa50, 0x10, 16, 1),
	PIN_FIELD(78, 78, 0xa50, 0x10, 17, 1),
	PIN_FIELD(79, 79, 0xa50, 0x10, 18, 1),
	PIN_FIELD(80, 80, 0xa50, 0x10, 19, 1),
	PIN_FIELD(87, 87, 0xa50, 0x10, 30, 1),
	PIN_FIELD(88, 88, 0xa50, 0x10, 31, 1),
	PIN_FIELD(89, 89, 0xa60, 0x10, 0, 1),
	PIN_FIELD(90, 90, 0xa60, 0x10, 1, 1),
	PIN_FIELD(91, 91, 0xa60, 0x10, 2, 1),
	PIN_FIELD(92, 92, 0xa60, 0x10, 3, 1),
	PIN_FIELD(93, 93, 0xa60, 0x10, 4, 1),
	PIN_FIELD(94, 94, 0xa60, 0x10, 5, 1),
	PIN_FIELD(95, 95, 0xa60, 0x10, 6, 1),
	PIN_FIELD(96, 96, 0xa60, 0x10, 7, 1),
	PIN_FIELD(97, 97, 0xa60, 0x10, 8, 1),
	PIN_FIELD(98, 98, 0xa60, 0x10, 9, 1),
	PIN_FIELD(99, 99, 0xa60, 0x10, 10, 1),
	PIN_FIELD(100, 100, 0xa60, 0x10, 11, 1),
	PIN_FIELD(101, 101, 0xa60, 0x10, 12, 1),
	PIN_FIELD(102, 102, 0xa60, 0x10, 13, 1),
	PIN_FIELD(103, 103, 0xa60, 0x10, 14, 1),
	PIN_FIELD(104, 104, 0xa60, 0x10, 15, 1),
	PIN_FIELD(105, 105, 0xa60, 0x10, 16, 1),
	PIN_FIELD(106, 106, 0xa60, 0x10, 17, 1),
	PIN_FIELD(107, 107, 0xa60, 0x10, 18, 1),
	PIN_FIELD(108, 108, 0xa60, 0x10, 19, 1),
	PIN_FIELD(109, 109, 0xa60, 0x10, 20, 1),
	PIN_FIELD(110, 110, 0xa60, 0x10, 21, 1),
	PIN_FIELD(111, 111, 0xa60, 0x10, 22, 1),
	PIN_FIELD(112, 112, 0xa60, 0x10, 23, 1),
	PIN_FIELD(113, 113, 0xa60, 0x10, 24, 1),
	PIN_FIELD(114, 114, 0xa60, 0x10, 25, 1),
	PIN_FIELD(118, 118, 0xb50, 0x10, 0, 1),
	PIN_FIELD(119, 119, 0xb50, 0x10, 1, 1),
	PIN_FIELD(120, 120, 0xb50, 0x10, 2, 1),
	PIN_FIELD(121, 121, 0xb50, 0x10, 3, 1),
	PIN_FIELD(122, 122, 0xb50, 0x10, 4, 1),
	PIN_FIELD(123, 123, 0xb50, 0x10, 5, 1),
	PIN_FIELD(124, 124, 0xb50, 0x10, 6, 1),
	PIN_FIELD(125, 125, 0xb50, 0x10, 7, 1),
	PIN_FIELD(126, 126, 0xb50, 0x10, 8, 1),
	PIN_FIELD(127, 127, 0xb50, 0x10, 9, 1),
	PIN_FIELD(128, 128, 0xb50, 0x10, 10, 1),
	PIN_FIELD(129, 129, 0xb50, 0x10, 11, 1),
	PIN_FIELD(130, 130, 0xb50, 0x10, 12, 1),
	PIN_FIELD(131, 131, 0xb50, 0x10, 13, 1),
	PIN_FIELD(132, 132, 0xb50, 0x10, 14, 1),
	PIN_FIELD(133, 133, 0xb50, 0x10, 15, 1),
	PIN_FIELD(134, 134, 0xb50, 0x10, 16, 1),
	PIN_FIELD(135, 135, 0xb50, 0x10, 17, 1),
	PIN_FIELD(136, 136, 0xb50, 0x10, 18, 1),
	PIN_FIELD(137, 137, 0xb50, 0x10, 19, 1),
	PIN_FIELD(138, 138, 0xb50, 0x10, 20, 1),
	PIN_FIELD(139, 139, 0xb50, 0x10, 21, 1),
	PIN_FIELD(140, 140, 0xb50, 0x10, 22, 1),
	PIN_FIELD(141, 141, 0xb50, 0x10, 23, 1),
	PIN_FIELD(142, 142, 0xb50, 0x10, 24, 1),
	PIN_FIELD(143, 143, 0xb50, 0x10, 25, 1),
	PIN_FIELD(144, 144, 0xb50, 0x10, 26, 1),
	PIN_FIELD(145, 145, 0xb50, 0x10, 27, 1),
    PIN_FIELD(146, 146, 0xb50, 0x10, 28, 1),
    PIN_FIELD(147, 147, 0xb50, 0x10, 29, 1),
    PIN_FIELD(148, 148, 0xb50, 0x10, 30, 1),
    PIN_FIELD(149, 149, 0xb50, 0x10, 31, 1),
    PIN_FIELD(184, 184, 0xd50, 0x10, 12, 1),
    PIN_FIELD(185, 185, 0xd50, 0x10, 13, 1),
    PIN_FIELD(186, 186, 0xd50, 0x10, 14, 1),
    PIN_FIELD(187, 187, 0xd50, 0x10, 15, 1),
    PIN_FIELD(188, 188, 0xd50, 0x10, 16, 1),
    PIN_FIELD(189, 189, 0xd50, 0x10, 17, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_drv_range[] = {
	PINS_FIELD(0, 4, 0x974, 0x10, 8, 1),
	PINS_FIELD(5, 8, 0xa70, 0x10, 12, 1),
	PINS_FIELD(9, 12, 0xa70, 0x10, 24, 1),
	PINS_FIELD(13, 18, 0x870, 0x10, 0, 1),
	PINS_FIELD(19, 21, 0x870, 0x10, 4, 1),
	PINS_FIELD(42, 44, 0x970, 0x10, 0, 1),
	PINS_FIELD(45, 46, 0x970, 0x10, 4, 1),
	PINS_FIELD(47, 48, 0x970, 0x10, 8, 1),
	PINS_FIELD(49, 50, 0x970, 0x10, 12, 1),
	PINS_FIELD(51, 52, 0x970, 0x10, 16, 1),
	PINS_FIELD(53, 54, 0x970, 0x10, 20, 1),
	PIN_FIELD(55, 55, 0x970, 0x10, 24, 1),
	PIN_FIELD(56, 56, 0x970, 0x10, 28, 1),
	PINS_FIELD(57, 60, 0x974, 0x10, 0, 1),
	PINS_FIELD(61, 64, 0x974, 0x10, 4, 1),
	PINS_FIELD(65, 68, 0xa70, 0x10, 0, 1),
	PIN_FIELD(69, 69, 0xa70, 0x10, 4, 1),
	PINS_FIELD(70, 73, 0xa70, 0x10, 8, 1),
	PINS_FIELD(74, 77, 0xa70, 0x10, 16, 1),
	PINS_FIELD(78, 80, 0xa70, 0x10, 20, 1),
	PINS_FIELD(81, 86, 0xa70, 0x10, 28, 1),
	PINS_FIELD(87, 103, 0xa74, 0x10, 0, 1),
	PINS_FIELD(104, 114, 0xa74, 0x10, 4, 1),
	PINS_FIELD(118, 136, 0xb70, 0x10, 0, 1),
	PIN_FIELD(137, 137, 0xb70, 0x10, 4, 1),
	PINS_FIELD(138, 142, 0xb70, 0x10, 8, 1),
	PINS_FIELD(143, 145, 0xb70, 0x10, 12, 1),
	PINS_FIELD(146, 147, 0xb70, 0x10, 16, 1),
	PIN_FIELD(148, 148, 0xb70, 0x10, 20, 1),
	PIN_FIELD(149, 149, 0xb70, 0x10, 4, 1),
	PINS_FIELD(160, 162, 0xc70, 0x10, 0, 1),
	PINS_FIELD(163, 165, 0xc70, 0x10, 4, 1),
	PIN_FIELD(166, 166, 0xc70, 0x10, 8, 1),
	PIN_FIELD(167, 167, 0xc70, 0x10, 12, 1),
	PINS_FIELD(168, 171, 0xc70, 0x10, 16, 1),
	PIN_FIELD(172, 172, 0xd70, 0x10, 0, 1),
	PIN_FIELD(173, 173, 0xd70, 0x10, 4, 1),
	PIN_FIELD(174, 174, 0xd70, 0x10, 8, 1),
	PINS_FIELD(175, 182, 0xd70, 0x10, 12, 1),
	PIN_FIELD(183, 183, 0xd70, 0x10, 16, 1),
	PINS_FIELD(184, 185,0xd70, 0x10, 20, 1),
	PINS_FIELD(186, 189,0xd70, 0x10, 24, 1),
	PIN_FIELD(198, 198, 0x870, 0x10, 8, 1),
	PIN_FIELD(199, 199, 0x870, 0x10, 12, 1),
	PIN_FIELD(200, 203, 0x870, 0x10, 16, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_pupd_range[] = {
	PIN_FIELD(45, 45, 0x980, 0x10, 2, 1),
	PIN_FIELD(46, 46, 0x980, 0x10, 6, 1),
	PIN_FIELD(81, 81, 0xa80, 0x10, 2, 1),
	PIN_FIELD(82, 82, 0xa80, 0x10, 6, 1),
	PIN_FIELD(83, 83, 0xa80, 0x10, 10, 1),
	PIN_FIELD(84, 84, 0xa80, 0x10, 18, 1),
	PIN_FIELD(85, 85, 0xa80, 0x10, 22, 1),
	PIN_FIELD(86, 86, 0xa80, 0x10, 26, 1),
	PIN_FIELD(160, 160, 0xc90, 0x10, 2, 1),
	PIN_FIELD(161, 161, 0xc90, 0x10, 6, 1),
	PIN_FIELD(162, 162, 0xc90, 0x10, 10, 1),
	PIN_FIELD(163, 163, 0xc90, 0x10, 18, 1),
	PIN_FIELD(164, 164, 0xc90, 0x10, 22, 1),
	PIN_FIELD(165, 165, 0xc90, 0x10, 26, 1),
	PIN_FIELD(166, 166, 0xc80, 0x10, 2, 1),
	PIN_FIELD(167, 167, 0xc80, 0x10, 6, 1),
	PIN_FIELD(168, 168, 0xc80, 0x10, 10, 1),
	PIN_FIELD(169, 169, 0xc80, 0x10, 14, 1),
	PIN_FIELD(170, 170, 0xc80, 0x10, 18, 1),
	PIN_FIELD(171, 171, 0xc80, 0x10, 22, 1),
	PIN_FIELD(172, 172, 0xd80, 0x10, 2, 1),
	PIN_FIELD(173, 173, 0xd80, 0x10, 10, 1),
	PIN_FIELD(174, 174, 0xd80, 0x10, 6, 1),
	PIN_FIELD(175, 175, 0xd80, 0x10, 14, 1),
	PIN_FIELD(176, 176, 0xd80, 0x10, 18, 1),
	PIN_FIELD(177, 177, 0xd80, 0x10, 22, 1),
	PIN_FIELD(178, 178, 0xd80, 0x10, 26, 1),
	PIN_FIELD(179, 179, 0xd80, 0x10, 30, 1),
	PIN_FIELD(180, 180, 0xd90, 0x10, 2, 1),
	PIN_FIELD(181, 181, 0xd90, 0x10, 6, 1),
	PIN_FIELD(182, 182, 0xd90, 0x10, 10, 1),
	PIN_FIELD(183, 183, 0xd90, 0x10, 14, 1),
	PIN_FIELD(198, 198, 0x880, 0x10, 2, 1),
	PIN_FIELD(199, 199, 0x880, 0x10, 6, 1),
	PIN_FIELD(200, 200, 0x880, 0x10, 10, 1),
	PIN_FIELD(201, 201, 0x880, 0x10, 14, 1),
	PIN_FIELD(202, 202, 0x880, 0x10, 18, 1),
	PIN_FIELD(203, 203, 0x880, 0x10, 22, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_r1_range[] = {
	PIN_FIELD(45, 45, 0x980, 0x10, 1, 1),
	PIN_FIELD(46, 46, 0x980, 0x10, 5, 1),
	PIN_FIELD(81, 81, 0xa80, 0x10, 1, 1),
	PIN_FIELD(82, 82, 0xa80, 0x10, 5, 1),
	PIN_FIELD(83, 83, 0xa80, 0x10, 9, 1),
	PIN_FIELD(84, 84, 0xa80, 0x10, 17, 1),
	PIN_FIELD(85, 85, 0xa80, 0x10, 21, 1),
	PIN_FIELD(86, 86, 0xa80, 0x10, 25, 1),
	PIN_FIELD(160, 160, 0xc90, 0x10, 1, 1),
	PIN_FIELD(161, 161, 0xc90, 0x10, 5, 1),
	PIN_FIELD(162, 162, 0xc90, 0x10, 9, 1),
	PIN_FIELD(163, 163, 0xc90, 0x10, 17, 1),
	PIN_FIELD(164, 164, 0xc90, 0x10, 21, 1),
	PIN_FIELD(165, 165, 0xc90, 0x10, 25, 1),
	PIN_FIELD(166, 166, 0xc80, 0x10, 0, 1),
	PIN_FIELD(167, 167, 0xc80, 0x10, 4, 1),
	PIN_FIELD(168, 168, 0xc80, 0x10, 8, 1),
	PIN_FIELD(169, 169, 0xc80, 0x10, 12, 1),
	PIN_FIELD(170, 170, 0xc80, 0x10, 16, 1),
	PIN_FIELD(171, 171, 0xc80, 0x10, 20, 1),
	PIN_FIELD(172, 172, 0xd80, 0x10, 1, 1),
	PIN_FIELD(173, 173, 0xd80, 0x10, 8, 1),
	PIN_FIELD(174, 174, 0xd80, 0x10, 4, 1),
	PIN_FIELD(175, 175, 0xd80, 0x10, 13, 1),
	PIN_FIELD(176, 176, 0xd80, 0x10, 17, 1),
	PIN_FIELD(177, 177, 0xd80, 0x10, 21, 1),
	PIN_FIELD(178, 178, 0xd80, 0x10, 25, 1),
	PIN_FIELD(179, 179, 0xd80, 0x10, 29, 1),
	PIN_FIELD(180, 180, 0xd90, 0x10, 1, 1),
	PIN_FIELD(181, 181, 0xd90, 0x10, 5, 1),
	PIN_FIELD(182, 182, 0xd90, 0x10, 9, 1),
	PIN_FIELD(183, 183, 0xd90, 0x10, 12, 1),
	PIN_FIELD(198, 198, 0x880, 0x10, 1, 1),
	PIN_FIELD(199, 199, 0x880, 0x10, 5, 1),
	PIN_FIELD(200, 200, 0x880, 0x10, 9, 1),
	PIN_FIELD(201, 201, 0x880, 0x10, 13, 1),
	PIN_FIELD(202, 202, 0x880, 0x10, 17, 1),
	PIN_FIELD(203, 203, 0x880, 0x10, 21, 1),
};

static const struct mtk_pin_field_calc mt6735_pin_r0_range[] = {
	PIN_FIELD(45, 45, 0x980, 0x10, 0, 1),
	PIN_FIELD(46, 46, 0x980, 0x10, 4, 1),
	PIN_FIELD(81, 81, 0xa80, 0x10, 0, 1),
	PIN_FIELD(82, 82, 0xa80, 0x10, 4, 1),
	PIN_FIELD(83, 83, 0xa80, 0x10, 8, 1),
	PIN_FIELD(84, 84, 0xa80, 0x10, 16, 1),
	PIN_FIELD(85, 85, 0xa80, 0x10, 20, 1),
	PIN_FIELD(86, 86, 0xa80, 0x10, 24, 1),
	PIN_FIELD(160, 160, 0xc90, 0x10, 0, 1),
	PIN_FIELD(161, 161, 0xc90, 0x10, 4, 1),
	PIN_FIELD(162, 162, 0xc90, 0x10, 8, 1),
	PIN_FIELD(163, 163, 0xc90, 0x10, 16, 1),
	PIN_FIELD(164, 164, 0xc90, 0x10, 20, 1),
	PIN_FIELD(165, 165, 0xc90, 0x10, 24, 1),
	PIN_FIELD(166, 166, 0xc80, 0x10, 1, 1),
	PIN_FIELD(167, 167, 0xc80, 0x10, 5, 1),
	PIN_FIELD(168, 168, 0xc80, 0x10, 9, 1),
	PIN_FIELD(169, 169, 0xc80, 0x10, 13, 1),
	PIN_FIELD(170, 170, 0xc80, 0x10, 17, 1),
	PIN_FIELD(171, 171, 0xc80, 0x10, 21, 1),
	PIN_FIELD(172, 172, 0xd80, 0x10, 0, 1),
	PIN_FIELD(173, 173, 0xd80, 0x10, 9, 1),
	PIN_FIELD(174, 174, 0xd80, 0x10, 5, 1),
	PIN_FIELD(175, 175, 0xd80, 0x10, 12, 1),
	PIN_FIELD(176, 176, 0xd80, 0x10, 16, 1),
	PIN_FIELD(177, 177, 0xd80, 0x10, 20, 1),
	PIN_FIELD(178, 178, 0xd80, 0x10, 24, 1),
	PIN_FIELD(179, 179, 0xd80, 0x10, 28, 1),
	PIN_FIELD(180, 180, 0xd90, 0x10, 0, 1),
	PIN_FIELD(181, 181, 0xd90, 0x10, 4, 1),
	PIN_FIELD(182, 182, 0xd90, 0x10, 8, 1),
	PIN_FIELD(183, 183, 0xd90, 0x10, 13, 1),
	PIN_FIELD(198, 198, 0x880, 0x10, 0, 1),
	PIN_FIELD(199, 199, 0x880, 0x10, 4, 1),
	PIN_FIELD(200, 200, 0x880, 0x10, 8, 1),
	PIN_FIELD(201, 201, 0x880, 0x10, 12, 1),
	PIN_FIELD(202, 202, 0x880, 0x10, 16, 1),
	PIN_FIELD(203, 203, 0x880, 0x10, 20, 1),
};

static const struct mtk_pin_reg_calc mt6735_reg_cals[] = {
	[PINCTRL_PIN_REG_MODE] = MTK_RANGE(mt6735_pin_mode_range),
	[PINCTRL_PIN_REG_DIR] = MTK_RANGE(mt6735_pin_dir_range),
	[PINCTRL_PIN_REG_DI] = MTK_RANGE(mt6735_pin_di_range),
	[PINCTRL_PIN_REG_DO] = MTK_RANGE(mt6735_pin_do_range),
	[PINCTRL_PIN_REG_IES] = MTK_RANGE(mt6735_pin_ies_range),
	[PINCTRL_PIN_REG_SMT] = MTK_RANGE(mt6735_pin_smt_range),
	[PINCTRL_PIN_REG_PULLSEL] = MTK_RANGE(mt6735_pin_pullsel_range),
	[PINCTRL_PIN_REG_PULLEN] = MTK_RANGE(mt6735_pin_pullen_range),
	[PINCTRL_PIN_REG_DRV] = MTK_RANGE(mt6735_pin_drv_range),
	[PINCTRL_PIN_REG_PUPD] = MTK_RANGE(mt6735_pin_pupd_range),
	[PINCTRL_PIN_REG_R0] = MTK_RANGE(mt6735_pin_r0_range),
	[PINCTRL_PIN_REG_R1] = MTK_RANGE(mt6735_pin_r1_range),
};



static const struct mtk_pin_desc mt6735_pins[] = {
	MTK_PIN(0, "IDDIG", DRV_GRP3),
	MTK_PIN(1, "GPIO1", DRV_GRP3),
	MTK_PIN(2, "GPIO2", DRV_GRP3),
	MTK_PIN(3, "GPIO3", DRV_GRP3),
	MTK_PIN(4, "GPIO4", DRV_GRP3),
    MTK_PIN(5, "GPIO5", DRV_GRP3),
    MTK_PIN(6, "GPIO6", DRV_GRP3),
    MTK_PIN(7, "GPIO7", DRV_GRP3),
    MTK_PIN(8, "MD_EINT1", DRV_GRP3),
    MTK_PIN(9, "MD_EINT2", DRV_GRP3),
    MTK_PIN(10, "GPIO10", DRV_GRP3),
    MTK_PIN(11, "GPIO11", DRV_GRP3),
    MTK_PIN(12, "GPIO12", DRV_GRP3),
    MTK_PIN(13, "WB_CTRL0", DRV_GRP4),
    MTK_PIN(14, "WB_CTRL1", DRV_GRP4),
    MTK_PIN(15, "WB_CTRL2", DRV_GRP4),
    MTK_PIN(16, "WB_CTRL3", DRV_GRP4),
    MTK_PIN(17, "WB_CTRL4", DRV_GRP4),
    MTK_PIN(18, "WB_CTRL5", DRV_GRP4),
    MTK_PIN(19, "LTE_UTXD", DRV_GRP1),
    MTK_PIN(20, "LTE_URXD", DRV_GRP1),
    MTK_PIN(21, "CORESONIC_SWCK", DRV_GRP3),
    MTK_PIN(22, "RDN0", DRV_FIXED),
    MTK_PIN(23, "RDP0", DRV_FIXED),
    MTK_PIN(24, "RDN1", DRV_FIXED),
    MTK_PIN(25, "RDP1", DRV_FIXED),
    MTK_PIN(26, "RCN", DRV_FIXED),
    MTK_PIN(27, "RCP", DRV_FIXED),
    MTK_PIN(28, "RDN2", DRV_FIXED),
    MTK_PIN(29, "RDP2", DRV_FIXED),
    MTK_PIN(30, "RDN3", DRV_FIXED),
    MTK_PIN(31, "RDP3", DRV_FIXED),
    MTK_PIN(32, "RDN0_A", DRV_FIXED),
    MTK_PIN(33, "RDP0_A", DRV_FIXED),
    MTK_PIN(34, "RDN1_A", DRV_FIXED),
    MTK_PIN(35, "RDP1_A", DRV_FIXED),
    MTK_PIN(36, "RCN_A", DRV_FIXED),
    MTK_PIN(37, "RCP_A", DRV_FIXED),
    MTK_PIN(38, "RDN2_A", DRV_FIXED),
    MTK_PIN(39, "RDP2_A", DRV_FIXED),
    MTK_PIN(40, "RDN3_A", DRV_FIXED),
    MTK_PIN(41, "RDP3_A", DRV_FIXED),
    MTK_PIN(42, "GPIO42", DRV_GRP3),
    MTK_PIN(43, "GPIO43", DRV_GRP3),
    MTK_PIN(44, "GPIO44", DRV_GRP3),
    MTK_PIN(45, "CMMCLK0", DRV_GRP3),
    MTK_PIN(46, "CMMCLK1", DRV_GRP3),
    MTK_PIN(47, "SDA0", DRV_FIXED),
    MTK_PIN(48, "SCL0", DRV_FIXED),
    MTK_PIN(49, "SDA1", DRV_FIXED),
    MTK_PIN(50, "SCL1", DRV_FIXED),
    MTK_PIN(51, "SDA2", DRV_FIXED),
    MTK_PIN(52, "SCL2", DRV_FIXED),
    MTK_PIN(53, "SDA3", DRV_FIXED),
    MTK_PIN(54, "SCL3", DRV_FIXED),
    MTK_PIN(55, "SRCLKENAI0", DRV_GRP3),
    MTK_PIN(56, "SRCLKENA1", DRV_GRP3),
    MTK_PIN(57, "URXD2", DRV_FIXED),
    MTK_PIN(58, "TDD_TXD", DRV_GRP4),
    MTK_PIN(59, "URXD3", DRV_FIXED),
    MTK_PIN(60, "UTXD3", DRV_FIXED),
    MTK_PIN(61, "GPIO61", DRV_GRP3),
    MTK_PIN(62, "GPIO62", DRV_GRP3),
    MTK_PIN(63, "GPIO63", DRV_GRP3),
    MTK_PIN(64, "GPIO64", DRV_GRP3),
    MTK_PIN(65, "SPI_CSA", DRV_GRP3),
    MTK_PIN(66, "SPI_CKA", DRV_GRP3),
    MTK_PIN(67, "SPI_MIA", DRV_GRP3),
    MTK_PIN(68, "SPI_MOA", DRV_GRP3),
    MTK_PIN(69, "DISP_PWM", DRV_GRP3),
    MTK_PIN(70, "JTMS", DRV_GRP4),
    MTK_PIN(71, "JTCK", DRV_GRP4),
    MTK_PIN(72, "JTDI", DRV_GRP4),
    MTK_PIN(73, "JTDO", DRV_GRP4),
    MTK_PIN(74, "URXD0", DRV_FIXED),
    MTK_PIN(75, "UTXD0", DRV_FIXED),
    MTK_PIN(76, "URXD1", DRV_FIXED),
    MTK_PIN(77, "UTXD1", DRV_FIXED),
    MTK_PIN(78, "GPIO78", DRV_GRP3),
    MTK_PIN(79, "GPIO79", DRV_GRP3),
    MTK_PIN(80, "GPIO80", DRV_GRP3),
    MTK_PIN(81, "KROW0", DRV_GRP3),
    MTK_PIN(82, "CORESONIC_SWD", DRV_GRP3),
    MTK_PIN(83, "USB_DRVVBUS", DRV_GRP3),
    MTK_PIN(84, "KCOL0", DRV_GRP3),
    MTK_PIN(85, "KCOL1", DRV_GRP3),
    MTK_PIN(86, "GPIO86", DRV_GRP3),
    MTK_PIN(87, "LTE_C2K_BPI_BUS5", DRV_GRP1),
    MTK_PIN(88, "LTE_C2K_BPI_BUS6", DRV_GRP1),
    MTK_PIN(89, "GPIO89", DRV_GRP3),
    MTK_PIN(90, "GPIO90", DRV_GRP3),
    MTK_PIN(91, "LTE_C2K_BPI_BUS9", DRV_GRP3),
    MTK_PIN(92, "GPIO92", DRV_GRP3),
    MTK_PIN(93, "GPIO93", DRV_GRP3),
    MTK_PIN(94, "LTE_C2K_BPI_BUS12", DRV_GRP3),
    MTK_PIN(95, "GPIO95", DRV_GRP3),
    MTK_PIN(96, "GPIO96", DRV_GRP3),
    MTK_PIN(97, "GPIO97", DRV_GRP3),
    MTK_PIN(98, "C2K_BPI_BUS16", DRV_GRP1),
    MTK_PIN(99, "C2K_BPI_BUS17", DRV_GRP1),
    MTK_PIN(100, "C2K_BPI_BUS18", DRV_GRP1),
    MTK_PIN(101, "C2K_BPI_BUS19", DRV_GRP1),
    MTK_PIN(102, "C2K_BPI_BUS20", DRV_GRP1),
    MTK_PIN(103, "C2K_TXBPI", DRV_GRP3),
    MTK_PIN(104, "C2K_RX_BSI_EN", DRV_GRP3),
    MTK_PIN(105, "C2K_RX_BSI_CLK", DRV_GRP3),
    MTK_PIN(106, "C2K_RX_BSI_DATA", DRV_GRP3),
    MTK_PIN(107, "C2K_TX_BSI_EN", DRV_GRP3),
    MTK_PIN(108, "C2K_TX_BSI_CLK", DRV_GRP3),
    MTK_PIN(109, "C2K_TX_BSI_DATA", DRV_GRP3),
    MTK_PIN(110, "RFIC0_BSI_EN", DRV_GRP3),
    MTK_PIN(111, "RFIC0_BSI_CK", DRV_GRP3),
    MTK_PIN(112, "RFIC0_BSI_D2", DRV_GRP3),
    MTK_PIN(113, "RFIC0_BSI_D1", DRV_GRP3),
    MTK_PIN(114, "RFIC0_BSI_D0", DRV_GRP4),
    MTK_PIN(115, "GPIO115", DRV_GRP3),
    MTK_PIN(116, "GPIO116", DRV_GRP3),
    MTK_PIN(117, "GPIO117", DRV_GRP3),
    MTK_PIN(118, "TXBPI", DRV_GRP3),
    MTK_PIN(119, "BPI_BUS0", DRV_GRP4),
    MTK_PIN(120, "BPI_BUS1", DRV_GRP4),
    MTK_PIN(121, "BPI_BUS2", DRV_GRP4),
    MTK_PIN(122, "BPI_BUS3", DRV_GRP4),
    MTK_PIN(123, "BPI_BUS4", DRV_GRP4),
    MTK_PIN(124, "BPI_BUS21", DRV_GRP3),
    MTK_PIN(125, "BPI_BUS22", DRV_GRP3),
    MTK_PIN(126, "BPI_BUS23", DRV_GRP3),
    MTK_PIN(127, "BPI_BUS24", DRV_GRP3),
    MTK_PIN(128, "GPIO128", DRV_GRP3),
    MTK_PIN(129, "BPI_BUS26", DRV_GRP3),
    MTK_PIN(130, "GPIO130", DRV_GRP3),
    MTK_PIN(131, "LTE_PAVM0", DRV_GRP3),
    MTK_PIN(132, "LTE_PAVM1", DRV_GRP3),
    MTK_PIN(133, "MIPI1_SCLK", DRV_GRP3),
    MTK_PIN(134, "MIPI1_SDATA", DRV_GRP3),
    MTK_PIN(135, "MIPI0_SCLK", DRV_GRP3),
    MTK_PIN(136, "MIPI0_SDATA", DRV_GRP3),
    MTK_PIN(137, "RTC32K_CK", DRV_GRP3),
    MTK_PIN(138, "PWRAP_SPIDO", DRV_GRP3),
    MTK_PIN(139, "PWRAP_SPIDI", DRV_GRP3),
    MTK_PIN(140, "GPIO140", DRV_GRP3),
    MTK_PIN(141, "PWRAP_SPICK_I", DRV_GRP3),
    MTK_PIN(142, "PWRAP_SPICS_B_I", DRV_GRP3),
    MTK_PIN(143, "GPIO143", DRV_GRP3),
    MTK_PIN(144, "GPIO144", DRV_GRP3),
    MTK_PIN(145, "GPIO145", DRV_GRP3),
    MTK_PIN(146, "LCM_RST", DRV_GRP3),
    MTK_PIN(147, "DSI_TE", DRV_GRP3),
    MTK_PIN(148, "SRCLKENA", DRV_GRP3),
    MTK_PIN(149, "WATCHDOG", DRV_GRP3),
    MTK_PIN(150, "TDP0", DRV_FIXED),
    MTK_PIN(151, "TDN0", DRV_FIXED),
    MTK_PIN(152, "TDP1", DRV_FIXED),
    MTK_PIN(153, "TDN1", DRV_FIXED),
    MTK_PIN(154, "TCP", DRV_FIXED),
    MTK_PIN(155, "TCN", DRV_FIXED),
    MTK_PIN(156, "TDP2", DRV_FIXED),
    MTK_PIN(157, "TDN2", DRV_FIXED),
    MTK_PIN(158, "TDP3", DRV_FIXED),
    MTK_PIN(159, "TDN3", DRV_FIXED),
    MTK_PIN(160, "MD_SIM2_SCLK", DRV_GRP1),
    MTK_PIN(161, "MD_SIM2_SRST", DRV_GRP1),
    MTK_PIN(162, "MD_SIM2_SDAT", DRV_GRP1),
    MTK_PIN(163, "MD_SIM1_SCLK", DRV_GRP1),
    MTK_PIN(164, "MD_SIM1_SRST", DRV_GRP1),
    MTK_PIN(165, "MD_SIM1_SDAT", DRV_GRP1),
    MTK_PIN(166, "MSDC1_CMD", DRV_GRP4),
    MTK_PIN(167, "MSDC1_CLK", DRV_GRP4),
    MTK_PIN(168, "MSDC1_DAT0", DRV_GRP4),
    MTK_PIN(169, "MSDC1_DAT1", DRV_GRP4),
    MTK_PIN(170, "MSDC1_DAT2", DRV_GRP4),
    MTK_PIN(171, "MSDC1_DAT3", DRV_GRP4),
    MTK_PIN(172, "MSDC0_CMD", DRV_GRP4),
    MTK_PIN(173, "MSDC0_DSL", DRV_GRP4),
    MTK_PIN(174, "MSDC0_CLK", DRV_GRP4),
    MTK_PIN(175, "MSDC0_D0", DRV_GRP4),
    MTK_PIN(176, "MSDC0_D1", DRV_GRP4),
    MTK_PIN(177, "MSDC0_D2", DRV_GRP4),
    MTK_PIN(178, "MSDC0_D3", DRV_GRP4),
    MTK_PIN(179, "MSDC0_D4", DRV_GRP4),
    MTK_PIN(180, "MSDC0_D5", DRV_GRP4),
    MTK_PIN(181, "MSDC0_D6", DRV_GRP4),
    MTK_PIN(182, "MSDC0_D7", DRV_GRP4),
    MTK_PIN(183, "MSDC0_RSTB", DRV_GRP4),
    MTK_PIN(184, "F2W_DATA", DRV_GRP3),
    MTK_PIN(185, "F2W_CK", DRV_GRP3),
    MTK_PIN(186, "WB_RSTB", DRV_GRP3),
    MTK_PIN(187, "WB_SCLK", DRV_GRP3),
    MTK_PIN(188, "WB_SDATA", DRV_GRP3),
    MTK_PIN(189, "WB_SEN", DRV_GRP3),
    MTK_PIN(190, "GPS_RXQN", DRV_GRP3),
    MTK_PIN(191, "GPS_RXQP", DRV_GRP3),
    MTK_PIN(192, "GPS_RXIN", DRV_GRP3),
    MTK_PIN(193, "GPS_RXIP", DRV_GRP3),
    MTK_PIN(194, "WB_RXQN", DRV_GRP3),
    MTK_PIN(195, "WB_RXQP", DRV_GRP3),
    MTK_PIN(196, "WB_RXIN", DRV_GRP3),
    MTK_PIN(197, "WB_RXIP", DRV_GRP3),
    MTK_PIN(198, "C2K_UART0_RXD", DRV_GRP3),
    MTK_PIN(199, "C2K_UART0_TXD", DRV_GRP3),
    MTK_PIN(200, "GPIO200", DRV_GRP3),
    MTK_PIN(201, "GPIO201", DRV_GRP3),
    MTK_PIN(202, "GPIO202", DRV_GRP3),
    MTK_PIN(203, "GPIO203", DRV_GRP3),


};

/* List all groups consisting of these pins dedicated to the enablement of
 * certain hardware block and the corresponding mode for all of the pins.
 * The hardware probably has multiple combinations of these pinouts.
 */

/* DISP PWM */
static int mt6735_disp_pwm_0_pins[] = { 55, };
static int mt6735_disp_pwm_0_funcs[] = { 7, };
static int mt6735_disp_pwm_1_pins[] = { 69, };
static int mt6735_disp_pwm_1_funcs[] = { 1, };
static int mt6735_disp_pwm_2_pins[] = { 129, };
static int mt6735_disp_pwm_2_funcs[] = { 2, };



/* I2C */
static int mt6735_i2c0_pins[] = { 47, 48, };
static int mt6735_i2c0_funcs[] = { 1, 1, };
static int mt6735_i2c1_0_pins[] = { 7, 8, };
static int mt6735_i2c1_0_funcs[] = { 3, 3, };
static int mt6735_i2c1_1_pins[] = { 49, 50, };
static int mt6735_i2c1_1_funcs[] = { 1, 1, };
static int mt6735_i2c1_2_pins[] = { 198, 199, };
static int mt6735_i2c1_2_funcs[] = { 2, 2, };;
static int mt6735_i2c2_0_pins[] = { 51, 52, };
static int mt6735_i2c2_0_funcs[] = { 1, 1, };
static int mt6735_i2c2_1_pins[] = { 85, 86, };
static int mt6735_i2c2_1_funcs[] = { 4, 4, };
static int mt6735_i2c2_2_pins[] = { 202, 203, };
static int mt6735_i2c2_2_funcs[] = { 3, 3, };
static int mt6735_i2c3_0_pins[] = { 10, 11, };
static int mt6735_i2c3_0_funcs[] = { 6, 6, };
static int mt6735_i2c3_1_pins[] = { 53, 54, };
static int mt6735_i2c3_1_funcs[] = { 1, 1, };
static int mt6735_i2c3_2_pins[] = { 74, 75, };
static int mt6735_i2c3_2_funcs[] = { 4, 4, };

/* I2S */
static int mt6735_i2s0_0_bclk_lrclk_mclk_pins[] = { 61, 62, 64, };
static int mt6735_i2s0_0_bclk_lrclk_mclk_funcs[] = { 3, 3, 3, };
static int mt6735_i2s0_1_bclk_lrclk_mclk_pins[] = { 80, 79, 12, };
static int mt6735_i2s0_1_bclk_lrclk_mclk_funcs[] = { 1, 1, 1, };
static int mt6735_i2s0_data_in_0_pins[] = { 63, };
static int mt6735_i2s0_data_in_0_funcs[] = { 3, };
static int mt6735_i2s0_data_in_1_pins[] = { 78,  };
static int mt6735_i2s0_data_in_1_funcs[] = { 1, };
static int mt6735_i2s1_0_bclk_lrclk_mclk_pins[] = { 80, 79, 10, };
static int mt6735_i2s1_0_bclk_lrclk_mclk_funcs[] = { 4, 4, 5, };
static int mt6735_i2s1_1_bclk_lrclk_mclk_pins[] = { 6, 7, 9, };
static int mt6735_i2s1_1_bclk_lrclk_mclk_funcs[] = { 6, 6, 6, };
static int mt6735_i2s1_data_0_pins[] = { 5, };
static int mt6735_i2s1_data_0_funcs[] = { 6, };
static int mt6735_i2s1_data_1_pins[] = { 78,  };
static int mt6735_i2s1_data_1_funcs[] = { 4, };
static int mt6735_i2s2_0_bclk_lrclk_mclk_pins[] = { 130, 129, 11, };
static int mt6735_i2s2_0_bclk_lrclk_mclk_funcs[] = { 5, 5, 5, };
static int mt6735_i2s2_1_bclk_lrclk_mclk_pins[] = { 80, 79, 126, };
static int mt6735_i2s2_1_bclk_lrclk_mclk_funcs[] = { 6, 6, 6, };
static int mt6735_i2s2_data_in_0_pins[] = { 128, };
static int mt6735_i2s2_data_in_0_funcs[] = { 5, };
static int mt6735_i2s2_data_in_1_pins[] = { 78,  };
static int mt6735_i2s2_data_in_1_funcs[] = { 6, };
static int mt6735_i2s3_0_bclk_lrclk_mclk_pins[] = { 66, 68, 65, };
static int mt6735_i2s3_0_bclk_lrclk_mclk_funcs[] = { 3, 3, 3, };
static int mt6735_i2s3_1_bclk_lrclk_mclk_pins[] = { 80, 79, 9, };
static int mt6735_i2s3_1_bclk_lrclk_mclk_funcs[] = { 3, 3, 3, };
/* LCD */
static int mt6735_mipi_tx_pins[] = { 133, 134, 135, 136, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159 };
static int mt6735_mipi_tx_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
static int mt6735_dsi_te_pins[] = { 147, };
static int mt6735_dsi_te_funcs[] = { 1, };
static int mt6735_lcm_rst_pins[] = { 146, };
static int mt6735_lcm_rst_funcs[] = { 1, };


/* MSDC */
static int mt6735_msdc0_pins[] = { 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, };
static int mt6735_msdc0_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };
static int mt6735_msdc1_pins[] = { 166, 167, 168, 169, 170, 171, };
static int mt6735_msdc1_funcs[] = { 1, 1, 1, 1, 1, 1, };
static int mt6735_msdc2_pins[] = { 198, 199, 200, 201, 202, 203, };
static int mt6735_msdc2_funcs[] = { 1, 1, 1, 1, 1, 1, };

/* RTC */
static int mt6735_rtc_pins[] = { 137, };
static int mt6735_rtc_funcs[] = { 1, };

/* PCM */
static int mt6735_pcm_clk_0_pins[] = { 184, };
static int mt6735_pcm_clk_0_funcs[] = { 4, };
static int mt6735_pcm_clk_1_0_pins[] = { 61, };
static int mt6735_pcm_clk_1_0_funcs[] = { 1, };
static int mt6735_pcm_clk_1_1_pins[] = { 80, };
static int mt6735_pcm_clk_1_1_funcs[] = { 2, };
static int mt6735_pcm_sync_0_pins[] = { 188, };
static int mt6735_pcm_sync_0_funcs[] = { 4, };
static int mt6735_pcm_sync_1_0_pins[] = { 62, };
static int mt6735_pcm_sync_1_0_funcs[] = { 1, };
static int mt6735_pcm_sync_1_1_pins[] = { 79, };
static int mt6735_pcm_sync_1_1_funcs[] = { 2, };
static int mt6735_pcm_rx_0_pins[] = { 185, };
static int mt6735_pcm_rx_0_funcs[] = { 4, };
static int mt6735_pcm_rx_1_0_pins[] = { 63, };
static int mt6735_pcm_rx_1_0_funcs[] = { 1, };
static int mt6735_pcm_rx_1_1_pins[] = { 64, };
static int mt6735_pcm_rx_1_1_funcs[] = { 4, };
static int mt6735_pcm_rx_1_2_pins[] = { 78, };
static int mt6735_pcm_rx_1_2_funcs[] = { 2, };
static int mt6735_pcm_tx_0_pins[] = { 187, };
static int mt6735_pcm_tx_0_funcs[] = { 4, };
static int mt6735_pcm_tx_1_0_pins[] = { 9, };
static int mt6735_pcm_tx_1_0_funcs[] = { 2, };
static int mt6735_pcm_tx_1_1_pins[] = { 63, };
static int mt6735_pcm_tx_1_1_funcs[] = { 4, };
static int mt6735_pcm_tx_1_2_pins[] = { 64, };
static int mt6735_pcm_tx_1_2_funcs[] = { 1, };
static int mt6735_pcm_tx_1_3_pins[] = { 8, };
static int mt6735_pcm_tx_1_3_funcs[] = { 4, };
static int mt6735_pcm_tx_1_4_pins[] = { 12, };
static int mt6735_pcm_tx_1_4_funcs[] = { 7, };

/* PWM */
static int mt6735_pwm_ch1_0_pins[] = { 44, };
static int mt6735_pwm_ch1_0_funcs[] = { 6, };
static int mt6735_pwm_ch1_1_pins[] = { 78, };
static int mt6735_pwm_ch1_1_funcs[] = { 5, };
static int mt6735_pwm_ch1_2_pins[] = { 201, };
static int mt6735_pwm_ch1_2_funcs[] = { 3, };
static int mt6735_pwm_ch2_0_pins[] = { 10, };
static int mt6735_pwm_ch2_0_funcs[] = { 1, };
static int mt6735_pwm_ch2_1_pins[] = { 69, };
static int mt6735_pwm_ch2_1_funcs[] = { 2, };
static int mt6735_pwm_ch3_0_pins[] = { 1, };
static int mt6735_pwm_ch3_0_funcs[] = { 1, };
static int mt6735_pwm_ch3_1_pins[] = { 21, };
static int mt6735_pwm_ch3_1_funcs[] = { 2, };
static int mt6735_pwm_ch3_2_pins[] = { 55, };
static int mt6735_pwm_ch3_2_funcs[] = { 2, };
static int mt6735_pwm_ch4_0_pins[] = { 0, };
static int mt6735_pwm_ch4_0_funcs[] = { 5, };
static int mt6735_pwm_ch4_1_pins[] = { 59, };
static int mt6735_pwm_ch4_1_funcs[] = { 5, };
static int mt6735_pwm_ch4_2_pins[] = { 79, };
static int mt6735_pwm_ch4_2_funcs[] = { 5, };

/* PWRAP */
static int mt6735_pwrap_pins[] = { 138, 139, 141, 142, };
static int mt6735_pwrap_funcs[] = { 1, 1, 1, 1, };


/* SPI */
static int mt6735_spi0_pins[] = { 3, 4, 5, 6, };
static int mt6735_spi0_funcs[] = { 3, 3, 3, 3, };
static int mt6735_spi1_pins[] = { 65, 66, 67, 68, };
static int mt6735_spi1_funcs[] = { 1, 1, 1, 1, };


/* UART */
static int mt6735_uart0_0_txd_rxd_pins[] = { 8, 11, };
static int mt6735_uart0_0_txd_rxd_funcs[] = { 7, 7, };
static int mt6735_uart0_1_txd_rxd_pins[] = { 74, 75, };
static int mt6735_uart0_1_txd_rxd_funcs[] = { 2, 2, };
static int mt6735_uart0_2_txd_rxd_pins[] = { 75, 74, };
static int mt6735_uart0_2_txd_rxd_funcs[] = { 1, 1, };
static int mt6735_uart0_3_txd_rxd_pins[] = { 128, 127, };
static int mt6735_uart0_3_txd_rxd_funcs[] = { 7, 7, };
static int mt6735_uart0_4_txd_rxd_pins[] = { 200, 201, };
static int mt6735_uart0_4_txd_rxd_funcs[] = { 5, 5, };
static int mt6735_uart0_rts_cts_pins[] = { 84, 85, };
static int mt6735_uart0_rts_cts_funcs[] = { 2, 2, };
static int mt6735_uart1_0_txd_rxd_pins[] = { 76, 77, };
static int mt6735_uart1_0_txd_rxd_funcs[] = { 2, 2, };
static int mt6735_uart1_1_txd_rxd_pins[] = { 77, 76, };
static int mt6735_uart1_1_txd_rxd_funcs[] = { 1, 1, };
static int mt6735_uart1_2_txd_rxd_pins[] = { 202, 203, };
static int mt6735_uart1_2_txd_rxd_funcs[] = { 5, 5, };
static int mt6735_uart1_rts_cts_pins[] = { 86, 85, };
static int mt6735_uart1_rts_cts_funcs[] = { 3, 3, };
static int mt6735_uart2_0_txd_rxd_pins[] = { 57, 58, };
static int mt6735_uart2_0_txd_rxd_funcs[] = { 3, 3, };
static int mt6735_uart2_1_txd_rxd_pins[] = { 58, 57, };
static int mt6735_uart2_1_txd_rxd_funcs[] = { 1, 1, };
static int mt6735_uart2_rts_cts_pins[] = { 6, 5, };
static int mt6735_uart2_rts_cts_funcs[] = { 1, 1, };
static int mt6735_uart3_0_txd_rxd_pins[] = { 59, 60, };
static int mt6735_uart3_0_txd_rxd_funcs[] = { 3, 3, };
static int mt6735_uart3_1_txd_rxd_pins[] = { 60, 59, };
static int mt6735_uart3_1_txd_rxd_funcs[] = { 1, 1, };
static int mt6735_uart3_2_txd_rxd_pins[] = { 186, 189, };
static int mt6735_uart3_2_txd_rxd_funcs[] = { 5, 5, };
static int mt6735_uart3_3_txd_rxd_pins[] = { 189, 186, };
static int mt6735_uart3_3_txd_rxd_funcs[] = { 4, 4, };
static int mt6735_uart3_rts_cts_pins[] = { 8, 7, };
static int mt6735_uart3_rts_cts_funcs[] = { 1, 1, };
/* USB */
static int mt6735_usb_host_pins[] = { 0, 83, };
static int mt6735_usb_host_funcs[] = { 1, 4, };
/* Watchdog */
static int mt6735_watchdog_0_pins[] = { 17, };
static int mt6735_watchdog_0_funcs[] = { 4, };
static int mt6735_watchdog_1_pins[] = { 149, };
static int mt6735_watchdog_1_funcs[] = { 1, };

static const struct mtk_group_desc mt6735_groups[] = {
	PINCTRL_PIN_GROUP("dsi_te", mt6735_dsi_te),
	PINCTRL_PIN_GROUP("disp_pwm_0", mt6735_disp_pwm_0),
	PINCTRL_PIN_GROUP("disp_pwm_1", mt6735_disp_pwm_1),
	PINCTRL_PIN_GROUP("disp_pwm_2", mt6735_disp_pwm_2),
	PINCTRL_PIN_GROUP("i2c0", mt6735_i2c0),
	PINCTRL_PIN_GROUP("i2c1_0", mt6735_i2c1_0),
	PINCTRL_PIN_GROUP("i2c1_1", mt6735_i2c1_1),
	PINCTRL_PIN_GROUP("i2c1_2", mt6735_i2c1_2),
	PINCTRL_PIN_GROUP("i2c2_0", mt6735_i2c2_0),
	PINCTRL_PIN_GROUP("i2c2_1", mt6735_i2c2_1),
	PINCTRL_PIN_GROUP("i2c2_2", mt6735_i2c2_2),
	PINCTRL_PIN_GROUP("i2c3_0", mt6735_i2c3_0),
	PINCTRL_PIN_GROUP("i2c3_1", mt6735_i2c3_1),
	PINCTRL_PIN_GROUP("i2c3_2", mt6735_i2c3_2),
	PINCTRL_PIN_GROUP("i2s0_0_bclk_lrclk_mclk", mt6735_i2s0_0_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s0_1_bclk_lrclk_mclk", mt6735_i2s0_1_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s0_data_in_0", mt6735_i2s0_data_in_0),
	PINCTRL_PIN_GROUP("i2s0_data_in_1", mt6735_i2s0_data_in_1),
	PINCTRL_PIN_GROUP("i2s1_0_bclk_lrclk_mclk", mt6735_i2s1_0_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s1_1_bclk_lrclk_mclk", mt6735_i2s1_1_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s1_data_0", mt6735_i2s1_data_0),
	PINCTRL_PIN_GROUP("i2s1_data_1", mt6735_i2s1_data_1),
	PINCTRL_PIN_GROUP("i2s2_0_bclk_lrclk_mclk", mt6735_i2s2_0_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s2_1_bclk_lrclk_mclk", mt6735_i2s2_1_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s2_data_in_0", mt6735_i2s2_data_in_0),
	PINCTRL_PIN_GROUP("i2s2_data_in_1", mt6735_i2s2_data_in_1),
	PINCTRL_PIN_GROUP("i2s3_0_bclk_lrclk_mclk", mt6735_i2s3_0_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s3_1_bclk_lrclk_mclk", mt6735_i2s3_1_bclk_lrclk_mclk),
	PINCTRL_PIN_GROUP("i2s3_data_0", mt6735_i2s1_data_0),
	PINCTRL_PIN_GROUP("i2s3_data_1", mt6735_i2s1_data_1),
	PINCTRL_PIN_GROUP("lcm_rst", mt6735_lcm_rst),
	PINCTRL_PIN_GROUP("mipi_tx", mt6735_mipi_tx),
	PINCTRL_PIN_GROUP("msdc0", mt6735_msdc0),
	PINCTRL_PIN_GROUP("msdc1", mt6735_msdc1),
	PINCTRL_PIN_GROUP("msdc2", mt6735_msdc2),
	PINCTRL_PIN_GROUP("pcm_clk_0", mt6735_pcm_clk_0),
	PINCTRL_PIN_GROUP("pcm_clk_1_0", mt6735_pcm_clk_1_0),
	PINCTRL_PIN_GROUP("pcm_clk_1_1", mt6735_pcm_clk_1_1),
	PINCTRL_PIN_GROUP("pcm_sync_0", mt6735_pcm_sync_0),
	PINCTRL_PIN_GROUP("pcm_sync_1_0", mt6735_pcm_sync_1_0),
	PINCTRL_PIN_GROUP("pcm_sync_1_1", mt6735_pcm_sync_1_1),
	PINCTRL_PIN_GROUP("pcm_rx_0", mt6735_pcm_rx_0),
	PINCTRL_PIN_GROUP("pcm_rx_1_0", mt6735_pcm_rx_1_0),
	PINCTRL_PIN_GROUP("pcm_rx_1_1", mt6735_pcm_rx_1_1),
	PINCTRL_PIN_GROUP("pcm_rx_1_2", mt6735_pcm_rx_1_2),
	PINCTRL_PIN_GROUP("pcm_tx_0", mt6735_pcm_tx_0),
	PINCTRL_PIN_GROUP("pcm_tx_1_0", mt6735_pcm_tx_1_0),
	PINCTRL_PIN_GROUP("pcm_tx_1_1", mt6735_pcm_tx_1_1),
	PINCTRL_PIN_GROUP("pcm_tx_1_2", mt6735_pcm_tx_1_2),
	PINCTRL_PIN_GROUP("pcm_tx_1_3", mt6735_pcm_tx_1_3),
	PINCTRL_PIN_GROUP("pcm_tx_1_4", mt6735_pcm_tx_1_4),
	PINCTRL_PIN_GROUP("pwm_ch1_0", mt6735_pwm_ch1_0),
	PINCTRL_PIN_GROUP("pwm_ch1_1", mt6735_pwm_ch1_1),
	PINCTRL_PIN_GROUP("pwm_ch1_2", mt6735_pwm_ch1_2),
	PINCTRL_PIN_GROUP("pwm_ch2_0", mt6735_pwm_ch2_0),
	PINCTRL_PIN_GROUP("pwm_ch2_1", mt6735_pwm_ch2_1),
	PINCTRL_PIN_GROUP("pwm_ch3_0", mt6735_pwm_ch3_0),
	PINCTRL_PIN_GROUP("pwm_ch3_1", mt6735_pwm_ch3_1),
	PINCTRL_PIN_GROUP("pwm_ch3_2", mt6735_pwm_ch3_2),
	PINCTRL_PIN_GROUP("pwm_ch4_0", mt6735_pwm_ch4_0),
	PINCTRL_PIN_GROUP("pwm_ch4_1", mt6735_pwm_ch4_1),
	PINCTRL_PIN_GROUP("pwm_ch4_2", mt6735_pwm_ch4_2),
	PINCTRL_PIN_GROUP("pwrap", mt6735_pwrap),
	PINCTRL_PIN_GROUP("rtc", mt6735_rtc),
	PINCTRL_PIN_GROUP("spi0", mt6735_spi0),
	PINCTRL_PIN_GROUP("spi1", mt6735_spi1),
	PINCTRL_PIN_GROUP("usb", mt6735_usb_host),
	PINCTRL_PIN_GROUP("uart0_0_txd_rxd",  mt6735_uart0_0_txd_rxd),
	PINCTRL_PIN_GROUP("uart0_1_txd_rxd",  mt6735_uart0_1_txd_rxd),
	PINCTRL_PIN_GROUP("uart0_2_txd_rxd",  mt6735_uart0_2_txd_rxd),
	PINCTRL_PIN_GROUP("uart0_3_txd_rxd",  mt6735_uart0_3_txd_rxd),
	PINCTRL_PIN_GROUP("uart0_4_txd_rxd",  mt6735_uart0_4_txd_rxd),
	PINCTRL_PIN_GROUP("uart1_0_txd_rxd",  mt6735_uart1_0_txd_rxd),
	PINCTRL_PIN_GROUP("uart1_1_txd_rxd",  mt6735_uart1_1_txd_rxd),
	PINCTRL_PIN_GROUP("uart1_2_txd_rxd",  mt6735_uart1_2_txd_rxd),
	PINCTRL_PIN_GROUP("uart2_0_txd_rxd",  mt6735_uart2_0_txd_rxd),
	PINCTRL_PIN_GROUP("uart2_1_txd_rxd",  mt6735_uart2_1_txd_rxd),
	PINCTRL_PIN_GROUP("uart3_0_txd_rxd",  mt6735_uart3_0_txd_rxd),
	PINCTRL_PIN_GROUP("uart3_1_txd_rxd",  mt6735_uart3_1_txd_rxd),
	PINCTRL_PIN_GROUP("uart3_2_txd_rxd",  mt6735_uart3_2_txd_rxd),
	PINCTRL_PIN_GROUP("uart3_3_txd_rxd",  mt6735_uart3_3_txd_rxd),
	PINCTRL_PIN_GROUP("uart0_rts_cts",  mt6735_uart0_rts_cts),
	PINCTRL_PIN_GROUP("uart1_rts_cts",  mt6735_uart1_rts_cts),
	PINCTRL_PIN_GROUP("uart2_rts_cts",  mt6735_uart2_rts_cts),
	PINCTRL_PIN_GROUP("uart3_rts_cts",  mt6735_uart3_rts_cts),
	PINCTRL_PIN_GROUP("watchdog_0", mt6735_watchdog_0),
	PINCTRL_PIN_GROUP("watchdog_1", mt6735_watchdog_1),
};

/* Joint those groups owning the same capability in user point of view which
 * allows that people tend to use through the device tree.
 */

static const char *const mt6735_disp_pwm_groups[] = { "disp_pwm_0",
						"disp_pwm_1",
						"disp_pwm_2", };
static const char *const mt6735_i2c_groups[] = { "i2c0", "i2c1_0", "i2c1_1",
						"i2c1_2", "i2c1_3", "i2c1_4",
						"i2c2_0", "i2c2_1", "i2c2_2",
						"i2c2_3", };
static const char *const mt6735_i2s_groups[] = { "i2s0_0_bclk_lrclk_mclk", "i2s0_1_bclk_lrclk_mclk", "i2s1_0_bclk_lrclk_mclk",
						"i2s1_1_bclk_lrclk_mclk",
                        "i2s2_0_bclk_lrclk_mclk",
                        "i2s2_1_bclk_lrclk_mclk",
						"i2s3_0_bclk_lrclk_mclk",
                        "i2s3_1_bclk_lrclk_mclk",
                        "i2s0_data_in_0', i2s0_data_in_1",
						"i2s1_data_0", "i2s1_data_1",
						"i2s2_data_in_0", "i2s2_data_in_1",
						"i2s3_data_0", "i2s3_data_1",};
static const char *const mt6735_lcd_groups[] = { "dsi_te", "lcm_rst",
						"mipi_tx", };
static const char *const mt6735_msdc_groups[] = { "msdc0", "msdc1",
						"msdc2", "msdc3", };

static const char *const mt6735_pcm_groups[] = { "pcm_clk_0", "pcm_clk_1_0",
						"pcm_clk_1_1", "pcm_sync_0",
						"pcm_sync_1_0", "pcm_sync_1_1",
						"pcm_rx_0", "pcm_rx_1_0",
						"pcm_rx_1_1", "pcm_rx_1_2", "pcm_tx_0",
						"pcm_tx_1_0", "pcm_tx_1_1",
						"pcm_tx_1_2", "pcm_tx_1_3",
						"pcm_tx_1_4", };
static const char *const mt6735_pwm_groups[] = { "pwm_ch1_0", "pwm_ch1_1",
						"pwm_ch1_2", "pwm_ch2_0",
						"pwm_ch2_1", "pwm_ch2_2",
						"pwm_ch3_0", "pwm_ch3_1",
						"pwm_ch3_2", "pwm_ch4_0",
						"pwm_ch4_1", "pwm_ch4_2",
						"pwm_ch4_3", "pwm_ch5_0",
						"pwm_ch5_1", "pwm_ch5_2",
						"pwm_ch6_0", "pwm_ch6_1",
						"pwm_ch6_2", "pwm_ch6_3",
						"pwm_ch7_0", "pwm_ch7_1",
						"pwm_ch7_2", };
static const char *const mt6735_pwrap_groups[] = { "pwrap", };
static const char *const mt6735_rtc_groups[] = { "rtc", };
static const char *const mt6735_spi_groups[] = { "spi0", "spi1", };
static const char *const mt6735_uart_groups[] = { "uart0_0_txd_rxd",
						"uart0_0_txd_rxd",
						"uart0_1_txd_rxd",
						"uart0_2_txd_rxd",
						"uart0_3_txd_rxd",
						"uart0_4_txd_rxd",
						"uart1_0_txd_rxd",
						"uart1_1_txd_rxd",
						"uart1_2_txd_rxd",
						"uart2_0_txd_rxd",
						"uart2_1_txd_rxd",
						"uart3_0_txd_rxd",
						"uart3_1_txd_rxd",
						"uart3_2_txd_rxd",
                        "uart3_3_txd_rxd",
                        "uart0_rts_cts",
                        "uart1_rts_cts",
                        "uart2_rts_cts",
                        "uart3_rts_cts", };

static const char *const  mt6735_usb_groups[] = { "usb", };
static const char *const mt6735_wdt_groups[] = { "watchdog_0", "watchdog_1", };

static const struct mtk_function_desc mt6735_functions[] = {
	{"disp", mt6735_disp_pwm_groups, ARRAY_SIZE(mt6735_disp_pwm_groups)},
	{"i2c", mt6735_i2c_groups, ARRAY_SIZE(mt6735_i2c_groups)},
	{"i2s",	mt6735_i2s_groups, ARRAY_SIZE(mt6735_i2s_groups)},
	{"usb",	mt6735_usb_groups, ARRAY_SIZE(mt6735_usb_groups)},
	{"lcd", mt6735_lcd_groups, ARRAY_SIZE(mt6735_lcd_groups)},
	{"msdc", mt6735_msdc_groups, ARRAY_SIZE(mt6735_msdc_groups)},
	{"pcm",	mt6735_pcm_groups, ARRAY_SIZE(mt6735_pcm_groups)},
	{"pwm",	mt6735_pwm_groups, ARRAY_SIZE(mt6735_pwm_groups)},
	{"pwrap", mt6735_pwrap_groups, ARRAY_SIZE(mt6735_pwrap_groups)},
	{"rtc", mt6735_rtc_groups, ARRAY_SIZE(mt6735_rtc_groups)},
	{"spi",	mt6735_spi_groups, ARRAY_SIZE(mt6735_spi_groups)},
	{"uart", mt6735_uart_groups, ARRAY_SIZE(mt6735_uart_groups)},
	{"watchdog", mt6735_wdt_groups, ARRAY_SIZE(mt6735_wdt_groups)},
};

static struct mtk_pinctrl_soc mt6735_data = {
	.name = "mt6735_pinctrl",
	.reg_cal = mt6735_reg_cals,
	.pins = mt6735_pins,
	.npins = ARRAY_SIZE(mt6735_pins),
	.grps = mt6735_groups,
	.ngrps = ARRAY_SIZE(mt6735_groups),
	.funcs = mt6735_functions,
	.nfuncs = ARRAY_SIZE(mt6735_functions),
	.gpio_mode = 0,
	.rev = MTK_PINCTRL_V1,
};

static int mtk_pinctrl_mt6735_probe(struct udevice *dev)
{
	int err;

	err = mtk_pinctrl_common_probe(dev, &mt6735_data);
	if (err)
		return err;


	return 0;
}

static const struct udevice_id mt6735_pctrl_match[] = {
	{ .compatible = "mediatek,mt6735-pinctrl", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt6735_pinctrl) = {
	.name = "mt6735_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mt6735_pctrl_match,
	.ops = &mtk_pinctrl_ops,
	.probe = mtk_pinctrl_mt6735_probe,
	.priv_auto	= sizeof(struct mtk_pinctrl_priv),
};
