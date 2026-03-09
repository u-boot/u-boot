// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2026, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <log.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>

/* Package = bit 0:2 of OTP122 => STM32MP21_PKG defines
 * - 000: Custom package
 * - 001: VFBGA361 => AL = 10x10, 361 balls pith 0.5mm
 * - 011: VFBGA273 => AN = 11x11, 273 balls pith 0.5mm
 * - 100: VFBGA225 => AO =   8x8, 225 balls pith 0.5mm
 * - 101: TFBGA289 => AM = 14x14, 289 balls pith 0.8mm
 * - others: Reserved
 */

int get_eth_nb(void)
{
	int nb_eth;

	switch (get_cpu_type()) {
	case CPU_STM32MP215Axx:
		fallthrough;
	case CPU_STM32MP215Cxx:
		fallthrough;
	case CPU_STM32MP215Dxx:
		fallthrough;
	case CPU_STM32MP215Fxx:
		fallthrough;
	case CPU_STM32MP213Axx:
		fallthrough;
	case CPU_STM32MP213Cxx:
		fallthrough;
	case CPU_STM32MP213Dxx:
		fallthrough;
	case CPU_STM32MP213Fxx:
		nb_eth = 2; /* dual ETH */
		break;
	case CPU_STM32MP211Axx:
		fallthrough;
	case CPU_STM32MP211Cxx:
		fallthrough;
	case CPU_STM32MP211Dxx:
		fallthrough;
	case CPU_STM32MP211Fxx:
		nb_eth = 1; /* single ETH */
		break;
	default:
		nb_eth = 0;
		break;
	}

	return nb_eth;
}

void get_soc_name(char name[SOC_NAME_SIZE])
{
	char *cpu_s, *cpu_r, *package;

	cpu_s = "????";
	cpu_r = "?";
	package = "??";
	if (get_cpu_dev() == CPU_DEV_STM32MP21) {
		switch (get_cpu_type()) {
		case CPU_STM32MP215Fxx:
			cpu_s = "215F";
			break;
		case CPU_STM32MP215Dxx:
			cpu_s = "215D";
			break;
		case CPU_STM32MP215Cxx:
			cpu_s = "215C";
			break;
		case CPU_STM32MP215Axx:
			cpu_s = "215A";
			break;
		case CPU_STM32MP213Fxx:
			cpu_s = "213F";
			break;
		case CPU_STM32MP213Dxx:
			cpu_s = "213D";
			break;
		case CPU_STM32MP213Cxx:
			cpu_s = "213C";
			break;
		case CPU_STM32MP213Axx:
			cpu_s = "213A";
			break;
		case CPU_STM32MP211Fxx:
			cpu_s = "211F";
			break;
		case CPU_STM32MP211Dxx:
			cpu_s = "211D";
			break;
		case CPU_STM32MP211Cxx:
			cpu_s = "211C";
			break;
		case CPU_STM32MP211Axx:
			cpu_s = "211A";
			break;
		default:
			cpu_s = "21??";
			break;
		}
		/* REVISION */
		switch (get_cpu_rev()) {
		case OTP_REVID_1:
			cpu_r = "A";
			break;
		case OTP_REVID_1_1:
			cpu_r = "Z";
			break;
		case OTP_REVID_2:
			cpu_r = "B";
			break;
		default:
			break;
		}
		/* PACKAGE */
		switch (get_cpu_package()) {
		case STM32MP25_PKG_CUSTOM:
			package = "XX";
			break;
		case STM32MP21_PKG_AL_VFBGA361:
			package = "AL";
			break;
		case STM32MP21_PKG_AN_VFBGA273:
			package = "AN";
			break;
		case STM32MP21_PKG_AO_VFBGA225:
			package = "AO";
			break;
		case STM32MP21_PKG_AM_TFBGA289:
			package = "AM";
			break;
		default:
			break;
		}
	}

	snprintf(name, SOC_NAME_SIZE, "STM32MP%s%s Rev.%s", cpu_s, package, cpu_r);
}
