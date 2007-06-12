#include <common.h>
#include <mpc8xx.h>
#include <pcmcia.h>

#undef	CONFIG_PCMCIA

#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA) || defined(CONFIG_CMD_PCMCIA)
#define	CONFIG_PCMCIA
#endif

#if ((CONFIG_COMMANDS & CFG_CMD_IDE) || defined(CONFIG_CMD_IDE)) && defined(CONFIG_IDE_8xx_PCCARD)
#define	CONFIG_PCMCIA
#endif

#ifdef	CONFIG_PCMCIA

/* some sane bit macros */
#define _BD(_b)				(1U << (31-(_b)))
#define _BDR(_l, _h)			(((((1U << (31-(_l))) - 1) << 1) | 1) & ~((1U << (31-(_h))) - 1))

#define _BW(_b)				(1U << (15-(_b)))
#define _BWR(_l, _h)			(((((1U << (15-(_l))) - 1) << 1) | 1) & ~((1U << (15-(_h))) - 1))

#define _BB(_b)				(1U << (7-(_b)))
#define _BBR(_l, _h)			(((((1U << (7-(_l))) - 1) << 1) | 1) & ~((1U << (7-(_h))) - 1))

#define _B(_b)				_BD(_b)
#define _BR(_l, _h)			_BDR(_l, _h)

#define PCMCIA_BOARD_MSG "NETTA"

static const unsigned short vppd_masks[2] = { _BW(14), _BW(15) };

static void cfg_vppd(int no)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	unsigned short mask;

	if ((unsigned int)no >= sizeof(vppd_masks)/sizeof(vppd_masks[0]))
		return;

	mask = vppd_masks[no];

	immap->im_ioport.iop_papar &= ~mask;
	immap->im_ioport.iop_paodr &= ~mask;
	immap->im_ioport.iop_padir |=  mask;
}

static void set_vppd(int no, int what)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	unsigned short mask;

	if ((unsigned int)no >= sizeof(vppd_masks)/sizeof(vppd_masks[0]))
		return;

	mask = vppd_masks[no];

	if (what)
		immap->im_ioport.iop_padat |= mask;
	else
		immap->im_ioport.iop_padat &= ~mask;
}

static const unsigned short vccd_masks[2] = { _BW(10), _BW(6) };

static void cfg_vccd(int no)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	unsigned short mask;

	if ((unsigned int)no >= sizeof(vccd_masks)/sizeof(vccd_masks[0]))
		return;

	mask = vccd_masks[no];

	immap->im_ioport.iop_papar &= ~mask;
	immap->im_ioport.iop_paodr &= ~mask;
	immap->im_ioport.iop_padir |=  mask;
}

static void set_vccd(int no, int what)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	unsigned short mask;

	if ((unsigned int)no >= sizeof(vccd_masks)/sizeof(vccd_masks[0]))
		return;

	mask = vccd_masks[no];

	if (what)
		immap->im_ioport.iop_padat |= mask;
	else
		immap->im_ioport.iop_padat &= ~mask;
}

static const unsigned short oc_mask = _BW(8);

static void cfg_oc(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	unsigned short mask = oc_mask;

	immap->im_ioport.iop_pcdir &= ~mask;
	immap->im_ioport.iop_pcso  &= ~mask;
	immap->im_ioport.iop_pcint &= ~mask;
	immap->im_ioport.iop_pcpar &= ~mask;
}

static int get_oc(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	unsigned short mask = oc_mask;
	int what;

	what = !!(immap->im_ioport.iop_pcdat & mask);;
	return what;
}

static const unsigned short shdn_mask = _BW(12);

static void cfg_shdn(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	unsigned short mask;

	mask = shdn_mask;

	immap->im_ioport.iop_papar &= ~mask;
	immap->im_ioport.iop_paodr &= ~mask;
	immap->im_ioport.iop_padir |=  mask;
}

static void set_shdn(int what)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	unsigned short mask;

	mask = shdn_mask;

	if (what)
		immap->im_ioport.iop_padat |= mask;
	else
		immap->im_ioport.iop_padat &= ~mask;
}

static void cfg_ports (void)
{
	volatile immap_t	*immap;
	volatile cpm8xx_t	*cp;

	immap = (immap_t *)CFG_IMMR;
	cp    = (cpm8xx_t *)(&(((immap_t *)CFG_IMMR)->im_cpm));


	cfg_vppd(0); cfg_vppd(1);	/* VPPD0,VPPD1 VAVPP => Hi-Z */
	cfg_vccd(0); cfg_vccd(1);	/* 3V and 5V off */
	cfg_shdn();
	cfg_oc();

	/*
	* Configure Port A for TPS2211 PC-Card Power-Interface Switch
	*
	* Switch off all voltages, assert shutdown
	*/
	set_vppd(0, 1); set_vppd(1, 1);
	set_vccd(0, 0); set_vccd(1, 0);
	set_shdn(1);

	udelay(100000);
}

