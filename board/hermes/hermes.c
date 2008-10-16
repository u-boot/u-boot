/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <commproc.h>
#include <mpc8xx.h>

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
# define SHOW_BOOT_PROGRESS(arg)	show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);
static ulong board_init (void);
static void send_smi_frame (volatile scc_t * sp, volatile cbd_t * bd,
							uchar * msg);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1f07fc04, 0xeeaefc04, 0x11adfc04, 0xefbbbc00,
	0x1ff77c47,					/* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1fe77c35, 0xffaffc34, 0x1fa57c35,	/* last */
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1f07fc04, 0xeeaefc04, 0x10adfc04, 0xf0affc00,
	0xf0affc00, 0xf1affc00, 0xefbbbc00, 0x1ff77c47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1f27fc04, 0xeeaebc00, 0x01b93c04, 0x1ff77c47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1f07fc04, 0xeeaebc00, 0x10ad4c00, 0xf0afcc00,
	0xf0afcc00, 0xe1bb8c06, 0x1ff77c47,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1ff5fc84, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07,		/* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7ffffc07,					/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Test ID string (HERMES...)
 *
 * Return code for board revision and network speed
 */

int checkboard (void)
{
	char *s = getenv ("serial#");
	char *e;

	puts ("Board: ");

	if (!s || strncmp (s, "HERMES", 6)) {
		puts ("### No HW ID - assuming HERMES-PRO");
	} else {
		for (e = s; *e; ++e) {
			if (*e == ' ')
				break;
		}

		for (; s < e; ++s) {
			putc (*s);
		}
	}

	gd->board_type = board_init ();

	printf ("  Rev. %ld.x\n", (gd->board_type >> 16));

	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size, size8, size9;

	upmconfig (UPMA, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh
	 */
	memctl->memc_mptpr = 0x0400;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller banks 1 to the SDRAM banks at preliminary address
	 */
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;

	/* HERMES-PRO boards have only one bank SDRAM */


	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mamr = 0xD0802114;
	memctl->memc_mcr = 0x80002105;
	udelay (1);
	memctl->memc_mamr = 0xD0802118;
	memctl->memc_mcr = 0x80002130;
	udelay (1);
	memctl->memc_mamr = 0xD0802114;
	memctl->memc_mcr = 0x80002106;

	udelay (1000);

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CONFIG_SYS_MAMR_8COL, (long *) SDRAM_BASE_PRELIM,
					   SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CONFIG_SYS_MAMR_9COL, (long *) SDRAM_BASE_PRELIM,
					   SDRAM_MAX_SIZE);

	if (size8 < size9) {		/* leave configuration at 9 columns */
		size = size9;
/*	debug ("SDRAM Bank 0 in 9 column mode: %ld MB\n", size >> 20);	*/
	} else {					/* back to 8 columns            */
		size = size8;
		memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;
		udelay (500);
/*	debug ("SDRAM Bank 0 in 8 column mode: %ld MB\n", size >> 20);	*/
	}

	udelay (1000);

	memctl->memc_or1 = ((-size) & 0xFFFF0000) | SDRAM_TIMING;
	memctl->memc_br1 = (CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	udelay (10000);

	return (size);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base,
						   long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size(base, maxsize));
}

/* ------------------------------------------------------------------------- */

#define	PB_LED_3	0x00020000	/* Status LED's */
#define PB_LED_2	0x00010000
#define PB_LED_1	0x00008000
#define PB_LED_0	0x00004000

#define PB_LED_ALL	(PB_LED_0 | PB_LED_1 | PB_LED_2 | PB_LED_3)

#define	PC_REP_SPD1	0x00000800
#define PC_REP_SPD0	0x00000400

#define PB_RESET_2081	0x00000020	/* Reset PEB2081 */

#define PB_MAI_4	0x00000010	/* Configuration */
#define PB_MAI_3	0x00000008
#define PB_MAI_2	0x00000004
#define PB_MAI_1	0x00000002
#define PB_MAI_0	0x00000001

#define PB_MAI_ALL	(PB_MAI_0 | PB_MAI_1 | PB_MAI_2 | PB_MAI_3 | PB_MAI_4)


#define	PC_REP_MGRPRS	0x0200
#define PC_REP_SPD	0x0040		/* Select 100 Mbps */
#define PC_REP_RES	0x0004
#define PC_BIT14	0x0002		/* ??? */
#define PC_BIT15	0x0001		/* ??? ENDSL ?? */

/* ------------------------------------------------------------------------- */

