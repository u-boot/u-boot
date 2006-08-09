/*
 * (C) Copyright 2001
 * Bill Hunter, Wave 7 Optics, williamhunter@attbi.com
 *
 * Based on code by:
 *
 * Kenneth Johansson ,Ericsson AB.
 * kenneth.johansson@etx.ericsson.se
 *
 * hacked up by bill hunter. fixed so we could run before
 * serial_init and console_init. previous version avoided this by
 * running out of cache memory during serial/console init, then running
 * this code later.
 *
 * (C) Copyright 2002
 * Jun Gu, Artesyn Technology, jung@artesyncp.com
 * Support for AMCC 440 based on OpenBIOS draminit.c from IBM.
 *
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>
#include <i2c.h>
#include <ppc4xx.h>

#ifdef CONFIG_SPD_EEPROM

/*
 * Set default values
 */
#ifndef CFG_I2C_SPEED
#define CFG_I2C_SPEED	50000
#endif

#ifndef CFG_I2C_SLAVE
#define CFG_I2C_SLAVE	0xFE
#endif

#define ONE_BILLION	1000000000

#ifndef	 CONFIG_440		/* for 405 WALNUT/SYCAMORE/BUBINGA boards */

#define	 SDRAM0_CFG_DCE		0x80000000
#define	 SDRAM0_CFG_SRE		0x40000000
#define	 SDRAM0_CFG_PME		0x20000000
#define	 SDRAM0_CFG_MEMCHK	0x10000000
#define	 SDRAM0_CFG_REGEN	0x08000000
#define	 SDRAM0_CFG_ECCDD	0x00400000
#define	 SDRAM0_CFG_EMDULR	0x00200000
#define	 SDRAM0_CFG_DRW_SHIFT	(31-6)
#define	 SDRAM0_CFG_BRPF_SHIFT	(31-8)

#define	 SDRAM0_TR_CASL_SHIFT	(31-8)
#define	 SDRAM0_TR_PTA_SHIFT	(31-13)
#define	 SDRAM0_TR_CTP_SHIFT	(31-15)
#define	 SDRAM0_TR_LDF_SHIFT	(31-17)
#define	 SDRAM0_TR_RFTA_SHIFT	(31-29)
#define	 SDRAM0_TR_RCD_SHIFT	(31-31)

#define	 SDRAM0_RTR_SHIFT	(31-15)
#define	 SDRAM0_ECCCFG_SHIFT	(31-11)

/* SDRAM0_CFG enable macro  */
#define SDRAM0_CFG_BRPF(x) ( ( x & 0x3)<< SDRAM0_CFG_BRPF_SHIFT )

#define SDRAM0_BXCR_SZ_MASK	0x000e0000
#define SDRAM0_BXCR_AM_MASK	0x0000e000

#define SDRAM0_BXCR_SZ_SHIFT	(31-14)
#define SDRAM0_BXCR_AM_SHIFT	(31-18)

#define SDRAM0_BXCR_SZ(x)	( (( x << SDRAM0_BXCR_SZ_SHIFT) & SDRAM0_BXCR_SZ_MASK) )
#define SDRAM0_BXCR_AM(x)	( (( x << SDRAM0_BXCR_AM_SHIFT) & SDRAM0_BXCR_AM_MASK) )

#ifdef CONFIG_SPDDRAM_SILENT
# define SPD_ERR(x) do { return 0; } while (0)
#else
# define SPD_ERR(x) do { printf(x); return(0); } while (0)
#endif

#define sdram_HZ_to_ns(hertz) (1000000000/(hertz))

/* function prototypes */
int spd_read(uint addr);


/*
 * This function is reading data from the DIMM module EEPROM over the SPD bus
 * and uses that to program the sdram controller.
 *
 * This works on boards that has the same schematics that the AMCC walnut has.
 *
 * Input: null for default I2C spd functions or a pointer to a custom function
 * returning spd_data.
 */