int pcmcia_hardware_enable(int slot)
{
	volatile immap_t	*immap;
	volatile cpm8xx_t	*cp;
	volatile pcmconf8xx_t	*pcmp;
	volatile sysconf8xx_t	*sysp;
	uint reg, pipr, mask;
	int i;

	debug ("hardware_enable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	udelay(10000);

	immap = (immap_t *)CFG_IMMR;
	sysp  = (sysconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_siu_conf));
	pcmp  = (pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));
	cp    = (cpm8xx_t *)(&(((immap_t *)CFG_IMMR)->im_cpm));

	/* Configure Ports for TPS2211A PC-Card Power-Interface Switch */
	cfg_ports ();

	/* clear interrupt state, and disable interrupts */
	pcmp->pcmc_pscr =  PCMCIA_MASK(_slot_);
	pcmp->pcmc_per &= ~PCMCIA_MASK(_slot_);

	/*
	* Disable interrupts, DMA, and PCMCIA buffers
	* (isolate the interface) and assert RESET signal
	*/
	debug ("Disable PCMCIA buffers and assert RESET\n");
	reg  = 0;
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;

	udelay(500);

	/*
	* Make sure there is a card in the slot, then configure the interface.
	*/
	udelay(10000);
	debug ("[%d] %s: PIPR(%p)=0x%x\n",
	       __LINE__,__FUNCTION__,
	       &(pcmp->pcmc_pipr),pcmp->pcmc_pipr);
	if (pcmp->pcmc_pipr & (0x18000000 >> (slot << 4))) {
		printf ("   No Card found\n");
		return (1);
	}

	/*
	* Power On: Set VAVCC to 3.3V or 5V, set VAVPP to Hi-Z
	*/
	mask = PCMCIA_VS1(slot) | PCMCIA_VS2(slot);
	pipr = pcmp->pcmc_pipr;
	debug ("PIPR: 0x%x ==> VS1=o%s, VS2=o%s\n",
	       pipr,
	       (reg&PCMCIA_VS1(slot))?"n":"ff",
	       (reg&PCMCIA_VS2(slot))?"n":"ff");

	if ((pipr & mask) == mask) {
		set_vppd(0, 1); set_vppd(1, 1); 		/* VAVPP => Hi-Z */
		set_vccd(0, 0); set_vccd(1, 1); 		/* 5V on, 3V off */
		puts (" 5.0V card found: ");
	} else {
		set_vppd(0, 1); set_vppd(1, 1); 		/* VAVPP => Hi-Z */
		set_vccd(0, 1); set_vccd(1, 0); 		/* 5V off, 3V on */
		puts (" 3.3V card found: ");
	}

	/*  Wait 500 ms; use this to check for over-current */
	for (i=0; i<5000; ++i) {
		if (!get_oc()) {
			printf ("   *** Overcurrent - Safety shutdown ***\n");
			set_vccd(0, 0); set_vccd(1, 0); 		/* VAVPP => Hi-Z */
			return (1);
		}
		udelay (100);
	}

	debug ("Enable PCMCIA buffers and stop RESET\n");
	reg  =  PCMCIA_PGCRX(_slot_);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg &= ~__MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;

	udelay(250000);	/* some cards need >150 ms to come up :-( */

	debug ("# hardware_enable done\n");

	return (0);
}


#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA) || defined(CONFIG_CMD_PCMCIA)
int pcmcia_hardware_disable(int slot)
{
	volatile immap_t	*immap;
	volatile pcmconf8xx_t	*pcmp;
	u_long reg;

	debug ("hardware_disable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	immap = (immap_t *)CFG_IMMR;
	pcmp  = (pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));

	/* Configure PCMCIA General Control Register */
	debug ("Disable PCMCIA buffers and assert RESET\n");
	reg  = 0;
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;

	/* All voltages off / Hi-Z */
	set_vppd(0, 1); set_vppd(1, 1);
	set_vccd(0, 1); set_vccd(1, 1);

	udelay(10000);

	return (0);
}
#endif	/* CFG_CMD_PCMCIA */


int pcmcia_voltage_set(int slot, int vcc, int vpp)
{
	volatile immap_t	*immap;
	volatile cpm8xx_t	*cp;
	volatile pcmconf8xx_t	*pcmp;
	u_long reg;
	ushort sreg;

	debug ("voltage_set: "
			PCMCIA_BOARD_MSG
			" Slot %c, Vcc=%d.%d, Vpp=%d.%d\n",
	'A'+slot, vcc/10, vcc%10, vpp/10, vcc%10);

	immap = (immap_t *)CFG_IMMR;
	cp    = (cpm8xx_t *)(&(((immap_t *)CFG_IMMR)->im_cpm));
	pcmp  = (pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));
	/*
	* Disable PCMCIA buffers (isolate the interface)
	* and assert RESET signal
	*/
	debug ("Disable PCMCIA buffers and assert RESET\n");
	reg  = PCMCIA_PGCRX(_slot_);
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;
	udelay(500);

	/*
	* Configure Port C pins for
	* 5 Volts Enable and 3 Volts enable,
	* Turn all power pins to Hi-Z
	*/
	debug ("PCMCIA power OFF\n");
	cfg_ports ();	/* Enables switch, but all in Hi-Z */

	sreg  = immap->im_ioport.iop_pcdat;
	set_vppd(0, 1); set_vppd(1, 1);

	switch(vcc) {
		case  0:
			break;	/* Switch off		*/

		case 33:
			set_vccd(0, 1); set_vccd(1, 0);
			break;

		case 50:
			set_vccd(0, 0); set_vccd(1, 1);
			break;

		default:
			goto done;
	}

	/* Checking supported voltages */

	debug ("PIPR: 0x%x --> %s\n",
	       pcmp->pcmc_pipr,
	       (pcmp->pcmc_pipr & 0x00008000) ? "only 5 V" : "can do 3.3V");

done:
			debug ("Enable PCMCIA buffers and stop RESET\n");
	reg  =  PCMCIA_PGCRX(_slot_);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg &= ~__MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;
	udelay(500);

	debug ("voltage_set: " PCMCIA_BOARD_MSG " Slot %c, DONE\n",
	       slot+'A');
	return (0);
}

#endif	/* CONFIG_PCMCIA */
