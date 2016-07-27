/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <sata.h>
#include <ahci.h>
#include <scsi.h>
#include <malloc.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	printf("EL Level:\tEL%d\n", current_el());

	return 0;
}

int board_early_init_r(void)
{
	u32 val;

	if (current_el() == 3) {
		val = readl(&crlapb_base->timestamp_ref_ctrl);
		val |= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;
		writel(val, &crlapb_base->timestamp_ref_ctrl);

		/* Program freq register in System counter */
		writel(zynqmp_get_system_timer_freq(),
		       &iou_scntr_secure->base_frequency_id_register);
		/* And enable system counter */
		writel(ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN,
		       &iou_scntr_secure->counter_control_register);
	}
	/* Program freq register in System counter and enable system counter */
	writel(gd->cpu_clk, &iou_scntr->base_frequency_id_register);
	writel(ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_HDBG |
	       ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN,
	       &iou_scntr->counter_control_register);

	return 0;
}

int zynq_board_read_rom_ethaddr(unsigned char *ethaddr)
{
#if defined(CONFIG_ZYNQ_GEM_EEPROM_ADDR) && \
    defined(CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET) && \
    defined(CONFIG_ZYNQ_EEPROM_BUS)
	i2c_set_bus_num(CONFIG_ZYNQ_EEPROM_BUS);

	if (eeprom_read(CONFIG_ZYNQ_GEM_EEPROM_ADDR,
			CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET,
			ethaddr, 6))
		printf("I2C EEPROM MAC address read failed\n");
#endif

	return 0;
}

#if !defined(CONFIG_SYS_SDRAM_BASE) && !defined(CONFIG_SYS_SDRAM_SIZE)
/*
 * fdt_get_reg - Fill buffer by information from DT
 */
static phys_size_t fdt_get_reg(const void *fdt, int nodeoffset, void *buf,
			       const u32 *cell, int n)
{
	int i = 0, b, banks;
	int parent_offset = fdt_parent_offset(fdt, nodeoffset);
	int address_cells = fdt_address_cells(fdt, parent_offset);
	int size_cells = fdt_size_cells(fdt, parent_offset);
	char *p = buf;
	u64 val;
	u64 vals;

	debug("%s: addr_cells=%x, size_cell=%x, buf=%p, cell=%p\n",
	      __func__, address_cells, size_cells, buf, cell);

	/* Check memory bank setup */
	banks = n % (address_cells + size_cells);
	if (banks)
		panic("Incorrect memory setup cells=%d, ac=%d, sc=%d\n",
		      n, address_cells, size_cells);

	banks = n / (address_cells + size_cells);

	for (b = 0; b < banks; b++) {
		debug("%s: Bank #%d:\n", __func__, b);
		if (address_cells == 2) {
			val = cell[i + 1];
			val <<= 32;
			val |= cell[i];
			val = fdt64_to_cpu(val);
			debug("%s: addr64=%llx, ptr=%p, cell=%p\n",
			      __func__, val, p, &cell[i]);
			*(phys_addr_t *)p = val;
		} else {
			debug("%s: addr32=%x, ptr=%p\n",
			      __func__, fdt32_to_cpu(cell[i]), p);
			*(phys_addr_t *)p = fdt32_to_cpu(cell[i]);
		}
		p += sizeof(phys_addr_t);
		i += address_cells;

		debug("%s: pa=%p, i=%x, size=%zu\n", __func__, p, i,
		      sizeof(phys_addr_t));

		if (size_cells == 2) {
			vals = cell[i + 1];
			vals <<= 32;
			vals |= cell[i];
			vals = fdt64_to_cpu(vals);

			debug("%s: size64=%llx, ptr=%p, cell=%p\n",
			      __func__, vals, p, &cell[i]);
			*(phys_size_t *)p = vals;
		} else {
			debug("%s: size32=%x, ptr=%p\n",
			      __func__, fdt32_to_cpu(cell[i]), p);
			*(phys_size_t *)p = fdt32_to_cpu(cell[i]);
		}
		p += sizeof(phys_size_t);
		i += size_cells;

		debug("%s: ps=%p, i=%x, size=%zu\n",
		      __func__, p, i, sizeof(phys_size_t));
	}

	/* Return the first address size */
	return *(phys_size_t *)((char *)buf + sizeof(phys_addr_t));
}

