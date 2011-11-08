/*
 * (C) Copyright 2010,2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor-flags.h>
#include <asm/arch/sc520.h>

DECLARE_GLOBAL_DATA_PTR;

struct sc520_sdram_info {
	u8 banks;
	u8 columns;
	u8 rows;
	u8 size;
};

static void sc520_sizemem(void);
static void sc520_set_dram_timing(void);
static void sc520_set_dram_refresh_rate(void);
static void sc520_enable_dram_refresh(void);
static void sc520_enable_sdram(void);

int dram_init_f(void)
{
	sc520_sizemem();
	sc520_set_dram_timing();
	sc520_set_dram_refresh_rate();
	sc520_enable_dram_refresh();
	sc520_enable_sdram();

	return 0;
}

static inline void sc520_dummy_write(void)
{
	writew(0x0000, CACHELINESZ);
}
static inline void sc520_issue_sdram_op_mode_select(u8 command)
{
	writeb(command, &sc520_mmcr->drcctl);
	sc520_dummy_write();
}

static inline int check_long(u32 test_long)
{
	u8 i;
	u8 tmp_byte = (u8)(test_long & 0x000000ff);

	for (i = 1; i < 4; i++) {
		if ((u8)((test_long >> (i * 8)) & 0x000000ff) != tmp_byte)
				return -1;
	}

	return 0;
}

static inline int write_and_test(u32 data, u32 address)
{
	writel(data, address);
	if (readl(address) == data)
		return 0; /* Good */
	else
		return -1; /* Bad */
}

static void sc520_enable_sdram(void)
{
	u32 par_config;

	/* Enable Writes, Caching and Code Execution to SDRAM */
	par_config = readl(&sc520_mmcr->par[3]);
	par_config &= ~(SC520_PAR_EXEC_DIS |
			SC520_PAR_CACHE_DIS |
			SC520_PAR_WRITE_DIS);
	writel(par_config, &sc520_mmcr->par[3]);

	par_config = readl(&sc520_mmcr->par[4]);
	par_config &= ~(SC520_PAR_EXEC_DIS |
			SC520_PAR_CACHE_DIS |
			SC520_PAR_WRITE_DIS);
	writel(par_config, &sc520_mmcr->par[4]);
}

static void sc520_set_dram_timing(void)
{
	u8 drctmctl = 0x00;

#if defined CONFIG_SYS_SDRAM_DRCTMCTL
	/* just have your hardware designer _GIVE_ you what you need here! */
	drctmctl = CONFIG_SYS_SDRAM_DRCTMCTL;
#else
	switch (CONFIG_SYS_SDRAM_RAS_CAS_DELAY) {
	case 2:
		break;
	case 3:
		drctmctl |= 0x01;
		break;
	case 4:
	default:
		drctmctl |= 0x02;
		break;
	}

	switch (CONFIG_SYS_SDRAM_PRECHARGE_DELAY) {
	case 2:
		break;
	case 3:
		drctmctl |= 0x04;
		break;
	case 4:
	default:
		drctmctl |= 0x08;
		break;

	case 6:
		drctmctl |= 0x0c;
		break;
	}

	switch (CONFIG_SYS_SDRAM_CAS_LATENCY) {
	case 2:
		break;
	case 3:
	default:
		drctmctl |= 0x10;
		break;
	}
#endif
	writeb(drctmctl, &sc520_mmcr->drctmctl);

	/* Issue load mode register command */
	sc520_issue_sdram_op_mode_select(0x03);
}

static void sc520_set_dram_refresh_rate(void)
{
	u8 drctl;

	drctl = readb(&sc520_mmcr->drcctl);
	drctl &= 0xcf;

	switch (CONFIG_SYS_SDRAM_REFRESH_RATE) {
	case 78:
		break;
	case 156:
	default:
		drctl |= 0x10;
		break;
	case 312:
		drctl |= 0x20;
		break;
	case 624:
		drctl |= 0x30;
		break;
	}

	writeb(drctl, &sc520_mmcr->drcctl);
}

static void sc520_enable_dram_refresh(void)
{
	u8 drctl;

	drctl = readb(&sc520_mmcr->drcctl);
	drctl &= 0x30; /* keep refresh rate */
	drctl |= 0x08; /* enable refresh, normal mode */

	writeb(drctl, &sc520_mmcr->drcctl);
}

