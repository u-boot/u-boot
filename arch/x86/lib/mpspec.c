/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Adapted from coreboot src/arch/x86/boot/mpspec.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <asm/cpu.h>
#include <asm/ioapic.h>
#include <asm/lapic.h>
#include <asm/mpspec.h>
#include <asm/tables.h>
#include <dm/uclass-internal.h>

struct mp_config_table *mp_write_floating_table(struct mp_floating_table *mf)
{
	u32 mc;

	memcpy(mf->mpf_signature, MPF_SIGNATURE, 4);
	mf->mpf_physptr = (u32)mf + sizeof(struct mp_floating_table);
	mf->mpf_length = 1;
	mf->mpf_spec = MPSPEC_V14;
	mf->mpf_checksum = 0;
	/* We don't use the default configuration table */
	mf->mpf_feature1 = 0;
	/* Indicate that virtual wire mode is always implemented */
	mf->mpf_feature2 = 0;
	mf->mpf_feature3 = 0;
	mf->mpf_feature4 = 0;
	mf->mpf_feature5 = 0;
	mf->mpf_checksum = table_compute_checksum(mf, mf->mpf_length * 16);

	mc = (u32)mf + sizeof(struct mp_floating_table);
	return (struct mp_config_table *)mc;
}

void mp_config_table_init(struct mp_config_table *mc)
{
	memcpy(mc->mpc_signature, MPC_SIGNATURE, 4);
	mc->mpc_length = sizeof(struct mp_config_table);
	mc->mpc_spec = MPSPEC_V14;
	mc->mpc_checksum = 0;
	mc->mpc_oemptr = 0;
	mc->mpc_oemsize = 0;
	mc->mpc_entry_count = 0;
	mc->mpc_lapic = LAPIC_DEFAULT_BASE;
	mc->mpe_length = 0;
	mc->mpe_checksum = 0;
	mc->reserved = 0;

	/* The oem/product id fields are exactly 8/12 bytes long */
	table_fill_string(mc->mpc_oem, CONFIG_SYS_VENDOR, 8, ' ');
	table_fill_string(mc->mpc_product, CONFIG_SYS_BOARD, 12, ' ');
}

void mp_write_processor(struct mp_config_table *mc)
{
	struct mpc_config_processor *mpc;
	struct udevice *dev;
	u8 boot_apicid, apicver;
	u32 cpusignature, cpufeature;
	struct cpuid_result result;

	boot_apicid = lapicid();
	apicver = lapic_read(LAPIC_LVR) & 0xff;
	result = cpuid(1);
	cpusignature = result.eax;
	cpufeature = result.edx;

	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct cpu_platdata *plat = dev_get_parent_platdata(dev);
		u8 cpuflag = MPC_CPU_EN;

		if (!device_active(dev))
			continue;

		mpc = (struct mpc_config_processor *)mp_next_mpc_entry(mc);
		mpc->mpc_type = MP_PROCESSOR;
		mpc->mpc_apicid = plat->cpu_id;
		mpc->mpc_apicver = apicver;
		if (boot_apicid == plat->cpu_id)
			cpuflag |= MPC_CPU_BP;
		mpc->mpc_cpuflag = cpuflag;
		mpc->mpc_cpusignature = cpusignature;
		mpc->mpc_cpufeature = cpufeature;
		mpc->mpc_reserved[0] = 0;
		mpc->mpc_reserved[1] = 0;
		mp_add_mpc_entry(mc, sizeof(*mpc));
	}
}

void mp_write_bus(struct mp_config_table *mc, int id, const char *bustype)
{
	struct mpc_config_bus *mpc;

	mpc = (struct mpc_config_bus *)mp_next_mpc_entry(mc);
	mpc->mpc_type = MP_BUS;
	mpc->mpc_busid = id;
	memcpy(mpc->mpc_bustype, bustype, 6);
	mp_add_mpc_entry(mc, sizeof(*mpc));
}

void mp_write_ioapic(struct mp_config_table *mc, int id, int ver, u32 apicaddr)
{
	struct mpc_config_ioapic *mpc;

	mpc = (struct mpc_config_ioapic *)mp_next_mpc_entry(mc);
	mpc->mpc_type = MP_IOAPIC;
	mpc->mpc_apicid = id;
	mpc->mpc_apicver = ver;
	mpc->mpc_flags = MPC_APIC_USABLE;
	mpc->mpc_apicaddr = apicaddr;
	mp_add_mpc_entry(mc, sizeof(*mpc));
}

