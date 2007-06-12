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

#define PCMCIA_BOARD_MSG "GTH COMPACT FLASH"

int pcmcia_voltage_set (int slot, int vcc, int vpp)
{	/* Do nothing */
	return 0;
}

int pcmcia_hardware_enable (int slot)
{
	volatile immap_t *immap;
	volatile cpm8xx_t *cp;
	volatile pcmconf8xx_t *pcmp;
	volatile sysconf8xx_t *sysp;
	uint reg, mask;

	debug ("hardware_enable: GTH Slot %c\n", 'A' + slot);

	immap = (immap_t *) CFG_IMMR;
	sysp = (sysconf8xx_t *) (&(((immap_t *) CFG_IMMR)->im_siu_conf));
	pcmp = (pcmconf8xx_t *) (&(((immap_t *) CFG_IMMR)->im_pcmcia));
	cp = (cpm8xx_t *) (&(((immap_t *) CFG_IMMR)->im_cpm));

	/* clear interrupt state, and disable interrupts */
	pcmp->pcmc_pscr = PCMCIA_MASK (_slot_);
	pcmp->pcmc_per &= ~PCMCIA_MASK (_slot_);

	/*
	* Disable interrupts, DMA, and PCMCIA buffers
	* (isolate the interface) and assert RESET signal
	*/
	debug ("Disable PCMCIA buffers and assert RESET\n");
	reg = 0;
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= __MY_PCMCIA_GCRX_CXOE;	/* active low  */
	PCMCIA_PGCRX (_slot_) = reg;
	udelay (500);

	/*
	* Make sure there is a card in the slot,
	* then configure the interface.
	*/
	udelay (10000);
	debug ("[%d] %s: PIPR(%p)=0x%x\n",
	       __LINE__, __FUNCTION__,
	       &(pcmp->pcmc_pipr), pcmp->pcmc_pipr);
	if (pcmp->pcmc_pipr & 0x98000000) {
		printf ("   No Card found\n");
		return (1);
	}

	mask = PCMCIA_VS1 (slot) | PCMCIA_VS2 (slot);
	reg = pcmp->pcmc_pipr;
	debug ("PIPR: 0x%x ==> VS1=o%s, VS2=o%s\n",
	       reg,
	       (reg & PCMCIA_VS1 (slot)) ? "n" : "ff",
	       (reg & PCMCIA_VS2 (slot)) ? "n" : "ff");

	debug ("Enable PCMCIA buffers and stop RESET\n");
	reg  =  PCMCIA_PGCRX (_slot_);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg &= ~__MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX (_slot_) = reg;

	udelay (250000);	/* some cards need >150 ms to come up :-( */

	debug ("# hardware_enable done\n");

	return 0;
}
#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA) || defined(CONFIG_CMD_PCMCIA)
int pcmcia_hardware_disable(int slot)
{
	return 0;	/* No hardware to disable */
}
#endif

#endif	/* CONFIG_PCMCIA */
