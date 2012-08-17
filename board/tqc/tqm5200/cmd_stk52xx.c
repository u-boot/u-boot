/*
 * (C) Copyright 2005
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * STK52XX specific functions
 */
/*#define DEBUG*/

#include <common.h>
#include <command.h>

#if defined(CONFIG_CMD_BSP)

#if defined(CONFIG_STK52XX) || defined(CONFIG_FO300)
#define DEFAULT_VOL	45
#define DEFAULT_FREQ	500
#define DEFAULT_DURATION	200
#define LEFT		1
#define RIGHT		2
#define LEFT_RIGHT	3
#define BL_OFF		0
#define BL_ON		1

#define SM501_GPIO_CTRL_LOW		0x00000008UL
#define SM501_GPIO_CTRL_HIGH		0x0000000CUL
#define SM501_POWER_MODE0_GATE		0x00000040UL
#define SM501_POWER_MODE1_GATE		0x00000048UL
#define POWER_MODE_GATE_GPIO_PWM_I2C	0x00000040UL
#define SM501_GPIO_DATA_LOW		0x00010000UL
#define SM501_GPIO_DATA_HIGH		0x00010004UL
#define SM501_GPIO_DATA_DIR_LOW		0x00010008UL
#define SM501_GPIO_DATA_DIR_HIGH	0x0001000CUL
#define SM501_PANEL_DISPLAY_CONTROL	0x00080000UL

static int i2s_squarewave(unsigned long duration, unsigned int freq,
			  unsigned int channel);
static int i2s_sawtooth(unsigned long duration, unsigned int freq,
			unsigned int channel);
static void spi_init(void);
static int spi_transmit(unsigned char data);
static void pcm1772_write_reg(unsigned char addr, unsigned char data);
static void set_attenuation(unsigned char attenuation);

static void spi_init(void)
{
	struct mpc5xxx_spi *spi = (struct mpc5xxx_spi*)MPC5XXX_SPI;
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio*)MPC5XXX_GPIO;

	/* PSC3 as SPI and GPIOs */
	gpio->port_config &= 0xFFFFF0FF;
	gpio->port_config |= 0x00000800;
	/*
	 * Its important to use the correct order when initializing the
	 * registers
	 */
	spi->ddr = 0x0F; /* set all SPI pins as output */
	spi->pdr = 0x08; /* set SS high */
	spi->cr1 = 0x50; /* SPI is master, SS is general purpose output */
	spi->cr2 = 0x00; /* normal operation */
	spi->brr = 0xFF; /* baud rate: IPB clock / 2048 */
}

static int spi_transmit(unsigned char data)
{
	struct mpc5xxx_spi *spi = (struct mpc5xxx_spi*)MPC5XXX_SPI;

	spi->dr = data;
	/* wait for SPI transmission completed */
	while (!(spi->sr & 0x80)) {
		if (spi->sr & 0x40) {	/* if write collision occured */
			int dummy;

			/* do dummy read to clear status register */
			dummy = spi->dr;
			printf("SPI write collision: dr=0x%x\n", dummy);
			return -1;
		}
	}
	return (spi->dr);
}

static void pcm1772_write_reg(unsigned char addr, unsigned char data)
{
	struct mpc5xxx_spi *spi = (struct mpc5xxx_spi*)MPC5XXX_SPI;

	spi->pdr = 0x00; /* Set SS low */
	spi_transmit(addr);
	spi_transmit(data);
	/* wait some time to meet MS# hold time of PCM1772 */
	udelay (1);
	spi->pdr = 0x08; /* set SS high */
}

static void set_attenuation(unsigned char attenuation)
{
	pcm1772_write_reg(0x01, attenuation); /* left channel */
	debug ("PCM1772 attenuation left set to %d.\n", attenuation);
	pcm1772_write_reg(0x02, attenuation); /* right channel */
	debug ("PCM1772 attenuation right set to %d.\n", attenuation);
}

void amplifier_init(void)
{
	static int init_done = 0;
	int i;
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio*)MPC5XXX_GPIO;

	/* Do this only once, because of the long time delay */
	if (!init_done) {
		/* configure PCM1772 audio format as I2S */
		pcm1772_write_reg(0x03, 0x01);
		/* enable audio amplifier */
		gpio->sint_gpioe |=  0x02;	/* PSC3_5 as GPIO */
		gpio->sint_ode &= ~0x02;	/* PSC3_5 is not open Drain */
		gpio->sint_dvo &= ~0x02;	/* PSC3_5 is LOW */
		gpio->sint_ddr |=  0x02;	/* PSC3_5 as output */
		/*
		 * wait some time to allow amplifier to recover from shutdown
		 * mode.
		 */
		for(i = 0; i < 350; i++)
			udelay(1000);
		/*
		 * The used amplifier (LM4867) has a so called "pop and click"
		 * elmination filter. The input signal of the amplifier must
		 * exceed a certain level once after power up to activate the
		 * generation of the output signal. This is achieved by
		 * sending a low frequent (nearly inaudible) sawtooth with a
		 * sufficient signal level.
		 */
		set_attenuation(50);
		i2s_sawtooth (200, 5, LEFT_RIGHT);
		init_done = 1;
	}
}