long int spd_sdram(int(read_spd)(uint addr))
{
	int tmp,row,col;
	int total_size,bank_size,bank_code;
	int ecc_on;
	int mode;
	int bank_cnt;

	int sdram0_pmit=0x07c00000;
#ifndef CONFIG_405EP /* not on PPC405EP */
	int sdram0_besr0=-1;
	int sdram0_besr1=-1;
	int sdram0_eccesr=-1;
#endif
	int sdram0_ecccfg;

	int sdram0_rtr=0;
	int sdram0_tr=0;

	int sdram0_b0cr;
	int sdram0_b1cr;
	int sdram0_b2cr;
	int sdram0_b3cr;

	int sdram0_cfg=0;

	int t_rp;
	int t_rcd;
	int t_ras;
	int t_rc;
	int min_cas;

	PPC405_SYS_INFO sys_info;
	unsigned long bus_period_x_10;

	/*
	 * get the board info
	 */
	get_sys_info(&sys_info);
	bus_period_x_10 = ONE_BILLION / (sys_info.freqPLB / 10);

	if (read_spd == 0){
		read_spd=spd_read;
		/*
		 * Make sure I2C controller is initialized
		 * before continuing.
		 */
		i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);
	}

	/* Make shure we are using SDRAM */
	if (read_spd(2) != 0x04) {
		SPD_ERR("SDRAM - non SDRAM memory module found\n");
	}

	/* ------------------------------------------------------------------
	 * configure memory timing register
	 *
	 * data from DIMM:
	 * 27	IN Row Precharge Time ( t RP)
	 * 29	MIN RAS to CAS Delay ( t RCD)
	 * 127	 Component and Clock Detail ,clk0-clk3, junction temp, CAS
	 * -------------------------------------------------------------------*/

	/*
	 * first figure out which cas latency mode to use
	 * use the min supported mode
	 */

	tmp = read_spd(127) & 0x6;
	if (tmp == 0x02) {		/* only cas = 2 supported */
		min_cas = 2;
/*		t_ck = read_spd(9); */
/*		t_ac = read_spd(10); */
	} else if (tmp == 0x04) {	/* only cas = 3 supported */
		min_cas = 3;
/*		t_ck = read_spd(9); */
/*		t_ac = read_spd(10); */
	} else if (tmp == 0x06) {	/* 2,3 supported, so use 2 */
		min_cas = 2;
/*		t_ck = read_spd(23); */
/*		t_ac = read_spd(24); */
	} else {
		SPD_ERR("SDRAM - unsupported CAS latency \n");
	}

	/* get some timing values, t_rp,t_rcd,t_ras,t_rc
	 */
	t_rp = read_spd(27);
	t_rcd = read_spd(29);
	t_ras = read_spd(30);
	t_rc = t_ras + t_rp;

	/* The following timing calcs subtract 1 before deviding.
	 * this has effect of using ceiling instead of floor rounding,
	 * and also subtracting 1 to convert number to reg value
	 */
	/* set up CASL */
	sdram0_tr = (min_cas - 1) << SDRAM0_TR_CASL_SHIFT;
	/* set up PTA */
	sdram0_tr |= ((((t_rp - 1) * 10)/bus_period_x_10) & 0x3) << SDRAM0_TR_PTA_SHIFT;
	/* set up CTP */
	tmp = (((t_rc - t_rcd - t_rp -1) * 10) / bus_period_x_10) & 0x3;
	if (tmp < 1)
		tmp = 1;
	sdram0_tr |= tmp << SDRAM0_TR_CTP_SHIFT;
	/* set LDF	= 2 cycles, reg value = 1 */
	sdram0_tr |= 1 << SDRAM0_TR_LDF_SHIFT;
	/* set RFTA = t_rfc/bus_period, use t_rfc = t_rc */
	tmp = (((t_rc - 1) * 10) / bus_period_x_10) - 3;
	if (tmp < 0)
		tmp = 0;
	if (tmp > 6)
		tmp = 6;
	sdram0_tr |= tmp << SDRAM0_TR_RFTA_SHIFT;
	/* set RCD = t_rcd/bus_period*/
	sdram0_tr |= ((((t_rcd - 1) * 10) / bus_period_x_10) &0x3) << SDRAM0_TR_RCD_SHIFT ;


	/*------------------------------------------------------------------
	 * configure RTR register
	 * -------------------------------------------------------------------*/
	row = read_spd(3);
	col = read_spd(4);
	tmp = read_spd(12) & 0x7f ; /* refresh type less self refresh bit */
	switch (tmp) {
	case 0x00:
		tmp = 15625;
		break;
	case 0x01:
		tmp = 15625 / 4;
		break;
	case 0x02:
		tmp = 15625 / 2;
		break;
	case 0x03:
		tmp = 15625 * 2;
		break;
	case 0x04:
		tmp = 15625 * 4;
		break;
	case 0x05:
		tmp = 15625 * 8;
		break;
	default:
		SPD_ERR("SDRAM - Bad refresh period \n");
	}
	/* convert from nsec to bus cycles */
	tmp = (tmp * 10) / bus_period_x_10;
	sdram0_rtr = (tmp & 0x3ff8) <<	SDRAM0_RTR_SHIFT;

	/*------------------------------------------------------------------
	 * determine the number of banks used
	 * -------------------------------------------------------------------*/
	/* byte 7:6 is module data width */
	if (read_spd(7) != 0)
		SPD_ERR("SDRAM - unsupported module width\n");
	tmp = read_spd(6);
	if (tmp < 32)
		SPD_ERR("SDRAM - unsupported module width\n");
	else if (tmp < 64)
		bank_cnt = 1;		/* one bank per sdram side */
	else if (tmp < 73)
		bank_cnt = 2;	/* need two banks per side */
	else if (tmp < 161)
		bank_cnt = 4;	/* need four banks per side */
	else
		SPD_ERR("SDRAM - unsupported module width\n");

	/* byte 5 is the module row count (refered to as dimm "sides") */
	tmp = read_spd(5);
	if (tmp == 1)
		;
	else if (tmp==2)
		bank_cnt *= 2;
	else if (tmp==4)
		bank_cnt *= 4;
	else
		bank_cnt = 8;		/* 8 is an error code */

	if (bank_cnt > 4)	/* we only have 4 banks to work with */
		SPD_ERR("SDRAM - unsupported module rows for this width\n");

	/* now check for ECC ability of module. We only support ECC
	 *   on 32 bit wide devices with 8 bit ECC.
	 */
	if ((read_spd(11)==2) && (read_spd(6)==40) && (read_spd(14)==8)) {
		sdram0_ecccfg = 0xf << SDRAM0_ECCCFG_SHIFT;
		ecc_on = 1;
	} else {
		sdram0_ecccfg = 0;
		ecc_on = 0;
	}

	/*------------------------------------------------------------------
	 * calculate total size
	 * -------------------------------------------------------------------*/
	/* calculate total size and do sanity check */
	tmp = read_spd(31);
	total_size = 1 << 22;	/* total_size = 4MB */
	/* now multiply 4M by the smallest device row density */
	/* note that we don't support asymetric rows */
	while (((tmp & 0x0001) == 0) && (tmp != 0)) {
		total_size = total_size << 1;
		tmp = tmp >> 1;
	}
	total_size *= read_spd(5);	/* mult by module rows (dimm sides) */

	/*------------------------------------------------------------------
	 * map	rows * cols * banks to a mode
	 * -------------------------------------------------------------------*/

	switch (row) {
	case 11:
		switch (col) {
		case 8:
			mode=4; /* mode 5 */
			break;
		case 9:
		case 10:
			mode=0; /* mode 1 */
			break;
		default:
			SPD_ERR("SDRAM - unsupported mode\n");
		}
		break;
	case 12:
		switch (col) {
		case 8:
			mode=3; /* mode 4 */
			break;
		case 9:
		case 10:
			mode=1; /* mode 2 */
			break;
		default:
			SPD_ERR("SDRAM - unsupported mode\n");
		}
		break;
	case 13:
		switch (col) {
		case 8:
			mode=5; /* mode 6 */
			break;
		case 9:
		case 10:
			if (read_spd(17) == 2)
				mode = 6; /* mode 7 */
			else
				mode = 2; /* mode 3 */
			break;
		case 11:
			mode = 2; /* mode 3 */
			break;
		default:
			SPD_ERR("SDRAM - unsupported mode\n");
		}
		break;
	default:
		SPD_ERR("SDRAM - unsupported mode\n");
	}

	/*------------------------------------------------------------------
	 * using the calculated values, compute the bank
	 * config register values.
	 * -------------------------------------------------------------------*/
	sdram0_b1cr = 0;
	sdram0_b2cr = 0;
	sdram0_b3cr = 0;

	/* compute the size of each bank */
	bank_size = total_size / bank_cnt;
	/* convert bank size to bank size code for ppc4xx
	   by takeing log2(bank_size) - 22 */
	tmp = bank_size;		/* start with tmp = bank_size */
	bank_code = 0;			/* and bank_code = 0 */
	while (tmp > 1) {		/* this takes log2 of tmp */
		bank_code++;		/* and stores result in bank_code */
		tmp = tmp >> 1;
	}				/* bank_code is now log2(bank_size) */
	bank_code -= 22;		/* subtract 22 to get the code */

	tmp = SDRAM0_BXCR_SZ(bank_code) | SDRAM0_BXCR_AM(mode) | 1;
	sdram0_b0cr = (bank_size * 0) | tmp;
#ifndef CONFIG_405EP /* not on PPC405EP */
	if (bank_cnt > 1)
		sdram0_b2cr = (bank_size * 1) | tmp;
	if (bank_cnt > 2)
		sdram0_b1cr = (bank_size * 2) | tmp;
	if (bank_cnt > 3)
		sdram0_b3cr = (bank_size * 3) | tmp;
#else
	/* PPC405EP chip only supports two SDRAM banks */
	if (bank_cnt > 1)
		sdram0_b1cr = (bank_size * 1) | tmp;
	if (bank_cnt > 2)
		total_size = 2 * bank_size;
#endif

	/*
	 *   enable sdram controller DCE=1
	 *  enable burst read prefetch to 32 bytes BRPF=2
	 *  leave other functions off
	 */

	/*------------------------------------------------------------------
	 * now that we've done our calculations, we are ready to
	 * program all the registers.
	 * -------------------------------------------------------------------*/

#define mtsdram0(reg, data)  mtdcr(memcfga,reg);mtdcr(memcfgd,data)
	/* disable memcontroller so updates work */
	mtsdram0( mem_mcopt1, 0 );

#ifndef CONFIG_405EP /* not on PPC405EP */
	mtsdram0( mem_besra , sdram0_besr0 );
	mtsdram0( mem_besrb , sdram0_besr1 );
	mtsdram0( mem_ecccf , sdram0_ecccfg );
	mtsdram0( mem_eccerr, sdram0_eccesr );
#endif
	mtsdram0( mem_rtr   , sdram0_rtr );
	mtsdram0( mem_pmit  , sdram0_pmit );
	mtsdram0( mem_mb0cf , sdram0_b0cr );
	mtsdram0( mem_mb1cf , sdram0_b1cr );
#ifndef CONFIG_405EP /* not on PPC405EP */
	mtsdram0( mem_mb2cf , sdram0_b2cr );
	mtsdram0( mem_mb3cf , sdram0_b3cr );
#endif
	mtsdram0( mem_sdtr1 , sdram0_tr );

	/* SDRAM have a power on delay,	 500 micro should do */
	udelay(500);
	sdram0_cfg = SDRAM0_CFG_DCE | SDRAM0_CFG_BRPF(1) | SDRAM0_CFG_ECCDD | SDRAM0_CFG_EMDULR;
	if (ecc_on)
		sdram0_cfg |= SDRAM0_CFG_MEMCHK;
	mtsdram0(mem_mcopt1, sdram0_cfg);

	return (total_size);
}

int spd_read(uint addr)
{
	uchar data[2];

	if (i2c_read(SPD_EEPROM_ADDRESS, addr, 1, data, 1) == 0)
		return (int)data[0];
	else
		return 0;
}

#else /* CONFIG_440 */

/*-----------------------------------------------------------------------------
  |  Memory Controller Options 0
  +-----------------------------------------------------------------------------*/