static void sc520_get_bank_info(int bank, struct sc520_sdram_info *bank_info)
{
	u32 col_data;
	u32 row_data;

	u32 drcbendadr;
	u16 drccfg;

	u8 banks = 0x00;
	u8 columns = 0x00;
	u8 rows = 0x00;

	bank_info->banks = 0x00;
	bank_info->columns = 0x00;
	bank_info->rows = 0x00;
	bank_info->size = 0x00;

	if ((bank < 0) || (bank > 3)) {
		printf("Bad Bank ID\n");
		return;
	}

	/* Save configuration */
	drcbendadr = readl(&sc520_mmcr->drcbendadr);
	drccfg = readw(&sc520_mmcr->drccfg);

	/* Setup SDRAM Bank to largest possible size */
	writew(0x000b << (bank * 4), &sc520_mmcr->drccfg);

	/* Set ending address for this bank */
	writel(0x000000ff << (bank * 8), &sc520_mmcr->drcbendadr);

	/* write col 11 wrap adr */
	if (write_and_test(COL11_DATA, COL11_ADR) != 0)
		goto restore_and_exit;

	/* write col 10 wrap adr */
	if (write_and_test(COL10_DATA, COL10_ADR) != 0)
		goto restore_and_exit;

	/* write col 9 wrap adr */
	if (write_and_test(COL09_DATA, COL09_ADR) != 0)
		goto restore_and_exit;

	/* write col 8 wrap adr */
	if (write_and_test(COL08_DATA, COL08_ADR) != 0)
		goto restore_and_exit;

	col_data = readl(COL11_ADR);

	/* All four bytes in the read long must be the same */
	if (check_long(col_data) < 0)
		goto restore_and_exit;

	if ((col_data >= COL08_DATA) && (col_data <= COL11_DATA))
		columns = (u8)(col_data & 0x000000ff);
	else
		goto restore_and_exit;

	/* write row 14 wrap adr */
	if (write_and_test(ROW14_DATA, ROW14_ADR) != 0)
		goto restore_and_exit;

	/* write row 13 wrap adr */
	if (write_and_test(ROW13_DATA, ROW13_ADR) != 0)
		goto restore_and_exit;

	/* write row 12 wrap adr */
	if (write_and_test(ROW12_DATA, ROW12_ADR) != 0)
		goto restore_and_exit;

	/* write row 11 wrap adr */
	if (write_and_test(ROW11_DATA, ROW11_ADR) != 0)
		goto restore_and_exit;

	if (write_and_test(ROW10_DATA, ROW10_ADR) != 0)
		goto restore_and_exit;

	/*
	 * read data @ row 12 wrap adr to determine number of banks,
	 * and read data @ row 14 wrap adr to determine number of rows.
	 * if data @ row 12 wrap adr is not AA, 11 or 12 we have bad RAM.
	 * if data @ row 12 wrap == AA, we only have 2 banks, NOT 4
	 * if data @ row 12 wrap == 11 or 12, we have 4 banks,
	 */
	row_data = readl(ROW12_ADR);

	/* All four bytes in the read long must be the same */
	if (check_long(row_data) != 0)
		goto restore_and_exit;

	switch (row_data) {
	case ROW10_DATA:
		banks = 2;
		break;

	case ROW11_DATA:
	case ROW12_DATA:
		banks = 4;
		break;

	default:
		goto restore_and_exit;
	}

	row_data = readl(ROW14_ADR);

	/* All four bytes in the read long must be the same */
	if (check_long(row_data) != 0)
		goto restore_and_exit;

	switch (row_data) {
	case ROW11_DATA:
	case ROW12_DATA:
	case ROW13_DATA:
	case ROW14_DATA:
		rows = (u8)(row_data & 0x000000ff);
		break;

	default:
		goto restore_and_exit;
	}

	bank_info->banks = banks;
	bank_info->columns = columns;
	bank_info->rows = rows;

	if ((bank_info->banks != 0) &&
	    (bank_info->columns != 0) &&
	    (bank_info->rows != 0)) {
		bank_info->size = bank_info->rows;
		bank_info->size >>= (11 - bank_info->columns);
		bank_info->size++;
	}

restore_and_exit:
	/* Restore configuration */
	writel(drcbendadr, &sc520_mmcr->drcbendadr);
	writew(drccfg, &sc520_mmcr->drccfg);
}

static void sc520_setup_sizemem(void)
{
	u8 i;

	/* Disable write buffer */
	writeb(0x00, &sc520_mmcr->dbctl);

	/* Disable ECC */
	writeb(0x00, &sc520_mmcr->eccctl);

	/* Set slowest SDRAM timing */
	writeb(0x1e, &sc520_mmcr->drctmctl);

	/* Issue a NOP to all SDRAM banks */
	sc520_issue_sdram_op_mode_select(0x01);

	/* Delay for 100 microseconds */
	udelay(100);

	/* Issue 'All Banks Precharge' command */
	sc520_issue_sdram_op_mode_select(0x02);

	/* Issue 2 'Auto Refresh Enable' command */
	sc520_issue_sdram_op_mode_select(0x04);
	sc520_dummy_write();

	/* Issue 'Load Mode Register' command */
	sc520_issue_sdram_op_mode_select(0x03);

	/* Issue 8 more 'Auto Refresh Enable' commands */
	sc520_issue_sdram_op_mode_select(0x04);
	for (i = 0; i < 7; i++)
		sc520_dummy_write();

	/* Set control register to 'Normal Mode' */
	writeb(0x00, &sc520_mmcr->drcctl);
}

