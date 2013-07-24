#include <common.h>
#include <mpc8xx.h>
#include <pcmcia.h>
#include <i2c.h>

#undef	CONFIG_PCMCIA

#if defined(CONFIG_CMD_PCMCIA)
#define	CONFIG_PCMCIA
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_8xx_PCCARD)
#define	CONFIG_PCMCIA
#endif

#ifdef	CONFIG_PCMCIA

#define PCMCIA_BOARD_MSG "LWMON"

/* #define's for MAX1604 Power Switch */
#define MAX1604_OP_SUS		0x80
#define MAX1604_VCCBON		0x40
#define MAX1604_VCC_35		0x20
#define MAX1604_VCCBHIZ		0x10
#define MAX1604_VPPBON		0x08
#define MAX1604_VPPBPBPGM	0x04
#define MAX1604_VPPBHIZ		0x02
/* reserved			0x01	*/

int pcmcia_hardware_enable(int slot)
{
	volatile pcmconf8xx_t	*pcmp;
	volatile sysconf8xx_t	*sysp;
	uint reg, mask;
	uchar val;


	debug ("hardware_enable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	/* Switch on PCMCIA port in PIC register 0x60 */
	reg = pic_read  (0x60);
	debug ("[%d] PIC read: reg_60 = 0x%02x\n", __LINE__, reg);
	reg &= ~0x10;
	/* reg |= 0x08; Vpp not needed */
	pic_write (0x60, reg);
#ifdef DEBUG
	reg = pic_read  (0x60);
	printf ("[%d] PIC read: reg_60 = 0x%02x\n", __LINE__, reg);
#endif
	udelay(10000);

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
		val = 0;		/* VCCB3/5 = 0 ==> use Vx = 5.0 V */
		puts (" 5.0V card found: ");
	} else {
		val = MAX1604_VCC_35;	/* VCCB3/5 = 1 ==> use Vy = 3.3 V */
		puts (" 3.3V card found: ");
	}

	/*  switch VCC on */
	val |= MAX1604_OP_SUS | MAX1604_VCCBON;
	i2c_set_bus_num(0);
	i2c_write (CONFIG_SYS_I2C_POWER_A_ADDR, 0, 0, &val, 1);

	udelay(500000);

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
	volatile pcmconf8xx_t	*pcmp;
	u_long reg;
	uchar val;

	debug ("hardware_disable: " PCMCIA_BOARD_MSG " Slot %c\n", 'A'+slot);

	immap = (immap_t *)CONFIG_SYS_IMMR;
	pcmp = (pcmconf8xx_t *)(&(((immap_t *)CONFIG_SYS_IMMR)->im_pcmcia));

	/* remove all power, put output in high impedance state */
	val  = MAX1604_VCCBHIZ | MAX1604_VPPBHIZ;
	i2c_init  (CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	i2c_write (CONFIG_SYS_I2C_POWER_A_ADDR, 0, 0, &val, 1);

	/* Configure PCMCIA General Control Register */
	debug ("Disable PCMCIA buffers and assert RESET\n");
	reg  = 0;
	reg |= __MY_PCMCIA_GCRX_CXRESET;	/* active high */
	reg |= __MY_PCMCIA_GCRX_CXOE;		/* active low  */
	PCMCIA_PGCRX(_slot_) = reg;

	/* Switch off PCMCIA port in PIC register 0x60 */
	reg = pic_read  (0x60);
	debug ("[%d] PIC read: reg_60 = 0x%02x\n", __LINE__, reg);
	reg |=  0x10;
	reg &= ~0x08;
	pic_write (0x60, reg);
#ifdef DEBUG
	reg = pic_read  (0x60);
	printf ("[%d] PIC read: reg_60 = 0x%02x\n", __LINE__, reg);
#endif
	udelay(10000);

	return (0);
}
#endif


int pcmcia_voltage_set(int slot, int vcc, int vpp)
{
	volatile pcmconf8xx_t	*pcmp;
	u_long reg;
	uchar val;

	debug ("voltage_set: "
		PCMCIA_BOARD_MSG
		" Slot %c, Vcc=%d.%d, Vpp=%d.%d\n",
		'A'+slot, vcc/10, vcc%10, vpp/10, vcc%10);

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
	 * Turn off all power (switch to high impedance)
	 */
	debug ("PCMCIA power OFF\n");
	val  = MAX1604_VCCBHIZ | MAX1604_VPPBHIZ;
	i2c_set_bus_num(0);
	i2c_write (CONFIG_SYS_I2C_POWER_A_ADDR, 0, 0, &val, 1);

	val = 0;
	switch(vcc) {
	case  0:			break;
	case 33: val = MAX1604_VCC_35;	break;
	case 50:			break;
	default:			goto done;
	}

	/* Checking supported voltages */

	debug ("PIPR: 0x%x --> %s\n",
		pcmp->pcmc_pipr,
		(pcmp->pcmc_pipr & 0x00008000) ? "only 5 V" : "can do 3.3V");

	i2c_write (CONFIG_SYS_I2C_POWER_A_ADDR, 0, 0, &val, 1);
	if (val) {
		debug ("PCMCIA powered at %sV\n",
			(val & MAX1604_VCC_35) ? "3.3" : "5.0");
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

#endif	/* CONFIG_PCMCIA */