static void i2s_init(void)
{
	unsigned long i;
	struct mpc5xxx_psc *psc = (struct mpc5xxx_psc*)MPC5XXX_PSC2;;
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio*)MPC5XXX_GPIO;

	gpio->port_config |= 0x00000070; /* PSC2 ports as Codec with MCLK */
	psc->command = (PSC_RX_DISABLE | PSC_TX_DISABLE);
	psc->sicr = 0x22E00000;		/* 16 bit data; I2S */

	*(vu_long *)(CONFIG_SYS_MBAR + 0x22C) = 0x805d; /* PSC2 CDM MCLK config; MCLK
						  * 5.617 MHz */
	*(vu_long *)(CONFIG_SYS_MBAR + 0x214) |= 0x00000040; /* CDM clock enable
						       * register */
	psc->ccr = 0x1F03;	/* 16 bit data width; 5.617MHz MCLK */
	psc->ctur = 0x0F;	/* 16 bit frame width */

	for (i = 0; i < 128; i++)
		psc->psc_buffer_32 = 0; /* clear tx fifo */
}

static int i2s_play_wave(unsigned long addr, unsigned long len)
{
	unsigned long i;
	unsigned char *wave_file = (uchar *)addr + 44;	/* quick'n dirty: skip
							 * wav header*/
	struct mpc5xxx_psc *psc = (struct mpc5xxx_psc*)MPC5XXX_PSC2;

	/*
	 * play wave file in memory; bytes/words are be swapped
	 */
	psc->command = (PSC_RX_ENABLE | PSC_TX_ENABLE);

	for(i = 0;i < (len / 4); i++) {
		unsigned char swapped[4];
		unsigned long *p = (unsigned long*)swapped;

		swapped[3] = *wave_file++;
		swapped[2] = *wave_file++;
		swapped[1] = *wave_file++;
		swapped[0] = *wave_file++;

		psc->psc_buffer_32 =  *p;

		while (psc->tfnum > 400) {
			if(ctrlc())
				return 0;
		}
	}
	while (psc->tfnum > 0);		/* wait for fifo empty */
	udelay (100);
	psc->command = (PSC_RX_DISABLE | PSC_TX_DISABLE);
	return 0;
}

static int i2s_sawtooth(unsigned long duration, unsigned int freq,
			unsigned int channel)
{
	long i,j;
	unsigned long data;
	struct mpc5xxx_psc *psc = (struct mpc5xxx_psc*)MPC5XXX_PSC2;

	psc->command = (PSC_RX_ENABLE | PSC_TX_ENABLE);

	/*
	 * Generate sawtooth. Start with middle level up to highest level. Then
	 * go to lowest level and back to middle level.
	 */
	for(j = 0; j < ((duration * freq) / 1000); j++)	{
		for(i = 0; i <= 0x7FFF; i += (0x7FFF/(44100/(freq*4))))	{
			data = (i & 0xFFFF);
			/* data format: right data left data) */
			if (channel == LEFT_RIGHT)
				data |= (data<<16);
			if (channel == RIGHT)
				data = (data<<16);
			psc->psc_buffer_32 = data;
			while (psc->tfnum > 400);
		}
		for(i = 0x7FFF; i >= -0x7FFF; i -= (0xFFFF/(44100/(freq*2)))) {
			data = (i & 0xFFFF);
			/* data format: right data left data) */
			if (channel == LEFT_RIGHT)
				data |= (data<<16);
			if (channel == RIGHT)
				data = (data<<16);
			psc->psc_buffer_32 = data;
			while (psc->tfnum > 400);
		}
		for(i = -0x7FFF; i <= 0; i += (0x7FFF/(44100/(freq*4)))) {
			data = (i & 0xFFFF);
			/* data format: right data left data) */
			if (channel == LEFT_RIGHT)
				data |= (data<<16);
			if (channel == RIGHT)
				data = (data<<16);
			psc->psc_buffer_32 = data;
			while (psc->tfnum > 400);
		}
	}
	while (psc->tfnum > 0);		/* wait for fifo empty */
	udelay (100);
	psc->command = (PSC_RX_DISABLE | PSC_TX_DISABLE);

	return 0;
}

