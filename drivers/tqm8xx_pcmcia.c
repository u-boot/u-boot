/* -------------------------------------------------------------------- */
/* TQM8xxL Boards by TQ Components					*/
/* SC8xx   Boards by SinoVee Microsystems				*/
/* -------------------------------------------------------------------- */
#include <common.h>
#ifdef CONFIG_8xx
#include <mpc8xx.h>
#endif
#include <pcmcia.h>

#undef	CONFIG_PCMCIA

#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA) || defined(CONFIG_CMD_PCMCIA)
#define	CONFIG_PCMCIA
#endif

#if ((CONFIG_COMMANDS & CFG_CMD_IDE) || defined(CONFIG_CMD_IDE)) && defined(CONFIG_IDE_8xx_PCCARD)
#define	CONFIG_PCMCIA
#endif

#if	defined(CONFIG_PCMCIA)	\
	&& (defined(CONFIG_TQM8xxL) || defined(CONFIG_SVM_SC8xx))

#if	defined(CONFIG_VIRTLAB2)
#define	PCMCIA_BOARD_MSG	"Virtlab2"
#elif	defined(CONFIG_TQM8xxL)
#define	PCMCIA_BOARD_MSG	"TQM8xxL"
#elif	defined(CONFIG_SVM_SC8xx)
#define	PCMCIA_BOARD_MSG	"SC8xx"
#endif

#if	defined(CONFIG_NSCU)

#define	power_config(slot)	do {} while (0)
#define	power_off(slot)		do {} while (0)
#define	power_on_5_0(slot)	do {} while (0)
#define	power_on_3_3(slot)	do {} while (0)

#elif	defined(CONFIG_HMI10)

static inline void power_config(int slot)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	/*
	 * Configure Port B  pins for
	 * 5 Volts Enable and 3 Volts enable
	*/
	immap->im_cpm.cp_pbpar &= ~(0x00000300);
}

static inline void power_off(int slot)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	/* remove all power */
	immap->im_cpm.cp_pbdat |= 0x00000300;
}

static inline void power_on_5_0(int slot)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	immap->im_cpm.cp_pbdat &= ~(0x0000100);
	immap->im_cpm.cp_pbdir |= 0x00000300;
}

static inline void power_on_3_3(int slot)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	immap->im_cpm.cp_pbdat &= ~(0x0000200);
	immap->im_cpm.cp_pbdir |= 0x00000300;
}

#elif	defined(CONFIG_VIRTLAB2)

#define	power_config(slot)	do {} while (0)
static inline void power_off(int slot)
{
	volatile unsigned char	*powerctl =
			(volatile unsigned char *)PCMCIA_CTRL;
	*powerctl = 0;
}

static inline void power_on_5_0(int slot)
{
	volatile unsigned char	*powerctl =
			(volatile unsigned char *)PCMCIA_CTRL;
			*powerctl = 2;	/* Enable 5V Vccout */
}

static inline void power_on_3_3(int slot)
{
	volatile unsigned char	*powerctl =
			(volatile unsigned char *)PCMCIA_CTRL;
			*powerctl = 1;	/* Enable 3.3V Vccout */
}

#else

static inline void power_config(int slot)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	/*
	* Configure Port C pins for
	* 5 Volts Enable and 3 Volts enable
	*/
	immap->im_ioport.iop_pcpar &= ~(0x0002 | 0x0004);
	immap->im_ioport.iop_pcso  &= ~(0x0002 | 0x0004);
}

static inline void power_off(int slot)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	immap->im_ioport.iop_pcdat &= ~(0x0002 | 0x0004);
}

static inline void power_on_5_0(int slot)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	immap->im_ioport.iop_pcdat |= 0x0004;
	immap->im_ioport.iop_pcdir |= (0x0002 | 0x0004);
}

static inline void power_on_3_3(int slot)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	immap->im_ioport.iop_pcdat |= 0x0002;
	immap->im_ioport.iop_pcdir |= (0x0002 | 0x0004);
}

#endif

#ifdef	CONFIG_HMI10
static inline int check_card_is_absent(int slot)
{
	volatile pcmconf8xx_t *pcmp =
		(pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));
	return pcmp->pcmc_pipr & (0x10000000 >> (slot << 4));
}
#else
static inline int check_card_is_absent(int slot)
{
	volatile pcmconf8xx_t *pcmp =
		(pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));
	return pcmp->pcmc_pipr & (0x18000000 >> (slot << 4));
}
#endif

#ifdef	NSCU_OE_INV
#define	NSCU_GCRX_CXOE	0
#else
#define	NSCU_GCRX_CXOE	__MY_PCMCIA_GCRX_CXOE
#endif

