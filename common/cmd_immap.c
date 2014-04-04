/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * MPC8xx/MPC8260 Internal Memory Map Functions
 */

#include <common.h>
#include <command.h>

#if defined(CONFIG_8xx) || defined(CONFIG_MPC8260)

#if defined(CONFIG_8xx)
#include <asm/8xx_immap.h>
#include <commproc.h>
#include <asm/iopin_8xx.h>
#elif defined(CONFIG_MPC8260)
#include <asm/immap_8260.h>
#include <asm/cpm_8260.h>
#include <asm/iopin_8260.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

static void
unimplemented ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf ("Sorry, but the '%s' command has not been implemented\n",
		cmdtp->name);
}

int
do_siuinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

#if defined(CONFIG_8xx)
	volatile sysconf8xx_t *sc = &immap->im_siu_conf;
#elif defined(CONFIG_MPC8260)
	volatile sysconf8260_t *sc = &immap->im_siu_conf;
#endif

	printf ("SIUMCR= %08x SYPCR = %08x\n", sc->sc_siumcr, sc->sc_sypcr);
#if defined(CONFIG_8xx)
	printf ("SWT   = %08x\n", sc->sc_swt);
	printf ("SIPEND= %08x SIMASK= %08x\n", sc->sc_sipend, sc->sc_simask);
	printf ("SIEL  = %08x SIVEC = %08x\n", sc->sc_siel, sc->sc_sivec);
	printf ("TESR  = %08x SDCR  = %08x\n", sc->sc_tesr, sc->sc_sdcr);
#elif defined(CONFIG_MPC8260)
	printf ("BCR   = %08x\n", sc->sc_bcr);
	printf ("P_ACR =       %02x P_ALRH= %08x P_ALRL= %08x\n",
		sc->sc_ppc_acr, sc->sc_ppc_alrh, sc->sc_ppc_alrl);
	printf ("L_ACR =       %02x L_ALRH= %08x L_ALRL= %08x\n",
		sc->sc_lcl_acr, sc->sc_lcl_alrh, sc->sc_lcl_alrl);
	printf ("PTESR1= %08x PTESR2= %08x\n", sc->sc_tescr1, sc->sc_tescr2);
	printf ("LTESR1= %08x LTESR2= %08x\n", sc->sc_ltescr1, sc->sc_ltescr2);
	printf ("PDTEA = %08x PDTEM =       %02x\n", sc->sc_pdtea, sc->sc_pdtem);
	printf ("LDTEA = %08x LDTEM =       %02x\n", sc->sc_ldtea, sc->sc_ldtem);
#endif
	return 0;
}

int
do_memcinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

#if defined(CONFIG_8xx)
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	int nbanks = 8;
#elif defined(CONFIG_MPC8260)
	volatile memctl8260_t *memctl = &immap->im_memctl;
	int nbanks = 12;
#endif
	volatile uint *p = &memctl->memc_br0;
	int i;

	for (i = 0; i < nbanks; i++, p += 2) {
		if (i < 10) {
			printf ("BR%d   = %08x OR%d   = %08x\n",
				i, p[0], i, p[1]);
		} else {
			printf ("BR%d  = %08x OR%d  = %08x\n",
				i, p[0], i, p[1]);
		}
	}

	printf ("MAR   = %08x", memctl->memc_mar);
#if defined(CONFIG_8xx)
	printf (" MCR   = %08x\n", memctl->memc_mcr);
#elif defined(CONFIG_MPC8260)
	putc ('\n');
#endif
	printf ("MAMR  = %08x MBMR  = %08x",
		memctl->memc_mamr, memctl->memc_mbmr);
#if defined(CONFIG_8xx)
	printf ("\nMSTAT =     %04x\n", memctl->memc_mstat);
#elif defined(CONFIG_MPC8260)
	printf (" MCMR  = %08x\n", memctl->memc_mcmr);
#endif
	printf ("MPTPR =     %04x MDR   = %08x\n",
		memctl->memc_mptpr, memctl->memc_mdr);
#if defined(CONFIG_MPC8260)
	printf ("PSDMR = %08x LSDMR = %08x\n",
		memctl->memc_psdmr, memctl->memc_lsdmr);
	printf ("PURT  =       %02x PSRT  =       %02x\n",
		memctl->memc_purt, memctl->memc_psrt);
	printf ("LURT  =       %02x LSRT  =       %02x\n",
		memctl->memc_lurt, memctl->memc_lsrt);
	printf ("IMMR  = %08x\n", memctl->memc_immr);
#endif
	return 0;
}

int
do_sitinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}

#ifdef CONFIG_MPC8260
int
do_icinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}
#endif

int
do_carinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

#if defined(CONFIG_8xx)
	volatile car8xx_t *car = &immap->im_clkrst;