static ulong board_init (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	ulong reg, revision, speed = 100;
	int ethspeed;
	char *s;

	if ((s = getenv ("ethspeed")) != NULL) {
		if (strcmp (s, "100") == 0) {
			ethspeed = 100;
		} else if (strcmp (s, "10") == 0) {
			ethspeed = 10;
		} else {
			ethspeed = 0;
		}
	} else {
		ethspeed = 0;
	}

	/* Configure Port B Output Pins => 0x0003cc3F */
	reg = PB_LED_ALL | PC_REP_SPD1 | PC_REP_SPD0 | PB_RESET_2081 |
			PB_MAI_ALL;
	immr->im_cpm.cp_pbpar &= ~reg;
	immr->im_cpm.cp_pbodr &= ~reg;
	immr->im_cpm.cp_pbdat &= ~reg;	/* all 0 */
	immr->im_cpm.cp_pbdir |= reg;

	/* Check hardware revision */
	if ((immr->im_ioport.iop_pcdat & 0x0003) == 0x0003) {
		/*
		 * Revision 3.x hardware
		 */
		revision = 3;

		immr->im_ioport.iop_pcdat = 0x0240;
		immr->im_ioport.iop_pcdir = (PC_REP_MGRPRS | PC_REP_SPD | PC_REP_RES | PC_BIT14);	/* = 0x0246 */
		immr->im_ioport.iop_pcdat |= PC_REP_RES;
	} else {
		immr->im_ioport.iop_pcdat = 0x0002;
		immr->im_ioport.iop_pcdir = (PC_REP_MGRPRS | PC_REP_RES | PC_BIT14 | PC_BIT15);	/* = 0x0207 */

		if ((immr->im_ioport.iop_pcdat & PC_REP_SPD) == 0) {
			/*
			 * Revision 2.x hardware: PC9 connected to PB21
			 */
			revision = 2;

			if (ethspeed == 0) {
				/* both 10 and 100 Mbps allowed:
				 * select 10 Mbps and autonegotiation
				 */
				puts ("  [10+100]");
				immr->im_cpm.cp_pbdat = 0;	/* SPD1:SPD0 = 0:0 - autonegot. */
				speed = 10;
			} else if (ethspeed == 10) {
				/* we are asked for 10 Mbps,
				 * so select 10 Mbps
				 */
				puts ("  [10]");
				immr->im_cpm.cp_pbdat = 0;	/* ??? */
				speed = 10;
			} else {
				/* anything else:
				 * select 100 Mbps
				 */
				puts ("  [100]");
				immr->im_cpm.cp_pbdat = PC_REP_SPD0 | PC_REP_SPD1;
				/* SPD1:SPD0 = 1:1 - 100 Mbps */
				speed = 100;
			}
			immr->im_ioport.iop_pcdat |= (PC_REP_RES | PC_BIT14);

			/* must be run from RAM  */
			/* start_lxt980 (speed); */
		/*************************/
		} else {
			/*
			 * Revision 1.x hardware
			 */
			revision = 1;

			immr->im_ioport.iop_pcdat = PC_REP_MGRPRS | PC_BIT14;	/* = 0x0202 */
			immr->im_ioport.iop_pcdir = (PC_REP_MGRPRS | PC_REP_SPD | PC_REP_RES | PC_BIT14 | PC_BIT15);	/* = 0x0247 */

			if (ethspeed == 0) {
				/* both 10 and 100 Mbps allowed:
				 * select 100 Mbps and autonegotiation
				 */
				puts ("  [10+100]");
				immr->im_cpm.cp_pbdat = 0;	/* SPD1:SPD0 = 0:0 - autonegot. */
				immr->im_ioport.iop_pcdat |= PC_REP_SPD;
			} else if (ethspeed == 10) {
				/* we are asked for 10 Mbps,
				   * so select 10 Mbps
				 */
				puts ("  [10]");
				immr->im_cpm.cp_pbdat = PC_REP_SPD0;	/* SPD1:SPD0 = 0:1 - 10 Mbps */
			} else {
				/* anything else:
				   * select 100 Mbps
				 */
				puts ("  [100]");
				immr->im_cpm.cp_pbdat = PC_REP_SPD0 | PC_REP_SPD1;
				/* SPD1:SPD0 = 1:1 - 100 Mbps */
				immr->im_ioport.iop_pcdat |= PC_REP_SPD;
			}

			immr->im_ioport.iop_pcdat |= PC_REP_RES;
		}
	}
	SHOW_BOOT_PROGRESS (0x00);

	return ((revision << 16) | (speed & 0xFFFF));
}

/* ------------------------------------------------------------------------- */

#define SCC_SM		1			/* Index => SCC2 */
#define	PROFF		PROFF_SCC2

#define SMI_MSGLEN	8			/* Length of SMI Messages        */