#define SDRAM_CFG0_DCEN		0x80000000	/* SDRAM Controller Enable	*/
#define SDRAM_CFG0_MCHK_MASK	0x30000000	/* Memory data errchecking mask */
#define SDRAM_CFG0_MCHK_NON	0x00000000	/* No ECC generation		*/
#define SDRAM_CFG0_MCHK_GEN	0x20000000	/* ECC generation		*/
#define SDRAM_CFG0_MCHK_CHK	0x30000000	/* ECC generation and checking	*/
#define SDRAM_CFG0_RDEN		0x08000000	/* Registered DIMM enable	*/
#define SDRAM_CFG0_PMUD		0x04000000	/* Page management unit		*/
#define SDRAM_CFG0_DMWD_MASK	0x02000000	/* DRAM width mask		*/
#define SDRAM_CFG0_DMWD_32	0x00000000	/* 32 bits			*/
#define SDRAM_CFG0_DMWD_64	0x02000000	/* 64 bits			*/
#define SDRAM_CFG0_UIOS_MASK	0x00C00000	/* Unused IO State		*/
#define SDRAM_CFG0_PDP		0x00200000	/* Page deallocation policy	*/

/*-----------------------------------------------------------------------------
  |  Memory Controller Options 1
  +-----------------------------------------------------------------------------*/
#define SDRAM_CFG1_SRE		0x80000000	/* Self-Refresh Entry		*/
#define SDRAM_CFG1_PMEN		0x40000000	/* Power Management Enable	*/

/*-----------------------------------------------------------------------------+
  |  SDRAM DEVPOT Options
  +-----------------------------------------------------------------------------*/
#define SDRAM_DEVOPT_DLL	0x80000000
#define SDRAM_DEVOPT_DS		0x40000000

/*-----------------------------------------------------------------------------+
  |  SDRAM MCSTS Options
  +-----------------------------------------------------------------------------*/
#define SDRAM_MCSTS_MRSC	0x80000000
#define SDRAM_MCSTS_SRMS	0x40000000
#define SDRAM_MCSTS_CIS		0x20000000

/*-----------------------------------------------------------------------------
  |  SDRAM Refresh Timer Register
  +-----------------------------------------------------------------------------*/
#define SDRAM_RTR_RINT_MASK	  0xFFFF0000
#define SDRAM_RTR_RINT_ENCODE(n)  (((n) << 16) & SDRAM_RTR_RINT_MASK)
#define sdram_HZ_to_ns(hertz)	  (1000000000/(hertz))

/*-----------------------------------------------------------------------------+
  |  SDRAM UABus Base Address Reg
  +-----------------------------------------------------------------------------*/
#define SDRAM_UABBA_UBBA_MASK	0x0000000F

/*-----------------------------------------------------------------------------+
  |  Memory Bank 0-7 configuration
  +-----------------------------------------------------------------------------*/
#define SDRAM_BXCR_SDBA_MASK	0xff800000	  /* Base address	      */
#define SDRAM_BXCR_SDSZ_MASK	0x000e0000	  /* Size		      */
#define SDRAM_BXCR_SDSZ_8	0x00020000	  /*   8M		      */
#define SDRAM_BXCR_SDSZ_16	0x00040000	  /*  16M		      */
#define SDRAM_BXCR_SDSZ_32	0x00060000	  /*  32M		      */
#define SDRAM_BXCR_SDSZ_64	0x00080000	  /*  64M		      */
#define SDRAM_BXCR_SDSZ_128	0x000a0000	  /* 128M		      */
#define SDRAM_BXCR_SDSZ_256	0x000c0000	  /* 256M		      */
#define SDRAM_BXCR_SDSZ_512	0x000e0000	  /* 512M		      */
#define SDRAM_BXCR_SDAM_MASK	0x0000e000	  /* Addressing mode	      */
#define SDRAM_BXCR_SDAM_1	0x00000000	  /*   Mode 1		      */
#define SDRAM_BXCR_SDAM_2	0x00002000	  /*   Mode 2		      */
#define SDRAM_BXCR_SDAM_3	0x00004000	  /*   Mode 3		      */
#define SDRAM_BXCR_SDAM_4	0x00006000	  /*   Mode 4		      */
#define SDRAM_BXCR_SDBE		0x00000001	  /* Memory Bank Enable	      */

/*-----------------------------------------------------------------------------+
  |  SDRAM TR0 Options
  +-----------------------------------------------------------------------------*/
#define SDRAM_TR0_SDWR_MASK	0x80000000
#define	 SDRAM_TR0_SDWR_2_CLK	0x00000000
#define	 SDRAM_TR0_SDWR_3_CLK	0x80000000
#define SDRAM_TR0_SDWD_MASK	0x40000000
#define	 SDRAM_TR0_SDWD_0_CLK	0x00000000
#define	 SDRAM_TR0_SDWD_1_CLK	0x40000000
#define SDRAM_TR0_SDCL_MASK	0x01800000
#define	 SDRAM_TR0_SDCL_2_0_CLK 0x00800000
#define	 SDRAM_TR0_SDCL_2_5_CLK 0x01000000
#define	 SDRAM_TR0_SDCL_3_0_CLK 0x01800000
#define SDRAM_TR0_SDPA_MASK	0x000C0000
#define	 SDRAM_TR0_SDPA_2_CLK	0x00040000
#define	 SDRAM_TR0_SDPA_3_CLK	0x00080000
#define	 SDRAM_TR0_SDPA_4_CLK	0x000C0000
#define SDRAM_TR0_SDCP_MASK	0x00030000
#define	 SDRAM_TR0_SDCP_2_CLK	0x00000000
#define	 SDRAM_TR0_SDCP_3_CLK	0x00010000
#define	 SDRAM_TR0_SDCP_4_CLK	0x00020000
#define	 SDRAM_TR0_SDCP_5_CLK	0x00030000
#define SDRAM_TR0_SDLD_MASK	0x0000C000
#define	 SDRAM_TR0_SDLD_1_CLK	0x00000000
#define	 SDRAM_TR0_SDLD_2_CLK	0x00004000
#define SDRAM_TR0_SDRA_MASK	0x0000001C
#define	 SDRAM_TR0_SDRA_6_CLK	0x00000000
#define	 SDRAM_TR0_SDRA_7_CLK	0x00000004
#define	 SDRAM_TR0_SDRA_8_CLK	0x00000008
#define	 SDRAM_TR0_SDRA_9_CLK	0x0000000C
#define	 SDRAM_TR0_SDRA_10_CLK	0x00000010
#define	 SDRAM_TR0_SDRA_11_CLK	0x00000014
#define	 SDRAM_TR0_SDRA_12_CLK	0x00000018
#define	 SDRAM_TR0_SDRA_13_CLK	0x0000001C
#define SDRAM_TR0_SDRD_MASK	0x00000003
#define	 SDRAM_TR0_SDRD_2_CLK	0x00000001
#define	 SDRAM_TR0_SDRD_3_CLK	0x00000002
#define	 SDRAM_TR0_SDRD_4_CLK	0x00000003

/*-----------------------------------------------------------------------------+
  |  SDRAM TR1 Options
  +-----------------------------------------------------------------------------*/
#define SDRAM_TR1_RDSS_MASK	0xC0000000
#define	 SDRAM_TR1_RDSS_TR0	0x00000000
#define	 SDRAM_TR1_RDSS_TR1	0x40000000
#define	 SDRAM_TR1_RDSS_TR2	0x80000000
#define	 SDRAM_TR1_RDSS_TR3	0xC0000000
#define SDRAM_TR1_RDSL_MASK	0x00C00000
#define	 SDRAM_TR1_RDSL_STAGE1	0x00000000
#define	 SDRAM_TR1_RDSL_STAGE2	0x00400000
#define	 SDRAM_TR1_RDSL_STAGE3	0x00800000
#define SDRAM_TR1_RDCD_MASK	0x00000800
#define	 SDRAM_TR1_RDCD_RCD_0_0 0x00000000
#define	 SDRAM_TR1_RDCD_RCD_1_2 0x00000800
#define SDRAM_TR1_RDCT_MASK	0x000001FF
#define	 SDRAM_TR1_RDCT_ENCODE(x)  (((x) << 0) & SDRAM_TR1_RDCT_MASK)
#define	 SDRAM_TR1_RDCT_DECODE(x)  (((x) & SDRAM_TR1_RDCT_MASK) >> 0)
#define	 SDRAM_TR1_RDCT_MIN	0x00000000
#define	 SDRAM_TR1_RDCT_MAX	0x000001FF