#elif defined(CONFIG_MPC8260)
	volatile car8260_t *car = &immap->im_clkrst;
#endif

#if defined(CONFIG_8xx)
	printf ("SCCR  = %08x\n", car->car_sccr);
	printf ("PLPRCR= %08x\n", car->car_plprcr);
	printf ("RSR   = %08x\n", car->car_rsr);
#elif defined(CONFIG_MPC8260)
	printf ("SCCR  = %08x\n", car->car_sccr);
	printf ("SCMR  = %08x\n", car->car_scmr);
	printf ("RSR   = %08x\n", car->car_rsr);
	printf ("RMR   = %08x\n", car->car_rmr);
#endif
	return 0;
}

static int counter;

static void
header(void)
{
	char *data = "\
       --------------------------------        --------------------------------\
       00000000001111111111222222222233        00000000001111111111222222222233\
       01234567890123456789012345678901        01234567890123456789012345678901\
       --------------------------------        --------------------------------\
    ";
	int i;

	if (counter % 2)
		putc('\n');
	counter = 0;

	for (i = 0; i < 4; i++, data += 79)
		printf("%.79s\n", data);
}

static void binary (char *label, uint value, int nbits)
{
	uint mask = 1 << (nbits - 1);
	int i, second = (counter++ % 2);

	if (second)
		putc (' ');
	puts (label);
	for (i = 32 + 1; i != nbits; i--)
		putc (' ');

	while (mask != 0) {
		if (value & mask)
			putc ('1');
		else
			putc ('0');
		mask >>= 1;
	}

	if (second)
		putc ('\n');
}

#if defined(CONFIG_8xx)
#define PA_NBITS	16
#define PA_NB_ODR	 8
#define PB_NBITS	18
#define PB_NB_ODR	16
#define PC_NBITS	12
#define PD_NBITS	13
#elif defined(CONFIG_MPC8260)
#define PA_NBITS	32
#define PA_NB_ODR	32
#define PB_NBITS	28
#define PB_NB_ODR	28
#define PC_NBITS	32
#define PD_NBITS	28
#endif

int
do_iopinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

#if defined(CONFIG_8xx)
	volatile iop8xx_t *iop = &immap->im_ioport;
	volatile ushort *l, *r;
#elif defined(CONFIG_MPC8260)
	volatile iop8260_t *iop = &immap->im_ioport;
	volatile uint *l, *r;
#endif
	volatile uint *R;

	counter = 0;
	header ();

	/*
	 * Ports A & B
	 */

#if defined(CONFIG_8xx)
	l = &iop->iop_padir;
	R = &immap->im_cpm.cp_pbdir;
#elif defined(CONFIG_MPC8260)
	l = &iop->iop_pdira;
	R = &iop->iop_pdirb;
#endif
	binary ("PA_DIR", *l++, PA_NBITS);
	binary ("PB_DIR", *R++, PB_NBITS);
	binary ("PA_PAR", *l++, PA_NBITS);
	binary ("PB_PAR", *R++, PB_NBITS);
#if defined(CONFIG_MPC8260)
	binary ("PA_SOR", *l++, PA_NBITS);
	binary ("PB_SOR", *R++, PB_NBITS);
#endif
	binary ("PA_ODR", *l++, PA_NB_ODR);
	binary ("PB_ODR", *R++, PB_NB_ODR);
	binary ("PA_DAT", *l++, PA_NBITS);
	binary ("PB_DAT", *R++, PB_NBITS);

	header ();

	/*
	 * Ports C & D
	 */

#if defined(CONFIG_8xx)
	l = &iop->iop_pcdir;
	r = &iop->iop_pddir;
#elif defined(CONFIG_MPC8260)
	l = &iop->iop_pdirc;
	r = &iop->iop_pdird;
#endif
	binary ("PC_DIR", *l++, PC_NBITS);
	binary ("PD_DIR", *r++, PD_NBITS);
	binary ("PC_PAR", *l++, PC_NBITS);
	binary ("PD_PAR", *r++, PD_NBITS);
#if defined(CONFIG_8xx)
	binary ("PC_SO ", *l++, PC_NBITS);
	binary ("      ", 0, 0);
	r++;
#elif defined(CONFIG_MPC8260)
	binary ("PC_SOR", *l++, PC_NBITS);
	binary ("PD_SOR", *r++, PD_NBITS);
	binary ("PC_ODR", *l++, PC_NBITS);
	binary ("PD_ODR", *r++, PD_NBITS);
#endif
	binary ("PC_DAT", *l++, PC_NBITS);
	binary ("PD_DAT", *r++, PD_NBITS);
#if defined(CONFIG_8xx)
	binary ("PC_INT", *l++, PC_NBITS);
