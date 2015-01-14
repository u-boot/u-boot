/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010,2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * Portions from Coreboot mainboard/google/link/romstage.c
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2011 Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/gpio.h>
#include <asm/global_data.h>
#include <asm/pci.h>
#include <asm/arch/me.h>
#include <asm/arch/pei_data.h>
#include <asm/arch/pch.h>
#include <asm/post.h>
#include <asm/arch/sandybridge.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary.
 * It overrides the default implementation found elsewhere which simply
 * picks the end of ram, wherever that may be. The location of the stack,
 * the relocation address, and how far U-Boot is moved by relocation are
 * set in the global data structure.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	struct memory_info *info = &gd->arch.meminfo;
	uintptr_t dest_addr = 0;
	struct memory_area *largest = NULL;
	int i;

	/* Find largest area of memory below 4GB */

	for (i = 0; i < info->num_areas; i++) {
		struct memory_area *area = &info->area[i];

		if (area->start >= 1ULL << 32)
			continue;
		if (!largest || area->size > largest->size)
			largest = area;
	}

	/* If no suitable area was found, return an error. */
	assert(largest);
	if (!largest || largest->size < (2 << 20))
		panic("No available memory found for relocation");

	dest_addr = largest->start + largest->size;

	return (ulong)dest_addr;
}

void dram_init_banksize(void)
{
	struct memory_info *info = &gd->arch.meminfo;
	int num_banks;
	int i;

	for (i = 0, num_banks = 0; i < info->num_areas; i++) {
		struct memory_area *area = &info->area[i];

		if (area->start >= 1ULL << 32)
			continue;
		gd->bd->bi_dram[num_banks].start = area->start;
		gd->bd->bi_dram[num_banks].size = area->size;
		num_banks++;
	}
}

static const char *const ecc_decoder[] = {
	"inactive",
	"active on IO",
	"disabled on IO",
	"active"
};

/*
 * Dump in the log memory controller configuration as read from the memory
 * controller registers.
 */
static void report_memory_config(void)
{
	u32 addr_decoder_common, addr_decode_ch[2];
	int i;

	addr_decoder_common = readl(MCHBAR_REG(0x5000));
	addr_decode_ch[0] = readl(MCHBAR_REG(0x5004));
	addr_decode_ch[1] = readl(MCHBAR_REG(0x5008));

	debug("memcfg DDR3 clock %d MHz\n",
	      (readl(MCHBAR_REG(0x5e04)) * 13333 * 2 + 50) / 100);
	debug("memcfg channel assignment: A: %d, B % d, C % d\n",
	      addr_decoder_common & 3,
	      (addr_decoder_common >> 2) & 3,
	      (addr_decoder_common >> 4) & 3);

	for (i = 0; i < ARRAY_SIZE(addr_decode_ch); i++) {
		u32 ch_conf = addr_decode_ch[i];
		debug("memcfg channel[%d] config (%8.8x):\n", i, ch_conf);
		debug("   ECC %s\n", ecc_decoder[(ch_conf >> 24) & 3]);
		debug("   enhanced interleave mode %s\n",
		      ((ch_conf >> 22) & 1) ? "on" : "off");
		debug("   rank interleave %s\n",
		      ((ch_conf >> 21) & 1) ? "on" : "off");
		debug("   DIMMA %d MB width x%d %s rank%s\n",
		      ((ch_conf >> 0) & 0xff) * 256,
		      ((ch_conf >> 19) & 1) ? 16 : 8,
		      ((ch_conf >> 17) & 1) ? "dual" : "single",
		      ((ch_conf >> 16) & 1) ? "" : ", selected");
		debug("   DIMMB %d MB width x%d %s rank%s\n",
		      ((ch_conf >> 8) & 0xff) * 256,
		      ((ch_conf >> 20) & 1) ? 16 : 8,
		      ((ch_conf >> 18) & 1) ? "dual" : "single",
		      ((ch_conf >> 16) & 1) ? ", selected" : "");
	}
}

static void post_system_agent_init(struct pei_data *pei_data)
{
	/* If PCIe init is skipped, set the PEG clock gating */
	if (!pei_data->pcie_init)
		setbits_le32(MCHBAR_REG(0x7010), 1);
}

static asmlinkage void console_tx_byte(unsigned char byte)
{
#ifdef DEBUG
	putc(byte);
#endif
}