/*-----------------------------------------------------------------------------+
  |  SDRAM WDDCTR Options
  +-----------------------------------------------------------------------------*/
#define SDRAM_WDDCTR_WRCP_MASK	0xC0000000
#define	 SDRAM_WDDCTR_WRCP_0DEG	  0x00000000
#define	 SDRAM_WDDCTR_WRCP_90DEG  0x40000000
#define	 SDRAM_WDDCTR_WRCP_180DEG 0x80000000
#define SDRAM_WDDCTR_DCD_MASK	0x000001FF

/*-----------------------------------------------------------------------------+
  |  SDRAM CLKTR Options
  +-----------------------------------------------------------------------------*/
#define SDRAM_CLKTR_CLKP_MASK	0xC0000000
#define	 SDRAM_CLKTR_CLKP_0DEG	  0x00000000
#define	 SDRAM_CLKTR_CLKP_90DEG	  0x40000000
#define	 SDRAM_CLKTR_CLKP_180DEG  0x80000000
#define SDRAM_CLKTR_DCDT_MASK	0x000001FF

/*-----------------------------------------------------------------------------+
  |  SDRAM DLYCAL Options
  +-----------------------------------------------------------------------------*/
#define SDRAM_DLYCAL_DLCV_MASK	0x000003FC
#define	 SDRAM_DLYCAL_DLCV_ENCODE(x) (((x)<<2) & SDRAM_DLYCAL_DLCV_MASK)
#define	 SDRAM_DLYCAL_DLCV_DECODE(x) (((x) & SDRAM_DLYCAL_DLCV_MASK)>>2)

/*-----------------------------------------------------------------------------+
  |  General Definition
  +-----------------------------------------------------------------------------*/
#define DEFAULT_SPD_ADDR1	0x53
#define DEFAULT_SPD_ADDR2	0x52
#define MAXBANKS		4		/* at most 4 dimm banks */
#define MAX_SPD_BYTES		256
#define NUMHALFCYCLES		4
#define NUMMEMTESTS		8
#define NUMMEMWORDS		8
#define MAXBXCR			4
#define TRUE			1
#define FALSE			0

const unsigned long test[NUMMEMTESTS][NUMMEMWORDS] = {
	{0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
	 0xFFFFFFFF, 0xFFFFFFFF},
	{0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
	 0x00000000, 0x00000000},
	{0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
	 0x55555555, 0x55555555},
	{0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
	 0xAAAAAAAA, 0xAAAAAAAA},
	{0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
	 0x5A5A5A5A, 0x5A5A5A5A},
	{0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
	 0xA5A5A5A5, 0xA5A5A5A5},
	{0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55,
	 0x55AA55AA, 0x55AA55AA},
	{0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
	 0xAA55AA55, 0xAA55AA55}
};

/* bank_parms is used to sort the bank sizes by descending order */
struct bank_param {
	unsigned long cr;
	unsigned long bank_size_bytes;
};

typedef struct bank_param BANKPARMS;

#ifdef CFG_SIMULATE_SPD_EEPROM
extern unsigned char cfg_simulate_spd_eeprom[128];
#endif

unsigned char spd_read(uchar chip, uint addr);

void get_spd_info(unsigned long* dimm_populated,
		  unsigned char* iic0_dimm_addr,
		  unsigned long	 num_dimm_banks);

void check_mem_type
(unsigned long* dimm_populated,
 unsigned char* iic0_dimm_addr,
 unsigned long	num_dimm_banks);

void check_volt_type
(unsigned long* dimm_populated,
 unsigned char* iic0_dimm_addr,
 unsigned long	num_dimm_banks);

void program_cfg0(unsigned long* dimm_populated,
		  unsigned char* iic0_dimm_addr,
		  unsigned long	 num_dimm_banks);

void program_cfg1(unsigned long* dimm_populated,
		  unsigned char* iic0_dimm_addr,
		  unsigned long	 num_dimm_banks);

void program_rtr (unsigned long* dimm_populated,
		  unsigned char* iic0_dimm_addr,
		  unsigned long	 num_dimm_banks);

void program_tr0 (unsigned long* dimm_populated,
		  unsigned char* iic0_dimm_addr,
		  unsigned long	 num_dimm_banks);

void program_tr1 (void);

void program_ecc (unsigned long	 num_bytes);

unsigned
long  program_bxcr(unsigned long* dimm_populated,
		   unsigned char* iic0_dimm_addr,
		   unsigned long  num_dimm_banks);

/*
 * This function is reading data from the DIMM module EEPROM over the SPD bus
 * and uses that to program the sdram controller.
 *
 * This works on boards that has the same schematics that the AMCC walnut has.
 *
 * BUG: Don't handle ECC memory
 * BUG: A few values in the TR register is currently hardcoded
 */

long int spd_sdram(void) {
	unsigned char iic0_dimm_addr[] = SPD_EEPROM_ADDRESS;
	unsigned long dimm_populated[sizeof(iic0_dimm_addr)];
	unsigned long total_size;
	unsigned long cfg0;
	unsigned long mcsts;
	unsigned long num_dimm_banks;		    /* on board dimm banks */

	num_dimm_banks = sizeof(iic0_dimm_addr);

	/*
	 * Make sure I2C controller is initialized
	 * before continuing.
	 */
	i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);

	/*
	 * Read the SPD information using I2C interface. Check to see if the
	 * DIMM slots are populated.
	 */
	get_spd_info(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * Check the memory type for the dimms plugged.
	 */
	check_mem_type(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * Check the voltage type for the dimms plugged.
	 */
	check_volt_type(dimm_populated, iic0_dimm_addr, num_dimm_banks);

#if defined(CONFIG_440GX) || defined(CONFIG_440EP) || defined(CONFIG_440GR) || defined(CONFIG_440SP)
	/*
	 * Soft-reset SDRAM controller.
	 */
	mtsdr(sdr_srst, SDR0_SRST_DMC);
	mtsdr(sdr_srst, 0x00000000);
#endif

	/*
	 * program 440GP SDRAM controller options (SDRAM0_CFG0)
	 */
	program_cfg0(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * program 440GP SDRAM controller options (SDRAM0_CFG1)
	 */
	program_cfg1(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * program SDRAM refresh register (SDRAM0_RTR)
	 */
	program_rtr(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * program SDRAM Timing Register 0 (SDRAM0_TR0)
	 */
	program_tr0(dimm_populated, iic0_dimm_addr, num_dimm_banks);

	/*
	 * program the BxCR registers to find out total sdram installed
	 */
	total_size = program_bxcr(dimm_populated, iic0_dimm_addr,
				  num_dimm_banks);

	/*
	 * program SDRAM Clock Timing Register (SDRAM0_CLKTR)
	 */
	mtsdram(mem_clktr, 0x40000000);

	/*
	 * delay to ensure 200 usec has elapsed
	 */
	udelay(400);

	/*
	 * enable the memory controller
	 */
	mfsdram(mem_cfg0, cfg0);
	mtsdram(mem_cfg0, cfg0 | SDRAM_CFG0_DCEN);

	/*
	 * wait for SDRAM_CFG0_DC_EN to complete
	 */
	while (1) {
		mfsdram(mem_mcsts, mcsts);
		if ((mcsts & SDRAM_MCSTS_MRSC) != 0) {
			break;
		}
	}

	/*
	 * program SDRAM Timing Register 1, adding some delays
	 */
	program_tr1();

	/*
	 * if ECC is enabled, initialize parity bits
	 */

	return total_size;
}

unsigned char spd_read(uchar chip, uint addr)
{
	unsigned char data[2];

#ifdef CFG_SIMULATE_SPD_EEPROM
	if (chip == CFG_SIMULATE_SPD_EEPROM) {
		/*
		 * Onboard spd eeprom requested -> simulate values
		 */
		return cfg_simulate_spd_eeprom[addr];
	}
#endif /* CFG_SIMULATE_SPD_EEPROM */

	if (i2c_probe(chip) == 0) {
		if (i2c_read(chip, addr, 1, data, 1) == 0) {
			return data[0];
		}
	}

	return 0;
}

void get_spd_info(unsigned long*   dimm_populated,
		  unsigned char*   iic0_dimm_addr,
		  unsigned long	   num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long dimm_found;
	unsigned char num_of_bytes;
	unsigned char total_size;

	dimm_found = FALSE;
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		num_of_bytes = 0;
		total_size = 0;

		num_of_bytes = spd_read(iic0_dimm_addr[dimm_num], 0);
		total_size = spd_read(iic0_dimm_addr[dimm_num], 1);

		if ((num_of_bytes != 0) && (total_size != 0)) {
			dimm_populated[dimm_num] = TRUE;
			dimm_found = TRUE;
#if 0
			printf("DIMM slot %lu: populated\n", dimm_num);
#endif
		} else {
			dimm_populated[dimm_num] = FALSE;
#if 0
			printf("DIMM slot %lu: Not populated\n", dimm_num);
#endif
		}
	}

	if (dimm_found == FALSE) {
		printf("ERROR - No memory installed. Install a DDR-SDRAM DIMM.\n\n");
		hang();
	}
}

void check_mem_type(unsigned long*   dimm_populated,
		    unsigned char*   iic0_dimm_addr,
		    unsigned long    num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned char dimm_type;

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			dimm_type = spd_read(iic0_dimm_addr[dimm_num], 2);
			switch (dimm_type) {
			case 7:
#if 0
				printf("DIMM slot %lu: DDR SDRAM detected\n", dimm_num);
#endif
				break;
			default:
				printf("ERROR: Unsupported DIMM detected in slot %lu.\n",
				       dimm_num);
				printf("Only DDR SDRAM DIMMs are supported.\n");
				printf("Replace the DIMM module with a supported DIMM.\n\n");
				hang();
				break;
			}
		}
	}
}


void check_volt_type(unsigned long*   dimm_populated,
		     unsigned char*   iic0_dimm_addr,
		     unsigned long    num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long voltage_type;

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			voltage_type = spd_read(iic0_dimm_addr[dimm_num], 8);
			if (voltage_type != 0x04) {
				printf("ERROR: DIMM %lu with unsupported voltage level.\n",
				       dimm_num);
				hang();
			} else {
#if 0
				printf("DIMM %lu voltage level supported.\n", dimm_num);
#endif
			}
			break;
		}
	}
}