static int i2s_squarewave(unsigned long duration, unsigned int freq,
			 unsigned int channel)
{
	long i,j;
	unsigned long data;
	struct mpc5xxx_psc *psc = (struct mpc5xxx_psc*)MPC5XXX_PSC2;

	psc->command = (PSC_RX_ENABLE | PSC_TX_ENABLE);

	/*
	 * Generate sqarewave. Start with high level, duty cycle 1:1.
	 */
	for(j = 0; j < ((duration * freq) / 1000); j++)	{
		for(i = 0; i < (44100/(freq*2)); i ++) {
			data = 0x7FFF;
			/* data format: right data left data) */
			if (channel == LEFT_RIGHT)
				data |= (data<<16);
			if (channel == RIGHT)
				data = (data<<16);
			psc->psc_buffer_32 = data;
			while (psc->tfnum > 400);
		}
		for(i = 0; i < (44100/(freq*2)); i ++) {
			data = 0x8000;
			/* data format: right data left data) */
			if (channel == LEFT_RIGHT)
				data |= (data<<16);
			if (channel == RIGHT)
				data = (data<<16);
			psc->psc_buffer_32 = data;
			while (psc->tfnum > 400);
		}
	}
	while (psc->tfnum > 0);		/* wait for fifo empty */
	udelay (100);
	psc->command = (PSC_RX_DISABLE | PSC_TX_DISABLE);

	return 0;
}

static int cmd_sound(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long reg, val, duration;
	char *tmp;
	unsigned int freq, channel;
	unsigned char volume;
	int rcode = 1;

#ifdef CONFIG_STK52XX_REV100
	printf ("Revision 100 of STK52XX not supported!\n");
	return 1;
#endif
	spi_init();
	i2s_init();
	amplifier_init();

	if ((tmp = getenv ("volume")) != NULL) {
		volume = simple_strtoul (tmp, NULL, 10);
	} else {
		volume = DEFAULT_VOL;
	}
	set_attenuation(volume);

	switch (argc) {
	case 0:
	case 1:
		return cmd_usage(cmdtp);
	case 2:
		if (strncmp(argv[1],"saw",3) == 0) {
			printf ("Play sawtooth\n");
			rcode = i2s_sawtooth (DEFAULT_DURATION, DEFAULT_FREQ,
					      LEFT_RIGHT);
			return rcode;
		} else if (strncmp(argv[1],"squ",3) == 0) {
			printf ("Play squarewave\n");
			rcode = i2s_squarewave (DEFAULT_DURATION, DEFAULT_FREQ,
						LEFT_RIGHT);
			return rcode;
		}

		return cmd_usage(cmdtp);
	case 3:
		if (strncmp(argv[1],"saw",3) == 0) {
			duration = simple_strtoul(argv[2], NULL, 10);
			printf ("Play sawtooth\n");
			rcode = i2s_sawtooth (duration, DEFAULT_FREQ,
					      LEFT_RIGHT);
			return rcode;
		} else if (strncmp(argv[1],"squ",3) == 0) {
			duration = simple_strtoul(argv[2], NULL, 10);
			printf ("Play squarewave\n");
			rcode = i2s_squarewave (duration, DEFAULT_FREQ,
						LEFT_RIGHT);
			return rcode;
		}
		return cmd_usage(cmdtp);
	case 4:
		if (strncmp(argv[1],"saw",3) == 0) {
			duration = simple_strtoul(argv[2], NULL, 10);
			freq = (unsigned int)simple_strtoul(argv[3], NULL, 10);
			printf ("Play sawtooth\n");
			rcode = i2s_sawtooth (duration, freq,
					      LEFT_RIGHT);
			return rcode;
		} else if (strncmp(argv[1],"squ",3) == 0) {
			duration = simple_strtoul(argv[2], NULL, 10);
			freq = (unsigned int)simple_strtoul(argv[3], NULL, 10);
			printf ("Play squarewave\n");
			rcode = i2s_squarewave (duration, freq,
						LEFT_RIGHT);
			return rcode;
		} else if (strcmp(argv[1],"pcm1772") == 0) {
			reg = simple_strtoul(argv[2], NULL, 10);
			val = simple_strtoul(argv[3], NULL, 10);
			printf("Set PCM1772 %lu. %lu\n", reg, val);
			pcm1772_write_reg((uchar)reg, (uchar)val);
			return 0;
		}
		return cmd_usage(cmdtp);
	case 5:
		if (strncmp(argv[1],"saw",3) == 0) {
			duration = simple_strtoul(argv[2], NULL, 10);
			freq = (unsigned int)simple_strtoul(argv[3], NULL, 10);
			if (strncmp(argv[4],"l",1) == 0)
				channel = LEFT;
			else if (strncmp(argv[4],"r",1) == 0)
				channel = RIGHT;
			else
				channel = LEFT_RIGHT;
			printf ("Play squarewave\n");
			rcode = i2s_sawtooth (duration, freq,
					      channel);
			return rcode;
		} else if (strncmp(argv[1],"squ",3) == 0) {
			duration = simple_strtoul(argv[2], NULL, 10);
			freq = (unsigned int)simple_strtoul(argv[3], NULL, 10);
			if (strncmp(argv[4],"l",1) == 0)
				channel = LEFT;
			else if (strncmp(argv[4],"r",1) == 0)
				channel = RIGHT;
			else
				channel = LEFT_RIGHT;
			printf ("Play squarewave\n");
			rcode = i2s_squarewave (duration, freq,
						channel);
			return rcode;
		}
		return cmd_usage(cmdtp);
	}
	printf ("Usage:\nsound cmd [arg1] [arg2] ...\n");
	return 1;
}