/**
 * Find the PEI executable in the ROM and execute it.
 *
 * @param pei_data: configuration data for UEFI PEI reference code
 */
int sdram_initialise(struct pei_data *pei_data)
{
	unsigned version;
	const char *data;
	uint16_t done;
	int ret;

	report_platform_info();

	/* Wait for ME to be ready */
	ret = intel_early_me_init();
	if (ret)
		return ret;
	ret = intel_early_me_uma_size();
	if (ret < 0)
		return ret;

	debug("Starting UEFI PEI System Agent\n");

	/* If MRC data is not found we cannot continue S3 resume. */
	if (pei_data->boot_mode == PEI_BOOT_RESUME && !pei_data->mrc_input) {
		debug("Giving up in sdram_initialize: No MRC data\n");
		outb(0x6, PORT_RESET);
		cpu_hlt();
	}

	/* Pass console handler in pei_data */
	pei_data->tx_byte = console_tx_byte;

	debug("PEI data at %p, size %x:\n", pei_data, sizeof(*pei_data));

	data = (char *)CONFIG_X86_MRC_ADDR;
	if (data) {
		int rv;
		int (*func)(struct pei_data *);

		debug("Calling MRC at %p\n", data);
		post_code(POST_PRE_MRC);
		func = (int (*)(struct pei_data *))data;
		rv = func(pei_data);
		post_code(POST_MRC);
		if (rv) {
			switch (rv) {
			case -1:
				printf("PEI version mismatch.\n");
				break;
			case -2:
				printf("Invalid memory frequency.\n");
				break;
			default:
				printf("MRC returned %x.\n", rv);
			}
			printf("Nonzero MRC return value.\n");
			return -EFAULT;
		}
	} else {
		printf("UEFI PEI System Agent not found.\n");
		return -ENOSYS;
	}

#if CONFIG_USBDEBUG
	/* mrc.bin reconfigures USB, so reinit it to have debug */
	early_usbdebug_init();
#endif

	version = readl(MCHBAR_REG(0x5034));
	debug("System Agent Version %d.%d.%d Build %d\n",
	      version >> 24 , (version >> 16) & 0xff,
	      (version >> 8) & 0xff, version & 0xff);

	/*
	 * Send ME init done for SandyBridge here.  This is done inside the
	 * SystemAgent binary on IvyBridge
	 */
	done = pci_read_config32(PCH_DEV, PCI_DEVICE_ID);
	done &= BASE_REV_MASK;
	if (BASE_REV_SNB == done)
		intel_early_me_init_done(ME_INIT_STATUS_SUCCESS);
	else
		intel_early_me_status();

	post_system_agent_init(pei_data);
	report_memory_config();

	return 0;
}

static int copy_spd(struct pei_data *peid)
{
	const int gpio_vector[] = {41, 42, 43, 10, -1};
	int spd_index;
	const void *blob = gd->fdt_blob;
	int node, spd_node;
	int ret, i;

	for (i = 0; ; i++) {
		if (gpio_vector[i] == -1)
			break;
		ret = gpio_requestf(gpio_vector[i], "spd_id%d", i);
		if (ret) {
			debug("%s: Could not request gpio %d\n", __func__,
			      gpio_vector[i]);
			return ret;
		}
	}
	spd_index = gpio_get_values_as_int(gpio_vector);
	debug("spd index %d\n", spd_index);
	node = fdtdec_next_compatible(blob, 0, COMPAT_MEMORY_SPD);
	if (node < 0) {
		printf("SPD data not found.\n");
		return -ENOENT;
	}

	for (spd_node = fdt_first_subnode(blob, node);
	     spd_node > 0;
	     spd_node = fdt_next_subnode(blob, spd_node)) {
		const char *data;
		int len;

		if (fdtdec_get_int(blob, spd_node, "reg", -1) != spd_index)
			continue;
		data = fdt_getprop(blob, spd_node, "data", &len);
		if (len < sizeof(peid->spd_data[0])) {
			printf("Missing SPD data\n");
			return -EINVAL;
		}

		debug("Using SDRAM SPD data for '%s'\n",
		      fdt_get_name(blob, spd_node, NULL));
		memcpy(peid->spd_data[0], data, sizeof(peid->spd_data[0]));
		break;
	}

	if (spd_node < 0) {
		printf("No SPD data found for index %d\n", spd_index);
		return -ENOENT;
	}

	return 0;
}

