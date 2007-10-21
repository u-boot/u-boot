/*
 * cpu/ppc4xx/40x_spd_sdram.c
 * This SPD SDRAM detection code supports IBM/AMCC PPC44x cpu with a
 * SDRAM controller. Those are all current 405 PPC's.
 *
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

#if defined(CONFIG_SPD_EEPROM) && !defined(CONFIG_440)

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

	PPC4xx_SYS_INFO sys_info;
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

#endif /* CONFIG_SPD_EEPROM */