#endif

	header ();
	return 0;
}

/*
 * set the io pins
 * this needs a clean up for smaller tighter code
 * use *uint and set the address based on cmd + port
 */
int
do_iopset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint rcode = 0;
	iopin_t iopin;
	static uint port = 0;
	static uint pin = 0;
	static uint value = 0;
	static enum {
		DIR,
		PAR,
		SOR,
		ODR,
		DAT,
#if defined(CONFIG_8xx)
		INT
#endif
	} cmd = DAT;

	if (argc != 5) {
		puts ("iopset PORT PIN CMD VALUE\n");
		return 1;
	}
	port = argv[1][0] - 'A';
	if (port > 3)
		port -= 0x20;
	if (port > 3)
		rcode = 1;
	pin = simple_strtol (argv[2], NULL, 10);
	if (pin > 31)
		rcode = 1;


	switch (argv[3][0]) {
	case 'd':
		if (argv[3][1] == 'a')
			cmd = DAT;
		else if (argv[3][1] == 'i')
			cmd = DIR;
		else
			rcode = 1;
		break;
	case 'p':
		cmd = PAR;
		break;
	case 'o':
		cmd = ODR;
		break;
	case 's':
		cmd = SOR;
		break;
#if defined(CONFIG_8xx)
	case 'i':
		cmd = INT;
		break;
#endif
	default:
		printf ("iopset: unknown command %s\n", argv[3]);
		rcode = 1;
	}
	if (argv[4][0] == '1')
		value = 1;
	else if (argv[4][0] == '0')
		value = 0;
	else
		rcode = 1;
	if (rcode == 0) {
		iopin.port = port;
		iopin.pin = pin;
		iopin.flag = 0;
		switch (cmd) {
		case DIR:
			if (value)
				iopin_set_out (&iopin);
			else
				iopin_set_in (&iopin);
			break;
		case PAR:
			if (value)
				iopin_set_ded (&iopin);
			else
				iopin_set_gen (&iopin);
			break;
		case SOR:
			if (value)
				iopin_set_opt2 (&iopin);
			else
				iopin_set_opt1 (&iopin);
			break;
		case ODR:
			if (value)
				iopin_set_odr (&iopin);
			else
				iopin_set_act (&iopin);
			break;
		case DAT:
			if (value)
				iopin_set_high (&iopin);
			else
				iopin_set_low (&iopin);
			break;
#if defined(CONFIG_8xx)
		case INT:
			if (value)
				iopin_set_falledge (&iopin);
			else
				iopin_set_anyedge (&iopin);
			break;
#endif
		}

	}
	return rcode;
}

int
do_dmainfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}

int
do_fccinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}

static void prbrg (int n, uint val)
{
	uint extc = (val >> 14) & 3;
	uint cd = (val & CPM_BRG_CD_MASK) >> 1;
	uint div16 = (val & CPM_BRG_DIV16) != 0;

#if defined(CONFIG_8xx)
	ulong clock = gd->cpu_clk;
#elif defined(CONFIG_MPC8260)
	ulong clock = gd->arch.brg_clk;
#endif

	printf ("BRG%d:", n);

	if (val & CPM_BRG_RST)
		puts (" RESET");
	else
		puts ("      ");

	if (val & CPM_BRG_EN)
		puts ("  ENABLED");
	else
		puts (" DISABLED");

	printf (" EXTC=%d", extc);

	if (val & CPM_BRG_ATB)
		puts (" ATB");
	else
		puts ("    ");

	printf (" DIVIDER=%4d", cd);
	if (extc == 0 && cd != 0) {
		uint baudrate;

		if (div16)
			baudrate = (clock / 16) / (cd + 1);
		else
			baudrate = clock / (cd + 1);

		printf ("=%6d bps", baudrate);
	} else {
		puts ("           ");
	}

	if (val & CPM_BRG_DIV16)
		puts (" DIV16");
	else
		puts ("      ");

	putc ('\n');
}

int
do_brginfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

#if defined(CONFIG_8xx)
	volatile cpm8xx_t *cp = &immap->im_cpm;
	volatile uint *p = &cp->cp_brgc1;
#elif defined(CONFIG_MPC8260)
	volatile uint *p = &immap->im_brgc1;
#endif
	int i = 1;

	while (i <= 4)
		prbrg (i++, *p++);

#if defined(CONFIG_MPC8260)
	p = &immap->im_brgc5;
	while (i <= 8)
		prbrg (i++, *p++);
#endif
	return 0;
}

int
do_i2cinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;

#if defined(CONFIG_8xx)
	volatile i2c8xx_t *i2c = &immap->im_i2c;
	volatile cpm8xx_t *cp = &immap->im_cpm;
	volatile iic_t *iip = (iic_t *) & cp->cp_dparam[PROFF_IIC];
