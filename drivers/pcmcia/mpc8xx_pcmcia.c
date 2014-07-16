#include <common.h>
#include <mpc8xx.h>
#include <pcmcia.h>
#include <linux/compiler.h>

#undef	CONFIG_PCMCIA

#if defined(CONFIG_CMD_PCMCIA)
#define	CONFIG_PCMCIA
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_8xx_PCCARD)
#define	CONFIG_PCMCIA
#endif

#if defined(CONFIG_PCMCIA)

#if	defined(CONFIG_IDE_8xx_PCCARD)
extern int check_ide_device (int slot);
#endif

extern int pcmcia_hardware_enable (int slot);
extern int pcmcia_voltage_set(int slot, int vcc, int vpp);

#if defined(CONFIG_CMD_PCMCIA)
extern int pcmcia_hardware_disable(int slot);
#endif

static u_int m8xx_get_graycode(u_int size);
#if 0 /* Disabled */
static u_int m8xx_get_speed(u_int ns, u_int is_io);
#endif

/* look up table for pgcrx registers */
u_int *pcmcia_pgcrx[2] = {
	&((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia.pcmc_pgcra,
	&((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia.pcmc_pgcrb,
};

/*
 * Search this table to see if the windowsize is
 * supported...
 */

#define M8XX_SIZES_NO 32

static const u_int m8xx_size_to_gray[M8XX_SIZES_NO] =
{ 0x00000001, 0x00000002, 0x00000008, 0x00000004,
  0x00000080, 0x00000040, 0x00000010, 0x00000020,
  0x00008000, 0x00004000, 0x00001000, 0x00002000,
  0x00000100, 0x00000200, 0x00000800, 0x00000400,

  0x0fffffff, 0xffffffff, 0xffffffff, 0xffffffff,
  0x01000000, 0x02000000, 0xffffffff, 0x04000000,
  0x00010000, 0x00020000, 0x00080000, 0x00040000,
  0x00800000, 0x00400000, 0x00100000, 0x00200000 };


/* -------------------------------------------------------------------- */

#if	defined(CONFIG_LWMON) || defined(CONFIG_NSCU)
#define	CONFIG_SYS_PCMCIA_TIMING	(	PCMCIA_SHT(9)	\
				|	PCMCIA_SST(3)	\
				|	PCMCIA_SL(12))
#else
#define	CONFIG_SYS_PCMCIA_TIMING	(	PCMCIA_SHT(2)	\
				|	PCMCIA_SST(4)	\
				|	PCMCIA_SL(9))
#endif

/* -------------------------------------------------------------------- */

int pcmcia_on (void)
{
	u_long reg, base;
	pcmcia_win_t *win;
	u_int rc, slot;
	__maybe_unused u_int slotbit;
	int i;

	debug ("Enable PCMCIA " PCMCIA_SLOT_MSG "\n");

	/* intialize the fixed memory windows */
	win = (pcmcia_win_t *)(&((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia.pcmc_pbr0);
	base = CONFIG_SYS_PCMCIA_MEM_ADDR;

	if((reg = m8xx_get_graycode(CONFIG_SYS_PCMCIA_MEM_SIZE)) == -1) {
		printf ("Cannot set window size to 0x%08x\n",
			CONFIG_SYS_PCMCIA_MEM_SIZE);
		return (1);
	}

	slotbit = PCMCIA_SLOT_x;
	for (i=0; i<PCMCIA_MEM_WIN_NO; ++i) {
		win->br = base;

#if	(PCMCIA_SOCKETS_NO == 2)
		if (i == 4) /* Another slot starting from win 4 */
			slotbit = (slotbit ? PCMCIA_PSLOT_A : PCMCIA_PSLOT_B);
#endif
		switch (i) {
#ifdef	CONFIG_IDE_8xx_PCCARD
		case 4:
		case 0:	{	/* map attribute memory */
			win->or = (	PCMCIA_BSIZE_64M
				|	PCMCIA_PPS_8
				|	PCMCIA_PRS_ATTR
				|	slotbit
				|	PCMCIA_PV
				|	CONFIG_SYS_PCMCIA_TIMING );
			break;
		}
		case 5:
		case 1: {	/* map I/O window for data reg */
			win->or = (	PCMCIA_BSIZE_1K
				|	PCMCIA_PPS_16
				|	PCMCIA_PRS_IO
				|	slotbit
				|	PCMCIA_PV
				|	CONFIG_SYS_PCMCIA_TIMING );
			break;
		}
		case 6:
		case 2: {	/* map I/O window for cmd/ctrl reg block */
			win->or = (	PCMCIA_BSIZE_1K
				|	PCMCIA_PPS_8
				|	PCMCIA_PRS_IO
				|	slotbit
				|	PCMCIA_PV
				|	CONFIG_SYS_PCMCIA_TIMING );
			break;
		}
#endif	/* CONFIG_IDE_8xx_PCCARD */
		default:	/* set to not valid */
			win->or = 0;
			break;
		}

		debug ("MemWin %d: PBR 0x%08lX  POR %08lX\n",
		       i, win->br, win->or);
		base += CONFIG_SYS_PCMCIA_MEM_SIZE;
		++win;
	}

	for (i=0, rc=0, slot=_slot_; i<PCMCIA_SOCKETS_NO; i++, slot = !slot) {
		/* turn off voltage */
		if ((rc = pcmcia_voltage_set(slot, 0, 0)))
			continue;

		/* Enable external hardware */
		if ((rc = pcmcia_hardware_enable(slot)))
			continue;

#ifdef	CONFIG_IDE_8xx_PCCARD
		if ((rc = check_ide_device(i)))
			continue;
#endif
	}
	return rc;
}

#if defined(CONFIG_CMD_PCMCIA)
int pcmcia_off (void)
{
	int i;
	pcmcia_win_t *win;

	printf ("Disable PCMCIA " PCMCIA_SLOT_MSG "\n");

	/* clear interrupt state, and disable interrupts */
	((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia.pcmc_pscr =  PCMCIA_MASK(_slot_);
	((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia.pcmc_per &= ~PCMCIA_MASK(_slot_);

	/* turn off interrupt and disable CxOE */
	PCMCIA_PGCRX(_slot_) = __MY_PCMCIA_GCRX_CXOE;

	/* turn off memory windows */
	win = (pcmcia_win_t *)(&((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia.pcmc_pbr0);

	for (i=0; i<PCMCIA_MEM_WIN_NO; ++i) {
		/* disable memory window */
		win->or = 0;
		++win;
	}

	/* turn off voltage */
	pcmcia_voltage_set(_slot_, 0, 0);

	/* disable external hardware */
	printf ("Shutdown and Poweroff " PCMCIA_SLOT_MSG "\n");
	pcmcia_hardware_disable(_slot_);
	return 0;
}
#endif


static u_int m8xx_get_graycode(u_int size)
{
	u_int k;

	for (k = 0; k < M8XX_SIZES_NO; k++) {
		if(m8xx_size_to_gray[k] == size)
			break;
	}

	if((k == M8XX_SIZES_NO) || (m8xx_size_to_gray[k] == -1))
		k = -1;

	return k;
}

#if	0

static u_int m8xx_get_speed(u_int ns, u_int is_io)
{
	u_int reg, clocks, psst, psl, psht;

	if(!ns) {

		/*
		* We get called with IO maps setup to 0ns
		* if not specified by the user.
		* They should be 255ns.
		*/

		if(is_io)
			ns = 255;
		else
			ns = 100;  /* fast memory if 0 */
	}

	/*
	* In PSST, PSL, PSHT fields we tell the controller
	* timing parameters in CLKOUT clock cycles.
	* CLKOUT is the same as GCLK2_50.
	*/

	/* how we want to adjust the timing - in percent */

#define ADJ 180 /* 80 % longer accesstime - to be sure */

	clocks = ((M8XX_BUSFREQ / 1000) * ns) / 1000;
	clocks = (clocks * ADJ) / (100*1000);

	if(clocks >= PCMCIA_BMT_LIMIT) {
		DEBUG(0, "Max access time limit reached\n");
		clocks = PCMCIA_BMT_LIMIT-1;
	}

	psst = clocks / 7;          /* setup time */
	psht = clocks / 7;          /* hold time */
	psl  = (clocks * 5) / 7;    /* strobe length */

	psst += clocks - (psst + psht + psl);

	reg =  psst << 12;
	reg |= psl  << 7;
	reg |= psht << 16;

	return reg;
}
#endif	/* 0 */

#endif	/* CONFIG_PCMCIA */
