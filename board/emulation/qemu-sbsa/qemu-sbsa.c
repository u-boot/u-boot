/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Tuomas Tynkkynen
 */

#include <cpu_func.h>
#include <dm.h>
#include <env.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <init.h>
#include <log.h>
#include <usb.h>
#include <asm/armv8/mmu.h>

#include "qemu-sbsa.h"

/* Assigned in lowlevel_init.S
 * Push the variable into the .data section so that it
 * does not get cleared later.
 */
unsigned long __section(".data") fw_dtb_pointer;

static struct mm_region qemu_sbsa_mem_map[] = {
	{
		/* Secure flash */
		.virt = SBSA_SECURE_FLASH_BASE_ADDR,
		.phys = SBSA_SECURE_FLASH_BASE_ADDR,
		.size = SBSA_SECURE_FLASH_LENGTH,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_INNER_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Flash */
		.virt = SBSA_FLASH_BASE_ADDR,
		.phys = SBSA_FLASH_BASE_ADDR,
		.size = SBSA_FLASH_LENGTH,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* Lowmem peripherals */
		.virt = SBSA_PERIPH_BASE_ADDR,
		.phys = SBSA_PERIPH_BASE_ADDR,
		.size = SBSA_PCIE_MMIO_BASE_ADDR - SBSA_PERIPH_BASE_ADDR,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* 32-bit address PCIE MMIO space */
		.virt = SBSA_PCIE_MMIO_BASE_ADDR,
		.phys = SBSA_PCIE_MMIO_BASE_ADDR,
		.size = SBSA_PCIE_MMIO_LENGTH,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* PCI-E ECAM memory area */
		.virt = SBSA_PCIE_ECAM_BASE_ADDR,
		.phys = SBSA_PCIE_ECAM_BASE_ADDR,
		.size = SBSA_PCIE_ECAM_LENGTH,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Highmem PCI-E MMIO memory area */
		.virt = SBSA_PCIE_MMIO_HIGH_BASE_ADDR,
		.phys = SBSA_PCIE_MMIO_HIGH_BASE_ADDR,
		.size = SBSA_PCIE_MMIO_HIGH_LENGTH,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* DRAM */
		.virt = SBSA_MEM_BASE_ADDR,
		.phys = SBSA_MEM_BASE_ADDR,
		.size = 0x800000000000ULL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = qemu_sbsa_mem_map;

int board_late_init(void)
{
	/* start usb so that usb keyboard can be used as input device */
	if (CONFIG_IS_ENABLED(USB_KEYBOARD))
		usb_init();

	return 0;
}

/**
 * dtb_dt_qemu - Return the address of the QEMU provided FDT.
 *
 * @return: Pointer to FDT or NULL on failure
 */
static void *dtb_dt_qemu(void)
{
	/* FDT might be at start of DRAM */
	if (fdt_magic(SBSA_MEM_BASE_ADDR) == FDT_MAGIC)
		return (void *)(u64)SBSA_MEM_BASE_ADDR;

	/* When ARM_LINUX_KERNEL_AS_BL33 is enabled in ATF, it's passed in x0 */
	if (fw_dtb_pointer >= SBSA_MEM_BASE_ADDR &&
	    fdt_magic(fw_dtb_pointer) == FDT_MAGIC) {
		return (void *)fw_dtb_pointer;
	}

	return NULL;
}

/*
 * QEMU doesn't set compatible on cpus.
 * Add them to make sure the U-Boot driver properly bind.
 */
static int fdtdec_fix_cpus(void *fdt_blob)
{
	int cpus_offset, off, ret;
	u64 mpidr, i = 0;

	cpus_offset = fdt_path_offset(fdt_blob, "/cpus");
	if (cpus_offset < 0) {
		puts("couldn't find /cpus node\n");
		return cpus_offset;
	}

	fdt_for_each_subnode(off, fdt_blob, cpus_offset) {
		if (strncmp(fdt_get_name(fdt_blob, off, NULL), "cpu@", 4))
			continue;

		mpidr = 0;
		ret = smc_get_mpidr(i, &mpidr);
		if (ret) {
			log_warning("Failed to get MPIDR for processor %lld from SMC: %d\n",
				    i, ret);
			mpidr = i;
		}

		ret = fdt_setprop_string(fdt_blob, off, "compatible", "arm,armv8");
		if (ret < 0)
			return ret;

		ret = fdt_setprop_string(fdt_blob, off, "device_type", "cpu");
		if (ret < 0)
			return ret;

		ret = fdt_setprop_u64(fdt_blob, off, "reg", mpidr);
		if (ret < 0)
			return ret;
		i++;
	}
	return 0;
}

/*
 * Update the GIC node when necessary and add optional ITS when it has a
 * non zero base-address.
 */
static int fdtdec_fix_gic(void *fdt)
{
	u64 gic_dist_base = SBSA_GIC_DIST_BASE_ADDR;
	u64 gic_redist_base = SBSA_GIC_REDIST_BASE_ADDR;
	u64 gic_its_base = 0;
	int offs, ret;
	u64 reg[10];

	/* Invoke SMC to get real base-address */
	smc_get_gic_dist_base(&gic_dist_base);
	smc_get_gic_redist_base(&gic_redist_base);

	if ((gic_dist_base != SBSA_GIC_DIST_BASE_ADDR) ||
	    (gic_redist_base != SBSA_GIC_REDIST_BASE_ADDR)) {
		offs = fdt_path_offset(fdt, "/interrupt-controller");
		if (offs < 0) {
			puts("couldn't find /interrupt-controller node\n");
			return offs;
		}

		reg[0] = cpu_to_fdt64(gic_dist_base);
		reg[1] = cpu_to_fdt64((u64)SBSA_GIC_DIST_LENGTH);
		reg[2] = cpu_to_fdt64(gic_redist_base);
		reg[3] = cpu_to_fdt64((u64)SBSA_GIC_REDIST_LENGTH);
		reg[4] = cpu_to_fdt64(0);
		reg[5] = cpu_to_fdt64(0);
		reg[6] = cpu_to_fdt64(SBSA_GIC_HBASE_ADDR);
		reg[7] = cpu_to_fdt64((u64)SBSA_GIC_HBASE_LENGTH);
		reg[8] = cpu_to_fdt64(SBSA_GIC_VBASE_ADDR);
		reg[9] = cpu_to_fdt64((u64)SBSA_GIC_VBASE_LENGTH);

		ret = fdt_setprop_inplace(fdt, offs, "reg", reg, sizeof(reg));
	}

	smc_get_gic_its_base(&gic_its_base);

	if (gic_its_base != 0) {
		offs = fdt_path_offset(fdt, "/interrupt-controller/msi-controller");
		if (offs < 0)
			return offs;

		ret = fdt_setprop_string(fdt, offs, "status", "okay");
		if (ret < 0)
			return ret;

		reg[0] = cpu_to_fdt64(gic_its_base);
		reg[1] = 0;

		ret = fdt_setprop(fdt, offs, "reg", reg, sizeof(u64) * 2);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int fdtdec_board_setup(const void *fdt_blob)
{
	void *qemu_fdt;
	int ret;

	/*
	 * Locate the QEMU provided DTB that contains the CPUs and amount of DRAM.
	 */
	qemu_fdt = dtb_dt_qemu();
	if (!qemu_fdt) {
		log_err("QEMU FDT not found\n");
		return -ENODEV;
	}

	ret = fdt_increase_size((void *)fdt_blob, 1024 + fdt_totalsize(qemu_fdt));
	if (ret)
		return -ENOMEM;

	/*
	 * Merge the QEMU DTB as overlay into the U-Boot provided DTB.
	 */
	ret = fdt_overlay_apply_node((void *)fdt_blob, 0, qemu_fdt, 0);
	if (ret < 0)
		log_err("Failed to apply overlay: %d\n", ret);

	/* Fix QEMU nodes to make sure U-Boot drivers are properly working */
	ret = fdtdec_fix_cpus((void *)fdt_blob);
	if (ret < 0)
		log_err("Failed to fix CPUs in FDT: %d\n", ret);

	ret = fdtdec_fix_gic((void *)fdt_blob);
	if (ret < 0)
		log_err("Failed to fix INTC in FDT: %d\n", ret);

	return 0;
}

int misc_init_r(void)
{
	return env_set_hex("fdt_addr", (uintptr_t)gd->fdt_blob);
}

void reset_cpu(void)
{
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}