static int cmd_wav(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long length, addr;
	unsigned char volume;
	int rcode = 1;
	char *tmp;

#ifdef CONFIG_STK52XX_REV100
	printf ("Revision 100 of STK52XX not supported!\n");
	return 1;
#endif
	spi_init();
	i2s_init();
	amplifier_init();

	switch (argc) {

	case 3:
		length = simple_strtoul(argv[2], NULL, 16);
		addr = simple_strtoul(argv[1], NULL, 16);
		break;

	case 2:
		if ((tmp = getenv ("filesize")) != NULL) {
			length = simple_strtoul (tmp, NULL, 16);
		} else {
			puts ("No filesize provided\n");
			return 1;
		}
		addr = simple_strtoul(argv[1], NULL, 16);

	case 1:
		if ((tmp = getenv ("filesize")) != NULL) {
			length = simple_strtoul (tmp, NULL, 16);
		} else {
			puts ("No filesize provided\n");
			return 1;
		}
		if ((tmp = getenv ("loadaddr")) != NULL) {
			addr = simple_strtoul (tmp, NULL, 16);
		} else {
			puts ("No loadaddr provided\n");
			return 1;
		}
		break;

	default:
		printf("Usage:\nwav <addr> <length[s]\n");
		return 1;
		break;
	}

	if ((tmp = getenv ("volume")) != NULL) {
		volume = simple_strtoul (tmp, NULL, 10);
	} else {
		volume = DEFAULT_VOL;
	}
	set_attenuation(volume);

	printf("Play wave file at %lX with length %lX\n", addr, length);
	rcode = i2s_play_wave(addr, length);

	return rcode;
}

static int cmd_beep(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char volume;
	unsigned int channel;
	int rcode;
	char *tmp;

#ifdef CONFIG_STK52XX_REV100
	printf ("Revision 100 of STK52XX not supported!\n");
	return 1;
#endif
	spi_init();
	i2s_init();
	amplifier_init();

	switch (argc) {
	case 0:
	case 1:
		channel = LEFT_RIGHT;
		break;
	case 2:
		if (strncmp(argv[1],"l",1) == 0)
			channel = LEFT;
		else if (strncmp(argv[1],"r",1) == 0)
			channel = RIGHT;
		else
			channel = LEFT_RIGHT;
		break;
	default:
		return cmd_usage(cmdtp);
	}

	if ((tmp = getenv ("volume")) != NULL) {
		volume = simple_strtoul (tmp, NULL, 10);
	} else {
		volume = DEFAULT_VOL;
	}
	set_attenuation(volume);

	printf("Beep on ");
	if (channel == LEFT)
		printf ("left ");
	else if (channel == RIGHT)
		printf ("right ");
	else
		printf ("left and right ");
	printf ("channel\n");

	rcode = i2s_squarewave (DEFAULT_DURATION, DEFAULT_FREQ, channel);

	return rcode;
}
#endif

#if defined(CONFIG_STK52XX)
void led_init(void)
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	struct mpc5xxx_gpt_0_7 *gpt = (struct mpc5xxx_gpt_0_7 *)MPC5XXX_GPT;

	/* configure PSC3 for SPI and GPIO */
	gpio->port_config &= ~(0x00000F00);
	gpio->port_config |=   0x00000800;

	gpio->simple_gpioe &= ~(0x00000F00);
	gpio->simple_gpioe |=   0x00000F00;

	gpio->simple_ddr &= ~(0x00000F00);
	gpio->simple_ddr |=   0x00000F00;

	/* configure timer 4-7 for simple GPIO output */
	gpt->gpt4.emsr |=  0x00000024;
	gpt->gpt5.emsr |=  0x00000024;
	gpt->gpt6.emsr |=  0x00000024;
	gpt->gpt7.emsr |=  0x00000024;

#ifndef CONFIG_TQM5200S
	/* enable SM501 GPIO control (in both power modes) */
	*(vu_long *) (SM501_MMIO_BASE+SM501_POWER_MODE0_GATE) |=
		POWER_MODE_GATE_GPIO_PWM_I2C;
	*(vu_long *) (SM501_MMIO_BASE+SM501_POWER_MODE1_GATE) |=
		POWER_MODE_GATE_GPIO_PWM_I2C;

	/* configure SM501 gpio pins 24-27 as output */
	*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_CTRL_LOW) &= ~(0xF << 24);
	*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_DIR_LOW) |= (0xF << 24);

	/* configure SM501 gpio pins 48-51 as output */
	*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_DIR_HIGH) |= (0xF << 16);