/**
 * add_memory_area() - Add a new usable memory area to our list
 *
 * Note: @start and @end must not span the first 4GB boundary
 *
 * @info:	Place to store memory info
 * @start:	Start of this memory area
 * @end:	End of this memory area + 1
 */
static int add_memory_area(struct memory_info *info,
			   uint64_t start, uint64_t end)
{
	struct memory_area *ptr;

	if (info->num_areas == CONFIG_NR_DRAM_BANKS)
		return -ENOSPC;

	ptr = &info->area[info->num_areas];
	ptr->start = start;
	ptr->size = end - start;
	info->total_memory += ptr->size;
	if (ptr->start < (1ULL << 32))
		info->total_32bit_memory += ptr->size;
	debug("%d: memory %llx size %llx, total now %llx / %llx\n",
	      info->num_areas, ptr->start, ptr->size,
	      info->total_32bit_memory, info->total_memory);
	info->num_areas++;

	return 0;
}

/**
 * sdram_find() - Find available memory
 *
 * This is a bit complicated since on x86 there are system memory holes all
 * over the place. We create a list of available memory blocks
 */
static int sdram_find(pci_dev_t dev)
{
	struct memory_info *info = &gd->arch.meminfo;
	uint32_t tseg_base, uma_size, tolud;
	uint64_t tom, me_base, touud;
	uint64_t uma_memory_base = 0;
	uint64_t uma_memory_size;
	unsigned long long tomk;
	uint16_t ggc;

	/* Total Memory 2GB example:
	 *
	 *  00000000  0000MB-1992MB  1992MB  RAM     (writeback)
	 *  7c800000  1992MB-2000MB     8MB  TSEG    (SMRR)
	 *  7d000000  2000MB-2002MB     2MB  GFX GTT (uncached)
	 *  7d200000  2002MB-2034MB    32MB  GFX UMA (uncached)
	 *  7f200000   2034MB TOLUD
	 *  7f800000   2040MB MEBASE
	 *  7f800000  2040MB-2048MB     8MB  ME UMA  (uncached)
	 *  80000000   2048MB TOM
	 * 100000000  4096MB-4102MB     6MB  RAM     (writeback)
	 *
	 * Total Memory 4GB example:
	 *
	 *  00000000  0000MB-2768MB  2768MB  RAM     (writeback)
	 *  ad000000  2768MB-2776MB     8MB  TSEG    (SMRR)
	 *  ad800000  2776MB-2778MB     2MB  GFX GTT (uncached)
	 *  ada00000  2778MB-2810MB    32MB  GFX UMA (uncached)
	 *  afa00000   2810MB TOLUD
	 *  ff800000   4088MB MEBASE
	 *  ff800000  4088MB-4096MB     8MB  ME UMA  (uncached)
	 * 100000000   4096MB TOM
	 * 100000000  4096MB-5374MB  1278MB  RAM     (writeback)
	 * 14fe00000   5368MB TOUUD
	 */

	/* Top of Upper Usable DRAM, including remap */
	touud = pci_read_config32(dev, TOUUD+4);
	touud <<= 32;
	touud |= pci_read_config32(dev, TOUUD);

	/* Top of Lower Usable DRAM */
	tolud = pci_read_config32(dev, TOLUD);

	/* Top of Memory - does not account for any UMA */
	tom = pci_read_config32(dev, 0xa4);
	tom <<= 32;
	tom |= pci_read_config32(dev, 0xa0);

	debug("TOUUD %llx TOLUD %08x TOM %llx\n", touud, tolud, tom);

	/* ME UMA needs excluding if total memory <4GB */
	me_base = pci_read_config32(dev, 0x74);
	me_base <<= 32;
	me_base |= pci_read_config32(dev, 0x70);

	debug("MEBASE %llx\n", me_base);

	/* TODO: Get rid of all this shifting by 10 bits */
	tomk = tolud >> 10;
	if (me_base == tolud) {
		/* ME is from MEBASE-TOM */
		uma_size = (tom - me_base) >> 10;
		/* Increment TOLUD to account for ME as RAM */
		tolud += uma_size << 10;
		/* UMA starts at old TOLUD */
		uma_memory_base = tomk * 1024ULL;
		uma_memory_size = uma_size * 1024ULL;
		debug("ME UMA base %llx size %uM\n", me_base, uma_size >> 10);
	}

	/* Graphics memory comes next */
	ggc = pci_read_config16(dev, GGC);
	if (!(ggc & 2)) {
		debug("IGD decoded, subtracting ");

		/* Graphics memory */
		uma_size = ((ggc >> 3) & 0x1f) * 32 * 1024ULL;
		debug("%uM UMA", uma_size >> 10);
		tomk -= uma_size;
		uma_memory_base = tomk * 1024ULL;
		uma_memory_size += uma_size * 1024ULL;

		/* GTT Graphics Stolen Memory Size (GGMS) */
		uma_size = ((ggc >> 8) & 0x3) * 1024ULL;
		tomk -= uma_size;
		uma_memory_base = tomk * 1024ULL;
		uma_memory_size += uma_size * 1024ULL;
		debug(" and %uM GTT\n", uma_size >> 10);
	}

	/* Calculate TSEG size from its base which must be below GTT */
	tseg_base = pci_read_config32(dev, 0xb8);
	uma_size = (uma_memory_base - tseg_base) >> 10;
	tomk -= uma_size;
	uma_memory_base = tomk * 1024ULL;
	uma_memory_size += uma_size * 1024ULL;
	debug("TSEG base 0x%08x size %uM\n", tseg_base, uma_size >> 10);

	debug("Available memory below 4GB: %lluM\n", tomk >> 10);

	/* Report the memory regions */
	add_memory_area(info, 1 << 20, 2 << 28);
	add_memory_area(info, (2 << 28) + (2 << 20), 4 << 28);
	add_memory_area(info, (4 << 28) + (2 << 20), tseg_base);
	add_memory_area(info, 1ULL << 32, touud);
	/*
	 * If >= 4GB installed then memory from TOLUD to 4GB
	 * is remapped above TOM, TOUUD will account for both
	 */
	if (touud > (1ULL << 32ULL)) {
		debug("Available memory above 4GB: %lluM\n",
		      (touud >> 20) - 4096);
	}

	return 0;
}

