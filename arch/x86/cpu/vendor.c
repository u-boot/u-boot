// SPDX-License-Identifier: GPL-2.0+
/*
 * Shared x86 CPU-vendor helpers
 */

#include <linux/types.h>
#include <asm/cpu.h>
#include <linux/string.h>
#include <linux/kernel.h>

static const struct {
	int vendor;
	const char *name;
} x86_vendors[] = {
	{ X86_VENDOR_INTEL,     "GenuineIntel" },
	{ X86_VENDOR_CYRIX,     "CyrixInstead" },
	{ X86_VENDOR_AMD,       "AuthenticAMD" },
	{ X86_VENDOR_UMC,       "UMC UMC UMC " },
	{ X86_VENDOR_NEXGEN,    "NexGenDriven" },
	{ X86_VENDOR_CENTAUR,   "CentaurHauls" },
	{ X86_VENDOR_RISE,      "RiseRiseRise" },
	{ X86_VENDOR_TRANSMETA, "GenuineTMx86" },
	{ X86_VENDOR_TRANSMETA, "TransmetaCPU" },
	{ X86_VENDOR_NSC,       "Geode by NSC" },
	{ X86_VENDOR_SIS,       "SiS SiS SiS " },
};

int x86_get_vendor_name(char *name)
{
	struct cpuid_result result;
	unsigned int *name_as_ints = (unsigned int *)name;

	result = cpuid(0x00000000);
	name_as_ints[0] = result.ebx;
	name_as_ints[1] = result.edx;
	name_as_ints[2] = result.ecx;
	name[12] = '\0';

	return result.eax;
}

int x86_vendor_name_to_id(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(x86_vendors); i++) {
		if (!memcmp(name, x86_vendors[i].name, 12))
			return x86_vendors[i].vendor;
	}

	return X86_VENDOR_UNKNOWN;
}
