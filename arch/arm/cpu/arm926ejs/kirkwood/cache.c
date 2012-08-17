/*
 * Copyright (c) 2012 Michael Walle
 * Michael Walle <michael@walle.cc>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */
#include <common.h>
#include <asm/arch/cpu.h>

#define FEROCEON_EXTRA_FEATURE_L2C_EN (1<<22)

void l2_cache_disable()
{
	u32 ctrl;

	ctrl = readfr_extra_feature_reg();
	ctrl &= ~FEROCEON_EXTRA_FEATURE_L2C_EN;
	writefr_extra_feature_reg(ctrl);
}
