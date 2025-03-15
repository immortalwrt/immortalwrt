// SPDX-License-Identifier: GPL-2.0
/*
 * The MT7986 driver based on Linux generic pinctrl binding.
 *
 * Copyright (C) 2020 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 */

#include "pinctrl-moore.h"

#define MT7986_PIN(_number, _name)				\
	MTK_PIN(_number, _name, 0, _number, DRV_GRP4)

#define PIN_FIELD_BASE(_s_pin, _e_pin, _i_base, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _i_base, _s_addr, _x_addrs, _s_bit,	\
		       _x_bits, 32, 0)

#define PINS_FIELD_BASE(_s_pin, _e_pin, _i_base, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _i_base, _s_addr, _x_addrs, _s_bit,	\
		      _x_bits, 32, 1)

static const struct mtk_pin_field_calc mt7986_pin_mode_range[] = {
	PIN_FIELD(0, 100, 0x300, 0x10, 0, 4),
};

static const struct mtk_pin_field_calc mt7986_pin_dir_range[] = {
	PIN_FIELD(0, 100, 0x0, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7986_pin_di_range[] = {
	PIN_FIELD(0, 100, 0x200, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7986_pin_do_range[] = {
	PIN_FIELD(0, 100, 0x100, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt7986_pin_ies_range[] = {
	PIN_FIELD_BASE(0, 0, 2, 0x40, 0x10, 17, 1),
	PIN_FIELD_BASE(1, 1, 3, 0x20, 0x10, 10, 1),
	PIN_FIELD_BASE(2, 2, 3, 0x20, 0x10, 11, 1),
	PIN_FIELD_BASE(3, 3, 4, 0x20, 0x10, 0, 1),
	PIN_FIELD_BASE(4, 4, 4, 0x20, 0x10, 1, 1),
	PIN_FIELD_BASE(5, 5, 2, 0x40, 0x10, 0, 1),
	PIN_FIELD_BASE(6, 6, 2, 0x40, 0x10, 1, 1),
	PIN_FIELD_BASE(7, 7, 3, 0x20, 0x10, 0, 1),
	PIN_FIELD_BASE(8, 8, 3, 0x20, 0x10, 1, 1),
	PIN_FIELD_BASE(9, 9, 3, 0x20, 0x10, 2, 1),
	PIN_FIELD_BASE(10, 10, 3, 0x20, 0x10, 3, 1),
	PIN_FIELD_BASE(11, 11, 2, 0x40, 0x10, 8, 1),
	PIN_FIELD_BASE(12, 12, 2, 0x40, 0x10, 9, 1),
	PIN_FIELD_BASE(13, 13, 2, 0x40, 0x10, 10, 1),
	PIN_FIELD_BASE(14, 14, 2, 0x40, 0x10, 11, 1),
	PIN_FIELD_BASE(15, 15, 2, 0x40, 0x10, 2, 1),
	PIN_FIELD_BASE(16, 16, 2, 0x40, 0x10, 3, 1),
	PIN_FIELD_BASE(17, 17, 2, 0x40, 0x10, 4, 1),
	PIN_FIELD_BASE(18, 18, 2, 0x40, 0x10, 5, 1),
	PIN_FIELD_BASE(19, 19, 2, 0x40, 0x10, 6, 1),
	PIN_FIELD_BASE(20, 20, 2, 0x40, 0x10, 7, 1),
	PIN_FIELD_BASE(21, 21, 1, 0x30, 0x10, 12, 1),
	PIN_FIELD_BASE(22, 22, 1, 0x30, 0x10, 13, 1),
	PIN_FIELD_BASE(23, 23, 1, 0x30, 0x10, 14, 1),
	PIN_FIELD_BASE(24, 24, 1, 0x30, 0x10, 18, 1),
	PIN_FIELD_BASE(25, 25, 1, 0x30, 0x10, 17, 1),
	PIN_FIELD_BASE(26, 26, 1, 0x30, 0x10, 15, 1),
	PIN_FIELD_BASE(27, 27, 1, 0x30, 0x10, 16, 1),
	PIN_FIELD_BASE(28, 28, 1, 0x30, 0x10, 19, 1),
	PIN_FIELD_BASE(29, 29, 1, 0x30, 0x10, 20, 1),
	PIN_FIELD_BASE(30, 30, 1, 0x30, 0x10, 23, 1),
	PIN_FIELD_BASE(31, 31, 1, 0x30, 0x10, 22, 1),
	PIN_FIELD_BASE(32, 32, 1, 0x30, 0x10, 21, 1),
	PIN_FIELD_BASE(33, 33, 3, 0x20, 0x10, 4, 1),
	PIN_FIELD_BASE(34, 34, 3, 0x20, 0x10, 8, 1),
	PIN_FIELD_BASE(35, 35, 3, 0x20, 0x10, 7, 1),
	PIN_FIELD_BASE(36, 36, 3, 0x20, 0x10, 5, 1),
	PIN_FIELD_BASE(37, 37, 3, 0x20, 0x10, 6, 1),
	PIN_FIELD_BASE(38, 38, 3, 0x20, 0x10, 9, 1),
	PIN_FIELD_BASE(39, 39, 2, 0x40, 0x10, 18, 1),
	PIN_FIELD_BASE(40, 40, 2, 0x40, 0x10, 19, 1),
	PIN_FIELD_BASE(41, 41, 2, 0x40, 0x10, 12, 1),
	PIN_FIELD_BASE(42, 42, 2, 0x40, 0x10, 22, 1),
	PIN_FIELD_BASE(43, 43, 2, 0x40, 0x10, 23, 1),
	PIN_FIELD_BASE(44, 44, 2, 0x40, 0x10, 20, 1),
	PIN_FIELD_BASE(45, 45, 2, 0x40, 0x10, 21, 1),
	PIN_FIELD_BASE(46, 46, 2, 0x40, 0x10, 26, 1),
	PIN_FIELD_BASE(47, 47, 2, 0x40, 0x10, 27, 1),
	PIN_FIELD_BASE(48, 48, 2, 0x40, 0x10, 24, 1),
	PIN_FIELD_BASE(49, 49, 2, 0x40, 0x10, 25, 1),
	PIN_FIELD_BASE(50, 50, 1, 0x30, 0x10, 2, 1),
	PIN_FIELD_BASE(51, 51, 1, 0x30, 0x10, 3, 1),
	PIN_FIELD_BASE(52, 52, 1, 0x30, 0x10, 4, 1),
	PIN_FIELD_BASE(53, 53, 1, 0x30, 0x10, 5, 1),
	PIN_FIELD_BASE(54, 54, 1, 0x30, 0x10, 6, 1),
	PIN_FIELD_BASE(55, 55, 1, 0x30, 0x10, 7, 1),
	PIN_FIELD_BASE(56, 56, 1, 0x30, 0x10, 8, 1),
	PIN_FIELD_BASE(57, 57, 1, 0x30, 0x10, 9, 1),
	PIN_FIELD_BASE(58, 58, 1, 0x30, 0x10, 1, 1),
	PIN_FIELD_BASE(59, 59, 1, 0x30, 0x10, 0, 1),
	PIN_FIELD_BASE(60, 60, 1, 0x30, 0x10, 10, 1),
	PIN_FIELD_BASE(61, 61, 1, 0x30, 0x10, 11, 1),
	PIN_FIELD_BASE(62, 62, 2, 0x40, 0x10, 15, 1),
	PIN_FIELD_BASE(63, 63, 2, 0x40, 0x10, 14, 1),
	PIN_FIELD_BASE(64, 64, 2, 0x40, 0x10, 13, 1),
	PIN_FIELD_BASE(65, 65, 2, 0x40, 0x10, 16, 1),
	PIN_FIELD_BASE(66, 66, 4, 0x20, 0x10, 2, 1),
	PIN_FIELD_BASE(67, 67, 4, 0x20, 0x10, 3, 1),
	PIN_FIELD_BASE(68, 68, 4, 0x20, 0x10, 4, 1),
	PIN_FIELD_BASE(69, 69, 5, 0x30, 0x10, 1, 1),
	PIN_FIELD_BASE(70, 70, 5, 0x30, 0x10, 0, 1),
	PIN_FIELD_BASE(71, 71, 5, 0x30, 0x10, 16, 1),
	PIN_FIELD_BASE(72, 72, 5, 0x30, 0x10, 14, 1),
	PIN_FIELD_BASE(73, 73, 5, 0x30, 0x10, 15, 1),
	PIN_FIELD_BASE(74, 74, 5, 0x30, 0x10, 4, 1),
	PIN_FIELD_BASE(75, 75, 5, 0x30, 0x10, 6, 1),
	PIN_FIELD_BASE(76, 76, 5, 0x30, 0x10, 7, 1),
	PIN_FIELD_BASE(77, 77, 5, 0x30, 0x10, 8, 1),
	PIN_FIELD_BASE(78, 78, 5, 0x30, 0x10, 2, 1),
	PIN_FIELD_BASE(79, 79, 5, 0x30, 0x10, 3, 1),
	PIN_FIELD_BASE(80, 80, 5, 0x30, 0x10, 9, 1),
	PIN_FIELD_BASE(81, 81, 5, 0x30, 0x10, 10, 1),
	PIN_FIELD_BASE(82, 82, 5, 0x30, 0x10, 11, 1),
	PIN_FIELD_BASE(83, 83, 5, 0x30, 0x10, 12, 1),
	PIN_FIELD_BASE(84, 84, 5, 0x30, 0x10, 13, 1),
	PIN_FIELD_BASE(85, 85, 5, 0x30, 0x10, 5, 1),
	PIN_FIELD_BASE(86, 86, 6, 0x30, 0x10, 1, 1),
	PIN_FIELD_BASE(87, 87, 6, 0x30, 0x10, 0, 1),
	PIN_FIELD_BASE(88, 88, 6, 0x30, 0x10, 14, 1),
	PIN_FIELD_BASE(89, 89, 6, 0x30, 0x10, 12, 1),
	PIN_FIELD_BASE(90, 90, 6, 0x30, 0x10, 13, 1),
	PIN_FIELD_BASE(91, 91, 6, 0x30, 0x10, 4, 1),
	PIN_FIELD_BASE(92, 92, 6, 0x30, 0x10, 5, 1),
	PIN_FIELD_BASE(93, 93, 6, 0x30, 0x10, 6, 1),
	PIN_FIELD_BASE(94, 94, 6, 0x30, 0x10, 7, 1),
	PIN_FIELD_BASE(95, 95, 6, 0x30, 0x10, 2, 1),
	PIN_FIELD_BASE(96, 96, 6, 0x30, 0x10, 3, 1),
	PIN_FIELD_BASE(97, 97, 6, 0x30, 0x10, 8, 1),
	PIN_FIELD_BASE(98, 98, 6, 0x30, 0x10, 9, 1),
	PIN_FIELD_BASE(99, 99, 6, 0x30, 0x10, 10, 1),
	PIN_FIELD_BASE(100, 100, 6, 0x30, 0x10, 11, 1),
};

static const struct mtk_pin_field_calc mt7986_pin_smt_range[] = {
	PIN_FIELD_BASE(0, 0, 2, 0xf0, 0x10, 17, 1),
	PIN_FIELD_BASE(1, 1, 3, 0x90, 0x10, 10, 1),
	PIN_FIELD_BASE(2, 2, 3, 0x90, 0x10, 11, 1),
	PIN_FIELD_BASE(3, 3, 4, 0x90, 0x10, 0, 1),
	PIN_FIELD_BASE(4, 4, 4, 0x90, 0x10, 1, 1),
	PIN_FIELD_BASE(5, 5, 2, 0xf0, 0x10, 0, 1),
	PIN_FIELD_BASE(6, 6, 2, 0xf0, 0x10, 1, 1),
	PIN_FIELD_BASE(7, 7, 3, 0x90, 0x10, 0, 1),
	PIN_FIELD_BASE(8, 8, 3, 0x90, 0x10, 1, 1),
	PIN_FIELD_BASE(9, 9, 3, 0x90, 0x10, 2, 1),
	PIN_FIELD_BASE(10, 10, 3, 0x90, 0x10, 3, 1),
	PIN_FIELD_BASE(11, 11, 2, 0xf0, 0x10, 8, 1),
	PIN_FIELD_BASE(12, 12, 2, 0xf0, 0x10, 9, 1),
	PIN_FIELD_BASE(13, 13, 2, 0xf0, 0x10, 10, 1),
	PIN_FIELD_BASE(14, 14, 2, 0xf0, 0x10, 11, 1),
	PIN_FIELD_BASE(15, 15, 2, 0xf0, 0x10, 2, 1),
	PIN_FIELD_BASE(16, 16, 2, 0xf0, 0x10, 3, 1),
	PIN_FIELD_BASE(17, 17, 2, 0xf0, 0x10, 4, 1),
	PIN_FIELD_BASE(18, 18, 2, 0xf0, 0x10, 5, 1),
	PIN_FIELD_BASE(19, 19, 2, 0xf0, 0x10, 6, 1),
	PIN_FIELD_BASE(20, 20, 2, 0xf0, 0x10, 7, 1),
	PIN_FIELD_BASE(21, 21, 1, 0xc0, 0x10, 12, 1),
	PIN_FIELD_BASE(22, 22, 1, 0xc0, 0x10, 13, 1),
	PIN_FIELD_BASE(23, 23, 1, 0xc0, 0x10, 14, 1),
	PIN_FIELD_BASE(24, 24, 1, 0xc0, 0x10, 18, 1),
	PIN_FIELD_BASE(25, 25, 1, 0xc0, 0x10, 17, 1),
	PIN_FIELD_BASE(26, 26, 1, 0xc0, 0x10, 15, 1),
	PIN_FIELD_BASE(27, 27, 1, 0xc0, 0x10, 16, 1),
	PIN_FIELD_BASE(28, 28, 1, 0xc0, 0x10, 19, 1),
	PIN_FIELD_BASE(29, 29, 1, 0xc0, 0x10, 20, 1),
	PIN_FIELD_BASE(30, 30, 1, 0xc0, 0x10, 23, 1),
	PIN_FIELD_BASE(31, 31, 1, 0xc0, 0x10, 22, 1),
	PIN_FIELD_BASE(32, 32, 1, 0xc0, 0x10, 21, 1),
	PIN_FIELD_BASE(33, 33, 3, 0x90, 0x10, 4, 1),
	PIN_FIELD_BASE(34, 34, 3, 0x90, 0x10, 8, 1),
	PIN_FIELD_BASE(35, 35, 3, 0x90, 0x10, 7, 1),
	PIN_FIELD_BASE(36, 36, 3, 0x90, 0x10, 5, 1),
	PIN_FIELD_BASE(37, 37, 3, 0x90, 0x10, 6, 1),
	PIN_FIELD_BASE(38, 38, 3, 0x90, 0x10, 9, 1),
	PIN_FIELD_BASE(39, 39, 2, 0xf0, 0x10, 18, 1),
	PIN_FIELD_BASE(40, 40, 2, 0xf0, 0x10, 19, 1),
	PIN_FIELD_BASE(41, 41, 2, 0xf0, 0x10, 12, 1),
	PIN_FIELD_BASE(42, 42, 2, 0xf0, 0x10, 22, 1),
	PIN_FIELD_BASE(43, 43, 2, 0xf0, 0x10, 23, 1),
	PIN_FIELD_BASE(44, 44, 2, 0xf0, 0x10, 20, 1),
	PIN_FIELD_BASE(45, 45, 2, 0xf0, 0x10, 21, 1),
	PIN_FIELD_BASE(46, 46, 2, 0xf0, 0x10, 26, 1),
	PIN_FIELD_BASE(47, 47, 2, 0xf0, 0x10, 27, 1),
	PIN_FIELD_BASE(48, 48, 2, 0xf0, 0x10, 24, 1),
	PIN_FIELD_BASE(49, 49, 2, 0xf0, 0x10, 25, 1),
	PIN_FIELD_BASE(50, 50, 1, 0xc0, 0x10, 2, 1),
	PIN_FIELD_BASE(51, 51, 1, 0xc0, 0x10, 3, 1),
	PIN_FIELD_BASE(52, 52, 1, 0xc0, 0x10, 4, 1),
	PIN_FIELD_BASE(53, 53, 1, 0xc0, 0x10, 5, 1),
	PIN_FIELD_BASE(54, 54, 1, 0xc0, 0x10, 6, 1),
	PIN_FIELD_BASE(55, 55, 1, 0xc0, 0x10, 7, 1),
	PIN_FIELD_BASE(56, 56, 1, 0xc0, 0x10, 8, 1),
	PIN_FIELD_BASE(57, 57, 1, 0xc0, 0x10, 9, 1),
	PIN_FIELD_BASE(58, 58, 1, 0xc0, 0x10, 1, 1),
	PIN_FIELD_BASE(59, 59, 1, 0xc0, 0x10, 0, 1),
	PIN_FIELD_BASE(60, 60, 1, 0xc0, 0x10, 10, 1),
	PIN_FIELD_BASE(61, 61, 1, 0xc0, 0x10, 11, 1),
	PIN_FIELD_BASE(62, 62, 2, 0xf0, 0x10, 15, 1),
	PIN_FIELD_BASE(63, 63, 2, 0xf0, 0x10, 14, 1),
	PIN_FIELD_BASE(64, 64, 2, 0xf0, 0x10, 13, 1),
	PIN_FIELD_BASE(65, 65, 2, 0xf0, 0x10, 16, 1),
	PIN_FIELD_BASE(66, 66, 4, 0x90, 0x10, 2, 1),
	PIN_FIELD_BASE(67, 67, 4, 0x90, 0x10, 3, 1),
	PIN_FIELD_BASE(68, 68, 4, 0x90, 0x10, 4, 1),
	PIN_FIELD_BASE(69, 69, 5, 0x80, 0x10, 1, 1),
	PIN_FIELD_BASE(70, 70, 5, 0x80, 0x10, 0, 1),
	PIN_FIELD_BASE(71, 71, 5, 0x80, 0x10, 16, 1),
	PIN_FIELD_BASE(72, 72, 5, 0x80, 0x10, 14, 1),
	PIN_FIELD_BASE(73, 73, 5, 0x80, 0x10, 15, 1),
	PIN_FIELD_BASE(74, 74, 5, 0x80, 0x10, 4, 1),
	PIN_FIELD_BASE(75, 75, 5, 0x80, 0x10, 6, 1),
	PIN_FIELD_BASE(76, 76, 5, 0x80, 0x10, 7, 1),
	PIN_FIELD_BASE(77, 77, 5, 0x80, 0x10, 8, 1),
	PIN_FIELD_BASE(78, 78, 5, 0x80, 0x10, 2, 1),
	PIN_FIELD_BASE(79, 79, 5, 0x80, 0x10, 3, 1),
	PIN_FIELD_BASE(80, 80, 5, 0x80, 0x10, 9, 1),
	PIN_FIELD_BASE(81, 81, 5, 0x80, 0x10, 10, 1),
	PIN_FIELD_BASE(82, 82, 5, 0x80, 0x10, 11, 1),
	PIN_FIELD_BASE(83, 83, 5, 0x80, 0x10, 12, 1),
	PIN_FIELD_BASE(84, 84, 5, 0x80, 0x10, 13, 1),
	PIN_FIELD_BASE(85, 85, 5, 0x80, 0x10, 5, 1),
	PIN_FIELD_BASE(86, 86, 6, 0x70, 0x10, 1, 1),
	PIN_FIELD_BASE(87, 87, 6, 0x70, 0x10, 0, 1),
	PIN_FIELD_BASE(88, 88, 6, 0x70, 0x10, 14, 1),
	PIN_FIELD_BASE(89, 89, 6, 0x70, 0x10, 12, 1),
	PIN_FIELD_BASE(90, 90, 6, 0x70, 0x10, 13, 1),
	PIN_FIELD_BASE(91, 91, 6, 0x70, 0x10, 4, 1),
	PIN_FIELD_BASE(92, 92, 6, 0x70, 0x10, 5, 1),
	PIN_FIELD_BASE(93, 93, 6, 0x70, 0x10, 6, 1),
	PIN_FIELD_BASE(94, 94, 6, 0x70, 0x10, 7, 1),
	PIN_FIELD_BASE(95, 95, 6, 0x70, 0x10, 2, 1),
	PIN_FIELD_BASE(96, 96, 6, 0x70, 0x10, 3, 1),
	PIN_FIELD_BASE(97, 97, 6, 0x70, 0x10, 8, 1),
	PIN_FIELD_BASE(98, 98, 6, 0x70, 0x10, 9, 1),
	PIN_FIELD_BASE(99, 99, 6, 0x70, 0x10, 10, 1),
	PIN_FIELD_BASE(100, 100, 6, 0x70, 0x10, 11, 1),
};

static const struct mtk_pin_field_calc mt7986_pin_pu_range[] = {
	PIN_FIELD_BASE(69, 69, 5, 0x50, 0x10, 1, 1),
	PIN_FIELD_BASE(70, 70, 5, 0x50, 0x10, 0, 1),
	PIN_FIELD_BASE(71, 71, 5, 0x50, 0x10, 16, 1),
	PIN_FIELD_BASE(72, 72, 5, 0x50, 0x10, 14, 1),
	PIN_FIELD_BASE(73, 73, 5, 0x50, 0x10, 15, 1),
	PIN_FIELD_BASE(74, 74, 5, 0x50, 0x10, 4, 1),
	PIN_FIELD_BASE(75, 75, 5, 0x50, 0x10, 6, 1),
	PIN_FIELD_BASE(76, 76, 5, 0x50, 0x10, 7, 1),
	PIN_FIELD_BASE(77, 77, 5, 0x50, 0x10, 8, 1),
	PIN_FIELD_BASE(78, 78, 5, 0x50, 0x10, 2, 1),
	PIN_FIELD_BASE(79, 79, 5, 0x50, 0x10, 3, 1),
	PIN_FIELD_BASE(80, 80, 5, 0x50, 0x10, 9, 1),
	PIN_FIELD_BASE(81, 81, 5, 0x50, 0x10, 10, 1),
	PIN_FIELD_BASE(82, 82, 5, 0x50, 0x10, 11, 1),
	PIN_FIELD_BASE(83, 83, 5, 0x50, 0x10, 12, 1),
	PIN_FIELD_BASE(84, 84, 5, 0x50, 0x10, 13, 1),
	PIN_FIELD_BASE(85, 85, 5, 0x50, 0x10, 5, 1),
	PIN_FIELD_BASE(86, 86, 6, 0x50, 0x10, 1, 1),
	PIN_FIELD_BASE(87, 87, 6, 0x50, 0x10, 0, 1),
	PIN_FIELD_BASE(88, 88, 6, 0x50, 0x10, 14, 1),
	PIN_FIELD_BASE(89, 89, 6, 0x50, 0x10, 12, 1),
	PIN_FIELD_BASE(90, 90, 6, 0x50, 0x10, 13, 1),
	PIN_FIELD_BASE(91, 91, 6, 0x50, 0x10, 4, 1),
	PIN_FIELD_BASE(92, 92, 6, 0x50, 0x10, 5, 1),
	PIN_FIELD_BASE(93, 93, 6, 0x50, 0x10, 6, 1),
	PIN_FIELD_BASE(94, 94, 6, 0x50, 0x10, 7, 1),
	PIN_FIELD_BASE(95, 95, 6, 0x50, 0x10, 2, 1),
	PIN_FIELD_BASE(96, 96, 6, 0x50, 0x10, 3, 1),
	PIN_FIELD_BASE(97, 97, 6, 0x50, 0x10, 8, 1),
	PIN_FIELD_BASE(98, 98, 6, 0x50, 0x10, 9, 1),
	PIN_FIELD_BASE(99, 99, 6, 0x50, 0x10, 10, 1),
	PIN_FIELD_BASE(100, 100, 6, 0x50, 0x10, 11, 1),
};

static const struct mtk_pin_field_calc mt7986_pin_pd_range[] = {
	PIN_FIELD_BASE(69, 69, 5, 0x40, 0x10, 1, 1),
	PIN_FIELD_BASE(70, 70, 5, 0x40, 0x10, 0, 1),
	PIN_FIELD_BASE(71, 71, 5, 0x40, 0x10, 16, 1),
	PIN_FIELD_BASE(72, 72, 5, 0x40, 0x10, 14, 1),
	PIN_FIELD_BASE(73, 73, 5, 0x40, 0x10, 15, 1),
	PIN_FIELD_BASE(74, 74, 5, 0x40, 0x10, 4, 1),
	PIN_FIELD_BASE(75, 75, 5, 0x40, 0x10, 6, 1),
	PIN_FIELD_BASE(76, 76, 5, 0x40, 0x10, 7, 1),
	PIN_FIELD_BASE(77, 77, 5, 0x40, 0x10, 8, 1),
	PIN_FIELD_BASE(78, 78, 5, 0x40, 0x10, 2, 1),
	PIN_FIELD_BASE(79, 79, 5, 0x40, 0x10, 3, 1),
	PIN_FIELD_BASE(80, 80, 5, 0x40, 0x10, 9, 1),
	PIN_FIELD_BASE(81, 81, 5, 0x40, 0x10, 10, 1),
	PIN_FIELD_BASE(82, 82, 5, 0x40, 0x10, 11, 1),
	PIN_FIELD_BASE(83, 83, 5, 0x40, 0x10, 12, 1),
	PIN_FIELD_BASE(84, 84, 5, 0x40, 0x10, 13, 1),
	PIN_FIELD_BASE(85, 85, 5, 0x40, 0x10, 5, 1),
	PIN_FIELD_BASE(86, 86, 6, 0x40, 0x10, 1, 1),
	PIN_FIELD_BASE(87, 87, 6, 0x40, 0x10, 0, 1),
	PIN_FIELD_BASE(88, 88, 6, 0x40, 0x10, 14, 1),
	PIN_FIELD_BASE(89, 89, 6, 0x40, 0x10, 12, 1),
	PIN_FIELD_BASE(90, 90, 6, 0x40, 0x10, 13, 1),
	PIN_FIELD_BASE(91, 91, 6, 0x40, 0x10, 4, 1),
	PIN_FIELD_BASE(92, 92, 6, 0x40, 0x10, 5, 1),
	PIN_FIELD_BASE(93, 93, 6, 0x40, 0x10, 6, 1),
	PIN_FIELD_BASE(94, 94, 6, 0x40, 0x10, 7, 1),
	PIN_FIELD_BASE(95, 95, 6, 0x40, 0x10, 2, 1),
	PIN_FIELD_BASE(96, 96, 6, 0x40, 0x10, 3, 1),
	PIN_FIELD_BASE(97, 97, 6, 0x40, 0x10, 8, 1),
	PIN_FIELD_BASE(98, 98, 6, 0x40, 0x10, 9, 1),
	PIN_FIELD_BASE(99, 99, 6, 0x40, 0x10, 10, 1),
	PIN_FIELD_BASE(100, 100, 6, 0x40, 0x10, 11, 1),
};

static const struct mtk_pin_field_calc mt7986_pin_drv_range[] = {
	PIN_FIELD_BASE(0, 0, 2, 0x10, 0x10, 21, 3),
	PIN_FIELD_BASE(1, 1, 3, 0x10, 0x10, 0, 3),
	PIN_FIELD_BASE(2, 2, 3, 0x10, 0x10, 3, 3),
	PIN_FIELD_BASE(3, 3, 4, 0x00, 0x10, 0, 1),
	PIN_FIELD_BASE(4, 4, 4, 0x00, 0x10, 1, 1),
	PIN_FIELD_BASE(5, 5, 2, 0x00, 0x10, 0, 3),
	PIN_FIELD_BASE(6, 6, 2, 0x00, 0x10, 21, 3),
	PIN_FIELD_BASE(7, 7, 3, 0x00, 0x10, 0, 3),
	PIN_FIELD_BASE(8, 8, 3, 0x00, 0x10, 3, 3),
	PIN_FIELD_BASE(9, 9, 3, 0x00, 0x10, 6, 3),
	PIN_FIELD_BASE(10, 10, 3, 0x00, 0x10, 9, 3),
	PIN_FIELD_BASE(11, 11, 2, 0x00, 0x10, 24, 3),
	PIN_FIELD_BASE(12, 12, 2, 0x00, 0x10, 27, 3),
	PIN_FIELD_BASE(13, 13, 2, 0x10, 0x10, 0, 3),
	PIN_FIELD_BASE(14, 14, 2, 0x10, 0x10, 3, 3),
	PIN_FIELD_BASE(15, 15, 2, 0x00, 0x10, 3, 3),
	PIN_FIELD_BASE(16, 16, 2, 0x00, 0x10, 6, 3),
	PIN_FIELD_BASE(17, 17, 2, 0x00, 0x10, 9, 3),
	PIN_FIELD_BASE(18, 18, 2, 0x00, 0x10, 12, 3),
	PIN_FIELD_BASE(19, 19, 2, 0x00, 0x10, 15, 3),
	PIN_FIELD_BASE(20, 20, 2, 0x00, 0x10, 18, 3),
	PIN_FIELD_BASE(21, 21, 1, 0x10, 0x10, 6, 3),
	PIN_FIELD_BASE(22, 22, 1, 0x10, 0x10, 9, 3),
	PIN_FIELD_BASE(23, 23, 1, 0x10, 0x10, 12, 3),
	PIN_FIELD_BASE(24, 24, 1, 0x10, 0x10, 24, 3),
	PIN_FIELD_BASE(25, 25, 1, 0x10, 0x10, 21, 3),
	PIN_FIELD_BASE(26, 26, 1, 0x10, 0x10, 15, 3),
	PIN_FIELD_BASE(27, 27, 1, 0x10, 0x10, 18, 3),
	PIN_FIELD_BASE(28, 28, 1, 0x10, 0x10, 27, 3),
	PIN_FIELD_BASE(29, 29, 1, 0x20, 0x10, 0, 3),
	PIN_FIELD_BASE(30, 30, 1, 0x20, 0x10, 9, 3),
	PIN_FIELD_BASE(31, 31, 1, 0x20, 0x10, 6, 3),
	PIN_FIELD_BASE(32, 32, 1, 0x20, 0x10, 3, 3),
	PIN_FIELD_BASE(33, 33, 3, 0x00, 0x10, 12, 3),
	PIN_FIELD_BASE(34, 34, 3, 0x00, 0x10, 24, 3),
	PIN_FIELD_BASE(35, 35, 3, 0x00, 0x10, 21, 3),
	PIN_FIELD_BASE(36, 36, 3, 0x00, 0x10, 15, 3),
	PIN_FIELD_BASE(37, 37, 3, 0x00, 0x10, 18, 3),
	PIN_FIELD_BASE(38, 38, 3, 0x00, 0x10, 27, 3),
	PIN_FIELD_BASE(39, 39, 2, 0x10, 0x10, 27, 3),
	PIN_FIELD_BASE(40, 40, 2, 0x20, 0x10, 0, 3),
	PIN_FIELD_BASE(41, 41, 2, 0x10, 0x10, 6, 3),
	PIN_FIELD_BASE(42, 42, 2, 0x20, 0x10, 9, 3),
	PIN_FIELD_BASE(43, 43, 2, 0x20, 0x10, 12, 3),
	PIN_FIELD_BASE(44, 44, 2, 0x20, 0x10, 3, 3),
	PIN_FIELD_BASE(45, 45, 2, 0x20, 0x10, 6, 3),
	PIN_FIELD_BASE(46, 46, 2, 0x20, 0x10, 21, 3),
	PIN_FIELD_BASE(47, 47, 2, 0x20, 0x10, 24, 3),
	PIN_FIELD_BASE(48, 48, 2, 0x20, 0x10, 15, 3),
	PIN_FIELD_BASE(49, 49, 2, 0x20, 0x10, 18, 3),
	PIN_FIELD_BASE(50, 50, 1, 0x00, 0x10, 6, 3),
	PIN_FIELD_BASE(51, 51, 1, 0x00, 0x10, 9, 3),
	PIN_FIELD_BASE(52, 52, 1, 0x00, 0x10, 12, 3),
	PIN_FIELD_BASE(53, 53, 1, 0x00, 0x10, 15, 3),
	PIN_FIELD_BASE(54, 54, 1, 0x00, 0x10, 18, 3),
	PIN_FIELD_BASE(55, 55, 1, 0x00, 0x10, 21, 3),
	PIN_FIELD_BASE(56, 56, 1, 0x00, 0x10, 24, 3),
	PIN_FIELD_BASE(57, 57, 1, 0x00, 0x10, 27, 3),
	PIN_FIELD_BASE(58, 58, 1, 0x00, 0x10, 3, 3),
	PIN_FIELD_BASE(59, 59, 1, 0x00, 0x10, 0, 3),
	PIN_FIELD_BASE(60, 60, 1, 0x10, 0x10, 0, 3),
	PIN_FIELD_BASE(61, 61, 1, 0x10, 0x10, 3, 3),
	PIN_FIELD_BASE(62, 62, 2, 0x10, 0x10, 15, 3),
	PIN_FIELD_BASE(63, 63, 2, 0x10, 0x10, 12, 3),
	PIN_FIELD_BASE(64, 64, 2, 0x10, 0x10, 9, 3),
	PIN_FIELD_BASE(65, 65, 2, 0x10, 0x10, 18, 3),
	PIN_FIELD_BASE(66, 66, 4, 0x00, 0x10, 2, 3),
	PIN_FIELD_BASE(67, 67, 4, 0x00, 0x10, 5, 3),
	PIN_FIELD_BASE(68, 68, 4, 0x00, 0x10, 8, 3),
	PIN_FIELD_BASE(69, 69, 5, 0x00, 0x10, 3, 3),
	PIN_FIELD_BASE(70, 70, 5, 0x00, 0x10, 0, 3),
	PIN_FIELD_BASE(71, 71, 5, 0x10, 0x10, 18, 3),
	PIN_FIELD_BASE(72, 72, 5, 0x10, 0x10, 12, 3),
	PIN_FIELD_BASE(73, 73, 5, 0x10, 0x10, 15, 3),
	PIN_FIELD_BASE(74, 74, 5, 0x00, 0x10, 15, 3),
	PIN_FIELD_BASE(75, 75, 5, 0x00, 0x10, 18, 3),
	PIN_FIELD_BASE(76, 76, 5, 0x00, 0x10, 21, 3),
	PIN_FIELD_BASE(77, 77, 5, 0x00, 0x10, 24, 3),
	PIN_FIELD_BASE(78, 78, 5, 0x00, 0x10, 6, 3),
	PIN_FIELD_BASE(79, 79, 5, 0x00, 0x10, 9, 3),
	PIN_FIELD_BASE(80, 80, 5, 0x00, 0x10, 27, 3),
	PIN_FIELD_BASE(81, 81, 5, 0x10, 0x10, 0, 3),
	PIN_FIELD_BASE(82, 82, 5, 0x10, 0x10, 3, 3),
	PIN_FIELD_BASE(83, 83, 5, 0x10, 0x10, 6, 3),
	PIN_FIELD_BASE(84, 84, 5, 0x10, 0x10, 9, 3),
	PIN_FIELD_BASE(85, 85, 5, 0x00, 0x10, 12, 3),
	PIN_FIELD_BASE(86, 86, 6, 0x00, 0x10, 3, 3),
	PIN_FIELD_BASE(87, 87, 6, 0x00, 0x10, 0, 3),
	PIN_FIELD_BASE(88, 88, 6, 0x10, 0x10, 12, 3),
	PIN_FIELD_BASE(89, 89, 6, 0x10, 0x10, 6, 3),
	PIN_FIELD_BASE(90, 90, 6, 0x10, 0x10, 9, 3),
	PIN_FIELD_BASE(91, 91, 6, 0x00, 0x10, 12, 3),
	PIN_FIELD_BASE(92, 92, 6, 0x00, 0x10, 15, 3),
	PIN_FIELD_BASE(93, 93, 6, 0x00, 0x10, 18, 3),
	PIN_FIELD_BASE(94, 94, 6, 0x00, 0x10, 21, 3),
	PIN_FIELD_BASE(95, 95, 6, 0x00, 0x10, 6, 3),
	PIN_FIELD_BASE(96, 96, 6, 0x00, 0x10, 9, 3),
	PIN_FIELD_BASE(97, 97, 6, 0x00, 0x10, 24, 3),
	PIN_FIELD_BASE(98, 98, 6, 0x00, 0x10, 27, 3),
	PIN_FIELD_BASE(99, 99, 6, 0x10, 0x10, 2, 3),
	PIN_FIELD_BASE(100, 100, 6, 0x10, 0x10, 5, 3),
};

static const struct mtk_pin_field_calc mt7986_pin_pupd_range[] = {
	PIN_FIELD_BASE(0, 0, 2, 0x60, 0x10, 17, 1),
	PIN_FIELD_BASE(1, 1, 3, 0x30, 0x10, 10, 1),
	PIN_FIELD_BASE(2, 2, 3, 0x30, 0x10, 11, 1),
	PIN_FIELD_BASE(3, 3, 4, 0x40, 0x10, 0, 1),
	PIN_FIELD_BASE(4, 4, 4, 0x40, 0x10, 1, 1),
	PIN_FIELD_BASE(5, 5, 2, 0x60, 0x10, 0, 1),
	PIN_FIELD_BASE(6, 6, 2, 0x60, 0x10, 1, 1),
	PIN_FIELD_BASE(7, 7, 3, 0x30, 0x10, 0, 1),
	PIN_FIELD_BASE(8, 8, 3, 0x30, 0x10, 1, 1),
	PIN_FIELD_BASE(9, 9, 3, 0x30, 0x10, 2, 1),
	PIN_FIELD_BASE(10, 10, 3, 0x30, 0x10, 3, 1),
	PIN_FIELD_BASE(11, 11, 2, 0x60, 0x10, 8, 1),
	PIN_FIELD_BASE(12, 12, 2, 0x60, 0x10, 9, 1),
	PIN_FIELD_BASE(13, 13, 2, 0x60, 0x10, 10, 1),
	PIN_FIELD_BASE(14, 14, 2, 0x60, 0x10, 11, 1),
	PIN_FIELD_BASE(15, 15, 2, 0x60, 0x10, 2, 1),
	PIN_FIELD_BASE(16, 16, 2, 0x60, 0x10, 3, 1),
	PIN_FIELD_BASE(17, 17, 2, 0x60, 0x10, 4, 1),
	PIN_FIELD_BASE(18, 18, 2, 0x60, 0x10, 5, 1),
	PIN_FIELD_BASE(19, 19, 2, 0x60, 0x10, 6, 1),
	PIN_FIELD_BASE(20, 20, 2, 0x60, 0x10, 7, 1),
	PIN_FIELD_BASE(21, 21, 1, 0x40, 0x10, 12, 1),
	PIN_FIELD_BASE(22, 22, 1, 0x40, 0x10, 13, 1),
	PIN_FIELD_BASE(23, 23, 1, 0x40, 0x10, 14, 1),
	PIN_FIELD_BASE(24, 24, 1, 0x40, 0x10, 18, 1),
	PIN_FIELD_BASE(25, 25, 1, 0x40, 0x10, 17, 1),
	PIN_FIELD_BASE(26, 26, 1, 0x40, 0x10, 15, 1),
	PIN_FIELD_BASE(27, 27, 1, 0x40, 0x10, 16, 1),
	PIN_FIELD_BASE(28, 28, 1, 0x40, 0x10, 19, 1),
	PIN_FIELD_BASE(29, 29, 1, 0x40, 0x10, 20, 1),
	PIN_FIELD_BASE(30, 30, 1, 0x40, 0x10, 23, 1),
	PIN_FIELD_BASE(31, 31, 1, 0x40, 0x10, 22, 1),
	PIN_FIELD_BASE(32, 32, 1, 0x40, 0x10, 21, 1),
	PIN_FIELD_BASE(33, 33, 3, 0x30, 0x10, 4, 1),
	PIN_FIELD_BASE(34, 34, 3, 0x30, 0x10, 8, 1),
	PIN_FIELD_BASE(35, 35, 3, 0x30, 0x10, 7, 1),
	PIN_FIELD_BASE(36, 36, 3, 0x30, 0x10, 5, 1),
	PIN_FIELD_BASE(37, 37, 3, 0x30, 0x10, 6, 1),
	PIN_FIELD_BASE(38, 38, 3, 0x30, 0x10, 9, 1),
	PIN_FIELD_BASE(39, 39, 2, 0x60, 0x10, 18, 1),
	PIN_FIELD_BASE(40, 40, 2, 0x60, 0x10, 19, 1),
	PIN_FIELD_BASE(41, 41, 2, 0x60, 0x10, 12, 1),
	PIN_FIELD_BASE(42, 42, 2, 0x60, 0x10, 23, 1),
	PIN_FIELD_BASE(43, 43, 2, 0x60, 0x10, 24, 1),
	PIN_FIELD_BASE(44, 44, 2, 0x60, 0x10, 21, 1),
	PIN_FIELD_BASE(45, 45, 2, 0x60, 0x10, 22, 1),
	PIN_FIELD_BASE(46, 46, 2, 0x60, 0x10, 27, 1),
	PIN_FIELD_BASE(47, 47, 2, 0x60, 0x10, 28, 1),
	PIN_FIELD_BASE(48, 48, 2, 0x60, 0x10, 25, 1),
	PIN_FIELD_BASE(49, 49, 2, 0x60, 0x10, 26, 1),
	PIN_FIELD_BASE(50, 50, 1, 0x40, 0x10, 2, 1),
	PIN_FIELD_BASE(51, 51, 1, 0x40, 0x10, 3, 1),
	PIN_FIELD_BASE(52, 52, 1, 0x40, 0x10, 4, 1),
	PIN_FIELD_BASE(53, 53, 1, 0x40, 0x10, 5, 1),
	PIN_FIELD_BASE(54, 54, 1, 0x40, 0x10, 6, 1),
	PIN_FIELD_BASE(55, 55, 1, 0x40, 0x10, 7, 1),
	PIN_FIELD_BASE(56, 56, 1, 0x40, 0x10, 8, 1),
	PIN_FIELD_BASE(57, 57, 1, 0x40, 0x10, 9, 1),
	PIN_FIELD_BASE(58, 58, 1, 0x40, 0x10, 1, 1),
	PIN_FIELD_BASE(59, 59, 1, 0x40, 0x10, 0, 1),
	PIN_FIELD_BASE(60, 60, 1, 0x40, 0x10, 10, 1),
	PIN_FIELD_BASE(61, 61, 1, 0x40, 0x10, 11, 1),
	PIN_FIELD_BASE(62, 62, 2, 0x60, 0x10, 15, 1),
	PIN_FIELD_BASE(63, 63, 2, 0x60, 0x10, 14, 1),
	PIN_FIELD_BASE(64, 64, 2, 0x60, 0x10, 13, 1),
	PIN_FIELD_BASE(65, 65, 2, 0x60, 0x10, 16, 1),
	PIN_FIELD_BASE(66, 66, 4, 0x40, 0x10, 2, 1),
	PIN_FIELD_BASE(67, 67, 4, 0x40, 0x10, 3, 1),
	PIN_FIELD_BASE(68, 68, 4, 0x40, 0x10, 4, 1),
};

static const struct mtk_pin_field_calc mt7986_pin_r0_range[] = {
	PIN_FIELD_BASE(0, 0, 2, 0x70, 0x10, 17, 1),
	PIN_FIELD_BASE(1, 1, 3, 0x40, 0x10, 10, 1),
	PIN_FIELD_BASE(2, 2, 3, 0x40, 0x10, 11, 1),
	PIN_FIELD_BASE(3, 3, 4, 0x50, 0x10, 0, 1),
	PIN_FIELD_BASE(4, 4, 4, 0x50, 0x10, 1, 1),
	PIN_FIELD_BASE(5, 5, 2, 0x70, 0x10, 0, 1),
	PIN_FIELD_BASE(6, 6, 2, 0x70, 0x10, 1, 1),
	PIN_FIELD_BASE(7, 7, 3, 0x40, 0x10, 0, 1),
	PIN_FIELD_BASE(8, 8, 3, 0x40, 0x10, 1, 1),
	PIN_FIELD_BASE(9, 9, 3, 0x40, 0x10, 2, 1),
	PIN_FIELD_BASE(10, 10, 3, 0x40, 0x10, 3, 1),
	PIN_FIELD_BASE(11, 11, 2, 0x70, 0x10, 8, 1),
	PIN_FIELD_BASE(12, 12, 2, 0x70, 0x10, 9, 1),
	PIN_FIELD_BASE(13, 13, 2, 0x70, 0x10, 10, 1),
	PIN_FIELD_BASE(14, 14, 2, 0x70, 0x10, 11, 1),
	PIN_FIELD_BASE(15, 15, 2, 0x70, 0x10, 2, 1),
	PIN_FIELD_BASE(16, 16, 2, 0x70, 0x10, 3, 1),
	PIN_FIELD_BASE(17, 17, 2, 0x70, 0x10, 4, 1),
	PIN_FIELD_BASE(18, 18, 2, 0x70, 0x10, 5, 1),
	PIN_FIELD_BASE(19, 19, 2, 0x70, 0x10, 6, 1),
	PIN_FIELD_BASE(20, 20, 2, 0x70, 0x10, 7, 1),
	PIN_FIELD_BASE(21, 21, 1, 0x50, 0x10, 12, 1),
	PIN_FIELD_BASE(22, 22, 1, 0x50, 0x10, 13, 1),
	PIN_FIELD_BASE(23, 23, 1, 0x50, 0x10, 14, 1),
	PIN_FIELD_BASE(24, 24, 1, 0x50, 0x10, 18, 1),
	PIN_FIELD_BASE(25, 25, 1, 0x50, 0x10, 17, 1),
	PIN_FIELD_BASE(26, 26, 1, 0x50, 0x10, 15, 1),
	PIN_FIELD_BASE(27, 27, 1, 0x50, 0x10, 16, 1),
	PIN_FIELD_BASE(28, 28, 1, 0x50, 0x10, 19, 1),
	PIN_FIELD_BASE(29, 29, 1, 0x50, 0x10, 20, 1),
	PIN_FIELD_BASE(30, 30, 1, 0x50, 0x10, 23, 1),
	PIN_FIELD_BASE(31, 31, 1, 0x50, 0x10, 22, 1),
	PIN_FIELD_BASE(32, 32, 1, 0x50, 0x10, 21, 1),
	PIN_FIELD_BASE(33, 33, 3, 0x40, 0x10, 4, 1),
	PIN_FIELD_BASE(34, 34, 3, 0x40, 0x10, 8, 1),
	PIN_FIELD_BASE(35, 35, 3, 0x40, 0x10, 7, 1),
	PIN_FIELD_BASE(36, 36, 3, 0x40, 0x10, 5, 1),
	PIN_FIELD_BASE(37, 37, 3, 0x40, 0x10, 6, 1),
	PIN_FIELD_BASE(38, 38, 3, 0x40, 0x10, 9, 1),
	PIN_FIELD_BASE(39, 39, 2, 0x70, 0x10, 18, 1),
	PIN_FIELD_BASE(40, 40, 2, 0x70, 0x10, 19, 1),
	PIN_FIELD_BASE(41, 41, 2, 0x70, 0x10, 12, 1),
	PIN_FIELD_BASE(42, 42, 2, 0x70, 0x10, 23, 1),
	PIN_FIELD_BASE(43, 43, 2, 0x70, 0x10, 24, 1),
	PIN_FIELD_BASE(44, 44, 2, 0x70, 0x10, 21, 1),
	PIN_FIELD_BASE(45, 45, 2, 0x70, 0x10, 22, 1),
	PIN_FIELD_BASE(46, 46, 2, 0x70, 0x10, 27, 1),
	PIN_FIELD_BASE(47, 47, 2, 0x70, 0x10, 28, 1),
	PIN_FIELD_BASE(48, 48, 2, 0x70, 0x10, 25, 1),
	PIN_FIELD_BASE(49, 49, 2, 0x70, 0x10, 26, 1),
	PIN_FIELD_BASE(50, 50, 1, 0x50, 0x10, 2, 1),
	PIN_FIELD_BASE(51, 51, 1, 0x50, 0x10, 3, 1),
	PIN_FIELD_BASE(52, 52, 1, 0x50, 0x10, 4, 1),
	PIN_FIELD_BASE(53, 53, 1, 0x50, 0x10, 5, 1),
	PIN_FIELD_BASE(54, 54, 1, 0x50, 0x10, 6, 1),
	PIN_FIELD_BASE(55, 55, 1, 0x50, 0x10, 7, 1),
	PIN_FIELD_BASE(56, 56, 1, 0x50, 0x10, 8, 1),
	PIN_FIELD_BASE(57, 57, 1, 0x50, 0x10, 9, 1),
	PIN_FIELD_BASE(58, 58, 1, 0x50, 0x10, 1, 1),
	PIN_FIELD_BASE(59, 59, 1, 0x50, 0x10, 0, 1),
	PIN_FIELD_BASE(60, 60, 1, 0x50, 0x10, 10, 1),
	PIN_FIELD_BASE(61, 61, 1, 0x50, 0x10, 11, 1),
	PIN_FIELD_BASE(62, 62, 2, 0x70, 0x10, 15, 1),
	PIN_FIELD_BASE(63, 63, 2, 0x70, 0x10, 14, 1),
	PIN_FIELD_BASE(64, 64, 2, 0x70, 0x10, 13, 1),
	PIN_FIELD_BASE(65, 65, 2, 0x70, 0x10, 16, 1),
	PIN_FIELD_BASE(66, 66, 4, 0x50, 0x10, 2, 1),
	PIN_FIELD_BASE(67, 67, 4, 0x50, 0x10, 3, 1),
	PIN_FIELD_BASE(68, 68, 4, 0x50, 0x10, 4, 1),
};

static const struct mtk_pin_field_calc mt7986_pin_r1_range[] = {
	PIN_FIELD_BASE(0, 0, 2, 0x80, 0x10, 17, 1),
	PIN_FIELD_BASE(1, 1, 3, 0x50, 0x10, 10, 1),
	PIN_FIELD_BASE(2, 2, 3, 0x50, 0x10, 11, 1),
	PIN_FIELD_BASE(3, 3, 4, 0x60, 0x10, 0, 1),
	PIN_FIELD_BASE(4, 4, 4, 0x60, 0x10, 1, 1),
	PIN_FIELD_BASE(5, 5, 2, 0x80, 0x10, 0, 1),
	PIN_FIELD_BASE(6, 6, 2, 0x80, 0x10, 1, 1),
	PIN_FIELD_BASE(7, 7, 3, 0x50, 0x10, 0, 1),
	PIN_FIELD_BASE(8, 8, 3, 0x50, 0x10, 1, 1),
	PIN_FIELD_BASE(9, 9, 3, 0x50, 0x10, 2, 1),
	PIN_FIELD_BASE(10, 10, 3, 0x50, 0x10, 3, 1),
	PIN_FIELD_BASE(11, 11, 2, 0x80, 0x10, 8, 1),
	PIN_FIELD_BASE(12, 12, 2, 0x80, 0x10, 9, 1),
	PIN_FIELD_BASE(13, 13, 2, 0x80, 0x10, 10, 1),
	PIN_FIELD_BASE(14, 14, 2, 0x80, 0x10, 11, 1),
	PIN_FIELD_BASE(15, 15, 2, 0x80, 0x10, 2, 1),
	PIN_FIELD_BASE(16, 16, 2, 0x80, 0x10, 3, 1),
	PIN_FIELD_BASE(17, 17, 2, 0x80, 0x10, 4, 1),
	PIN_FIELD_BASE(18, 18, 2, 0x80, 0x10, 5, 1),
	PIN_FIELD_BASE(19, 19, 2, 0x80, 0x10, 6, 1),
	PIN_FIELD_BASE(20, 20, 2, 0x80, 0x10, 7, 1),
	PIN_FIELD_BASE(21, 21, 1, 0x60, 0x10, 12, 1),
	PIN_FIELD_BASE(22, 22, 1, 0x60, 0x10, 13, 1),
	PIN_FIELD_BASE(23, 23, 1, 0x60, 0x10, 14, 1),
	PIN_FIELD_BASE(24, 24, 1, 0x60, 0x10, 18, 1),
	PIN_FIELD_BASE(25, 25, 1, 0x60, 0x10, 17, 1),
	PIN_FIELD_BASE(26, 26, 1, 0x60, 0x10, 15, 1),
	PIN_FIELD_BASE(27, 27, 1, 0x60, 0x10, 16, 1),
	PIN_FIELD_BASE(28, 28, 1, 0x60, 0x10, 19, 1),
	PIN_FIELD_BASE(29, 29, 1, 0x60, 0x10, 20, 1),
	PIN_FIELD_BASE(30, 30, 1, 0x60, 0x10, 23, 1),
	PIN_FIELD_BASE(31, 31, 1, 0x60, 0x10, 22, 1),
	PIN_FIELD_BASE(32, 32, 1, 0x60, 0x10, 21, 1),
	PIN_FIELD_BASE(33, 33, 3, 0x50, 0x10, 4, 1),
	PIN_FIELD_BASE(34, 34, 3, 0x50, 0x10, 8, 1),
	PIN_FIELD_BASE(35, 35, 3, 0x50, 0x10, 7, 1),
	PIN_FIELD_BASE(36, 36, 3, 0x50, 0x10, 5, 1),
	PIN_FIELD_BASE(37, 37, 3, 0x50, 0x10, 6, 1),
	PIN_FIELD_BASE(38, 38, 3, 0x50, 0x10, 9, 1),
	PIN_FIELD_BASE(39, 39, 2, 0x80, 0x10, 18, 1),
	PIN_FIELD_BASE(40, 40, 2, 0x80, 0x10, 19, 1),
	PIN_FIELD_BASE(41, 41, 2, 0x80, 0x10, 12, 1),
	PIN_FIELD_BASE(42, 42, 2, 0x80, 0x10, 23, 1),
	PIN_FIELD_BASE(43, 43, 2, 0x80, 0x10, 24, 1),
	PIN_FIELD_BASE(44, 44, 2, 0x80, 0x10, 21, 1),
	PIN_FIELD_BASE(45, 45, 2, 0x80, 0x10, 22, 1),
	PIN_FIELD_BASE(46, 46, 2, 0x80, 0x10, 27, 1),
	PIN_FIELD_BASE(47, 47, 2, 0x80, 0x10, 28, 1),
	PIN_FIELD_BASE(48, 48, 2, 0x80, 0x10, 25, 1),
	PIN_FIELD_BASE(49, 49, 2, 0x80, 0x10, 26, 1),
	PIN_FIELD_BASE(50, 50, 1, 0x60, 0x10, 2, 1),
	PIN_FIELD_BASE(51, 51, 1, 0x60, 0x10, 3, 1),
	PIN_FIELD_BASE(52, 52, 1, 0x60, 0x10, 4, 1),
	PIN_FIELD_BASE(53, 53, 1, 0x60, 0x10, 5, 1),
	PIN_FIELD_BASE(54, 54, 1, 0x60, 0x10, 6, 1),
	PIN_FIELD_BASE(55, 55, 1, 0x60, 0x10, 7, 1),
	PIN_FIELD_BASE(56, 56, 1, 0x60, 0x10, 8, 1),
	PIN_FIELD_BASE(57, 57, 1, 0x60, 0x10, 9, 1),
	PIN_FIELD_BASE(58, 58, 1, 0x60, 0x10, 1, 1),
	PIN_FIELD_BASE(59, 59, 1, 0x60, 0x10, 0, 1),
	PIN_FIELD_BASE(60, 60, 1, 0x60, 0x10, 10, 1),
	PIN_FIELD_BASE(61, 61, 1, 0x60, 0x10, 11, 1),
	PIN_FIELD_BASE(62, 62, 2, 0x80, 0x10, 15, 1),
	PIN_FIELD_BASE(63, 63, 2, 0x80, 0x10, 14, 1),
	PIN_FIELD_BASE(64, 64, 2, 0x80, 0x10, 13, 1),
	PIN_FIELD_BASE(65, 65, 2, 0x80, 0x10, 16, 1),
	PIN_FIELD_BASE(66, 66, 4, 0x60, 0x10, 2, 1),
	PIN_FIELD_BASE(67, 67, 4, 0x60, 0x10, 3, 1),
	PIN_FIELD_BASE(68, 68, 4, 0x60, 0x10, 4, 1),
};

static const struct mtk_pin_reg_calc mt7986_reg_cals[] = {
	[PINCTRL_PIN_REG_MODE] = MTK_RANGE(mt7986_pin_mode_range),
	[PINCTRL_PIN_REG_DIR] = MTK_RANGE(mt7986_pin_dir_range),
	[PINCTRL_PIN_REG_DI] = MTK_RANGE(mt7986_pin_di_range),
	[PINCTRL_PIN_REG_DO] = MTK_RANGE(mt7986_pin_do_range),
	[PINCTRL_PIN_REG_SMT] = MTK_RANGE(mt7986_pin_smt_range),
	[PINCTRL_PIN_REG_IES] = MTK_RANGE(mt7986_pin_ies_range),
	[PINCTRL_PIN_REG_PU] = MTK_RANGE(mt7986_pin_pu_range),
	[PINCTRL_PIN_REG_PD] = MTK_RANGE(mt7986_pin_pd_range),
	[PINCTRL_PIN_REG_DRV] = MTK_RANGE(mt7986_pin_drv_range),
	[PINCTRL_PIN_REG_PUPD] = MTK_RANGE(mt7986_pin_pupd_range),
	[PINCTRL_PIN_REG_R0] = MTK_RANGE(mt7986_pin_r0_range),
	[PINCTRL_PIN_REG_R1] = MTK_RANGE(mt7986_pin_r1_range),
};

static const struct mtk_pin_desc mt7986_pins[] = {
	MT7986_PIN(0, "SYS_WATCHDOG"),
	MT7986_PIN(1, "WF2G_LED"),
	MT7986_PIN(2, "WF5G_LED"),
	MT7986_PIN(3, "I2C_SCL"),
	MT7986_PIN(4, "I2C_SDA"),
	MT7986_PIN(5, "GPIO_0"),
	MT7986_PIN(6, "GPIO_1"),
	MT7986_PIN(7, "GPIO_2"),
	MT7986_PIN(8, "GPIO_3"),
	MT7986_PIN(9, "GPIO_4"),
	MT7986_PIN(10, "GPIO_5"),
	MT7986_PIN(11, "GPIO_6"),
	MT7986_PIN(12, "GPIO_7"),
	MT7986_PIN(13, "GPIO_8"),
	MT7986_PIN(14, "GPIO_9"),
	MT7986_PIN(15, "GPIO_10"),
	MT7986_PIN(16, "GPIO_11"),
	MT7986_PIN(17, "GPIO_12"),
	MT7986_PIN(18, "GPIO_13"),
	MT7986_PIN(19, "GPIO_14"),
	MT7986_PIN(20, "GPIO_15"),
	MT7986_PIN(21, "PWM0"),
	MT7986_PIN(22, "PWM1"),
	MT7986_PIN(23, "SPI0_CLK"),
	MT7986_PIN(24, "SPI0_MOSI"),
	MT7986_PIN(25, "SPI0_MISO"),
	MT7986_PIN(26, "SPI0_CS"),
	MT7986_PIN(27, "SPI0_HOLD"),
	MT7986_PIN(28, "SPI0_WP"),
	MT7986_PIN(29, "SPI1_CLK"),
	MT7986_PIN(30, "SPI1_MOSI"),
	MT7986_PIN(31, "SPI1_MISO"),
	MT7986_PIN(32, "SPI1_CS"),
	MT7986_PIN(33, "SPI2_CLK"),
	MT7986_PIN(34, "SPI2_MOSI"),
	MT7986_PIN(35, "SPI2_MISO"),
	MT7986_PIN(36, "SPI2_CS"),
	MT7986_PIN(37, "SPI2_HOLD"),
	MT7986_PIN(38, "SPI2_WP"),
	MT7986_PIN(39, "UART0_RXD"),
	MT7986_PIN(40, "UART0_TXD"),
	MT7986_PIN(41, "PCIE_PERESET_N"),
	MT7986_PIN(42, "UART1_RXD"),
	MT7986_PIN(43, "UART1_TXD"),
	MT7986_PIN(44, "UART1_CTS"),
	MT7986_PIN(45, "UART1_RTS"),
	MT7986_PIN(46, "UART2_RXD"),
	MT7986_PIN(47, "UART2_TXD"),
	MT7986_PIN(48, "UART2_CTS"),
	MT7986_PIN(49, "UART2_RTS"),
	MT7986_PIN(50, "EMMC_DATA_0"),
	MT7986_PIN(51, "EMMC_DATA_1"),
	MT7986_PIN(52, "EMMC_DATA_2"),
	MT7986_PIN(53, "EMMC_DATA_3"),
	MT7986_PIN(54, "EMMC_DATA_4"),
	MT7986_PIN(55, "EMMC_DATA_5"),
	MT7986_PIN(56, "EMMC_DATA_6"),
	MT7986_PIN(57, "EMMC_DATA_7"),
	MT7986_PIN(58, "EMMC_CMD"),
	MT7986_PIN(59, "EMMC_CK"),
	MT7986_PIN(60, "EMMC_DSL"),
	MT7986_PIN(61, "EMMC_RSTB"),
	MT7986_PIN(62, "PCM_DTX"),
	MT7986_PIN(63, "PCM_DRX"),
	MT7986_PIN(64, "PCM_CLK"),
	MT7986_PIN(65, "PCM_FS"),
	MT7986_PIN(66, "MT7531_INT"),
	MT7986_PIN(67, "SMI_MDC"),
	MT7986_PIN(68, "SMI_MDIO"),
	MT7986_PIN(69, "WF0_DIG_RESETB"),
	MT7986_PIN(70, "WF0_CBA_RESETB"),
	MT7986_PIN(71, "WF0_XO_REQ"),
	MT7986_PIN(72, "WF0_TOP_CLK"),
	MT7986_PIN(73, "WF0_TOP_DATA"),
	MT7986_PIN(74, "WF0_HB1"),
	MT7986_PIN(75, "WF0_HB2"),
	MT7986_PIN(76, "WF0_HB3"),
	MT7986_PIN(77, "WF0_HB4"),
	MT7986_PIN(78, "WF0_HB0"),
	MT7986_PIN(79, "WF0_HB0_B"),
	MT7986_PIN(80, "WF0_HB5"),
	MT7986_PIN(81, "WF0_HB6"),
	MT7986_PIN(82, "WF0_HB7"),
	MT7986_PIN(83, "WF0_HB8"),
	MT7986_PIN(84, "WF0_HB9"),
	MT7986_PIN(85, "WF0_HB10"),
	MT7986_PIN(86, "WF1_DIG_RESETB"),
	MT7986_PIN(87, "WF1_CBA_RESETB"),
	MT7986_PIN(88, "WF1_XO_REQ"),
	MT7986_PIN(89, "WF1_TOP_CLK"),
	MT7986_PIN(90, "WF1_TOP_DATA"),
	MT7986_PIN(91, "WF1_HB1"),
	MT7986_PIN(92, "WF1_HB2"),
	MT7986_PIN(93, "WF1_HB3"),
	MT7986_PIN(94, "WF1_HB4"),
	MT7986_PIN(95, "WF1_HB0"),
	MT7986_PIN(96, "WF1_HB0_B"),
	MT7986_PIN(97, "WF1_HB5"),
	MT7986_PIN(98, "WF1_HB6"),
	MT7986_PIN(99, "WF1_HB7"),
	MT7986_PIN(100, "WF1_HB8"),
};

/* List all groups consisting of these pins dedicated to the enablement of
 * certain hardware block and the corresponding mode for all of the pins.
 * The hardware probably has multiple combinations of these pinouts.
 */

/* SYS_WATCHDOG */
static int mt7986_watchdog_pins[] = { 0, };
static int mt7986_watchdog_funcs[] = { 1, };

/* WF2G_LED(1), WF5G_LED */
static int mt7986_wifi_led_pins[] = { 1, 2, };
static int mt7986_wifi_led_funcs[] = { 1, 1, };

/* I2C */
static int mt7986_i2c_pins[] = { 3, 4, };
static int mt7986_i2c_funcs[] = { 1, 1, };

/* UART1 */
static int mt7986_uart1_0_pins[] = { 7, 8, 9, 10, };
static int mt7986_uart1_0_funcs[] = { 3, 3, 3, 3, };

/* JTAG */
static int mt7986_jtag_pins[] = { 11, 12, 13, 14, 15, };
static int mt7986_jtag_funcs[] = { 1, 1, 1, 1, 1, };

/* SPI1 */
static int mt7986_spi1_0_pins[] = { 11, 12, 13, 14, };
static int mt7986_spi1_0_funcs[] = { 3, 3, 3, 3, };

/* PWM */
static int mt7986_pwm1_1_pins[] = { 20, };
static int mt7986_pwm1_1_funcs[] = { 2, };

/* PWM */
static int mt7986_pwm0_pins[] = { 21, };
static int mt7986_pwm0_funcs[] = { 1, };

/* PWM */
static int mt7986_pwm1_0_pins[] = { 22, };
static int mt7986_pwm1_0_funcs[] = { 1, };

/* EMMC */
static int mt7986_emmc_45_pins[] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, };
static int mt7986_emmc_45_funcs[] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, };

/* SNFI */
static int mt7986_snfi_pins[] = { 23, 24, 25, 26, 27, 28, };
static int mt7986_snfi_funcs[] = { 1, 1, 1, 1, 1, 1, };

/* SPI1 */
static int mt7986_spi1_1_pins[] = { 23, 24, 25, 26, };
static int mt7986_spi1_1_funcs[] = { 3, 3, 3, 3, };

/* UART1 */
static int mt7986_uart1_1_pins[] = { 23, 24, 25, 26, };
static int mt7986_uart1_1_funcs[] = { 4, 4, 4, 4, };

/* SPI1 */
static int mt7986_spi1_2_pins[] = { 29, 30, 31, 32, };
static int mt7986_spi1_2_funcs[] = { 1, 1, 1, 1, };

/* UART1 */
static int mt7986_uart1_2_pins[] = { 29, 30, 31, 32, };
static int mt7986_uart1_2_funcs[] = { 3, 3, 3, 3, };

/* UART2 */
static int mt7986_uart2_0_pins[] = { 29, 30, 31, 32, };
static int mt7986_uart2_0_funcs[] = { 4, 4, 4, 4, };

/* SPI0 */
static int mt7986_spi0_pins[] = { 33, 34, 35, 36, };
static int mt7986_spi0_funcs[] = { 1, 1, 1, 1, };

/* SPI0 */
static int mt7986_spi0_wp_hold_pins[] = { 37, 38, };
static int mt7986_spi0_wp_hold_funcs[] = { 1, 1, };

/* UART2 */
static int mt7986_uart2_1_pins[] = { 33, 34, 35, 36, };
static int mt7986_uart2_1_funcs[] = { 3, 3, 3, 3, };

/* UART1 */
static int mt7986_uart1_3_rx_tx_pins[] = { 35, 36, };
static int mt7986_uart1_3_rx_tx_funcs[] = { 2, 2, };

/* UART1 */
static int mt7986_uart1_3_cts_rts_pins[] = { 37, 38, };
static int mt7986_uart1_3_cts_rts_funcs[] = { 2, 2, };

/* SPI1 */
static int mt7986_spi1_3_pins[] = { 33, 34, 35, 36, };
static int mt7986_spi1_3_funcs[] = { 4, 4, 4, 4, };

/* UART0 */
static int mt7986_uart0_pins[] = { 39, 40, };
static int mt7986_uart0_funcs[] = { 1, 1, };

/* PCIE_PERESET_N */
static int mt7986_pcie_reset_pins[] = { 41, };
static int mt7986_pcie_reset_funcs[] = { 1, };

/* UART1 */
static int mt7986_uart1_pins[] = { 42, 43, 44, 45, };
static int mt7986_uart1_funcs[] = { 1, 1, 1, 1, };

/* UART1 */
static int mt7986_uart2_pins[] = { 46, 47, 48, 49, };
static int mt7986_uart2_funcs[] = { 1, 1, 1, 1, };

/* EMMC */
static int mt7986_emmc_51_pins[] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, };
static int mt7986_emmc_51_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

/* PCM */
static int mt7986_pcm_pins[] = { 62, 63, 64, 65, };
static int mt7986_pcm_funcs[] = { 1, 1, 1, 1, };

/* MT7531_INT */
static int mt7986_switch_int_pins[] = { 66, };
static int mt7986_switch_int_funcs[] = { 1, };

/* MDC_MDIO */
static int mt7986_mdc_mdio_pins[] = { 67, 68, };
static int mt7986_mdc_mdio_funcs[] = { 1, 1, };

/* WF0_MODE1 */
static int mt7986_wf0_mode1_pins[] = { 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85 };
static int mt7986_wf0_mode1_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

static int mt7986_wf_2g_pins[] = {74, 75, 76, 77, 78, 79, 80, 81, 82, 83, };
static int mt7986_wf_2g_funcs[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

static int mt7986_wf_5g_pins[] = {91, 92, 93, 94, 95, 96, 97, 98, 99, 100, };
static int mt7986_wf_5g_funcs[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

static int mt7986_wf_dbdc_pins[] = {
	74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, };
static int mt7986_wf_dbdc_funcs[] = {
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, };

/* WF0_HB */
static int mt7986_wf0_hb_pins[] = { 74, 75, 76, 77, 78 };
static int mt7986_wf0_hb_funcs[] = { 2, 2, 2, 2, 2 };

/* WF0_MODE3 */
static int mt7986_wf0_mode3_pins[] = { 74, 75, 76, 77, 78, 80 };
static int mt7986_wf0_mode3_funcs[] = { 3, 3, 3, 3, 3, 3 };

/* WF1_HB */
static int mt7986_wf1_hb_pins[] = { 79, 80, 81, 82, 83, 84, 85 };
static int mt7986_wf1_hb_funcs[] = { 2, 2, 2, 2, 2, 2, 2 };

/* WF1_MODE1 */
static int mt7986_wf1_mode1_pins[] = { 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100 };
static int mt7986_wf1_mode1_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

/* WF1_MODE2 */
static int mt7986_wf1_mode2_pins[] = { 91, 92, 93, 94, 95, 97 };
static int mt7986_wf1_mode2_funcs[] = { 2, 2, 2, 2, 2, 2 };

/* PCIE_CLK_REQ */
static int mt7986_pcie_clk_pins[] = { 9, };
static int mt7986_pcie_clk_funcs[] = { 1, };

/* PCIE_WAKE_N */
static int mt7986_pcie_wake_pins[] = { 10, };
static int mt7986_pcie_wake_funcs[] = { 1, };

static const struct group_desc mt7986_groups[] = {
	/*  @GPIO(0): SYS_WATCHDOG(1) */
        PINCTRL_PIN_GROUP("watchdog", mt7986_watchdog),
	/*  @GPIO(1,2): WF2G_LED(1), WF5G_LED(1) */
        PINCTRL_PIN_GROUP("wifi_led", mt7986_wifi_led),
	/*  @GPIO(3,4): I2C(1) */
        PINCTRL_PIN_GROUP("i2c", mt7986_i2c),
	/*  @GPIO(7,10): UART1(3) */
        PINCTRL_PIN_GROUP("uart1_0", mt7986_uart1_0),
        /*  @GPIO(9): PCIE_CLK_REQ(9) */
        PINCTRL_PIN_GROUP("pcie_clk", mt7986_pcie_clk),
        /*  @GPIO(10): PCIE_WAKE_N(10) */
        PINCTRL_PIN_GROUP("pcie_wake", mt7986_pcie_wake),
	/*  @GPIO(11,15): JTAG(1) */
        PINCTRL_PIN_GROUP("jtag", mt7986_jtag),
	/*  @GPIO(11,15): SPI1(3) */
        PINCTRL_PIN_GROUP("spi1_0", mt7986_spi1_0),
	/*  @GPIO(20): PWM(2) */
        PINCTRL_PIN_GROUP("pwm1_1", mt7986_pwm1_1),
	/*  @GPIO(21): PWM(1) */
        PINCTRL_PIN_GROUP("pwm0", mt7986_pwm0),
	/*  @GPIO(22): PWM(1) */
        PINCTRL_PIN_GROUP("pwm1_0", mt7986_pwm1_0),
	/*  @GPIO(22,32): EMMC(2) */
        PINCTRL_PIN_GROUP("emmc_45", mt7986_emmc_45),
	/*  @GPIO(23,28): SNFI(1) */
        PINCTRL_PIN_GROUP("snfi", mt7986_snfi),
	/*  @GPIO(23,26): SPI1(2) */
        PINCTRL_PIN_GROUP("spi1_1", mt7986_spi1_1),
	/*  @GPIO(23,26): UART1(4) */
        PINCTRL_PIN_GROUP("uart1_1", mt7986_uart1_1),
	/*  @GPIO(29,32): SPI1(1) */
        PINCTRL_PIN_GROUP("spi1_2", mt7986_spi1_2),
	/*  @GPIO(29,32): UART1(3) */
        PINCTRL_PIN_GROUP("uart1_2", mt7986_uart1_2),
	/*  @GPIO(29,32): UART2(4) */
        PINCTRL_PIN_GROUP("uart2_0", mt7986_uart2_0),
	/*  @GPIO(33,36): SPI0(1) */
        PINCTRL_PIN_GROUP("spi0", mt7986_spi0),
	/*  @GPIO(37,38): SPI0(1) */
        PINCTRL_PIN_GROUP("spi0_wp_hold", mt7986_spi0_wp_hold),
	/*  @GPIO(33,36): UART2(3) */
        PINCTRL_PIN_GROUP("uart2_1", mt7986_uart2_1),
	/*  @GPIO(35,36): UART1(2) */
        PINCTRL_PIN_GROUP("uart1_3_rx_tx", mt7986_uart1_3_rx_tx),
	/*  @GPIO(37,38): UART1(2) */
        PINCTRL_PIN_GROUP("uart1_3_cts_rts", mt7986_uart1_3_cts_rts),
	/*  @GPIO(33,36): SPI1(4) */
        PINCTRL_PIN_GROUP("spi1_3", mt7986_spi1_3),
	/*  @GPIO(39,40): UART0(1) */
        PINCTRL_PIN_GROUP("uart0", mt7986_uart0),
	/*  @GPIO(41): PCIE_PERESET_N(1) */
        PINCTRL_PIN_GROUP("pcie_pereset", mt7986_pcie_reset),
	/*  @GPIO(42,45): UART1(1) */
        PINCTRL_PIN_GROUP("uart1", mt7986_uart1),
	/*  @GPIO(46,49): UART1(1) */
        PINCTRL_PIN_GROUP("uart2", mt7986_uart2),
	/*  @GPIO(50,61): EMMC(1) */
        PINCTRL_PIN_GROUP("emmc_51", mt7986_emmc_51),
	/*  @GPIO(62,65): PCM(1) */
        PINCTRL_PIN_GROUP("pcm", mt7986_pcm),
	/*  @GPIO(66): MT7531_INT(1) */
        PINCTRL_PIN_GROUP("switch_int", mt7986_switch_int),
	/*  @GPIO(67,68): MDC_MDIO(1) */
        PINCTRL_PIN_GROUP("mdc_mdio", mt7986_mdc_mdio),
    /*  @GPIO(69,85): WF0_MODE1(1) */
        PINCTRL_PIN_GROUP("wf0_mode1", mt7986_wf0_mode1),
    /*  @GPIO(74,78): WF0_HB(2) */
        PINCTRL_PIN_GROUP("wf0_hb", mt7986_wf0_hb),
    /*  @GPIO(74,80): WF0_MODE3(3) */
        PINCTRL_PIN_GROUP("wf0_mode3", mt7986_wf0_mode3),
    /*  @GPIO(79,85): WF1_HB(2) */
        PINCTRL_PIN_GROUP("wf1_hb", mt7986_wf1_hb),
    /*  @GPIO(86,100): WF1_MODE1(1) */
        PINCTRL_PIN_GROUP("wf1_mode1", mt7986_wf1_mode1),
     /*  @GPIO(91,97): WF1_MODE2(2) */
        PINCTRL_PIN_GROUP("wf1_mode2", mt7986_wf1_mode2),


	PINCTRL_PIN_GROUP("wf_2g", mt7986_wf_2g),
	PINCTRL_PIN_GROUP("wf_5g", mt7986_wf_5g),
	PINCTRL_PIN_GROUP("wf_dbdc", mt7986_wf_dbdc),
};

/* Joint those groups owning the same capability in user point of view which
 * allows that people tend to use through the device tree.
 */
static const char *mt7986_ethernet_groups[] = { "mdc_mdio", "wf0_mode1", "wf0_hb",
						"wf0_mode3", "wf1_hb", "wf1_mode1", "wf1_mode2" };
static const char *mt7986_i2c_groups[] = { "i2c", };
static const char *mt7986_led_groups[] = { "wifi_led", };
static const char *mt7986_pwm_groups[] = { "pwm0", "pwm1_0", "pwm1_1", };
static const char *mt7986_spi_groups[] = { "spi0", "spi1_0", "spi1_1",
					   "spi1_2", "spi1_3", };
static const char *mt7986_uart_groups[] = { "uart1_0", "uart1_1", "uart1_2",
					    "uart1_3_rx_tx", "uart1_3_cts_rts",
					    "uart2_0", "uart2_1",
					    "uart0", "uart1", "uart2", };
static const char *mt7986_wdt_groups[] = { "watchdog", };
static const char *mt7986_flash_groups[] = { "snfi", "emmc_45", "emmc_51", "spi0", "spi0_wp_hold"};
static const char *mt7986_pcie_groups[] = { "pcie_clk", "pcie_wake", "pcie_pereset"};
static const char *mt7986_wf_groups[] = { "wf_2g", "wf_5g", "wf_dbdc", };

static const struct function_desc mt7986_functions[] = {
	{"eth",	mt7986_ethernet_groups, ARRAY_SIZE(mt7986_ethernet_groups)},
	{"i2c", mt7986_i2c_groups, ARRAY_SIZE(mt7986_i2c_groups)},
	{"led",	mt7986_led_groups, ARRAY_SIZE(mt7986_led_groups)},
	{"pwm",	mt7986_pwm_groups, ARRAY_SIZE(mt7986_pwm_groups)},
	{"spi",	mt7986_spi_groups, ARRAY_SIZE(mt7986_spi_groups)},
	{"uart", mt7986_uart_groups, ARRAY_SIZE(mt7986_uart_groups)},
	{"watchdog", mt7986_wdt_groups, ARRAY_SIZE(mt7986_wdt_groups)},
	{"flash", mt7986_flash_groups, ARRAY_SIZE(mt7986_flash_groups)},
	{"pcie", mt7986_pcie_groups, ARRAY_SIZE(mt7986_pcie_groups)},
	{"wifi", mt7986_wf_groups, ARRAY_SIZE(mt7986_wf_groups)},
};

static const struct mtk_eint_hw mt7986_eint_hw = {
	.port_mask = 7,
	.ports     = 7,
	.ap_num    = ARRAY_SIZE(mt7986_pins),
	.db_cnt    = 16,
};

static const char * const mt7986_pinctrl_register_base_names[] = {
	"gpio_base", "iocfg_rt_base", "iocfg_rb_base", "iocfg_lt_base",
	"iocfg_lb_base", "iocfg_tr_base", "iocfg_tl_base",
};

static struct mtk_pin_soc mt7986_data = {
	.reg_cal = mt7986_reg_cals,
	.pins = mt7986_pins,
	.npins = ARRAY_SIZE(mt7986_pins),
	.grps = mt7986_groups,
	.ngrps = ARRAY_SIZE(mt7986_groups),
	.funcs = mt7986_functions,
	.nfuncs = ARRAY_SIZE(mt7986_functions),
	.eint_hw = &mt7986_eint_hw,
	.gpio_m = 0,
	.ies_present = false,
	.base_names = mt7986_pinctrl_register_base_names,
	.nbase_names = ARRAY_SIZE(mt7986_pinctrl_register_base_names),
	.bias_disable_set = mtk_pinconf_bias_disable_set,
	.bias_disable_get = mtk_pinconf_bias_disable_get,
	.bias_set = mtk_pinconf_bias_set,
	.bias_get = mtk_pinconf_bias_get,
	.drive_set = mtk_pinconf_drive_set_rev1,
	.drive_get = mtk_pinconf_drive_get_rev1,
	.adv_pull_get = mtk_pinconf_adv_pull_get,
	.adv_pull_set = mtk_pinconf_adv_pull_set,
};

static const struct of_device_id mt7986_pinctrl_of_match[] = {
	{ .compatible = "mediatek,mt7986-pinctrl", },
	{}
};

static int mt7986_pinctrl_probe(struct platform_device *pdev)
{
	return mtk_moore_pinctrl_probe(pdev, &mt7986_data);
}

static struct platform_driver mt7986_pinctrl_driver = {
	.driver = {
		.name = "mt7986-pinctrl",
		.of_match_table = mt7986_pinctrl_of_match,
	},
	.probe = mt7986_pinctrl_probe,
};

static int __init mt7986_pinctrl_init(void)
{
	return platform_driver_register(&mt7986_pinctrl_driver);
}
arch_initcall(mt7986_pinctrl_init);
