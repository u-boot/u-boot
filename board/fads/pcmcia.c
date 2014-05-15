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

#define	PCMCIA_BOARD_MSG "FADS"

int pcmcia_voltage_set(int slot, int vcc, int vpp)
{
	u_long reg = 0;

	switch(vpp) {
		case 0: reg = 0; break;
		case 50: reg = 1; break;
		case 120: reg = 2; break;
		default: return 1;
	}

	switch(vcc) {
		case 0: reg = 0; break;
#ifdef CONFIG_FADS
	case 33: reg = BCSR1_PCCVCC0 | BCSR1_PCCVCC1; break;
	case 50: reg = BCSR1_PCCVCC1; break;
#endif
	default: return 1;
	}

	/* first, turn off all power */

#ifdef CONFIG_FADS
	*((uint *)BCSR1) &= ~(BCSR1_PCCVCC0 | BCSR1_PCCVCC1);
#endif
	*((uint *)BCSR1) &= ~BCSR1_PCCVPP_MASK;

	/* enable new powersettings */

#ifdef CONFIG_FADS
	*((uint *)BCSR1) |= reg;
#endif

	*((uint *)BCSR1) |= reg << 20;

	return 0;
}

int pcmcia_hardware_enable(int slot)
{
	*((uint *)BCSR1) &= ~BCSR1_PCCEN;
	return 0;
}

#if defined(CONFIG_CMD_PCMCIA)
int pcmcia_hardware_disable(int slot)
{
	*((uint *)BCSR1) &= ~BCSR1_PCCEN;
	return 0;
}
#endif

#endif	/* CONFIG_PCMCIA */
