#include <common.h>
#include <mpc8xx.h>
#include <pcmcia.h>

#undef	CONFIG_PCMCIA

#if defined(CONFIG_CMD_PCMCIA)
#define	CONFIG_PCMCIA
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_8xx_PCCARD)
#define	CONFIG_PCMCIA
#endif

#ifdef	CONFIG_PCMCIA

#define PCMCIA_BOARD_MSG "R360MPI"

int pcmcia_hardware_enable(int slot)
{
	volatile immap_t	*immap;
	volatile pcmconf8xx_t	*pcmp;
	volatile sysconf8xx_t	*sysp;
	uint reg, mask;

	debug ("hardware_enable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	udelay(10000);

	immap = (immap_t *)CONFIG_SYS_IMMR;
	sysp  = (sysconf8xx_t *)(&(((immap_t *)CONFIG_SYS_IMMR)->im_siu_conf));
	pcmp  = (pcmconf8xx_t *)(&(((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia));

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
	* Configure Ports A, B & C pins for
	* 5 Volts Enable and 3 Volts enable
	*/
	immap->im_ioport.iop_pcpar &= ~(0x0400);
	immap->im_ioport.iop_pcso  &= ~(0x0400);/*
	immap->im_ioport.iop_pcdir |= 0x0400;*/

	immap->im_ioport.iop_papar &= ~(0x0200);/*
	immap->im_ioport.iop_padir |= 0x0200;*/
#if 0
	immap->im_ioport.iop_pbpar &= ~(0xC000);
	immap->im_ioport.iop_pbdir &= ~(0xC000);
#endif
	/* remove all power */

	immap->im_ioport.iop_pcdat |= 0x0400;
	immap->im_ioport.iop_padat |= 0x0200;

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
	* Power On.
	*/
	mask = PCMCIA_VS1(slot) | PCMCIA_VS2(slot);
	reg  = pcmp->pcmc_pipr;
	debug ("PIPR: 0x%x ==> VS1=o%s, VS2=o%s\n",
	       reg,
	       (reg&PCMCIA_VS1(slot))?"n":"ff",
	       (reg&PCMCIA_VS2(slot))?"n":"ff");
	if ((reg & mask) == mask) {
		immap->im_ioport.iop_pcdat &= ~(0x4000);
		puts (" 5.0V card found: ");
	} else {
		immap->im_ioport.iop_padat &= ~(0x0002);
		puts (" 3.3V card found: ");
	}
	immap->im_ioport.iop_pcdir |= 0x0400;
	immap->im_ioport.iop_padir |= 0x0200;
#if 0
	/*  VCC switch error flag, PCMCIA slot INPACK_ pin */
	cp->cp_pbdir &= ~(0x0020 | 0x0010);
	cp->cp_pbpar &= ~(0x0020 | 0x0010);
	udelay(500000);
#endif
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
	u_long reg;

	debug ("hardware_disable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	immap = (immap_t *)CONFIG_SYS_IMMR;

	/* remove all power */
	immap->im_ioport.iop_pcdat |= 0x0400;
	immap->im_ioport.iop_padat |= 0x0200;

	/* Configure PCMCIA General Control Register */
	debug ("Disable PCMCIA buffers and assert RESET\n");
	reg  = 0;
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;

	udelay(10000);

	return (0);
}
#endif


int pcmcia_voltage_set(int slot, int vcc, int vpp)
{
	volatile immap_t	*immap;
	volatile pcmconf8xx_t	*pcmp;
	u_long reg;

	debug ("voltage_set: "
			PCMCIA_BOARD_MSG
			" Slot %c, Vcc=%d.%d, Vpp=%d.%d\n",
	'A'+slot, vcc/10, vcc%10, vpp/10, vcc%10);

	immap = (immap_t *)CONFIG_SYS_IMMR;
	pcmp = (pcmconf8xx_t *)(&(((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia));
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
	* Configure Ports A & C pins for
	* 5 Volts Enable and 3 Volts enable,
	* Turn off all power
	*/
	debug ("PCMCIA power OFF\n");
	immap->im_ioport.iop_pcpar &= ~(0x0400);
	immap->im_ioport.iop_pcso  &= ~(0x0400);/*
	immap->im_ioport.iop_pcdir |= 0x0400;*/

	immap->im_ioport.iop_papar &= ~(0x0200);/*
	immap->im_ioport.iop_padir |= 0x0200;*/

	immap->im_ioport.iop_pcdat |= 0x0400;
	immap->im_ioport.iop_padat |= 0x0200;

	reg = 0;
	switch(vcc) {
		case  0:		break;
		case 33: reg |= 0x0200;	break;
		case 50: reg |= 0x0400;	break;
		default:		goto done;
	}

	/* Checking supported voltages */

	debug ("PIPR: 0x%x --> %s\n",
	       pcmp->pcmc_pipr,
	       (pcmp->pcmc_pipr & 0x00008000) ? "only 5 V" : "can do 3.3V");

	if (reg & 0x0200)
		immap->im_ioport.iop_pcdat &= !reg;
	if (reg & 0x0400)
		immap->im_ioport.iop_padat &= !reg;
	immap->im_ioport.iop_pcdir |= 0x0200;
	immap->im_ioport.iop_padir |= 0x0400;
	if (reg) {
		debug ("PCMCIA powered at %sV\n",
		       (reg&0x0400) ? "5.0" : "3.3");
	} else {
		debug ("PCMCIA powered down\n");
	}

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

#endif	/* CCONFIG_PCMCIA */