#endif /* !CONFIG_TQM5200S */
}

/*
 * return 1 if led number unknown
 * return 0 else
 */
int do_led(char * const argv[])
{
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	struct mpc5xxx_gpt_0_7 *gpt = (struct mpc5xxx_gpt_0_7 *)MPC5XXX_GPT;

	switch	(simple_strtoul(argv[2], NULL, 10)) {

	case 0:
		if (strcmp (argv[3], "on") == 0) {
			gpio->simple_dvo |=   (1 << 8);
		} else {
			gpio->simple_dvo &= ~(1 << 8);
		}
		break;

	case 1:
		if (strcmp (argv[3], "on") == 0) {
			gpio->simple_dvo |=   (1 << 9);
		} else {
			gpio->simple_dvo &= ~(1 << 9);
		}
		break;

	case 2:
		if (strcmp (argv[3], "on") == 0) {
			gpio->simple_dvo |=   (1 << 10);
		} else {
			gpio->simple_dvo &= ~(1 << 10);
		}
		break;

	case 3:
		if (strcmp (argv[3], "on") == 0) {
			gpio->simple_dvo |=   (1 << 11);
		} else {
			gpio->simple_dvo &= ~(1 << 11);
		}
		break;

	case 4:
		if (strcmp (argv[3], "on") == 0) {
			gpt->gpt4.emsr |=  (1 << 4);
		} else {
			gpt->gpt4.emsr &=  ~(1 << 4);
		}
		break;

	case 5:
		if (strcmp (argv[3], "on") == 0) {
			gpt->gpt5.emsr |=  (1 << 4);
		} else {
			gpt->gpt5.emsr &=  ~(1 << 4);
		}
		break;

	case 6:
		if (strcmp (argv[3], "on") == 0) {
			gpt->gpt6.emsr |=  (1 << 4);
		} else {
			gpt->gpt6.emsr &=  ~(1 << 4);
		}
		break;

	case 7:
		if (strcmp (argv[3], "on") == 0) {
			gpt->gpt7.emsr |=  (1 << 4);
		} else {
			gpt->gpt7.emsr &=  ~(1 << 4);
		}
		break;
#ifndef CONFIG_TQM5200S
	case 24:
		if (strcmp (argv[3], "on") == 0) {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_LOW) |=
				(0x1 << 24);
		} else {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_LOW) &=
				~(0x1 << 24);
		}
		break;

	case 25:
		if (strcmp (argv[3], "on") == 0) {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_LOW) |=
				(0x1 << 25);
		} else {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_LOW) &=
				~(0x1 << 25);
		}
		break;

	case 26:
		if (strcmp (argv[3], "on") == 0) {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_LOW) |=
				(0x1 << 26);
		} else {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_LOW) &=
				~(0x1 << 26);
		}
		break;

	case 27:
		if (strcmp (argv[3], "on") == 0) {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_LOW) |=
				(0x1 << 27);
		} else {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_LOW) &=
				~(0x1 << 27);
		}
		break;

	case 48:
		if (strcmp (argv[3], "on") == 0) {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) |=
				(0x1 << 16);
		} else {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) &=
				~(0x1 << 16);
		}
		break;

	case 49:
		if (strcmp (argv[3], "on") == 0) {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) |=
				(0x1 << 17);
		} else {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) &=
				~(0x1 << 17);
		}
		break;

	case 50:
		if (strcmp (argv[3], "on") == 0) {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) |=
				(0x1 << 18);
		} else {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) &=
				~(0x1 << 18);
		}
		break;

	case 51:
		if (strcmp (argv[3], "on") == 0) {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) |=
				(0x1 << 19);
		} else {
			*(vu_long *) (SM501_MMIO_BASE+SM501_GPIO_DATA_HIGH) &=
				~(0x1 << 19);
		}
		break;
#endif /* !CONFIG_TQM5200S */
	default:
		printf ("%s: invalid led number %s\n", __FUNCTION__, argv[2]);
		return 1;
	}

	return 0;
}
#endif

#if defined(CONFIG_STK52XX) || defined(CONFIG_FO300)
/*
 * return 1 on CAN initialization failure
 * return 0 if no failure
 */
