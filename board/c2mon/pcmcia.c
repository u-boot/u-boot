#include <common.h>
#include <mpc8xx.h>
#include <pcmcia.h>

#undef	CONFIG_PCMCIA

#if	defined(CONFIG_CMD_PCMCIA)
#define	CONFIG_PCMCIA
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_8xx_PCCARD)
#define	CONFIG_PCMCIA
#endif

#ifdef	CONFIG_PCMCIA

#define PCMCIA_BOARD_MSG "C2MON"

static void cfg_ports (void)
{
	volatile immap_t	*immap;
	volatile cpm8xx_t	*cp;
	ushort sreg;

	immap = (immap_t *)CFG_IMMR;
	cp    = (cpm8xx_t *)(&(((immap_t *)CFG_IMMR)->im_cpm));

	/*
	* Configure Port C for TPS2211 PC-Card Power-Interface Switch
	*
	* Switch off all voltages, assert shutdown
	*/
	sreg = immap->im_ioport.iop_pcdat;
	sreg |=  (TPS2211_VPPD0 | TPS2211_VPPD1);	/* VAVPP => Hi-Z */
	sreg &= ~(TPS2211_VCCD0 | TPS2211_VCCD1);	/* 3V and 5V off */
	immap->im_ioport.iop_pcdat = sreg;

	immap->im_ioport.iop_pcpar &= ~(TPS2211_OUTPUTS);
	immap->im_ioport.iop_pcdir |=   TPS2211_OUTPUTS;

	debug ("Set Port C: PAR:     %04x DIR:     %04x DAT:     %04x\n",
	       immap->im_ioport.iop_pcpar,
	       immap->im_ioport.iop_pcdir,
	       immap->im_ioport.iop_pcdat);

	/*
	* Configure Port B for TPS2211 PC-Card Power-Interface Switch
	*
	* Over-Current Input only
	*/
	cp->cp_pbpar &= ~(TPS2211_INPUTS);
	cp->cp_pbdir &= ~(TPS2211_INPUTS);

	debug ("Set Port B: PAR: %08x DIR: %08x DAT: %08x\n",
	       cp->cp_pbpar, cp->cp_pbdir, cp->cp_pbdat);
}

int pcmcia_hardware_enable(int slot)
{
	volatile immap_t	*immap;
	volatile cpm8xx_t	*cp;
	volatile pcmconf8xx_t	*pcmp;
	volatile sysconf8xx_t	*sysp;
	uint reg, pipr, mask;
	ushort sreg;
	int i;

	debug ("hardware_enable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	udelay(10000);

	immap = (immap_t *)CFG_IMMR;
	sysp  = (sysconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_siu_conf));
	pcmp  = (pcmconf8xx_t *)(&(((immap_t *)CFG_IMMR)->im_pcmcia));
	cp    = (cpm8xx_t *)(&(((immap_t *)CFG_IMMR)->im_cpm));

	/* Configure Ports for TPS2211A PC-Card Power-Interface Switch */
	cfg_ports ();

	/*
	* Configure SIUMCR to enable PCMCIA port B
	* (VFLS[0:1] are not used for debugging, we connect FRZ# instead)
	*/
	sysp->sc_siumcr &= ~SIUMCR_DBGC11;	/* set DBGC to 00 */

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

	sreg = immap->im_ioport.iop_pcdat;
	if ((pipr & mask) == mask) {
		sreg |=  (TPS2211_VPPD0 | TPS2211_VPPD1 |	/* VAVPP => Hi-Z */
				TPS2211_VCCD1);			/* 5V on	*/
		sreg &= ~(TPS2211_VCCD0);			/* 3V off	*/
		puts (" 5.0V card found: ");
	} else {
		sreg |=  (TPS2211_VPPD0 | TPS2211_VPPD1 |	/* VAVPP => Hi-Z */
				TPS2211_VCCD0);			/* 3V on	*/
		sreg &= ~(TPS2211_VCCD1);			/* 5V off	*/
		puts (" 3.3V card found: ");
	}

	debug ("\nPC DAT: %04x -> 3.3V %s 5.0V %s\n",
	       sreg,
	       ( (sreg & TPS2211_VCCD0) && !(sreg & TPS2211_VCCD1)) ? "on" : "off",
	       (!(sreg & TPS2211_VCCD0) &&  (sreg & TPS2211_VCCD1)) ? "on" : "off"
	      );

	immap->im_ioport.iop_pcdat = sreg;

	/*  Wait 500 ms; use this to check for over-current */
	for (i=0; i<5000; ++i) {
		if ((cp->cp_pbdat & TPS2211_OC) == 0) {
			printf ("   *** Overcurrent - Safety shutdown ***\n");
			immap->im_ioport.iop_pcdat &= ~(TPS2211_VCCD0|TPS2211_VCCD1);
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


#if defined(CONFIG_CMD_PCMCIA)
int pcmcia_hardware_disable(int slot)
{
	volatile immap_t	*immap;
	volatile cpm8xx_t	*cp;
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

	/* ALl voltages off / Hi-Z */
	immap->im_ioport.iop_pcdat |= (TPS2211_VPPD0 | TPS2211_VPPD1 |
			TPS2211_VCCD0 | TPS2211_VCCD1 );

	udelay(10000);

	return (0);
}
#endif


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
	sreg |= TPS2211_VPPD0 | TPS2211_VPPD1;		/* VAVPP always Hi-Z */

	switch(vcc) {
		case  0: 			break;	/* Switch off		*/
		case 33: sreg |=  TPS2211_VCCD0;	/* Switch on 3.3V	*/
		sreg &= ~TPS2211_VCCD1;
		break;
		case 50: sreg &= ~TPS2211_VCCD0;	/* Switch on 5.0V	*/
		sreg |=  TPS2211_VCCD1;
		break;
		default: 			goto done;
	}

	/* Checking supported voltages */

	debug ("PIPR: 0x%x --> %s\n",
	       pcmp->pcmc_pipr,
	       (pcmp->pcmc_pipr & 0x00008000) ? "only 5 V" : "can do 3.3V");

	immap->im_ioport.iop_pcdat = sreg;

#ifdef DEBUG
{
	char *s;

	if ((sreg & TPS2211_VCCD0) && !(sreg & TPS2211_VCCD1)) {
		s = "at 3.3V";
	} else if (!(sreg & TPS2211_VCCD0) &&  (sreg & TPS2211_VCCD1)) {
		s = "at 5.0V";
	} else {
		s = "down";
	}
	printf ("PCMCIA powered %s\n", s);
}
#endif

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
