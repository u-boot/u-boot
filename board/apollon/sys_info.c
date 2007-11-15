/*
 * (C) Copyright 2005-2007
 * Samsung Electronics,
 * Kyungmin Park <kyungmin.park@samsung.com>
 *
 * Derived from omap2420
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/omap2420.h>
#include <asm/io.h>
#include <asm/arch/bits.h>
#include <asm/arch/mem.h>	/* get mem tables */
#include <asm/arch/sys_proto.h>
#include <asm/arch/sys_info.h>
#include <i2c.h>

/**************************************************************************
 * get_prod_id() - get id info from chips
 ***************************************************************************/
static u32 get_prod_id(void)
{
	u32 p;
	p = __raw_readl(PRODUCTION_ID);	/* get production ID */
	return ((p & CPU_242X_PID_MASK) >> 16);
}

/**************************************************************************
 * get_cpu_type() - low level get cpu type
 * - no C globals yet.
 * - just looking to say if this is a 2422 or 2420 or ...
 * - to start with we will look at switch settings..
 * - 2422 id's same as 2420 for ES1 will rely on H4 board characteristics
 *   (mux for 2420, non-mux for 2422).
 ***************************************************************************/
u32 get_cpu_type(void)
{
	u32 v;

	switch (get_prod_id()) {
	case 1:;		/* 2420 */
	case 2:
		return (CPU_2420);
		break;		/* 2420 pop */
	case 4:
		return (CPU_2422);
		break;
	case 8:
		return (CPU_2423);
		break;
	default:
		break;		/* early 2420/2422's unmarked */
	}

	v = __raw_readl(TAP_IDCODE_REG);
	v &= CPU_24XX_ID_MASK;
	/* currently 2420 and 2422 have same id */
	if (v == CPU_2420_CHIPID) {
		if (is_gpmc_muxed() == GPMC_MUXED)	/* if mux'ed */
			return (CPU_2420);
		else
			return (CPU_2422);
	} else
		return (CPU_2420);	/* don't know, say 2420 */
}

/******************************************
 * get_cpu_rev(void) - extract version info
 ******************************************/
u32 get_cpu_rev(void)
{
	u32 v;
	v = __raw_readl(TAP_IDCODE_REG);
	v = v >> 28;
	return (v + 1);		/* currently 2422 and 2420 match up */
}

/****************************************************
 * is_mem_sdr() - return 1 if mem type in use is SDR
 ****************************************************/
u32 is_mem_sdr(void)
{
	volatile u32 *burst = (volatile u32 *)(SDRC_MR_0 + SDRC_CS0_OSET);
	if (*burst == H4_2420_SDRC_MR_0_SDR)
		return (1);
	return (0);
}

/***********************************************************
 * get_mem_type() - identify type of mDDR part used.
 * 2422 uses stacked DDR, 2 parts CS0/CS1.
 * 2420 may have 1 or 2, no good way to know...only init 1...
 * when eeprom data is up we can select 1 more.
 *************************************************************/
u32 get_mem_type(void)
{
	u32 cpu, sdr = is_mem_sdr();

	cpu = get_cpu_type();
	if (cpu == CPU_2422 || cpu == CPU_2423)
		return (DDR_STACKED);

	if (get_prod_id() == 0x2)
		return (XDR_POP);

	if (get_board_type() == BOARD_H4_MENELAUS)
		if (sdr)
			return (SDR_DISCRETE);
		else
			return (DDR_COMBO);
	else if (sdr)		/* SDP + SDR kit */
		return (SDR_DISCRETE);
	else
		return (DDR_DISCRETE);	/* origional SDP */
}

/***********************************************************************
 * get_cs0_size() - get size of chip select 0/1
 ************************************************************************/
u32 get_sdr_cs_size(u32 offset)
{
	u32 size;
	size = __raw_readl(SDRC_MCFG_0 + offset) >> 8;	/* get ram size field */
	size &= 0x2FF;		/* remove unwanted bits */
	size *= SZ_2M;		/* find size in MB */
	return (size);
}

/***********************************************************************
 * get_board_type() - get board type based on current production stats.
 *  --- NOTE: 2 I2C EEPROMs will someday be populated with proper info.
 *      when they are available we can get info from there.  This should
 *      be correct of all known boards up until today.
 ************************************************************************/
u32 get_board_type(void)
{
	return (BOARD_H4_SDP);
}

/******************************************************************
 * get_sysboot_value() - get init word settings (dip switch on h4)
 ******************************************************************/
inline u32 get_sysboot_value(void)
{
	return (0x00000FFF & __raw_readl(CONTROL_STATUS));
}

/***************************************************************************
 *  get_gpmc0_base() - Return current address hardware will be
 *     fetching from. The below effectively gives what is correct, its a bit
 *   mis-leading compared to the TRM.  For the most general case the mask
 *   needs to be also taken into account this does work in practice.
 *   - for u-boot we currently map:
 *       -- 0 to nothing,
 *       -- 4 to flash
 *       -- 8 to enent
 *       -- c to wifi
 ****************************************************************************/
u32 get_gpmc0_base(void)
{
	u32 b;

	b = __raw_readl(GPMC_CONFIG7_0);
	b &= 0x1F;		/* keep base [5:0] */
	b = b << 24;		/* ret 0x0b000000 */
	return (b);
}

/*****************************************************************
 *  is_gpmc_muxed() - tells if address/data lines are multiplexed
 *****************************************************************/
u32 is_gpmc_muxed(void)
{
	u32 mux;
	mux = get_sysboot_value();
	if ((mux & (BIT0 | BIT1 | BIT2 | BIT3)) == (BIT0 | BIT2 | BIT3))
		return (GPMC_MUXED);	/* NAND Boot mode */
	if (mux & BIT1)		/* if mux'ed */
		return (GPMC_MUXED);
	else
		return (GPMC_NONMUXED);
}