int can_init(void)
{
	static int init_done = 0;
	int i;
	struct mpc5xxx_mscan *can1 =
		(struct mpc5xxx_mscan *)(CONFIG_SYS_MBAR + 0x0900);
	struct mpc5xxx_mscan *can2 =
		(struct mpc5xxx_mscan *)(CONFIG_SYS_MBAR + 0x0980);

	/* GPIO configuration of the CAN pins is done in TQM5200.h */

	if (!init_done) {
		/* init CAN 1 */
		can1->canctl1 |= 0x80;	/* CAN enable */
		udelay(100);

		i = 0;
		can1->canctl0 |= 0x02;	/* sleep mode */
		/* wait until sleep mode reached */
		while (!(can1->canctl1 & 0x02)) {
			udelay(10);
		i++;
		if (i == 10) {
			printf ("%s: CAN1 initialize error, "
				"can not enter sleep mode!\n",
				__FUNCTION__);
			return 1;
		}
		}
		i = 0;
		can1->canctl0 = 0x01;	/* enter init mode */
		/* wait until init mode reached */
		while (!(can1->canctl1 & 0x01)) {
			udelay(10);
			i++;
			if (i == 10) {
				printf ("%s: CAN1 initialize error, "
					"can not enter init mode!\n",
					__FUNCTION__);
				return 1;
			}
		}
		can1->canctl1 = 0x80;
		can1->canctl1 |= 0x40;
		can1->canbtr0 = 0x0F;
		can1->canbtr1 = 0x7F;
		can1->canidac &= ~(0x30);
		can1->canidar1 = 0x00;
		can1->canidar3 = 0x00;
		can1->canidar5 = 0x00;
		can1->canidar7 = 0x00;
		can1->canidmr0 = 0xFF;
		can1->canidmr1 = 0xFF;
		can1->canidmr2 = 0xFF;
		can1->canidmr3 = 0xFF;
		can1->canidmr4 = 0xFF;
		can1->canidmr5 = 0xFF;
		can1->canidmr6 = 0xFF;
		can1->canidmr7 = 0xFF;

		i = 0;
		can1->canctl0 &= ~(0x01);	/* leave init mode */
		can1->canctl0 &= ~(0x02);
		/* wait until init and sleep mode left */
		while ((can1->canctl1 & 0x01) || (can1->canctl1 & 0x02)) {
			udelay(10);
			i++;
			if (i == 10) {
				printf ("%s: CAN1 initialize error, "
					"can not leave init/sleep mode!\n",
					__FUNCTION__);
				return 1;
			}
		}

		/* init CAN 2 */
		can2->canctl1 |= 0x80;	/* CAN enable */
		udelay(100);

		i = 0;
		can2->canctl0 |= 0x02;	/* sleep mode */
		/* wait until sleep mode reached */
		while (!(can2->canctl1 & 0x02))	{
			udelay(10);
			i++;
			if (i == 10) {
				printf ("%s: CAN2 initialize error, "
					"can not enter sleep mode!\n",
					__FUNCTION__);
				return 1;
			}
		}
		i = 0;
		can2->canctl0 = 0x01;	/* enter init mode */
		/* wait until init mode reached */
		while (!(can2->canctl1 & 0x01))	{
			udelay(10);
			i++;
			if (i == 10) {
				printf ("%s: CAN2 initialize error, "
					"can not enter init mode!\n",
					__FUNCTION__);
				return 1;
			}
		}
		can2->canctl1 = 0x80;
		can2->canctl1 |= 0x40;
		can2->canbtr0 = 0x0F;
		can2->canbtr1 = 0x7F;
		can2->canidac &= ~(0x30);
		can2->canidar1 = 0x00;
		can2->canidar3 = 0x00;
		can2->canidar5 = 0x00;
		can2->canidar7 = 0x00;
		can2->canidmr0 = 0xFF;
		can2->canidmr1 = 0xFF;
		can2->canidmr2 = 0xFF;
		can2->canidmr3 = 0xFF;
		can2->canidmr4 = 0xFF;
		can2->canidmr5 = 0xFF;
		can2->canidmr6 = 0xFF;
		can2->canidmr7 = 0xFF;
		can2->canctl0 &= ~(0x01);	/* leave init mode */
		can2->canctl0 &= ~(0x02);

		i = 0;
		/* wait until init mode left */
		while ((can2->canctl1 & 0x01) || (can2->canctl1 & 0x02)) {
			udelay(10);
			i++;
			if (i == 10) {
				printf ("%s: CAN2 initialize error, "
					"can not leave init/sleep mode!\n",
					__FUNCTION__);
				return 1;
			}
		}
		init_done = 1;
	}
	return 0;
}

/*
 * return 1 on CAN failure
 * return 0 if no failure
 */