void mp_write_intsrc(struct mp_config_table *mc, int irqtype, int irqflag,
		     int srcbus, int srcbusirq, int dstapic, int dstirq)
{
	struct mpc_config_intsrc *mpc;

	mpc = (struct mpc_config_intsrc *)mp_next_mpc_entry(mc);
	mpc->mpc_type = MP_INTSRC;
	mpc->mpc_irqtype = irqtype;
	mpc->mpc_irqflag = irqflag;
	mpc->mpc_srcbus = srcbus;
	mpc->mpc_srcbusirq = srcbusirq;
	mpc->mpc_dstapic = dstapic;
	mpc->mpc_dstirq = dstirq;
	mp_add_mpc_entry(mc, sizeof(*mpc));
}

void mp_write_pci_intsrc(struct mp_config_table *mc, int irqtype,
			 int srcbus, int dev, int pin, int dstapic, int dstirq)
{
	u8 srcbusirq = (dev << 2) | (pin - 1);

	mp_write_intsrc(mc, irqtype, MP_IRQ_TRIGGER_LEVEL | MP_IRQ_POLARITY_LOW,
			srcbus, srcbusirq, dstapic, dstirq);
}

void mp_write_lintsrc(struct mp_config_table *mc, int irqtype, int irqflag,
		      int srcbus, int srcbusirq, int destapic, int destlint)
{
	struct mpc_config_lintsrc *mpc;

	mpc = (struct mpc_config_lintsrc *)mp_next_mpc_entry(mc);
	mpc->mpc_type = MP_LINTSRC;
	mpc->mpc_irqtype = irqtype;
	mpc->mpc_irqflag = irqflag;
	mpc->mpc_srcbusid = srcbus;
	mpc->mpc_srcbusirq = srcbusirq;
	mpc->mpc_destapic = destapic;
	mpc->mpc_destlint = destlint;
	mp_add_mpc_entry(mc, sizeof(*mpc));
}

void mp_write_address_space(struct mp_config_table *mc,
			    int busid, int addr_type,
			    u32 addr_base_low, u32 addr_base_high,
			    u32 addr_length_low, u32 addr_length_high)
{
	struct mp_ext_system_address_space *mpe;

	mpe = (struct mp_ext_system_address_space *)mp_next_mpe_entry(mc);
	mpe->mpe_type = MPE_SYSTEM_ADDRESS_SPACE;
	mpe->mpe_length = sizeof(*mpe);
	mpe->mpe_busid = busid;
	mpe->mpe_addr_type = addr_type;
	mpe->mpe_addr_base_low = addr_base_low;
	mpe->mpe_addr_base_high = addr_base_high;
	mpe->mpe_addr_length_low = addr_length_low;
	mpe->mpe_addr_length_high = addr_length_high;
	mp_add_mpe_entry(mc, (struct mp_ext_config *)mpe);
}

void mp_write_bus_hierarchy(struct mp_config_table *mc,
			    int busid, int bus_info, int parent_busid)
{
	struct mp_ext_bus_hierarchy *mpe;

	mpe = (struct mp_ext_bus_hierarchy *)mp_next_mpe_entry(mc);
	mpe->mpe_type = MPE_BUS_HIERARCHY;
	mpe->mpe_length = sizeof(*mpe);
	mpe->mpe_busid = busid;
	mpe->mpe_bus_info = bus_info;
	mpe->mpe_parent_busid = parent_busid;
	mpe->reserved[0] = 0;
	mpe->reserved[1] = 0;
	mpe->reserved[2] = 0;
	mp_add_mpe_entry(mc, (struct mp_ext_config *)mpe);
}

void mp_write_compat_address_space(struct mp_config_table *mc, int busid,
				   int addr_modifier, u32 range_list)
{
	struct mp_ext_compat_address_space *mpe;

	mpe = (struct mp_ext_compat_address_space *)mp_next_mpe_entry(mc);
	mpe->mpe_type = MPE_COMPAT_ADDRESS_SPACE;
	mpe->mpe_length = sizeof(*mpe);
	mpe->mpe_busid = busid;
	mpe->mpe_addr_modifier = addr_modifier;
	mpe->mpe_range_list = range_list;
	mp_add_mpe_entry(mc, (struct mp_ext_config *)mpe);
}

u32 mptable_finalize(struct mp_config_table *mc)
{
	u32 end;

	mc->mpe_checksum = table_compute_checksum((void *)mp_next_mpc_entry(mc),
						  mc->mpe_length);
	mc->mpc_checksum = table_compute_checksum(mc, mc->mpc_length);
	end = mp_next_mpe_entry(mc);

	debug("Write the MP table at: %x - %x\n", (u32)mc, end);

	return end;
}