/************************************************************************
 *  get_gpmc0_type() - read sysboot lines to see type of memory attached
 ************************************************************************/
u32 get_gpmc0_type(void)
{
	u32 type;
	type = get_sysboot_value();
	if ((type & (BIT3 | BIT2)) == (BIT3 | BIT2))
		return (TYPE_NAND);
	else
		return (TYPE_NOR);
}

/*******************************************************************
 * get_gpmc0_width() - See if bus is in x8 or x16 (mainly for nand)
 *******************************************************************/
u32 get_gpmc0_width(void)
{
	u32 width;
	width = get_sysboot_value();
	if ((width & 0xF) == (BIT3 | BIT2))
		return (WIDTH_8BIT);
	else
		return (WIDTH_16BIT);
}

/*********************************************************************
 * wait_on_value() - common routine to allow waiting for changes in
 *   volatile regs.
 *********************************************************************/
u32 wait_on_value(u32 read_bit_mask, u32 match_value, u32 read_addr, u32 bound)
{
	u32 i = 0, val;
	do {
		++i;
		val = __raw_readl(read_addr) & read_bit_mask;
		if (val == match_value)
			return (1);
		if (i == bound)
			return (0);
	} while (1);
}

/*********************************************************************
 *  display_board_info() - print banner with board info.
 *********************************************************************/
void display_board_info(u32 btype)
{
	char cpu_2420[] = "2420";	/* cpu type */
	char cpu_2422[] = "2422";
	char cpu_2423[] = "2423";
	char db_men[] = "Menelaus";	/* board type */
	char db_ip[] = "IP";
	char mem_sdr[] = "mSDR";	/* memory type */
	char mem_ddr[] = "mDDR";
	char t_tst[] = "TST";	/* security level */
	char t_emu[] = "EMU";
	char t_hs[] = "HS";
	char t_gp[] = "GP";
	char unk[] = "?";

	char *cpu_s, *db_s, *mem_s, *sec_s;
	u32 cpu, rev, sec;

	rev = get_cpu_rev();
	cpu = get_cpu_type();
	sec = get_device_type();

	if (is_mem_sdr())
		mem_s = mem_sdr;
	else
		mem_s = mem_ddr;

	if (cpu == CPU_2423)
		cpu_s = cpu_2423;
	else if (cpu == CPU_2422)
		cpu_s = cpu_2422;
	else
		cpu_s = cpu_2420;

	if (btype == BOARD_H4_MENELAUS)
		db_s = db_men;
	else
		db_s = db_ip;

	switch (sec) {
	case TST_DEVICE:
		sec_s = t_tst;
		break;
	case EMU_DEVICE:
		sec_s = t_emu;
		break;
	case HS_DEVICE:
		sec_s = t_hs;
		break;
	case GP_DEVICE:
		sec_s = t_gp;
		break;
	default:
		sec_s = unk;
	}

	printf("OMAP%s-%s revision %d\n", cpu_s, sec_s, rev - 1);
	printf("Samsung Apollon SDP Base Board + %s \n", mem_s);
}

/*************************************************************************
 * get_board_rev() - setup to pass kernel board revision information
 *          0 = 242x IP platform (first 2xx boards)
 *          1 = 242x Menelaus platfrom.
 *************************************************************************/
u32 get_board_rev(void)
{
	u32 rev = 0;
	u32 btype = get_board_type();

	if (btype == BOARD_H4_MENELAUS)
		rev = 1;
	return (rev);
}

/********************************************************
 *  get_base(); get upper addr of current execution
 *******************************************************/
u32 get_base(void)
{
	u32 val;
	__asm__ __volatile__("mov %0, pc \n":"=r"(val)::"memory");
	val &= 0xF0000000;
	val >>= 28;
	return (val);
}

/********************************************************
 *  get_base2(); get 2upper addr of current execution
 *******************************************************/
u32 get_base2(void)
{
	u32 val;
	__asm__ __volatile__("mov %0, pc \n":"=r"(val)::"memory");
	val &= 0xFF000000;
	val >>= 24;
	return (val);
}

/********************************************************
 *  running_in_flash() - tell if currently running in
 *   flash.
 *******************************************************/
u32 running_in_flash(void)
{
	if (get_base() < 4)
		return (1);	/* in flash */
	return (0);		/* running in SRAM or SDRAM */
}

/********************************************************
 *  running_in_sram() - tell if currently running in
 *   sram.
 *******************************************************/
u32 running_in_sram(void)
{
	if (get_base() == 4)
		return (1);	/* in SRAM */
	return (0);		/* running in FLASH or SDRAM */
}

/********************************************************
 *  running_in_sdram() - tell if currently running in
 *   flash.
 *******************************************************/
u32 running_in_sdram(void)
{
	if (get_base() > 4)
		return (1);	/* in sdram */
	return (0);		/* running in SRAM or FLASH */
}

/*************************************************************
 *  running_from_internal_boot() - am I a signed NOR image.
 *************************************************************/
u32 running_from_internal_boot(void)
{
	u32 v, base;

	v = get_sysboot_value() & BIT3;
	base = get_base2();
	/* if running at mask rom flash address and
	 * sysboot3 says this was an internal boot
	 */
	if ((base == 0x08) && v)
		return (1);
	else
		return (0);
}

/*************************************************************
 *  get_device_type(): tell if GP/HS/EMU/TST
 *************************************************************/
u32 get_device_type(void)
{
	int mode;
	mode = __raw_readl(CONTROL_STATUS) & (BIT10 | BIT9 | BIT8);
	return (mode >>= 8);
}