int do_can(char * const argv[])
{
	int i;
	struct mpc5xxx_mscan *can1 =
		(struct mpc5xxx_mscan *)(CONFIG_SYS_MBAR + 0x0900);
	struct mpc5xxx_mscan *can2 =
		(struct mpc5xxx_mscan *)(CONFIG_SYS_MBAR + 0x0980);

	/* send a message on CAN1 */
	can1->cantbsel = 0x01;
	can1->cantxfg.idr[0] = 0x55;
	can1->cantxfg.idr[1] = 0x00;
	can1->cantxfg.idr[1] &= ~0x8;
	can1->cantxfg.idr[1] &= ~0x10;
	can1->cantxfg.dsr[0] = 0xCC;
	can1->cantxfg.dlr = 1;
	can1->cantxfg.tbpr = 0;
	can1->cantflg = 0x01;

	i = 0;
	while ((can1->cantflg & 0x01) == 0) {
		i++;
		if (i == 10) {
			printf ("%s: CAN1 send timeout, "
				"can not send message!\n",
				__FUNCTION__);
			return 1;
		}
		udelay(1000);
	}
	udelay(1000);

	i = 0;
	while (!(can2->canrflg & 0x01))	{
		i++;
		if (i == 10) {
			printf ("%s: CAN2 receive timeout, "
				"no message received!\n",
				__FUNCTION__);
			return 1;
		}
		udelay(1000);
	}

	if (can2->canrxfg.dsr[0] != 0xCC) {
		printf ("%s: CAN2 receive error, "
			 "data mismatch!\n",
			__FUNCTION__);
		return 1;
	}

	/* send a message on CAN2 */
	can2->cantbsel = 0x01;
	can2->cantxfg.idr[0] = 0x55;
	can2->cantxfg.idr[1] = 0x00;
	can2->cantxfg.idr[1] &= ~0x8;
	can2->cantxfg.idr[1] &= ~0x10;
	can2->cantxfg.dsr[0] = 0xCC;
	can2->cantxfg.dlr = 1;
	can2->cantxfg.tbpr = 0;
	can2->cantflg = 0x01;

	i = 0;
	while ((can2->cantflg & 0x01) == 0) {
		i++;
		if (i == 10) {
			printf ("%s: CAN2 send error, "
				"can not send message!\n",
				__FUNCTION__);
			return 1;
		}
		udelay(1000);
	}
	udelay(1000);

	i = 0;
	while (!(can1->canrflg & 0x01))	{
		i++;
		if (i == 10) {
			printf ("%s: CAN1 receive timeout, "
				"no message received!\n",
				__FUNCTION__);
			return 1;
		}
		udelay(1000);
	}

	if (can1->canrxfg.dsr[0] != 0xCC) {
		printf ("%s: CAN1 receive error 0x%02x\n",
			__FUNCTION__, (can1->canrxfg.dsr[0]));
		return 1;
	}

	return 0;
}

/*
 * return 1 if rs232 port unknown
 * return 2 on txd/rxd failure (only rs232 2)
 * return 3 on rts/cts failure
 * return 0 if no failure
 */
int do_rs232(char * const argv[])
{
	int error_status = 0;
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	struct mpc5xxx_psc *psc1 = (struct mpc5xxx_psc *)MPC5XXX_PSC1;

	switch	(simple_strtoul(argv[2], NULL, 10)) {

	case 1:
		/* check RTS <-> CTS loop */
		/* set rts to 0 */
		psc1->op1 |= 0x01;

		/* wait some time before requesting status */
		udelay(10);

		/* check status at cts */
		if ((psc1->ip & 0x01) != 0) {
			error_status = 3;
			printf ("%s: failure at rs232_1, cts status is %d "
				"(should be 0)\n",
				__FUNCTION__, (psc1->ip & 0x01));
		}

		/* set rts to 1 */
		psc1->op0 |= 0x01;

		/* wait some time before requesting status */
		udelay(10);

		/* check status at cts */
		if ((psc1->ip & 0x01) != 1) {
			error_status = 3;
			printf ("%s: failure at rs232_1, cts status is %d "
				"(should be 1)\n",
				__FUNCTION__, (psc1->ip & 0x01));
		}

		break;

	case 2:
		/* set PSC3_0, PSC3_2 as output and PSC3_1, PSC3_3 as input */
		gpio->simple_ddr &= ~(0x00000F00);
		gpio->simple_ddr |=   0x00000500;

		/* check TXD <-> RXD loop */
		/* set TXD to 1 */
		gpio->simple_dvo |=   (1 << 8);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000200) != 0x00000200) {
			error_status = 2;
			printf ("%s: failure at rs232_2, rxd status is %d "
				"(should be 1)\n",
				__FUNCTION__,
				(gpio->simple_ival & 0x00000200) >> 9);
		}

		/* set TXD to 0 */
		gpio->simple_dvo &= ~(1 << 8);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000200) != 0x00000000) {
			error_status = 2;
			printf ("%s: failure at rs232_2, rxd status is %d "
				"(should be 0)\n",
				__FUNCTION__,
				(gpio->simple_ival & 0x00000200) >> 9);
		}

		/* check RTS <-> CTS loop */
		/* set RTS to 1 */
		gpio->simple_dvo |=   (1 << 10);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000800) != 0x00000800) {
			error_status = 3;
			printf ("%s: failure at rs232_2, cts status is %d "
				"(should be 1)\n",
				__FUNCTION__,
				(gpio->simple_ival & 0x00000800) >> 11);
		}

		/* set RTS to 0 */
		gpio->simple_dvo &= ~(1 << 10);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000800) != 0x00000000) {
			error_status = 3;
			printf ("%s: failure at rs232_2, cts status is %d "
				"(should be 0)\n",
				__FUNCTION__,
				(gpio->simple_ival & 0x00000800) >> 11);
		}

		/* set PSC3_0, PSC3_1, PSC3_2 and PSC3_3 as output */
		gpio->simple_ddr &= ~(0x00000F00);
		gpio->simple_ddr |=   0x00000F00;
		break;

	default:
		printf ("%s: invalid rs232 number %s\n", __FUNCTION__, argv[2]);
		error_status = 1;
		break;
	}

	return error_status;
}