static void rcba_config(void)
{
	/*
	 *             GFX    INTA -> PIRQA (MSI)
	 * D28IP_P3IP  WLAN   INTA -> PIRQB
	 * D29IP_E1P   EHCI1  INTA -> PIRQD
	 * D26IP_E2P   EHCI2  INTA -> PIRQF
	 * D31IP_SIP   SATA   INTA -> PIRQF (MSI)
	 * D31IP_SMIP  SMBUS  INTB -> PIRQH
	 * D31IP_TTIP  THRT   INTC -> PIRQA
	 * D27IP_ZIP   HDA    INTA -> PIRQA (MSI)
	 *
	 * TRACKPAD                -> PIRQE (Edge Triggered)
	 * TOUCHSCREEN             -> PIRQG (Edge Triggered)
	 */

	/* Device interrupt pin register (board specific) */
	writel((INTC << D31IP_TTIP) | (NOINT << D31IP_SIP2) |
	       (INTB << D31IP_SMIP) | (INTA << D31IP_SIP), RCB_REG(D31IP));
	writel(NOINT << D30IP_PIP, RCB_REG(D30IP));
	writel(INTA << D29IP_E1P, RCB_REG(D29IP));
	writel(INTA << D28IP_P3IP, RCB_REG(D28IP));
	writel(INTA << D27IP_ZIP, RCB_REG(D27IP));
	writel(INTA << D26IP_E2P, RCB_REG(D26IP));
	writel(NOINT << D25IP_LIP, RCB_REG(D25IP));
	writel(NOINT << D22IP_MEI1IP, RCB_REG(D22IP));

	/* Device interrupt route registers */
	writel(DIR_ROUTE(PIRQB, PIRQH, PIRQA, PIRQC), RCB_REG(D31IR));
	writel(DIR_ROUTE(PIRQD, PIRQE, PIRQF, PIRQG), RCB_REG(D29IR));
	writel(DIR_ROUTE(PIRQB, PIRQC, PIRQD, PIRQE), RCB_REG(D28IR));
	writel(DIR_ROUTE(PIRQA, PIRQH, PIRQA, PIRQB), RCB_REG(D27IR));
	writel(DIR_ROUTE(PIRQF, PIRQE, PIRQG, PIRQH), RCB_REG(D26IR));
	writel(DIR_ROUTE(PIRQA, PIRQB, PIRQC, PIRQD), RCB_REG(D25IR));
	writel(DIR_ROUTE(PIRQA, PIRQB, PIRQC, PIRQD), RCB_REG(D22IR));

	/* Enable IOAPIC (generic) */
	writew(0x0100, RCB_REG(OIC));
	/* PCH BWG says to read back the IOAPIC enable register */
	(void)readw(RCB_REG(OIC));

	/* Disable unused devices (board specific) */
	setbits_le32(RCB_REG(FD), PCH_DISABLE_ALWAYS);
}