void program_cfg0(unsigned long* dimm_populated,
		  unsigned char* iic0_dimm_addr,
		  unsigned long	 num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long cfg0;
	unsigned long ecc_enabled;
	unsigned char ecc;
	unsigned char attributes;
	unsigned long data_width;
	unsigned long dimm_32bit;
	unsigned long dimm_64bit;

	/*
	 * get Memory Controller Options 0 data
	 */
	mfsdram(mem_cfg0, cfg0);

	/*
	 * clear bits
	 */
	cfg0 &= ~(SDRAM_CFG0_DCEN | SDRAM_CFG0_MCHK_MASK |
		  SDRAM_CFG0_RDEN | SDRAM_CFG0_PMUD |
		  SDRAM_CFG0_DMWD_MASK |
		  SDRAM_CFG0_UIOS_MASK | SDRAM_CFG0_PDP);


	/*
	 * FIXME: assume the DDR SDRAMs in both banks are the same
	 */
	ecc_enabled = TRUE;
	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			ecc = spd_read(iic0_dimm_addr[dimm_num], 11);
			if (ecc != 0x02) {
				ecc_enabled = FALSE;
			}

			/*
			 * program Registered DIMM Enable
			 */
			attributes = spd_read(iic0_dimm_addr[dimm_num], 21);
			if ((attributes & 0x02) != 0x00) {
				cfg0 |= SDRAM_CFG0_RDEN;
			}

			/*
			 * program DDR SDRAM Data Width
			 */
			data_width =
				(unsigned long)spd_read(iic0_dimm_addr[dimm_num],6) +
				(((unsigned long)spd_read(iic0_dimm_addr[dimm_num],7)) << 8);
			if (data_width == 64 || data_width == 72) {
				dimm_64bit = TRUE;
				cfg0 |= SDRAM_CFG0_DMWD_64;
			} else if (data_width == 32 || data_width == 40) {
				dimm_32bit = TRUE;
				cfg0 |= SDRAM_CFG0_DMWD_32;
			} else {
				printf("WARNING: DIMM with datawidth of %lu bits.\n",
				       data_width);
				printf("Only DIMMs with 32 or 64 bit datawidths supported.\n");
				hang();
			}
			break;
		}
	}

	/*
	 * program Memory Data Error Checking
	 */
	if (ecc_enabled == TRUE) {
		cfg0 |= SDRAM_CFG0_MCHK_GEN;
	} else {
		cfg0 |= SDRAM_CFG0_MCHK_NON;
	}

	/*
	 * program Page Management Unit (0 == enabled)
	 */
	cfg0 &= ~SDRAM_CFG0_PMUD;

	/*
	 * program Memory Controller Options 0
	 * Note: DCEN must be enabled after all DDR SDRAM controller
	 * configuration registers get initialized.
	 */
	mtsdram(mem_cfg0, cfg0);
}

void program_cfg1(unsigned long* dimm_populated,
		  unsigned char* iic0_dimm_addr,
		  unsigned long	 num_dimm_banks)
{
	unsigned long cfg1;
	mfsdram(mem_cfg1, cfg1);

	/*
	 * Self-refresh exit, disable PM
	 */
	cfg1 &= ~(SDRAM_CFG1_SRE | SDRAM_CFG1_PMEN);

	/*
	 * program Memory Controller Options 1
	 */
	mtsdram(mem_cfg1, cfg1);
}

void program_rtr (unsigned long* dimm_populated,
		  unsigned char* iic0_dimm_addr,
		  unsigned long	 num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long bus_period_x_10;
	unsigned long refresh_rate = 0;
	unsigned char refresh_rate_type;
	unsigned long refresh_interval;
	unsigned long sdram_rtr;
	PPC440_SYS_INFO sys_info;

	/*
	 * get the board info
	 */
	get_sys_info(&sys_info);
	bus_period_x_10 = ONE_BILLION / (sys_info.freqPLB / 10);


	for (dimm_num = 0;  dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			refresh_rate_type = 0x7F & spd_read(iic0_dimm_addr[dimm_num], 12);
			switch (refresh_rate_type) {
			case 0x00:
				refresh_rate = 15625;
				break;
			case 0x01:
				refresh_rate = 15625/4;
				break;
			case 0x02:
				refresh_rate = 15625/2;
				break;
			case 0x03:
				refresh_rate = 15626*2;
				break;
			case 0x04:
				refresh_rate = 15625*4;
				break;
			case 0x05:
				refresh_rate = 15625*8;
				break;
			default:
				printf("ERROR: DIMM %lu, unsupported refresh rate/type.\n",
				       dimm_num);
				printf("Replace the DIMM module with a supported DIMM.\n");
				break;
			}

			break;
		}
	}

	refresh_interval = refresh_rate * 10 / bus_period_x_10;
	sdram_rtr = (refresh_interval & 0x3ff8) <<  16;

	/*
	 * program Refresh Timer Register (SDRAM0_RTR)
	 */
	mtsdram(mem_rtr, sdram_rtr);
}