#define PHYGPCR_ADDR	0x109	/* Port Enable               */
#define PHYPCR_ADDR	0x132		/* PHY Port Control Reg. (port 1)    */
#define LEDPCR_ADDR	0x141		/* LED Port Control Reg.         */
#define RPRESET_ADDR	0x144	/* Repeater Reset            */

#define PHYPCR_SPEED	0x2000	/* on for 100 Mbps, off for 10 Mbps  */
#define PHYPCR_AN	0x1000		/* on to enable  Auto-Negotiation    */
#define PHYPCR_REST_AN	0x0200	/* on to restart Auto-Negotiation    */
#define PHYPCR_FDX	0x0100		/* on for Full Duplex, off for HDX   */
#define PHYPCR_COLT	0x0080		/* on to enable COL signal test      */

/* ------------------------------------------------------------------------- */

/*
 * Must run from RAM:
 * uses parameter RAM area which is used for stack while running from ROM
 */
void hermes_start_lxt980 (int speed)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = (cpm8xx_t *) & (immr->im_cpm);
	volatile scc_t *sp = (scc_t *) & (cp->cp_scc[SCC_SM]);
	volatile cbd_t *bd;
	volatile hdlc_pram_t *hp;
	uchar smimsg[SMI_MSGLEN];
	ushort phypcrval;
	uint bd_off;
	int pnr;

	printf ("LXT9880: %3d Mbps\n", speed);

	immr->im_ioport.iop_paodr |= 0x0008;	/* init PAODR: PA12 (TXD2) open drain */
	immr->im_ioport.iop_papar |= 0x400c;	/* init PAPAR: TXD2, RXD2, BRGO4 */
	immr->im_ioport.iop_padir &= 0xbff3;	/* init PADIR: BRGO4 */
	immr->im_ioport.iop_padir |= 0x4000;

	/* get temporary BD; no need for permanent alloc */
	bd_off = dpram_base_align (8);

	bd = (cbd_t *) (immr->im_cpm.cp_dpmem + bd_off);

	bd->cbd_bufaddr = 0;
	bd->cbd_datlen = 0;
	bd->cbd_sc = BD_SC_WRAP | BD_SC_LAST | BD_SC_INTRPT | BD_SC_TC;

	/* init. baudrate generator BRG4 */
	cp->cp_brgc4 = (0x00010000 | (50 << 1));	/* output 1 MHz */

	cp->cp_sicr &= 0xFFFF00FF;	/* SICR: mask SCC2 */
	cp->cp_sicr |= 0x00001B00;	/* SICR: SCC2 clk BRG4 */

	/* init SCC_SM register */
	sp->scc_psmr = 0x0000;		/* init PSMR: no additional flags */
	sp->scc_todr = 0x0000;
	sp->scc_dsr = 0x7e7e;

	/* init. SCC_SM parameter area */
	hp = (hdlc_pram_t *) & cp->cp_dparam[PROFF];

	hp->tbase = bd_off;			/* offset from beginning of DPRAM */

	hp->rfcr = 0x18;
	hp->tfcr = 0x18;
	hp->mrblr = 10;

	hp->c_mask = 0x0000f0b8;
	hp->c_pres = 0x0000ffff;

	hp->disfc = 0;
	hp->crcec = 0;
	hp->abtsc = 0;
	hp->nmarc = 0;
	hp->retrc = 0;

	hp->mflr = 10;

	hp->rfthr = 1;

	hp->hmask = 0;
	hp->haddr1 = 0;
	hp->haddr2 = 0;
	hp->haddr3 = 0;
	hp->haddr4 = 0;

	cp->cp_cpcr = SCC_SM << 6 | 0x0001;	/* SCC_SM: init TX/RX params */
	while (cp->cp_cpcr & CPM_CR_FLG);

	/* clear all outstanding SCC events */
	sp->scc_scce = ~0;

	/* enable transmitter: GSMR_L: TPL=2(16bits), TPP=3(all ones), ENT */
	sp->scc_gsmrh = 0;
	sp->scc_gsmrl |= SCC_GSMRL_TPL_16 | SCC_GSMRL_TPP_ALL1 |
			SCC_GSMRL_ENT | SCC_GSMRL_MODE_HDLC;

#if 0
	smimsg[0] = 0x00;			/* CHIP/HUB ID */
	smimsg[1] = 0x38;			/* WRITE CMD */
	smimsg[2] = (RPRESET_ADDR << 4) & 0xf0;
	smimsg[3] = RPRESET_ADDR >> 4;
	smimsg[4] = 0x01;
	smimsg[5] = 0x00;
	smimsg[6] = 0x00;
	smimsg[7] = 0x00;

	send_smi_frame (sp, bd, smimsg);