int dram_init(void)
{
	struct pei_data pei_data __aligned(8) = {
		.pei_version = PEI_VERSION,
		.mchbar = DEFAULT_MCHBAR,
		.dmibar = DEFAULT_DMIBAR,
		.epbar = DEFAULT_EPBAR,
		.pciexbar = CONFIG_MMCONF_BASE_ADDRESS,
		.smbusbar = SMBUS_IO_BASE,
		.wdbbar = 0x4000000,
		.wdbsize = 0x1000,
		.hpet_address = CONFIG_HPET_ADDRESS,
		.rcba = DEFAULT_RCBABASE,
		.pmbase = DEFAULT_PMBASE,
		.gpiobase = DEFAULT_GPIOBASE,
		.thermalbase = 0xfed08000,
		.system_type = 0, /* 0 Mobile, 1 Desktop/Server */
		.tseg_size = CONFIG_SMM_TSEG_SIZE,
		.ts_addresses = { 0x00, 0x00, 0x00, 0x00 },
		.ec_present = 1,
		.ddr3lv_support = 1,
		/*
		 * 0 = leave channel enabled
		 * 1 = disable dimm 0 on channel
		 * 2 = disable dimm 1 on channel
		 * 3 = disable dimm 0+1 on channel
		 */
		.dimm_channel0_disabled = 2,
		.dimm_channel1_disabled = 2,
		.max_ddr3_freq = 1600,
		.usb_port_config = {
			/*
			 * Empty and onboard Ports 0-7, set to un-used pin
			 * OC3
			 */
			{ 0, 3, 0x0000 }, /* P0= Empty */
			{ 1, 0, 0x0040 }, /* P1= Left USB 1  (OC0) */
			{ 1, 1, 0x0040 }, /* P2= Left USB 2  (OC1) */
			{ 1, 3, 0x0040 }, /* P3= SDCARD      (no OC) */
			{ 0, 3, 0x0000 }, /* P4= Empty */
			{ 1, 3, 0x0040 }, /* P5= WWAN        (no OC) */
			{ 0, 3, 0x0000 }, /* P6= Empty */
			{ 0, 3, 0x0000 }, /* P7= Empty */
			/*
			 * Empty and onboard Ports 8-13, set to un-used pin
			 * OC4
			 */
			{ 1, 4, 0x0040 }, /* P8= Camera      (no OC) */
			{ 1, 4, 0x0040 }, /* P9= Bluetooth   (no OC) */
			{ 0, 4, 0x0000 }, /* P10= Empty */
			{ 0, 4, 0x0000 }, /* P11= Empty */
			{ 0, 4, 0x0000 }, /* P12= Empty */
			{ 0, 4, 0x0000 }, /* P13= Empty */
		},
	};
	pci_dev_t dev = PCI_BDF(0, 0, 0);
	int ret;

	debug("Boot mode %d\n", gd->arch.pei_boot_mode);
	debug("mcr_input %p\n", pei_data.mrc_input);
	pei_data.boot_mode = gd->arch.pei_boot_mode;
	ret = copy_spd(&pei_data);
	if (!ret)
		ret = sdram_initialise(&pei_data);
	if (ret)
		return ret;

	rcba_config();
	quick_ram_check();

	writew(0xCAFE, MCHBAR_REG(SSKPD));

	post_code(POST_DRAM);

	ret = sdram_find(dev);
	if (ret)
		return ret;

	gd->ram_size = gd->arch.meminfo.total_32bit_memory;

	return 0;
}