void program_tr0 (unsigned long* dimm_populated,
		  unsigned char* iic0_dimm_addr,
		  unsigned long	 num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long tr0;
	unsigned char wcsbc;
	unsigned char t_rp_ns;
	unsigned char t_rcd_ns;
	unsigned char t_ras_ns;
	unsigned long t_rp_clk;
	unsigned long t_ras_rcd_clk;
	unsigned long t_rcd_clk;
	unsigned long t_rfc_clk;
	unsigned long plb_check;
	unsigned char cas_bit;
	unsigned long cas_index;
	unsigned char cas_2_0_available;
	unsigned char cas_2_5_available;
	unsigned char cas_3_0_available;
	unsigned long cycle_time_ns_x_10[3];
	unsigned long tcyc_3_0_ns_x_10;
	unsigned long tcyc_2_5_ns_x_10;
	unsigned long tcyc_2_0_ns_x_10;
	unsigned long tcyc_reg;
	unsigned long bus_period_x_10;
	PPC440_SYS_INFO sys_info;
	unsigned long residue;

	/*
	 * get the board info
	 */
	get_sys_info(&sys_info);
	bus_period_x_10 = ONE_BILLION / (sys_info.freqPLB / 10);

	/*
	 * get SDRAM Timing Register 0 (SDRAM_TR0) and clear bits
	 */
	mfsdram(mem_tr0, tr0);
	tr0 &= ~(SDRAM_TR0_SDWR_MASK | SDRAM_TR0_SDWD_MASK |
		 SDRAM_TR0_SDCL_MASK | SDRAM_TR0_SDPA_MASK |
		 SDRAM_TR0_SDCP_MASK | SDRAM_TR0_SDLD_MASK |
		 SDRAM_TR0_SDRA_MASK | SDRAM_TR0_SDRD_MASK);

	/*
	 * initialization
	 */
	wcsbc = 0;
	t_rp_ns = 0;
	t_rcd_ns = 0;
	t_ras_ns = 0;
	cas_2_0_available = TRUE;
	cas_2_5_available = TRUE;
	cas_3_0_available = TRUE;
	tcyc_2_0_ns_x_10 = 0;
	tcyc_2_5_ns_x_10 = 0;
	tcyc_3_0_ns_x_10 = 0;

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			wcsbc = spd_read(iic0_dimm_addr[dimm_num], 15);
			t_rp_ns	 = spd_read(iic0_dimm_addr[dimm_num], 27) >> 2;
			t_rcd_ns = spd_read(iic0_dimm_addr[dimm_num], 29) >> 2;
			t_ras_ns = spd_read(iic0_dimm_addr[dimm_num], 30);
			cas_bit = spd_read(iic0_dimm_addr[dimm_num], 18);

			for (cas_index = 0; cas_index < 3; cas_index++) {
				switch (cas_index) {
				case 0:
					tcyc_reg = spd_read(iic0_dimm_addr[dimm_num], 9);
					break;
				case 1:
					tcyc_reg = spd_read(iic0_dimm_addr[dimm_num], 23);
					break;
				default:
					tcyc_reg = spd_read(iic0_dimm_addr[dimm_num], 25);
					break;
				}

				if ((tcyc_reg & 0x0F) >= 10) {
					printf("ERROR: Tcyc incorrect for DIMM in slot %lu\n",
					       dimm_num);
					hang();
				}

				cycle_time_ns_x_10[cas_index] =
					(((tcyc_reg & 0xF0) >> 4) * 10) + (tcyc_reg & 0x0F);
			}

			cas_index = 0;

			if ((cas_bit & 0x80) != 0) {
				cas_index += 3;
			} else if ((cas_bit & 0x40) != 0) {
				cas_index += 2;
			} else if ((cas_bit & 0x20) != 0) {
				cas_index += 1;
			}

			if (((cas_bit & 0x10) != 0) && (cas_index < 3)) {
				tcyc_3_0_ns_x_10 = cycle_time_ns_x_10[cas_index];
				cas_index++;
			} else {
				if (cas_index != 0) {
					cas_index++;
				}
				cas_3_0_available = FALSE;
			}

			if (((cas_bit & 0x08) != 0) || (cas_index < 3)) {
				tcyc_2_5_ns_x_10 = cycle_time_ns_x_10[cas_index];
				cas_index++;
			} else {
				if (cas_index != 0) {
					cas_index++;
				}
				cas_2_5_available = FALSE;
			}

			if (((cas_bit & 0x04) != 0) || (cas_index < 3)) {
				tcyc_2_0_ns_x_10 = cycle_time_ns_x_10[cas_index];
				cas_index++;
			} else {
				if (cas_index != 0) {
					cas_index++;
				}
				cas_2_0_available = FALSE;
			}

			break;
		}
	}

	/*
	 * Program SD_WR and SD_WCSBC fields
	 */
	tr0 |= SDRAM_TR0_SDWR_2_CLK;		    /* Write Recovery: 2 CLK */
	switch (wcsbc) {
	case 0:
		tr0 |= SDRAM_TR0_SDWD_0_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDWD_1_CLK;
		break;
	}

	/*
	 * Program SD_CASL field
	 */
	if ((cas_2_0_available == TRUE) &&
	    (bus_period_x_10 >= tcyc_2_0_ns_x_10)) {
		tr0 |= SDRAM_TR0_SDCL_2_0_CLK;
	} else if ((cas_2_5_available == TRUE) &&
		 (bus_period_x_10 >= tcyc_2_5_ns_x_10)) {
		tr0 |= SDRAM_TR0_SDCL_2_5_CLK;
	} else if ((cas_3_0_available == TRUE) &&
		 (bus_period_x_10 >= tcyc_3_0_ns_x_10)) {
		tr0 |= SDRAM_TR0_SDCL_3_0_CLK;
	} else {
		printf("ERROR: No supported CAS latency with the installed DIMMs.\n");
		printf("Only CAS latencies of 2.0, 2.5, and 3.0 are supported.\n");
		printf("Make sure the PLB speed is within the supported range.\n");
		hang();
	}

	/*
	 * Calculate Trp in clock cycles and round up if necessary
	 * Program SD_PTA field
	 */
	t_rp_clk = sys_info.freqPLB * t_rp_ns / ONE_BILLION;
	plb_check = ONE_BILLION * t_rp_clk / t_rp_ns;
	if (sys_info.freqPLB != plb_check) {
		t_rp_clk++;
	}
	switch ((unsigned long)t_rp_clk) {
	case 0:
	case 1:
	case 2:
		tr0 |= SDRAM_TR0_SDPA_2_CLK;
		break;
	case 3:
		tr0 |= SDRAM_TR0_SDPA_3_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDPA_4_CLK;
		break;
	}

	/*
	 * Program SD_CTP field
	 */
	t_ras_rcd_clk = sys_info.freqPLB * (t_ras_ns - t_rcd_ns) / ONE_BILLION;
	plb_check = ONE_BILLION * t_ras_rcd_clk / (t_ras_ns - t_rcd_ns);
	if (sys_info.freqPLB != plb_check) {
		t_ras_rcd_clk++;
	}
	switch (t_ras_rcd_clk) {
	case 0:
	case 1:
	case 2:
		tr0 |= SDRAM_TR0_SDCP_2_CLK;
		break;
	case 3:
		tr0 |= SDRAM_TR0_SDCP_3_CLK;
		break;
	case 4:
		tr0 |= SDRAM_TR0_SDCP_4_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDCP_5_CLK;
		break;
	}

	/*
	 * Program SD_LDF field
	 */
	tr0 |= SDRAM_TR0_SDLD_2_CLK;

	/*
	 * Program SD_RFTA field
	 * FIXME tRFC hardcoded as 75 nanoseconds
	 */
	t_rfc_clk = sys_info.freqPLB / (ONE_BILLION / 75);
	residue = sys_info.freqPLB % (ONE_BILLION / 75);
	if (residue >= (ONE_BILLION / 150)) {
		t_rfc_clk++;
	}
	switch (t_rfc_clk) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		tr0 |= SDRAM_TR0_SDRA_6_CLK;
		break;
	case 7:
		tr0 |= SDRAM_TR0_SDRA_7_CLK;
		break;
	case 8:
		tr0 |= SDRAM_TR0_SDRA_8_CLK;
		break;
	case 9:
		tr0 |= SDRAM_TR0_SDRA_9_CLK;
		break;
	case 10:
		tr0 |= SDRAM_TR0_SDRA_10_CLK;
		break;
	case 11:
		tr0 |= SDRAM_TR0_SDRA_11_CLK;
		break;
	case 12:
		tr0 |= SDRAM_TR0_SDRA_12_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDRA_13_CLK;
		break;
	}

	/*
	 * Program SD_RCD field
	 */
	t_rcd_clk = sys_info.freqPLB * t_rcd_ns / ONE_BILLION;
	plb_check = ONE_BILLION * t_rcd_clk / t_rcd_ns;
	if (sys_info.freqPLB != plb_check) {
		t_rcd_clk++;
	}
	switch (t_rcd_clk) {
	case 0:
	case 1:
	case 2:
		tr0 |= SDRAM_TR0_SDRD_2_CLK;
		break;
	case 3:
		tr0 |= SDRAM_TR0_SDRD_3_CLK;
		break;
	default:
		tr0 |= SDRAM_TR0_SDRD_4_CLK;
		break;
	}

