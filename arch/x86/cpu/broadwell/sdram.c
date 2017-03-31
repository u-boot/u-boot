/*
 * Copyright (c) 2016 Google, Inc
 *
 * From coreboot src/soc/intel/broadwell/romstage/raminit.c
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <dm.h>
#include <pci.h>
#include <syscon.h>
#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/lpc_common.h>
#include <asm/mrccache.h>
#include <asm/mrc_common.h>
#include <asm/mtrr.h>
#include <asm/pci.h>
#include <asm/arch/iomap.h>
#include <asm/arch/me.h>
#include <asm/arch/pch.h>
#include <asm/arch/pei_data.h>
#include <asm/arch/pm.h>

ulong board_get_usable_ram_top(ulong total_size)
{
	return mrc_common_board_get_usable_ram_top(total_size);
}

int dram_init_banksize(void)
{
	mrc_common_dram_init_banksize();

	return 0;
}

void broadwell_fill_pei_data(struct pei_data *pei_data)
{
	pei_data->pei_version = PEI_VERSION;
	pei_data->board_type = BOARD_TYPE_ULT;
	pei_data->pciexbar = MCFG_BASE_ADDRESS;
	pei_data->smbusbar = SMBUS_BASE_ADDRESS;
	pei_data->ehcibar = EARLY_EHCI_BAR;
	pei_data->xhcibar = EARLY_XHCI_BAR;
	pei_data->gttbar = EARLY_GTT_BAR;
	pei_data->pmbase = ACPI_BASE_ADDRESS;
	pei_data->gpiobase = GPIO_BASE_ADDRESS;
	pei_data->tseg_size = CONFIG_SMM_TSEG_SIZE;
	pei_data->temp_mmio_base = EARLY_TEMP_MMIO;
	pei_data->tx_byte = sdram_console_tx_byte;
	pei_data->ddr_refresh_2x = 1;
}

static inline void pei_data_usb2_port(struct pei_data *pei_data, int port,
				      uint16_t length, uint8_t enable,
				      uint8_t oc_pin, uint8_t location)
{
	pei_data->usb2_ports[port].length   = length;
	pei_data->usb2_ports[port].enable   = enable;
	pei_data->usb2_ports[port].oc_pin   = oc_pin;
	pei_data->usb2_ports[port].location = location;
}

static inline void pei_data_usb3_port(struct pei_data *pei_data, int port,
				      uint8_t enable, uint8_t oc_pin,
				      uint8_t fixed_eq)
{
	pei_data->usb3_ports[port].enable   = enable;
	pei_data->usb3_ports[port].oc_pin   = oc_pin;
	pei_data->usb3_ports[port].fixed_eq = fixed_eq;
}

void mainboard_fill_pei_data(struct pei_data *pei_data)
{
	/* DQ byte map for Samus board */
	const u8 dq_map[2][6][2] = {
		{ { 0x0F, 0xF0 }, { 0x00, 0xF0 }, { 0x0F, 0xF0 },
		  { 0x0F, 0x00 }, { 0xFF, 0x00 }, { 0xFF, 0x00 } },
		{ { 0x0F, 0xF0 }, { 0x00, 0xF0 }, { 0x0F, 0xF0 },
		  { 0x0F, 0x00 }, { 0xFF, 0x00 }, { 0xFF, 0x00 } } };
	/* DQS CPU<>DRAM map for Samus board */
	const u8 dqs_map[2][8] = {
		{ 2, 0, 1, 3, 6, 4, 7, 5 },
		{ 2, 1, 0, 3, 6, 5, 4, 7 } };

	pei_data->ec_present = 1;

	/* One installed DIMM per channel */
	pei_data->dimm_channel0_disabled = 2;
	pei_data->dimm_channel1_disabled = 2;

	memcpy(pei_data->dq_map, dq_map, sizeof(dq_map));
	memcpy(pei_data->dqs_map, dqs_map, sizeof(dqs_map));

	/* P0: HOST PORT */
	pei_data_usb2_port(pei_data, 0, 0x0080, 1, 0,
			   USB_PORT_BACK_PANEL);
	/* P1: HOST PORT */
	pei_data_usb2_port(pei_data, 1, 0x0080, 1, 1,
			   USB_PORT_BACK_PANEL);
	/* P2: RAIDEN */
	pei_data_usb2_port(pei_data, 2, 0x0080, 1, USB_OC_PIN_SKIP,
			   USB_PORT_BACK_PANEL);
	/* P3: SD CARD */
	pei_data_usb2_port(pei_data, 3, 0x0040, 1, USB_OC_PIN_SKIP,
			   USB_PORT_INTERNAL);
	/* P4: RAIDEN */
	pei_data_usb2_port(pei_data, 4, 0x0080, 1, USB_OC_PIN_SKIP,
			   USB_PORT_BACK_PANEL);
	/* P5: WWAN (Disabled) */
	pei_data_usb2_port(pei_data, 5, 0x0000, 0, USB_OC_PIN_SKIP,
			   USB_PORT_SKIP);
	/* P6: CAMERA */
	pei_data_usb2_port(pei_data, 6, 0x0040, 1, USB_OC_PIN_SKIP,
			   USB_PORT_INTERNAL);
	/* P7: BT */
	pei_data_usb2_port(pei_data, 7, 0x0040, 1, USB_OC_PIN_SKIP,
			   USB_PORT_INTERNAL);

	/* P1: HOST PORT */
	pei_data_usb3_port(pei_data, 0, 1, 0, 0);
	/* P2: HOST PORT */
	pei_data_usb3_port(pei_data, 1, 1, 1, 0);
	/* P3: RAIDEN */
	pei_data_usb3_port(pei_data, 2, 1, USB_OC_PIN_SKIP, 0);
	/* P4: RAIDEN */
	pei_data_usb3_port(pei_data, 3, 1, USB_OC_PIN_SKIP, 0);
}