#endif

	smimsg[0] = 0x7f;			/* BROADCAST */
	smimsg[1] = 0x34;			/* ASSIGN HUB ID */
	smimsg[2] = 0x00;
	smimsg[3] = 0x00;
	smimsg[4] = 0x00;			/* HUB ID = 0 */
	smimsg[5] = 0x00;
	smimsg[6] = 0x00;
	smimsg[7] = 0x00;

	send_smi_frame (sp, bd, smimsg);

	smimsg[0] = 0x7f;			/* BROADCAST */
	smimsg[1] = 0x3c;			/* SET ARBOUT TO 0 */
	smimsg[2] = 0x00;			/* ADDRESS = 0 */
	smimsg[3] = 0x00;
	smimsg[4] = 0x00;			/* DATA = 0 */
	smimsg[5] = 0x00;
	smimsg[6] = 0x00;
	smimsg[7] = 0x00;

	send_smi_frame (sp, bd, smimsg);

	if (speed == 100) {
		phypcrval = PHYPCR_SPEED;	/* 100 MBIT, disable autoneg. */
	} else {
		phypcrval = 0;			/* 10 MBIT, disable autoneg. */
	}

	/* send MSGs */
	for (pnr = 0; pnr < 8; pnr++) {
		smimsg[0] = 0x00;		/* CHIP/HUB ID */
		smimsg[1] = 0x38;		/* WRITE CMD */
		smimsg[2] = ((PHYPCR_ADDR + pnr) << 4) & 0xf0;
		smimsg[3] = (PHYPCR_ADDR + pnr) >> 4;
		smimsg[4] = (unsigned char) (phypcrval & 0xff);
		smimsg[5] = (unsigned char) (phypcrval >> 8);
		smimsg[6] = 0x00;
		smimsg[7] = 0x00;

		send_smi_frame (sp, bd, smimsg);
	}

	smimsg[0] = 0x00;			/* CHIP/HUB ID */
	smimsg[1] = 0x38;			/* WRITE CMD */
	smimsg[2] = (PHYGPCR_ADDR << 4) & 0xf0;
	smimsg[3] = PHYGPCR_ADDR >> 4;
	smimsg[4] = 0xff;			/* enable port 1-8 */
	smimsg[5] = 0x01;			/* enable MII1 (0x01) */
	smimsg[6] = 0x00;
	smimsg[7] = 0x00;

	send_smi_frame (sp, bd, smimsg);

	smimsg[0] = 0x00;			/* CHIP/HUB ID */
	smimsg[1] = 0x38;			/* WRITE CMD */
	smimsg[2] = (LEDPCR_ADDR << 4) & 0xf0;
	smimsg[3] = LEDPCR_ADDR >> 4;
	smimsg[4] = 0xaa;			/* Port 1-8 Conf.bits = 10 (Hardware control) */
	smimsg[5] = 0xaa;
	smimsg[6] = 0x00;
	smimsg[7] = 0x00;

	send_smi_frame (sp, bd, smimsg);

	/*
	 * Disable Transmitter (so that we can free the BD, too)
	 */
	sp->scc_gsmrl &= ~SCC_GSMRL_ENT;
}

/* ------------------------------------------------------------------------- */

static void send_smi_frame (volatile scc_t * sp, volatile cbd_t * bd,
							uchar * msg)
{
#ifdef DEBUG
	unsigned hub, chip, cmd, length, addr;

	hub = msg[0] & 0x1F;
	chip = msg[0] >> 5;
	cmd = msg[1] & 0x1F;
	length = (msg[1] >> 5) | ((msg[2] & 0x0F) << 3);
	addr = (msg[2] >> 4) | (msg[3] << 4);

	printf ("SMI send: Hub %02x Chip %x Cmd %02x Len %d Addr %03x: "
			"%02x %02x %02x %02x\n",
			hub, chip, cmd, length, addr, msg[4], msg[5], msg[6], msg[7]);
#endif /* DEBUG */

	bd->cbd_bufaddr = (uint) msg;
	bd->cbd_datlen = SMI_MSGLEN;
	bd->cbd_sc |= BD_SC_READY;

	/* wait for msg transmitted */
	while ((sp->scc_scce & 0x0002) == 0);
	/* clear all events */
	sp->scc_scce = ~0;
}

/* ------------------------------------------------------------------------- */

void show_boot_progress (int status)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

	if (status < -32) status = -1;	/* let things compatible */
	status ^= 0x0F;
	status = (status & 0x0F) << 14;
	immr->im_cpm.cp_pbdat = (immr->im_cpm.cp_pbdat & ~PB_LED_ALL) | status;
}

/* ------------------------------------------------------------------------- */