#if 0
	printf("tr0: %x\n", tr0);
#endif
	mtsdram(mem_tr0, tr0);
}

void program_tr1 (void)
{
	unsigned long tr0;
	unsigned long tr1;
	unsigned long cfg0;
	unsigned long ecc_temp;
	unsigned long dlycal;
	unsigned long dly_val;
	unsigned long i, j, k;
	unsigned long bxcr_num;
	unsigned long max_pass_length;
	unsigned long current_pass_length;
	unsigned long current_fail_length;
	unsigned long current_start;
	unsigned long rdclt;
	unsigned long rdclt_offset;
	long max_start;
	long max_end;
	long rdclt_average;
	unsigned char window_found;
	unsigned char fail_found;
	unsigned char pass_found;
	unsigned long * membase;
	PPC440_SYS_INFO sys_info;

	/*
	 * get the board info
	 */
	get_sys_info(&sys_info);

	/*
	 * get SDRAM Timing Register 0 (SDRAM_TR0) and clear bits
	 */
	mfsdram(mem_tr1, tr1);
	tr1 &= ~(SDRAM_TR1_RDSS_MASK | SDRAM_TR1_RDSL_MASK |
		 SDRAM_TR1_RDCD_MASK | SDRAM_TR1_RDCT_MASK);

	mfsdram(mem_tr0, tr0);
	if (((tr0 & SDRAM_TR0_SDCL_MASK) == SDRAM_TR0_SDCL_2_5_CLK) &&
	    (sys_info.freqPLB > 100000000)) {
		tr1 |= SDRAM_TR1_RDSS_TR2;
		tr1 |= SDRAM_TR1_RDSL_STAGE3;
		tr1 |= SDRAM_TR1_RDCD_RCD_1_2;
	} else {
		tr1 |= SDRAM_TR1_RDSS_TR1;
		tr1 |= SDRAM_TR1_RDSL_STAGE2;
		tr1 |= SDRAM_TR1_RDCD_RCD_0_0;
	}

	/*
	 * save CFG0 ECC setting to a temporary variable and turn ECC off
	 */
	mfsdram(mem_cfg0, cfg0);
	ecc_temp = cfg0 & SDRAM_CFG0_MCHK_MASK;
	mtsdram(mem_cfg0, (cfg0 & ~SDRAM_CFG0_MCHK_MASK) | SDRAM_CFG0_MCHK_NON);

	/*
	 * get the delay line calibration register value
	 */
	mfsdram(mem_dlycal, dlycal);
	dly_val = SDRAM_DLYCAL_DLCV_DECODE(dlycal) << 2;

	max_pass_length = 0;
	max_start = 0;
	max_end = 0;
	current_pass_length = 0;
	current_fail_length = 0;
	current_start = 0;
	rdclt_offset = 0;
	window_found = FALSE;
	fail_found = FALSE;
	pass_found = FALSE;
#ifdef DEBUG
	printf("Starting memory test ");
#endif
	for (k = 0; k < NUMHALFCYCLES; k++) {
		for (rdclt = 0; rdclt < dly_val; rdclt++)  {
			/*
			 * Set the timing reg for the test.
			 */
			mtsdram(mem_tr1, (tr1 | SDRAM_TR1_RDCT_ENCODE(rdclt)));

			for (bxcr_num = 0; bxcr_num < MAXBXCR; bxcr_num++) {
				mtdcr(memcfga, mem_b0cr + (bxcr_num<<2));
				if ((mfdcr(memcfgd) & SDRAM_BXCR_SDBE) == SDRAM_BXCR_SDBE) {
					/* Bank is enabled */
					membase = (unsigned long*)
						(mfdcr(memcfgd) & SDRAM_BXCR_SDBA_MASK);

					/*
					 * Run the short memory test
					 */
					for (i = 0; i < NUMMEMTESTS; i++) {
						for (j = 0; j < NUMMEMWORDS; j++) {
							membase[j] = test[i][j];
							ppcDcbf((unsigned long)&(membase[j]));
						}

						for (j = 0; j < NUMMEMWORDS; j++) {
							if (membase[j] != test[i][j]) {
								ppcDcbf((unsigned long)&(membase[j]));
								break;
							}
							ppcDcbf((unsigned long)&(membase[j]));
						}

						if (j < NUMMEMWORDS) {
							break;
						}
					}

					/*
					 * see if the rdclt value passed
					 */
					if (i < NUMMEMTESTS) {
						break;
					}
				}
			}

			if (bxcr_num == MAXBXCR) {
				if (fail_found == TRUE) {
					pass_found = TRUE;
					if (current_pass_length == 0) {
						current_start = rdclt_offset + rdclt;
					}

					current_fail_length = 0;
					current_pass_length++;

					if (current_pass_length > max_pass_length) {
						max_pass_length = current_pass_length;
						max_start = current_start;
						max_end = rdclt_offset + rdclt;
					}
				}
			} else {
				current_pass_length = 0;
				current_fail_length++;

				if (current_fail_length >= (dly_val>>2)) {
					if (fail_found == FALSE) {
						fail_found = TRUE;
					} else if (pass_found == TRUE) {
						window_found = TRUE;
						break;
					}
				}
			}
		}
#ifdef DEBUG
		printf(".");
#endif
		if (window_found == TRUE) {
			break;
		}

		tr1 = tr1 ^ SDRAM_TR1_RDCD_MASK;
		rdclt_offset += dly_val;
	}
#ifdef DEBUG
	printf("\n");
#endif

	/*
	 * make sure we find the window
	 */
	if (window_found == FALSE) {
		printf("ERROR: Cannot determine a common read delay.\n");
		hang();
	}

	/*
	 * restore the orignal ECC setting
	 */
	mtsdram(mem_cfg0, (cfg0 & ~SDRAM_CFG0_MCHK_MASK) | ecc_temp);

	/*
	 * set the SDRAM TR1 RDCD value
	 */
	tr1 &= ~SDRAM_TR1_RDCD_MASK;
	if ((tr0 & SDRAM_TR0_SDCL_MASK) == SDRAM_TR0_SDCL_2_5_CLK) {
		tr1 |= SDRAM_TR1_RDCD_RCD_1_2;
	} else {
		tr1 |= SDRAM_TR1_RDCD_RCD_0_0;
	}

	/*
	 * set the SDRAM TR1 RDCLT value
	 */
	tr1 &= ~SDRAM_TR1_RDCT_MASK;
	while (max_end >= (dly_val << 1)) {
		max_end -= (dly_val << 1);
		max_start -= (dly_val << 1);
	}

	rdclt_average = ((max_start + max_end) >> 1);
	if (rdclt_average >= 0x60)
		while (1)
			;

	if (rdclt_average < 0) {
		rdclt_average = 0;
	}

	if (rdclt_average >= dly_val) {
		rdclt_average -= dly_val;
		tr1 = tr1 ^ SDRAM_TR1_RDCD_MASK;
	}
	tr1 |= SDRAM_TR1_RDCT_ENCODE(rdclt_average);

#if 0
	printf("tr1: %x\n", tr1);
#endif
	/*
	 * program SDRAM Timing Register 1 TR1
	 */
	mtsdram(mem_tr1, tr1);
}

