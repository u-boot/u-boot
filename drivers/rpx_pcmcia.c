/* -------------------------------------------------------------------- */
/* RPX Boards from Embedded Planet					*/
/* -------------------------------------------------------------------- */
#include <common.h>
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
	&& (defined(CONFIG_RPXCLASSIC) || defined(CONFIG_RPXLITE))

#define	PCMCIA_BOARD_MSG	"RPX CLASSIC or RPX LITE"

int pcmcia_voltage_set(int slot, int vcc, int vpp)
{
	u_long reg = 0;

	switch(vcc) {
		case 0: break;
		case 33: reg |= BCSR1_PCVCTL4; break;
		case 50: reg |= BCSR1_PCVCTL5; break;
		default: return 1;
	}

	switch(vpp) {
		case 0: break;
		case 33:
		case 50:
			if(vcc == vpp)
				reg |= BCSR1_PCVCTL6;
			else
				return 1;
			break;
		case 120:
			reg |= BCSR1_PCVCTL7;
			default: return 1;
	}

	/* first, turn off all power */
	*((uint *)RPX_CSR_ADDR) &= ~(BCSR1_PCVCTL4 | BCSR1_PCVCTL5
			| BCSR1_PCVCTL6 | BCSR1_PCVCTL7);

	/* enable new powersettings */
	*((uint *)RPX_CSR_ADDR) |= reg;

	return 0;
}

int pcmcia_hardware_enable (int slot)
{
	return 0;	/* No hardware to enable */
}

#if defined(CONFIG_CMD_PCMCIA)
static int pcmcia_hardware_disable(int slot)
{
	return 0;	/* No hardware to disable */
}
#endif


#endif	/* CONFIG_PCMCIA && (CONFIG_RPXCLASSIC || CONFIG_RPXLITE) */