#if !defined(CONFIG_FO300) && !defined(CONFIG_TQM5200S)
static void sm501_backlight (unsigned int state)
{
	if (state == BL_ON) {
		*(vu_long *)(SM501_MMIO_BASE+SM501_PANEL_DISPLAY_CONTROL) |=
			(1 << 26) | (1 << 27);
	} else if (state == BL_OFF)
		*(vu_long *)(SM501_MMIO_BASE+SM501_PANEL_DISPLAY_CONTROL) &=
			~((1 << 26) | (1 << 27));
}
#endif /* !CONFIG_FO300 & !CONFIG_TQM5200S */

int cmd_fkt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rcode;

#ifdef CONFIG_STK52XX_REV100
	printf ("Revision 100 of STK52XX not supported!\n");
	return 1;
#endif
#if defined(CONFIG_STK52XX)
	led_init();
#endif
	can_init();

	switch (argc) {

	case 0:
	case 1:
		break;

	case 2:
		if (strncmp (argv[1], "can", 3) == 0) {
			rcode = do_can (argv);
			if (rcode == 0)
				printf ("OK\n");
			else
				printf ("Error\n");
			return rcode;
		}
		break;

	case 3:
		if (strncmp (argv[1], "rs232", 3) == 0) {
			rcode = do_rs232 (argv);
			if (rcode == 0)
				printf ("OK\n");
			else
				printf ("Error\n");
			return rcode;
#if !defined(CONFIG_FO300) && !defined(CONFIG_TQM5200S)
		} else if (strncmp (argv[1], "backlight", 4) == 0) {
			if (strncmp (argv[2], "on", 2) == 0) {
				sm501_backlight (BL_ON);
				return 0;
			}
			else if (strncmp (argv[2], "off", 3) == 0) {
				sm501_backlight (BL_OFF);
				return 0;
			}
#endif /* !CONFIG_FO300 & !CONFIG_TQM5200S */
		}
		break;

#if defined(CONFIG_STK52XX)
	case 4:
		if (strcmp (argv[1], "led") == 0) {
			return (do_led (argv));
		}
		break;
#endif

	default:
		break;
	}

	printf ("Usage:\nfkt cmd [arg1] [arg2] ...\n");
	return 1;
}


U_BOOT_CMD(
	sound ,    5,    1,     cmd_sound,
	"Sound sub-system",
	"saw [duration] [freq] [channel]\n"
	"    - generate sawtooth for 'duration' ms with frequency 'freq'\n"
	"      on left \"l\" or right \"r\" channel\n"
	"sound square [duration] [freq] [channel]\n"
	"    - generate squarewave for 'duration' ms with frequency 'freq'\n"
	"      on left \"l\" or right \"r\" channel\n"
	"pcm1772 reg val"
);

U_BOOT_CMD(
	wav ,    3,    1,     cmd_wav,
	"play wav file",
	"[addr] [bytes]\n"
	"    - play wav file at address 'addr' with length 'bytes'"
);

U_BOOT_CMD(
	beep ,    2,    1,     cmd_beep,
	"play short beep",
	"[channel]\n"
	"    - play short beep on \"l\"eft or \"r\"ight channel"
);
#endif /* CONFIG_STK52XX  || CONFIG_FO300 */

#if defined(CONFIG_STK52XX)
U_BOOT_CMD(
	fkt ,	4,	1,	cmd_fkt,
	"Function test routines",
	"led number on/off\n"
	"     - 'number's like printed on STK52XX board\n"
	"fkt can\n"
	"     - loopback plug for X83 required\n"
	"fkt rs232 number\n"
	"     - loopback plug(s) for X2 required"
#ifndef CONFIG_TQM5200S
	"\n"
	"fkt backlight on/off\n"
	"     - switch backlight on or off"
#endif /* !CONFIG_TQM5200S */
);
#elif defined(CONFIG_FO300)
U_BOOT_CMD(
	fkt ,	3,	1,	cmd_fkt,
	"Function test routines",
	"fkt can\n"
	"     - loopback plug for X16/X29 required\n"
	"fkt rs232 number\n"
	"     - loopback plug(s) for X21/X22 required"
);
#endif
#endif
