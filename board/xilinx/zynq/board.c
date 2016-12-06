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
static xilinx_desc fpga007s = XILINX_XC7Z007S_DESC(0x7);
static xilinx_desc fpga010 = XILINX_XC7Z010_DESC(0x10);
static xilinx_desc fpga012s = XILINX_XC7Z012S_DESC(0x12);
static xilinx_desc fpga014s = XILINX_XC7Z014S_DESC(0x14);
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
	case XILINX_ZYNQ_7007S:
		fpga = fpga007s;
		break;
	case XILINX_ZYNQ_7010:
		fpga = fpga010;
		break;
	case XILINX_ZYNQ_7012S:
		fpga = fpga012s;
		break;
	case XILINX_ZYNQ_7014S:
		fpga = fpga014s;
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
static const void *get_memory_reg_prop(const void *fdt, int *lenp)
{
	int offset;

	offset = fdt_path_offset(fdt, "/memory");
	if (offset < 0)
		return NULL;

	return fdt_getprop(fdt, offset, "reg", lenp);
}

int dram_init(void)
{
	const void *fdt = gd->fdt_blob;
	const fdt32_t *val;
	int ac, sc, len;

	ac = fdt_address_cells(fdt, 0);
	sc = fdt_size_cells(fdt, 0);
	if (ac < 0 || sc < 1 || sc > 2) {
		printf("invalid address/size cells\n");
		return -EINVAL;
	}

	val = get_memory_reg_prop(fdt, &len);
	if (len / sizeof(*val) < ac + sc)
		return -EINVAL;

	val += ac;

	gd->ram_size = fdtdec_get_number(val, sc);

	debug("DRAM size = %08lx\n", (unsigned long)gd->ram_size);

	zynq_ddrc_init();

	return 0;
}

void dram_init_banksize(void)
{
	const void *fdt = gd->fdt_blob;
	const fdt32_t *val;
	int ac, sc, cells, len, i;

	val = get_memory_reg_prop(fdt, &len);
	if (len < 0)
		return;

	ac = fdt_address_cells(fdt, 0);
	sc = fdt_size_cells(fdt, 0);
	if (ac < 1 || sc > 2 || sc < 1 || sc > 2) {
		printf("invalid address/size cells\n");
		return;
	}

	cells = ac + sc;

	len /= sizeof(*val);

	for (i = 0; i < CONFIG_NR_DRAM_BANKS && len >= cells;
	     i++, len -= cells) {
		gd->bd->bi_dram[i].start = fdtdec_get_number(val, ac);
		val += ac;
		gd->bd->bi_dram[i].size = fdtdec_get_number(val, sc);
		val += sc;

		debug("DRAM bank %d: start = %08lx, size = %08lx\n",
		      i, (unsigned long)gd->bd->bi_dram[i].start,
		      (unsigned long)gd->bd->bi_dram[i].size);
	}
}
#else
int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	zynq_ddrc_init();

	return 0;
}
#endif