#define FDT_REG_SIZE  sizeof(u32)
/* Temp location for sharing data for storing */
/* Up to 64-bit address + 64-bit size */
static u8 tmp[CONFIG_NR_DRAM_BANKS * 16];

void dram_init_banksize(void)
{
	int bank;

	memcpy(&gd->bd->bi_dram[0], &tmp, sizeof(tmp));

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		debug("Bank #%d: start %llx\n", bank,
		      (unsigned long long)gd->bd->bi_dram[bank].start);
		debug("Bank #%d: size %llx\n", bank,
		      (unsigned long long)gd->bd->bi_dram[bank].size);
	}
}

int dram_init(void)
{
	int node, len;
	const void *blob = gd->fdt_blob;
	const u32 *cell;

	memset(&tmp, 0, sizeof(tmp));

	/* find or create "/memory" node. */
	node = fdt_subnode_offset(blob, 0, "memory");
	if (node < 0) {
		printf("%s: Can't get memory node\n", __func__);
		return node;
	}

	/* Get pointer to cells and lenght of it */
	cell = fdt_getprop(blob, node, "reg", &len);
	if (!cell) {
		printf("%s: Can't get reg property\n", __func__);
		return -1;
	}

	gd->ram_size = fdt_get_reg(blob, node, &tmp, cell, len / FDT_REG_SIZE);

	debug("%s: Initial DRAM size %llx\n", __func__, (u64)gd->ram_size);

	return 0;
}
#else
int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}
#endif

void reset_cpu(ulong addr)
{
}

#ifdef CONFIG_SCSI_AHCI_PLAT
void scsi_init(void)
{
#if defined(CONFIG_SATA_CEVA)
	init_sata(0);
#endif
	ahci_init((void __iomem *)ZYNQMP_SATA_BASEADDR);
	scsi_scan(1);
}
#endif

int board_late_init(void)
{
	u32 reg = 0;
	u8 bootmode;
	const char *mode;
	char *new_targets;

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	reg = readl(&crlapb_base->boot_mode);
	bootmode = reg & BOOT_MODES_MASK;

	puts("Bootmode: ");
	switch (bootmode) {
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		mode = "pxe dhcp";
		break;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		mode = "qspi0";
		puts("QSPI_MODE\n");
		break;
	case EMMC_MODE:
		puts("EMMC_MODE\n");
		mode = "mmc0";
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		mode = "mmc0";
		break;
	case SD_MODE1:
		puts("SD_MODE1\n");
#if defined(CONFIG_ZYNQ_SDHCI0) && defined(CONFIG_ZYNQ_SDHCI1)
		mode = "mmc1";
#else
		mode = "mmc0";
#endif
		break;
	case NAND_MODE:
		puts("NAND_MODE\n");
		mode = "nand0";
		break;
	default:
		mode = "";
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	/*
	 * One terminating char + one byte for space between mode
	 * and default boot_targets
	 */
	new_targets = calloc(1, strlen(mode) +
				strlen(getenv("boot_targets")) + 2);

	sprintf(new_targets, "%s %s", mode, getenv("boot_targets"));
	setenv("boot_targets", new_targets);

	return 0;
}

int checkboard(void)
{
	puts("Board: Xilinx ZynqMP\n");
	return 0;
}

#ifdef CONFIG_USB_DWC3
static struct dwc3_device dwc3_device_data = {
	.maximum_speed = USB_SPEED_HIGH,
	.base = ZYNQMP_USB0_XHCI_BASEADDR,
	.dr_mode = USB_DR_MODE_PERIPHERAL,
	.index = 0,
};

int usb_gadget_handle_interrupts(void)
{
	dwc3_uboot_handle_interrupt(0);
	return 0;
}

int board_usb_init(int index, enum usb_init_type init)
{
	return dwc3_uboot_init(&dwc3_device_data);
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	dwc3_uboot_exit(index);
	return 0;
}
#endif

void reset_misc(void)
{
	psci_system_reset(true);
}