static unsigned long get_top_of_ram(struct udevice *dev)
{
	/*
	 * Base of DPR is top of usable DRAM below 4GiB. The register has
	 * 1 MiB alignment and reports the TOP of the range, the base
	 * must be calculated from the size in MiB in bits 11:4.
	 */
	u32 dpr, tom;

	dm_pci_read_config32(dev, DPR, &dpr);
	tom = dpr & ~((1 << 20) - 1);

	debug("dpt %08x tom %08x\n", dpr, tom);
	/* Subtract DMA Protected Range size if enabled */
	if (dpr & DPR_EPM)
		tom -= (dpr & DPR_SIZE_MASK) << 16;

	return (unsigned long)tom;
}

/**
 * sdram_find() - Find available memory
 *
 * This is a bit complicated since on x86 there are system memory holes all
 * over the place. We create a list of available memory blocks
 *
 * @dev:	Northbridge device
 */
static int sdram_find(struct udevice *dev)
{
	struct memory_info *info = &gd->arch.meminfo;
	ulong top_of_ram;

	top_of_ram = get_top_of_ram(dev);
	mrc_add_memory_area(info, 0, top_of_ram);

	/* Add MTRRs for memory */
	mtrr_add_request(MTRR_TYPE_WRBACK, 0, 2ULL << 30);

	return 0;
}

static int prepare_mrc_cache(struct pei_data *pei_data)
{
	struct mrc_data_container *mrc_cache;
	struct mrc_region entry;
	int ret;

	ret = mrccache_get_region(NULL, &entry);
	if (ret)
		return ret;
	mrc_cache = mrccache_find_current(&entry);
	if (!mrc_cache)
		return -ENOENT;

	pei_data->saved_data = mrc_cache->data;
	pei_data->saved_data_size = mrc_cache->data_size;
	debug("%s: at %p, size %x checksum %04x\n", __func__,
	      pei_data->saved_data, pei_data->saved_data_size,
	      mrc_cache->checksum);

	return 0;
}

int dram_init(void)
{
	struct pei_data _pei_data __aligned(8);
	struct pei_data *pei_data = &_pei_data;
	struct udevice *dev, *me_dev, *pch_dev;
	struct chipset_power_state ps;
	const void *spd_data;
	int ret, size;

	memset(pei_data, '\0', sizeof(struct pei_data));

	/* Print ME state before MRC */
	ret = syscon_get_by_driver_data(X86_SYSCON_ME, &me_dev);
	if (ret)
		return ret;
	intel_me_status(me_dev);

	/* Save ME HSIO version */
	ret = uclass_first_device(UCLASS_PCH, &pch_dev);
	if (ret)
		return ret;
	if (!pch_dev)
		return -ENODEV;
	power_state_get(pch_dev, &ps);

	intel_me_hsio_version(me_dev, &ps.hsio_version, &ps.hsio_checksum);

	broadwell_fill_pei_data(pei_data);
	mainboard_fill_pei_data(pei_data);

	ret = uclass_first_device(UCLASS_NORTHBRIDGE, &dev);
	if (ret)
		return ret;
	if (!dev)
		return -ENODEV;
	size = 256;
	ret = mrc_locate_spd(dev, size, &spd_data);
	if (ret)
		return ret;
	memcpy(pei_data->spd_data[0][0], spd_data, size);
	memcpy(pei_data->spd_data[1][0], spd_data, size);

	ret = prepare_mrc_cache(pei_data);
	if (ret)
		debug("prepare_mrc_cache failed: %d\n", ret);

	debug("PEI version %#x\n", pei_data->pei_version);
	ret = mrc_common_init(dev, pei_data, true);
	if (ret)
		return ret;
	debug("Memory init done\n");

	ret = sdram_find(dev);
	if (ret)
		return ret;
	gd->ram_size = gd->arch.meminfo.total_32bit_memory;
	debug("RAM size %llx\n", (unsigned long long)gd->ram_size);

	debug("MRC output data length %#x at %p\n", pei_data->data_to_save_size,
	      pei_data->data_to_save);
	/* S3 resume: don't save scrambler seed or MRC data */
	if (pei_data->boot_mode != SLEEP_STATE_S3) {
		/*
		 * This will be copied to SDRAM in reserve_arch(), then written
		 * to SPI flash in mrccache_save()
		 */
		gd->arch.mrc_output = (char *)pei_data->data_to_save;
		gd->arch.mrc_output_len = pei_data->data_to_save_size;
	}
	gd->arch.pei_meminfo = pei_data->meminfo;

	return 0;
}

/* Use this hook to save our SDRAM parameters */
int misc_init_r(void)
{
	int ret;

	ret = mrccache_save();
	if (ret)
		printf("Unable to save MRC data: %d\n", ret);
	else
		debug("Saved MRC cache data\n");

	return 0;
}

void board_debug_uart_init(void)
{
	struct udevice *bus = NULL;

	/* com1 / com2 decode range */
	pci_x86_write_config(bus, PCH_DEV_LPC, LPC_IO_DEC, 1 << 4, PCI_SIZE_16);

	pci_x86_write_config(bus, PCH_DEV_LPC, LPC_EN, COMA_LPC_EN,
			     PCI_SIZE_16);
}

static const struct udevice_id broadwell_syscon_ids[] = {
	{ .compatible = "intel,me", .data = X86_SYSCON_ME },
	{ }
};

U_BOOT_DRIVER(syscon_intel_me) = {
	.name = "intel_me_syscon",
	.id = UCLASS_SYSCON,
	.of_match = broadwell_syscon_ids,
};
