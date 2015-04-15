/*
 * Copyright (c) 2014 Google, Inc
 * Copyright (C) 2000 Ronald G. Minnich
 *
 * Microcode update for Intel PIII and later CPUs
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <asm/cpu.h>
#include <asm/msr.h>
#include <asm/msr-index.h>
#include <asm/processor.h>
#include <asm/arch/microcode.h>

/**
 * struct microcode_update - standard microcode header from Intel
 *
 * We read this information out of the device tree and use it to determine
 * whether the update is applicable or not. We also use the same structure
 * to read information from the CPU.
 */
struct microcode_update {
	uint header_version;
	uint update_revision;
	uint date_code;
	uint processor_signature;
	uint checksum;
	uint loader_revision;
	uint processor_flags;
	const void *data;
	int size;
};

static int microcode_decode_node(const void *blob, int node,
				 struct microcode_update *update)
{
	update->data = fdt_getprop(blob, node, "data", &update->size);
	if (!update->data)
		return -EINVAL;
	update->data += UCODE_HEADER_LEN;
	update->size -= UCODE_HEADER_LEN;

	update->header_version = fdtdec_get_int(blob, node,
						"intel,header-version", 0);
	update->update_revision = fdtdec_get_int(blob, node,
						 "intel,update-revision", 0);
	update->date_code = fdtdec_get_int(blob, node,
					   "intel,date-code", 0);
	update->processor_signature = fdtdec_get_int(blob, node,
					"intel,processor-signature", 0);
	update->checksum = fdtdec_get_int(blob, node, "intel,checksum", 0);
	update->loader_revision = fdtdec_get_int(blob, node,
						 "intel,loader-revision", 0);
	update->processor_flags = fdtdec_get_int(blob, node,
						 "intel,processor-flags", 0);

	return 0;
}

static inline uint32_t microcode_read_rev(void)
{
	/*
	 * Some Intel CPUs can be very finicky about the CPUID sequence used.
	 * So this is implemented in assembly so that it works reliably.
	 */
	uint32_t low, high;

	asm volatile (
		"xorl %%eax, %%eax\n"
		"xorl %%edx, %%edx\n"
		"movl %2, %%ecx\n"
		"wrmsr\n"
		"movl $0x01, %%eax\n"
		"cpuid\n"
		"movl %2, %%ecx\n"
		"rdmsr\n"
		: /* outputs */
		"=a" (low), "=d" (high)
		: /* inputs */
		"i" (MSR_IA32_UCODE_REV)
		: /* clobbers */
		 "ebx", "ecx"
	);

	return high;
}

static void microcode_read_cpu(struct microcode_update *cpu)
{
	/* CPUID sets MSR 0x8B iff a microcode update has been loaded. */
	unsigned int x86_model, x86_family;
	struct cpuid_result result;
	uint32_t low, high;

	wrmsr(MSR_IA32_UCODE_REV, 0, 0);
	result = cpuid(1);
	rdmsr(MSR_IA32_UCODE_REV, low, cpu->update_revision);
	x86_model = (result.eax >> 4) & 0x0f;
	x86_family = (result.eax >> 8) & 0x0f;
	cpu->processor_signature = result.eax;

	cpu->processor_flags = 0;
	if ((x86_model >= 5) || (x86_family > 6)) {
		rdmsr(0x17, low, high);
		cpu->processor_flags = 1 << ((high >> 18) & 7);
	}
	debug("microcode: sig=%#x pf=%#x revision=%#x\n",
	      cpu->processor_signature, cpu->processor_flags,
	      cpu->update_revision);
}

/* Get a microcode update from the device tree and apply it */
int microcode_update_intel(void)
{
	struct microcode_update cpu, update;
	const void *blob = gd->fdt_blob;
	int skipped;
	int count;
	int node;
	int ret;
	int rev;

	microcode_read_cpu(&cpu);
	node = 0;
	count = 0;
	skipped = 0;
	do {
		node = fdtdec_next_compatible(blob, node,
					      COMPAT_INTEL_MICROCODE);
		if (node < 0) {
			debug("%s: Found %d updates\n", __func__, count);
			return count ? 0 : skipped ? -EEXIST : -ENOENT;
		}

		ret = microcode_decode_node(blob, node, &update);
		if (ret) {
			debug("%s: Unable to decode update: %d\n", __func__,
			      ret);
			return ret;
		}
		if (!(update.processor_signature == cpu.processor_signature &&
		      (update.processor_flags & cpu.processor_flags))) {
			debug("%s: Skipping non-matching update, sig=%x, pf=%x\n",
			      __func__, update.processor_signature,
			      update.processor_flags);
			skipped++;
			continue;
		}
		wrmsr(MSR_IA32_UCODE_WRITE, (ulong)update.data, 0);
		rev = microcode_read_rev();
		debug("microcode: updated to revision 0x%x date=%04x-%02x-%02x\n",
		      rev, update.date_code & 0xffff,
		      (update.date_code >> 24) & 0xff,
		      (update.date_code >> 16) & 0xff);
		if (update.update_revision != rev) {
			printf("Microcode update failed\n");
			return -EFAULT;
		}
		count++;
	} while (1);
}