static void sc520_sizemem(void)
{
	struct sc520_sdram_info sdram_info[4];
	u8 bank_config = 0x00;
	u8 end_addr = 0x00;
	u16 drccfg = 0x0000;
	u32 drcbendadr = 0x00000000;
	u8 i;

	/* Use PARs to disable caching of maximum allowable 256MB SDRAM */
	writel(SC520_SDRAM1_PAR | SC520_PAR_CACHE_DIS, &sc520_mmcr->par[3]);
	writel(SC520_SDRAM2_PAR | SC520_PAR_CACHE_DIS, &sc520_mmcr->par[4]);

	sc520_setup_sizemem();

	gd->ram_size = 0;

	/* Size each SDRAM bank */
	for (i = 0; i <= 3; i++) {
		sc520_get_bank_info(i, &sdram_info[i]);

		if (sdram_info[i].banks != 0) {
			/* Update Configuration register */
			bank_config = sdram_info[i].columns - 8;

			if (sdram_info[i].banks == 4)
				bank_config |= 0x08;

			drccfg |= bank_config << (i * 4);

			/* Update End Address register */
			end_addr += sdram_info[i].size;
			drcbendadr |= (end_addr | 0x80) << (i * 8);

			gd->ram_size += sdram_info[i].size << 22;
		}

		/* Issue 'All Banks Precharge' command */
		sc520_issue_sdram_op_mode_select(0x02);

		/* Set control register to 'Normal Mode' */
		writeb(0x00, &sc520_mmcr->drcctl);
	}

	writel(drcbendadr, &sc520_mmcr->drcbendadr);
	writew(drccfg, &sc520_mmcr->drccfg);

	/* Clear PARs preventing caching of SDRAM */
	writel(0x00000000, &sc520_mmcr->par[3]);
	writel(0x00000000, &sc520_mmcr->par[4]);
}

int dram_init(void)
{
	ulong dram_ctrl;
	ulong dram_present = 0x00000000;

	/*
	 * We read-back the configuration of the dram
	 * controller that the assembly code wrote
	 */
	dram_ctrl = readl(&sc520_mmcr->drcbendadr);

	gd->bd->bi_dram[0].start = 0;
	if (dram_ctrl & 0x80) {
		/* bank 0 enabled */
		gd->bd->bi_dram[1].start = (dram_ctrl & 0x7f) << 22;
		dram_present = gd->bd->bi_dram[1].start;
		gd->bd->bi_dram[0].size = gd->bd->bi_dram[1].start;
	} else {
		gd->bd->bi_dram[0].size = 0;
		gd->bd->bi_dram[1].start = gd->bd->bi_dram[0].start;
	}

	if (dram_ctrl & 0x8000) {
		/* bank 1 enabled */
		gd->bd->bi_dram[2].start = (dram_ctrl & 0x7f00) << 14;
		dram_present = gd->bd->bi_dram[2].start;
		gd->bd->bi_dram[1].size = gd->bd->bi_dram[2].start -
				gd->bd->bi_dram[1].start;
	} else {
		gd->bd->bi_dram[1].size = 0;
		gd->bd->bi_dram[2].start = gd->bd->bi_dram[1].start;
	}

	if (dram_ctrl & 0x800000) {
		/* bank 2 enabled */
		gd->bd->bi_dram[3].start = (dram_ctrl & 0x7f0000) << 6;
		dram_present = gd->bd->bi_dram[3].start;
		gd->bd->bi_dram[2].size = gd->bd->bi_dram[3].start -
				gd->bd->bi_dram[2].start;
	} else {
		gd->bd->bi_dram[2].size = 0;
		gd->bd->bi_dram[3].start = gd->bd->bi_dram[2].start;
	}

	if (dram_ctrl & 0x80000000) {
		/* bank 3 enabled */
		dram_present  = (dram_ctrl & 0x7f000000) >> 2;
		gd->bd->bi_dram[3].size = dram_present -
				gd->bd->bi_dram[3].start;
	} else {
		gd->bd->bi_dram[3].size = 0;
	}

	gd->ram_size = dram_present;

	return 0;
}
