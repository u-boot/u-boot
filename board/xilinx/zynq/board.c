/*
 * (C) Copyright 2012 Michal Simek <monstr@monstr.eu>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <fpga.h>
#include <mmc.h>
#include <zynqpl.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
static xilinx_desc fpga;

/* It can be done differently */
static xilinx_desc fpga010 = XILINX_XC7Z010_DESC(0x10);
static xilinx_desc fpga015 = XILINX_XC7Z015_DESC(0x15);
static xilinx_desc fpga020 = XILINX_XC7Z020_DESC(0x20);
static xilinx_desc fpga030 = XILINX_XC7Z030_DESC(0x30);
static xilinx_desc fpga035 = XILINX_XC7Z035_DESC(0x35);
static xilinx_desc fpga045 = XILINX_XC7Z045_DESC(0x45);
static xilinx_desc fpga100 = XILINX_XC7Z100_DESC(0x100);
#endif

int board_init(void)
{
#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	u32 idcode;

	idcode = zynq_slcr_get_idcode();

	switch (idcode) {
	case XILINX_ZYNQ_7010:
		fpga = fpga010;
		break;
	case XILINX_ZYNQ_7015:
		fpga = fpga015;
		break;
	case XILINX_ZYNQ_7020:
		fpga = fpga020;
		break;
	case XILINX_ZYNQ_7030:
		fpga = fpga030;
		break;
	case XILINX_ZYNQ_7035:
		fpga = fpga035;
		break;
	case XILINX_ZYNQ_7045:
		fpga = fpga045;
		break;
	case XILINX_ZYNQ_7100:
		fpga = fpga100;
		break;
	}
#endif

#if (defined(CONFIG_FPGA) && !defined(CONFIG_SPL_BUILD)) || \
    (defined(CONFIG_SPL_FPGA_SUPPORT) && defined(CONFIG_SPL_BUILD))
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif

	return 0;
}

int board_late_init(void)
{
	switch ((zynq_slcr_get_boot_mode()) & ZYNQ_BM_MASK) {
	case ZYNQ_BM_NOR:
		setenv("modeboot", "norboot");
		break;
	case ZYNQ_BM_SD:
		setenv("modeboot", "sdboot");
		break;
	case ZYNQ_BM_JTAG:
		setenv("modeboot", "jtagboot");
		break;
	default:
		setenv("modeboot", "");
		break;
	}

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board: Xilinx Zynq\n");
	return 0;
}
#endif

int zynq_board_read_rom_ethaddr(unsigned char *ethaddr)
{
#if defined(CONFIG_ZYNQ_GEM_EEPROM_ADDR) && \
    defined(CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET)
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

	zynq_ddrc_init();

	return 0;
}
#else
int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	zynq_ddrc_init();

	return 0;
}
#endif