int pcmcia_hardware_enable(int slot)
{
	volatile pcmconf8xx_t *pcmp =
		(pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));
	volatile sysconf8xx_t *sysp =
		(sysconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_siu_conf));
	uint reg, mask;

	debug ("hardware_enable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	udelay(10000);

	/*
	* Configure SIUMCR to enable PCMCIA port B
	* (VFLS[0:1] are not used for debugging, we connect FRZ# instead)
	*/
	sysp->sc_siumcr &= ~SIUMCR_DBGC11;	/* set DBGC to 00 */

	/* clear interrupt state, and disable interrupts */
	pcmp->pcmc_pscr =  PCMCIA_MASK(slot);
	pcmp->pcmc_per &= ~PCMCIA_MASK(slot);

	/*
	* Disable interrupts, DMA, and PCMCIA buffers
	* (isolate the interface) and assert RESET signal
	*/
	debug ("Disable PCMCIA buffers and assert RESET\n");
	reg  = 0;
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= NSCU_GCRX_CXOE;

	PCMCIA_PGCRX(slot) = reg;
	udelay(500);

	power_config(slot);
	power_off(slot);

	/*
	 * Make sure there is a card in the slot, then configure the interface.
	*/
	udelay(10000);
	debug ("[%d] %s: PIPR(%p)=0x%x\n", __LINE__,__FUNCTION__,
	       &(pcmp->pcmc_pipr),pcmp->pcmc_pipr);

	if (check_card_is_absent(slot)) {
		printf ("   No Card found\n");
		return (1);
	}

	/*
	* Power On.
	*/
	mask = PCMCIA_VS1(slot) | PCMCIA_VS2(slot);
	reg  = pcmp->pcmc_pipr;
	debug ("PIPR: 0x%x ==> VS1=o%s, VS2=o%s\n",
	       reg,
	       (reg&PCMCIA_VS1(slot))?"n":"ff",
	       (reg&PCMCIA_VS2(slot))?"n":"ff");

	if ((reg & mask) == mask) {
		power_on_5_0(slot);
		puts (" 5.0V card found: ");
	} else {
		power_on_3_3(slot);
		puts (" 3.3V card found: ");
	}

#if 0
	/*  VCC switch error flag, PCMCIA slot INPACK_ pin */
	cp->cp_pbdir &= ~(0x0020 | 0x0010);
	cp->cp_pbpar &= ~(0x0020 | 0x0010);
	udelay(500000);
#endif

	udelay(1000);
	debug ("Enable PCMCIA buffers and stop RESET\n");
	reg  =  PCMCIA_PGCRX(slot);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	reg &= ~NSCU_GCRX_CXOE;

	PCMCIA_PGCRX(slot) = reg;

	udelay(250000);	/* some cards need >150 ms to come up :-( */

	debug ("# hardware_enable done\n");

	return (0);
}


#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA) || defined(CONFIG_CMD_PCMCIA)
int pcmcia_hardware_disable(int slot)
{
	u_long reg;

	debug ("hardware_disable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);


	/* remove all power */
	power_off(slot);

	debug ("Disable PCMCIA buffers and assert RESET\n");
	reg  = 0;
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= NSCU_GCRX_CXOE;			/* active low  */

	PCMCIA_PGCRX(slot) = reg;

	udelay(10000);

	return (0);
}
#endif	/* CFG_CMD_PCMCIA */

int pcmcia_voltage_set(int slot, int vcc, int vpp)
{
#ifndef CONFIG_NSCU
	u_long reg;
# ifdef DEBUG
	volatile pcmconf8xx_t *pcmp =
		(pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));
# endif

	debug ("voltage_set: " PCMCIA_BOARD_MSG
		" Slot %c, Vcc=%d.%d, Vpp=%d.%d\n",
		'A'+slot, vcc/10, vcc%10, vpp/10, vcc%10);

	/*
	* Disable PCMCIA buffers (isolate the interface)
	* and assert RESET signal
	*/
	debug ("Disable PCMCIA buffers and assert RESET\n");
	reg  = PCMCIA_PGCRX(slot);
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg &= ~__MY_PCMCIA_GCRX_CXOE;		/* active low  */
	reg |= NSCU_GCRX_CXOE;			/* active low  */

	PCMCIA_PGCRX(slot) = reg;
	udelay(500);

	debug ("PCMCIA power OFF\n");
	power_config(slot);
	power_off(slot);

	switch(vcc) {
		case  0: 			break;
		case 33: power_on_3_3(slot);	break;
		case 50: power_on_5_0(slot);	break;
		default: 			goto done;
	}

	/* Checking supported voltages */

	debug("PIPR: 0x%x --> %s\n", pcmp->pcmc_pipr,
	       (pcmp->pcmc_pipr & 0x00008000) ? "only 5 V" : "can do 3.3V");

	if (vcc)
		debug("PCMCIA powered at %sV\n", (vcc == 50) ? "5.0" : "3.3");
	else
		debug("PCMCIA powered down\n");

done:
	debug("Enable PCMCIA buffers and stop RESET\n");
	reg  =  PCMCIA_PGCRX(slot);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	reg &= ~NSCU_GCRX_CXOE;			/* active low  */

	PCMCIA_PGCRX(slot) = reg;
	udelay(500);

	debug("voltage_set: " PCMCIA_BOARD_MSG " Slot %c, DONE\n", slot+'A');
#endif	/* CONFIG_NSCU */
	return (0);
}

#endif	/* CONFIG_PCMCIA && (CONFIG_TQM8xxL || CONFIG_SVM_SC8xx) */
