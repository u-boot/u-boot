// SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY LOGC_ARCH

#include <log.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/stm32.h>
#include <asm/arch/sys_proto.h>

/* Package = bit 0:2 of OTP122 => STM32MP23_PKG defines
 * - 000: Custom package
 * - 011: TFBGA361 => AL = 10x10, 361 balls pith 0.5mm
 * - 100: TFBGA424 => AK = 14x14, 424 balls pith 0.5mm
 * - 101: TFBGA436 => AI = 18x18, 436 balls pith 0.5mm
 * - others: Reserved
 */

int get_eth_nb(void)
{
	int nb_eth;

	switch (get_cpu_type()) {
	case CPU_STM32MP235Fxx:
		fallthrough;
	case CPU_STM32MP235Dxx:
		fallthrough;
	case CPU_STM32MP235Cxx:
		fallthrough;
	case CPU_STM32MP235Axx:
		fallthrough;
	case CPU_STM32MP233Fxx:
		fallthrough;
	case CPU_STM32MP233Dxx:
		fallthrough;
	case CPU_STM32MP233Cxx:
		fallthrough;
	case CPU_STM32MP233Axx:
		nb_eth = 2; /* dual ETH */
		break;
	case CPU_STM32MP231Fxx:
		fallthrough;
	case CPU_STM32MP231Dxx:
		fallthrough;
	case CPU_STM32MP231Cxx:
		fallthrough;
	case CPU_STM32MP231Axx:
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
	if (get_cpu_dev() == CPU_DEV_STM32MP23) {
		switch (get_cpu_type()) {
		case CPU_STM32MP235Fxx:
			cpu_s = "235F";
			break;
		case CPU_STM32MP235Dxx:
			cpu_s = "235D";
			break;
		case CPU_STM32MP235Cxx:
			cpu_s = "235C";
			break;
		case CPU_STM32MP235Axx:
			cpu_s = "235A";
			break;
		case CPU_STM32MP233Fxx:
			cpu_s = "233F";
			break;
		case CPU_STM32MP233Dxx:
			cpu_s = "233D";
			break;
		case CPU_STM32MP233Cxx:
			cpu_s = "233C";
			break;
		case CPU_STM32MP233Axx:
			cpu_s = "233A";
			break;
		case CPU_STM32MP231Fxx:
			cpu_s = "231F";
			break;
		case CPU_STM32MP231Dxx:
			cpu_s = "231D";
			break;
		case CPU_STM32MP231Cxx:
			cpu_s = "231C";
			break;
		case CPU_STM32MP231Axx:
			cpu_s = "231A";
			break;
		default:
			cpu_s = "23??";
			break;
		}
		/* REVISION */
		switch (get_cpu_rev()) {
		case OTP_REVID_1:
			cpu_r = "A";
			break;
		case OTP_REVID_2:
			cpu_r = "B";
			break;
		case OTP_REVID_2_1:
			cpu_r = "Y";
			break;
		case OTP_REVID_2_2:
			cpu_r = "X";
			break;
		default:
			break;
		}
		/* PACKAGE */
		switch (get_cpu_package()) {
		case STM32MP23_PKG_CUSTOM:
			package = "XX";
			break;
		case STM32MP23_PKG_AL_VFBGA361:
			package = "AL";
			break;
		case STM32MP23_PKG_AK_VFBGA424:
			package = "AK";
			break;
		case STM32MP23_PKG_AJ_TFBGA361:
			package = "AJ";
			break;
		default:
			break;
		}
	}

	snprintf(name, SOC_NAME_SIZE, "STM32MP%s%s Rev.%s", cpu_s, package, cpu_r);
}