unsigned long program_bxcr(unsigned long* dimm_populated,
			   unsigned char* iic0_dimm_addr,
			   unsigned long  num_dimm_banks)
{
	unsigned long dimm_num;
	unsigned long bank_base_addr;
	unsigned long cr;
	unsigned long i;
	unsigned long j;
	unsigned long temp;
	unsigned char num_row_addr;
	unsigned char num_col_addr;
	unsigned char num_banks;
	unsigned char bank_size_id;
	unsigned long ctrl_bank_num[MAXBANKS];
	unsigned long bx_cr_num;
	unsigned long largest_size_index;
	unsigned long largest_size;
	unsigned long current_size_index;
	BANKPARMS bank_parms[MAXBXCR];
	unsigned long sorted_bank_num[MAXBXCR]; /* DDR Controller bank number table (sorted by size) */
	unsigned long sorted_bank_size[MAXBXCR]; /* DDR Controller bank size table (sorted by size)*/

	/*
	 * Set the BxCR regs.  First, wipe out the bank config registers.
	 */
	for (bx_cr_num = 0; bx_cr_num < MAXBXCR; bx_cr_num++) {
		mtdcr(memcfga, mem_b0cr + (bx_cr_num << 2));
		mtdcr(memcfgd, 0x00000000);
		bank_parms[bx_cr_num].bank_size_bytes = 0;
	}

#ifdef CONFIG_BAMBOO
	/*
	 * This next section is hardware dependent and must be programmed
	 * to match the hardware.  For bammboo, the following holds...
	 * 1. SDRAM0_B0CR: Bank 0 of dimm 0 ctrl_bank_num : 0
	 * 2. SDRAM0_B1CR: Bank 0 of dimm 1 ctrl_bank_num : 1
	 * 3. SDRAM0_B2CR: Bank 1 of dimm 1 ctrl_bank_num : 1
	 * 4. SDRAM0_B3CR: Bank 0 of dimm 2 ctrl_bank_num : 3
	 * ctrl_bank_num corresponds to the first usable DDR controller bank number by DIMM
	 */
	ctrl_bank_num[0] = 0;
	ctrl_bank_num[1] = 1;
	ctrl_bank_num[2] = 3;
#else
	ctrl_bank_num[0] = 0;
	ctrl_bank_num[1] = 1;
	ctrl_bank_num[2] = 2;
	ctrl_bank_num[3] = 3;
#endif

	/*
	 * reset the bank_base address
	 */
	bank_base_addr = CFG_SDRAM_BASE;

	for (dimm_num = 0; dimm_num < num_dimm_banks; dimm_num++) {
		if (dimm_populated[dimm_num] == TRUE) {
			num_row_addr = spd_read(iic0_dimm_addr[dimm_num], 3);
			num_col_addr = spd_read(iic0_dimm_addr[dimm_num], 4);
			num_banks    = spd_read(iic0_dimm_addr[dimm_num], 5);
			bank_size_id = spd_read(iic0_dimm_addr[dimm_num], 31);

			/*
			 * Set the SDRAM0_BxCR regs
			 */
			cr = 0;
			switch (bank_size_id) {
			case 0x02:
				cr |= SDRAM_BXCR_SDSZ_8;
				break;
			case 0x04:
				cr |= SDRAM_BXCR_SDSZ_16;
				break;
			case 0x08:
				cr |= SDRAM_BXCR_SDSZ_32;
				break;
			case 0x10:
				cr |= SDRAM_BXCR_SDSZ_64;
				break;
			case 0x20:
				cr |= SDRAM_BXCR_SDSZ_128;
				break;
			case 0x40:
				cr |= SDRAM_BXCR_SDSZ_256;
				break;
			case 0x80:
				cr |= SDRAM_BXCR_SDSZ_512;
				break;
			default:
				printf("DDR-SDRAM: DIMM %lu BxCR configuration.\n",
				       dimm_num);
				printf("ERROR: Unsupported value for the banksize: %d.\n",
				       bank_size_id);
				printf("Replace the DIMM module with a supported DIMM.\n\n");
				hang();
			}

			switch (num_col_addr) {
			case 0x08:
				cr |= SDRAM_BXCR_SDAM_1;
				break;
			case 0x09:
				cr |= SDRAM_BXCR_SDAM_2;
				break;
			case 0x0A:
				cr |= SDRAM_BXCR_SDAM_3;
				break;
			case 0x0B:
				cr |= SDRAM_BXCR_SDAM_4;
				break;
			default:
				printf("DDR-SDRAM: DIMM %lu BxCR configuration.\n",
				       dimm_num);
				printf("ERROR: Unsupported value for number of "
				       "column addresses: %d.\n", num_col_addr);
				printf("Replace the DIMM module with a supported DIMM.\n\n");
				hang();
			}

			/*
			 * enable the bank
			 */
			cr |= SDRAM_BXCR_SDBE;

			for (i = 0; i < num_banks; i++) {
				bank_parms[ctrl_bank_num[dimm_num]+i].bank_size_bytes =
					(4 * 1024 * 1024) * bank_size_id;
				bank_parms[ctrl_bank_num[dimm_num]+i].cr = cr;
			}
		}
	}

	/* Initialize sort tables */
	for (i = 0; i < MAXBXCR; i++) {
		sorted_bank_num[i] = i;
		sorted_bank_size[i] = bank_parms[i].bank_size_bytes;
	}

	for (i = 0; i < MAXBXCR-1; i++) {
		largest_size = sorted_bank_size[i];
		largest_size_index = 255;

		/* Find the largest remaining value */
		for (j = i + 1; j < MAXBXCR; j++) {
			if (sorted_bank_size[j] > largest_size) {
				/* Save largest remaining value and its index */
				largest_size = sorted_bank_size[j];
				largest_size_index = j;
			}
		}

		if (largest_size_index != 255) {
			/* Swap the current and largest values */
			current_size_index = sorted_bank_num[largest_size_index];
			sorted_bank_size[largest_size_index] = sorted_bank_size[i];
			sorted_bank_size[i] = largest_size;
			sorted_bank_num[largest_size_index] = sorted_bank_num[i];
			sorted_bank_num[i] = current_size_index;
		}
	}

	/* Set the SDRAM0_BxCR regs thanks to sort tables */
	for (bx_cr_num = 0, bank_base_addr = 0; bx_cr_num < MAXBXCR; bx_cr_num++) {
		if (bank_parms[sorted_bank_num[bx_cr_num]].bank_size_bytes) {
			mtdcr(memcfga, mem_b0cr + (sorted_bank_num[bx_cr_num] << 2));
			temp = mfdcr(memcfgd) & ~(SDRAM_BXCR_SDBA_MASK | SDRAM_BXCR_SDSZ_MASK |
						  SDRAM_BXCR_SDAM_MASK | SDRAM_BXCR_SDBE);
			temp = temp | (bank_base_addr & SDRAM_BXCR_SDBA_MASK) |
				bank_parms[sorted_bank_num[bx_cr_num]].cr;
			mtdcr(memcfgd, temp);
			bank_base_addr += bank_parms[sorted_bank_num[bx_cr_num]].bank_size_bytes;
		}
	}

	return(bank_base_addr);
}

void program_ecc (unsigned long	 num_bytes)
{
	unsigned long bank_base_addr;
	unsigned long current_address;
	unsigned long end_address;
	unsigned long address_increment;
	unsigned long cfg0;

	/*
	 * get Memory Controller Options 0 data
	 */
	mfsdram(mem_cfg0, cfg0);

	/*
	 * reset the bank_base address
	 */
	bank_base_addr = CFG_SDRAM_BASE;

	if ((cfg0 & SDRAM_CFG0_MCHK_MASK) != SDRAM_CFG0_MCHK_NON) {
		mtsdram(mem_cfg0, (cfg0 & ~SDRAM_CFG0_MCHK_MASK) |
			SDRAM_CFG0_MCHK_GEN);

		if ((cfg0 & SDRAM_CFG0_DMWD_MASK) == SDRAM_CFG0_DMWD_32) {
			address_increment = 4;
		} else {
			address_increment = 8;
		}

		current_address = (unsigned long)(bank_base_addr);
		end_address = (unsigned long)(bank_base_addr) + num_bytes;

		while (current_address < end_address) {
			*((unsigned long*)current_address) = 0x00000000;
			current_address += address_increment;
		}

		mtsdram(mem_cfg0, (cfg0 & ~SDRAM_CFG0_MCHK_MASK) |
			SDRAM_CFG0_MCHK_CHK);
	}
}

#endif /* CONFIG_440 */

#endif /* CONFIG_SPD_EEPROM */