#elif defined(CONFIG_MPC8260)
	volatile i2c8260_t *i2c = &immap->im_i2c;
	volatile iic_t *iip;
	uint dpaddr;

	dpaddr = immap->im_dprambase16[PROFF_I2C_BASE / sizeof(u16)];
	if (dpaddr == 0)
		iip = NULL;
	else
		iip = (iic_t *) & immap->im_dprambase[dpaddr];
#endif

	printf ("I2MOD = %02x I2ADD = %02x\n", i2c->i2c_i2mod, i2c->i2c_i2add);
	printf ("I2BRG = %02x I2COM = %02x\n", i2c->i2c_i2brg, i2c->i2c_i2com);
	printf ("I2CER = %02x I2CMR = %02x\n", i2c->i2c_i2cer, i2c->i2c_i2cmr);

	if (iip == NULL)
		puts ("i2c parameter ram not allocated\n");
	else {
		printf ("RBASE = %08x TBASE = %08x\n",
			iip->iic_rbase, iip->iic_tbase);
		printf ("RFCR  =       %02x TFCR  =       %02x\n",
			iip->iic_rfcr, iip->iic_tfcr);
		printf ("MRBLR =     %04x\n", iip->iic_mrblr);
		printf ("RSTATE= %08x RDP   = %08x\n",
			iip->iic_rstate, iip->iic_rdp);
		printf ("RBPTR =     %04x RBC   =     %04x\n",
			iip->iic_rbptr, iip->iic_rbc);
		printf ("RXTMP = %08x\n", iip->iic_rxtmp);
		printf ("TSTATE= %08x TDP   = %08x\n",
			iip->iic_tstate, iip->iic_tdp);
		printf ("TBPTR =     %04x TBC   =     %04x\n",
			iip->iic_tbptr, iip->iic_tbc);
		printf ("TXTMP = %08x\n", iip->iic_txtmp);
	}
	return 0;
}

int
do_sccinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}

int
do_smcinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}

int
do_spiinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}

int
do_muxinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}

int
do_siinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}

int
do_mccinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unimplemented (cmdtp, flag, argc, argv);
	return 0;
}

/***************************************************/

U_BOOT_CMD(
	siuinfo,	1,	1,	do_siuinfo,
	"print System Interface Unit (SIU) registers",
	""
);

U_BOOT_CMD(
	memcinfo,	1,	1,	do_memcinfo,
	"print Memory Controller registers",
	""
);

U_BOOT_CMD(
	sitinfo,	1,	1,	do_sitinfo,
	"print System Integration Timers (SIT) registers",
	""
);

#ifdef CONFIG_MPC8260
U_BOOT_CMD(
	icinfo,	1,	1,	do_icinfo,
	"print Interrupt Controller registers",
	""
);
#endif

U_BOOT_CMD(
	carinfo,	1,	1,	do_carinfo,
	"print Clocks and Reset registers",
	""
);

U_BOOT_CMD(
	iopinfo,	1,	1,	do_iopinfo,
	"print I/O Port registers",
	""
);

U_BOOT_CMD(
	iopset,	5,	0,	do_iopset,
	"set I/O Port registers",
	"PORT PIN CMD VALUE\nPORT: A-D, PIN: 0-31, CMD: [dat|dir|odr|sor], VALUE: 0|1"
);

U_BOOT_CMD(
	dmainfo,	1,	1,	do_dmainfo,
	"print SDMA/IDMA registers",
	""
);

U_BOOT_CMD(
	fccinfo,	1,	1,	do_fccinfo,
	"print FCC registers",
	""
);

U_BOOT_CMD(
	brginfo,	1,	1,	do_brginfo,
	"print Baud Rate Generator (BRG) registers",
	""
);

U_BOOT_CMD(
	i2cinfo,	1,	1,	do_i2cinfo,
	"print I2C registers",
	""
);

U_BOOT_CMD(
	sccinfo,	1,	1,	do_sccinfo,
	"print SCC registers",
	""
);

U_BOOT_CMD(
	smcinfo,	1,	1,	do_smcinfo,
	"print SMC registers",
	""
);

U_BOOT_CMD(
	spiinfo,	1,	1,	do_spiinfo,
	"print Serial Peripheral Interface (SPI) registers",
	""
);

U_BOOT_CMD(
	muxinfo,	1,	1,	do_muxinfo,
	"print CPM Multiplexing registers",
	""
);

U_BOOT_CMD(
	siinfo,	1,	1,	do_siinfo,
	"print Serial Interface (SI) registers",
	""
);

U_BOOT_CMD(
	mccinfo,	1,	1,	do_mccinfo,
	"print MCC registers",
	""
);

#endif
