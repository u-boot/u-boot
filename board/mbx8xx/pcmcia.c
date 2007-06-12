#include <common.h>
#include <mpc8xx.h>
#include <pcmcia.h>

#include "csr.h"

#undef	CONFIG_PCMCIA

#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA) || defined(CONFIG_CMD_PCMCIA)
#define	CONFIG_PCMCIA
#endif

#if ((CONFIG_COMMANDS & CFG_CMD_IDE) || defined(CONFIG_CMD_IDE)) && defined(CONFIG_IDE_8xx_PCCARD)
#define	CONFIG_PCMCIA
#endif

#ifdef	CONFIG_PCMCIA

/* A lot of this has been taken from the RPX code in this file it works from me.
   I have added the voltage selection for the MBX board. */

/* MBX voltage bit in control register #2 */
#define CR2_VPP12       ((uchar)0x10)
#define CR2_VPPVDD      ((uchar)0x20)
#define CR2_VDD5        ((uchar)0x40)
#define CR2_VDD3        ((uchar)0x80)

#define PCMCIA_BOARD_MSG "MBX860"

int pcmcia_voltage_set (int slot, int vcc, int vpp)
{
	uchar reg = 0;

	debug ("voltage_set: PCMCIA_BOARD_MSG Slot %c, Vcc=%d.%d, Vpp=%d.%d\n",
	       'A' + slot, vcc / 10, vcc % 10, vpp / 10, vcc % 10);

	switch (vcc) {
		case 0:
			break;
		case 33:
			reg |= CR2_VDD3;
			break;
		case 50:
			reg |= CR2_VDD5;
			break;
		default:
			return 1;
	}

	switch (vpp) {
		case 0:
			break;
		case 33:
		case 50:
			if (vcc == vpp) {
				reg |= CR2_VPPVDD;
			} else {
				return 1;
			}
			break;
		case 120:
			reg |= CR2_VPP12;
			break;
		default:
			return 1;
	}

	/* first, turn off all power */
	MBX_CSR2 &= ~(CR2_VDDSEL | CR2_VPPSEL);

	/* enable new powersettings */
	MBX_CSR2 |= reg;
	debug ("MBX_CSR2 read = 0x%02x\n", MBX_CSR2);

	return (0);
}

int pcmcia_hardware_enable (int slot)
{
	volatile immap_t *immap;
	volatile cpm8xx_t *cp;
	volatile pcmconf8xx_t *pcmp;
	volatile sysconf8xx_t *sysp;
	uint reg, mask;

	debug ("hardware_enable: " PCMCIA_BOARD_MSG " Slot %c\n",
	       'A' + slot);

	udelay (10000);

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

	/* remove all power */
	pcmcia_voltage_set (slot, 0, 0);
	/*
	* Make sure there is a card in the slot, then configure the interface.
	*/
	udelay(10000);
	debug ("[%d] %s: PIPR(%p)=0x%x\n",
	       __LINE__,__FUNCTION__,
	       &(pcmp->pcmc_pipr),pcmp->pcmc_pipr);
#ifndef CONFIG_HMI10
	if (pcmp->pcmc_pipr & (0x18000000 >> (slot << 4))) {
#else
	if (pcmp->pcmc_pipr & (0x10000000 >> (slot << 4))) {
#endif	/* CONFIG_HMI10 */
		printf ("   No Card found\n");
		return (1);
	}

	/*
	* Power On.
	*/
	mask = PCMCIA_VS1 (_slot_) | PCMCIA_VS2 (_slot_);
	reg = pcmp->pcmc_pipr;
	debug ("PIPR: 0x%x ==> VS1=o%s, VS2=o%s\n", reg,
	       (reg & PCMCIA_VS1 (slot)) ? "n" : "ff",
	       (reg & PCMCIA_VS2 (slot)) ? "n" : "ff");

	if ((reg & mask) == mask) {
		pcmcia_voltage_set (_slot_, 50, 0);
		printf (" 5.0V card found: ");
	} else {
		pcmcia_voltage_set (_slot_, 33, 0);
		printf (" 3.3V card found: ");
	}

	debug ("Enable PCMCIA buffers and stop RESET\n");
	reg = PCMCIA_PGCRX (_slot_);
	reg &= ~__MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg &= ~__MY_PCMCIA_GCRX_CXOE;	/* active low  */
	PCMCIA_PGCRX (_slot_) = reg;

	udelay (250000);	/* some cards need >150 ms to come up :-( */

	debug ("# hardware_enable done\n");

	return (0);
	}

#if (CONFIG_COMMANDS & CFG_CMD_PCMCIA) || defined(CONFIG_CMD_PCMCIA)
int pcmcia_hardware_disable (int slot)
{
	return 0;	/* No hardware to disable */
}
#endif /* CFG_CMD_PCMCIA */

#endif	/* CONFIG_PCMCIA */
