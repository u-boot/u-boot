/* -------------------------------------------------------------------- */
/* TQM8xxL Boards by TQ Components					*/
/* SC8xx   Boards by SinoVee Microsystems				*/
/* -------------------------------------------------------------------- */
#include <common.h>
#include <asm/io.h>
#ifdef CONFIG_8xx
#include <mpc8xx.h>
#endif
#include <pcmcia.h>

#undef	CONFIG_PCMCIA

#if defined(CONFIG_CMD_PCMCIA)
#define	CONFIG_PCMCIA
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_8xx_PCCARD)
#define	CONFIG_PCMCIA
#endif

#if	defined(CONFIG_PCMCIA)	\
	&& defined(CONFIG_TQM8xxL)

#if	defined(CONFIG_TQM8xxL)
#define	PCMCIA_BOARD_MSG	"TQM8xxL"
#endif

static inline void power_config(int slot)
{
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	/*
	 * Configure Port C pins for
	 * 5 Volts Enable and 3 Volts enable
	 */
	clrbits_be16(&immap->im_ioport.iop_pcpar, 0x0002 | 0x0004);
	clrbits_be16(&immap->im_ioport.iop_pcso, 0x0002 | 0x0004);
}

static inline void power_off(int slot)
{
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	clrbits_be16(&immap->im_ioport.iop_pcdat, 0x0002 | 0x0004);
}

static inline void power_on_5_0(int slot)
{
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	setbits_be16(&immap->im_ioport.iop_pcdat, 0x0004);
	setbits_be16(&immap->im_ioport.iop_pcdir, 0x0002 | 0x0004);
}

static inline void power_on_3_3(int slot)
{
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	setbits_be16(&immap->im_ioport.iop_pcdat, 0x0002);
	setbits_be16(&immap->im_ioport.iop_pcdir, 0x0002 | 0x0004);
}

/*
 * Function to retrieve the PIPR register, used for debuging purposes.
 */
static inline uint32_t debug_get_pipr(void)
{
	uint32_t pipr = 0;
#ifdef	DEBUG
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	pipr = in_be32(&immap->im_pcmcia.pcmc_pipr);
#endif
	return pipr;
}


static inline int check_card_is_absent(int slot)
{
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	uint32_t pipr = in_be32(&immap->im_pcmcia.pcmc_pipr);
	return pipr & (0x18000000 >> (slot << 4));
}

#define	NSCU_GCRX_CXOE	__MY_PCMCIA_GCRX_CXOE

int pcmcia_hardware_enable(int slot)
{
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	uint reg, mask;

	debug("hardware_enable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	udelay(10000);

	/*
	 * Configure SIUMCR to enable PCMCIA port B
	 * (VFLS[0:1] are not used for debugging, we connect FRZ# instead)
	 */

	/* Set DBGC to 00 */
	clrbits_be32(&immap->im_siu_conf.sc_siumcr, SIUMCR_DBGC11);

	/* Clear interrupt state, and disable interrupts */
	out_be32(&immap->im_pcmcia.pcmc_pscr, PCMCIA_MASK(slot));
	clrbits_be32(&immap->im_pcmcia.pcmc_per, PCMCIA_MASK(slot));

	/*
	 * Disable interrupts, DMA, and PCMCIA buffers
	 * (isolate the interface) and assert RESET signal
	 */
	debug("Disable PCMCIA buffers and assert RESET\n");
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
	reg = debug_get_pipr();
	debug("[%d] %s: PIPR(%p)=0x%x\n", __LINE__, __FUNCTION__,
		&immap->im_pcmcia.pcmc_pipr, reg);

	if (check_card_is_absent(slot)) {
		printf ("   No Card found\n");
		return (1);
	}

	/*
	 * Power On.
	 */
	mask = PCMCIA_VS1(slot) | PCMCIA_VS2(slot);
	reg = in_be32(&immap->im_pcmcia.pcmc_pipr);
	debug ("PIPR: 0x%x ==> VS1=o%s, VS2=o%s\n",
	       reg,
	       (reg & PCMCIA_VS1(slot)) ? "n" : "ff",
	       (reg & PCMCIA_VS2(slot)) ? "n" : "ff");

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
	debug("Enable PCMCIA buffers and stop RESET\n");
	reg  =  PCMCIA_PGCRX(slot);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	reg &= ~NSCU_GCRX_CXOE;

	PCMCIA_PGCRX(slot) = reg;

	udelay(250000);	/* some cards need >150 ms to come up :-( */

	debug("# hardware_enable done\n");

	return (0);
}


#if defined(CONFIG_CMD_PCMCIA)
int pcmcia_hardware_disable(int slot)
{
	u_long reg;

	debug("hardware_disable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	/* remove all power */
	power_off(slot);

	debug("Disable PCMCIA buffers and assert RESET\n");
	reg  = 0;
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= NSCU_GCRX_CXOE;			/* active low  */

	PCMCIA_PGCRX(slot) = reg;

	udelay(10000);

	return (0);
}
#endif

int pcmcia_voltage_set(int slot, int vcc, int vpp)
{
	u_long reg;
	uint32_t pipr = 0;

	debug("voltage_set: " PCMCIA_BOARD_MSG
		" Slot %c, Vcc=%d.%d, Vpp=%d.%d\n",
		'A'+slot, vcc/10, vcc%10, vpp/10, vcc%10);

	/*
	 * Disable PCMCIA buffers (isolate the interface)
	 * and assert RESET signal
	 */
	debug("Disable PCMCIA buffers and assert RESET\n");
	reg  = PCMCIA_PGCRX(slot);
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg &= ~__MY_PCMCIA_GCRX_CXOE;		/* active low  */
	reg |= NSCU_GCRX_CXOE;			/* active low  */

	PCMCIA_PGCRX(slot) = reg;
	udelay(500);

	debug("PCMCIA power OFF\n");
	power_config(slot);
	power_off(slot);

	switch(vcc) {
		case  0:			break;
		case 33: power_on_3_3(slot);	break;
		case 50: power_on_5_0(slot);	break;
		default:			goto done;
	}

	/* Checking supported voltages */
	pipr = debug_get_pipr();
	debug("PIPR: 0x%x --> %s\n", pipr,
	       (pipr & 0x00008000) ? "only 5 V" : "can do 3.3V");

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
	return 0;
}

#endif	/* CONFIG_PCMCIA && CONFIG_TQM8xxL */
