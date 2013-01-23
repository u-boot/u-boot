/*
 * (C) Copyright 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 *
 * This has been changed substantially by Gerald Van Baren, Custom IDEAS,
 * vanbaren@cideas.com.  It was heavily influenced by LiMon, written by
 * Neil Russell.
 */

#include <common.h>
#ifdef	CONFIG_MPC8260			/* only valid for MPC8260 */
#include <ioports.h>
#include <asm/io.h>
#endif
#if defined(CONFIG_AVR32)
#include <asm/arch/portmux.h>
#endif
#if defined(CONFIG_AT91FAMILY)
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pio.h>
#ifdef CONFIG_AT91_LEGACY
#include <asm/arch/gpio.h>
#endif
#endif
#ifdef	CONFIG_IXP425			/* only valid for IXP425 */
#include <asm/arch/ixp425.h>
#endif
#if defined(CONFIG_MPC852T) || defined(CONFIG_MPC866)
#include <asm/io.h>
#endif
#include <i2c.h>

#if defined(CONFIG_SOFT_I2C_GPIO_SCL)
# include <asm/gpio.h>

# ifndef I2C_GPIO_SYNC
#  define I2C_GPIO_SYNC
# endif

# ifndef I2C_INIT
#  define I2C_INIT \
	do { \
		gpio_request(CONFIG_SOFT_I2C_GPIO_SCL, "soft_i2c"); \
		gpio_request(CONFIG_SOFT_I2C_GPIO_SDA, "soft_i2c"); \
	} while (0)
# endif

# ifndef I2C_ACTIVE
#  define I2C_ACTIVE do { } while (0)
# endif

# ifndef I2C_TRISTATE
#  define I2C_TRISTATE do { } while (0)
# endif

# ifndef I2C_READ
#  define I2C_READ gpio_get_value(CONFIG_SOFT_I2C_GPIO_SDA)
# endif

# ifndef I2C_SDA
#  define I2C_SDA(bit) \
	do { \
		if (bit) \
			gpio_direction_input(CONFIG_SOFT_I2C_GPIO_SDA); \
		else \
			gpio_direction_output(CONFIG_SOFT_I2C_GPIO_SDA, 0); \
		I2C_GPIO_SYNC; \
	} while (0)
# endif

# ifndef I2C_SCL
#  define I2C_SCL(bit) \
	do { \
		gpio_direction_output(CONFIG_SOFT_I2C_GPIO_SCL, bit); \
		I2C_GPIO_SYNC; \
	} while (0)
# endif

# ifndef I2C_DELAY
#  define I2C_DELAY udelay(5)	/* 1/4 I2C clock duration */
# endif

#endif

/* #define	DEBUG_I2C	*/

#ifdef DEBUG_I2C
DECLARE_GLOBAL_DATA_PTR;
#endif

/*-----------------------------------------------------------------------
 * Definitions
 */

#define RETRIES		0

#define I2C_ACK		0		/* PD_SDA level to ack a byte */
#define I2C_NOACK	1		/* PD_SDA level to noack a byte */


#ifdef DEBUG_I2C
#define PRINTD(fmt,args...)	do {	\
		printf (fmt ,##args);	\
	} while (0)
#else
#define PRINTD(fmt,args...)
#endif

#if defined(CONFIG_I2C_MULTI_BUS)
static unsigned int i2c_bus_num __attribute__ ((section (".data"))) = 0;
#endif /* CONFIG_I2C_MULTI_BUS */

/*-----------------------------------------------------------------------
 * Local functions
 */
#if !defined(CONFIG_SYS_I2C_INIT_BOARD)
static void  send_reset	(void);
#endif
static void  send_start	(void);
static void  send_stop	(void);
static void  send_ack	(int);
static int   write_byte	(uchar byte);
static uchar read_byte	(int);

#if !defined(CONFIG_SYS_I2C_INIT_BOARD)
/*-----------------------------------------------------------------------
 * Send a reset sequence consisting of 9 clocks with the data signal high
 * to clock any confused device back into an idle state.  Also send a
 * <stop> at the end of the sequence for belts & suspenders.
 */
static void send_reset(void)
{
	I2C_SOFT_DECLARATIONS	/* intentional without ';' */
	int j;

	I2C_SCL(1);
	I2C_SDA(1);
#ifdef	I2C_INIT
	I2C_INIT;
#endif
	I2C_TRISTATE;
	for(j = 0; j < 9; j++) {
		I2C_SCL(0);
		I2C_DELAY;
		I2C_DELAY;
		I2C_SCL(1);
		I2C_DELAY;
		I2C_DELAY;
	}
	send_stop();
	I2C_TRISTATE;
}
#endif

/*-----------------------------------------------------------------------
 * START: High -> Low on SDA while SCL is High
 */
static void send_start(void)
{
	I2C_SOFT_DECLARATIONS	/* intentional without ';' */

	I2C_DELAY;
	I2C_SDA(1);
	I2C_ACTIVE;
	I2C_DELAY;
	I2C_SCL(1);
	I2C_DELAY;
	I2C_SDA(0);
	I2C_DELAY;
}

/*-----------------------------------------------------------------------
 * STOP: Low -> High on SDA while SCL is High
 */
static void send_stop(void)
{
	I2C_SOFT_DECLARATIONS	/* intentional without ';' */

	I2C_SCL(0);
	I2C_DELAY;
	I2C_SDA(0);
	I2C_ACTIVE;
	I2C_DELAY;
	I2C_SCL(1);
	I2C_DELAY;
	I2C_SDA(1);
	I2C_DELAY;
	I2C_TRISTATE;
}

/*-----------------------------------------------------------------------
 * ack should be I2C_ACK or I2C_NOACK
 */
static void send_ack(int ack)
{
	I2C_SOFT_DECLARATIONS	/* intentional without ';' */

	I2C_SCL(0);
	I2C_DELAY;
	I2C_ACTIVE;
	I2C_SDA(ack);
	I2C_DELAY;
	I2C_SCL(1);
	I2C_DELAY;
	I2C_DELAY;
	I2C_SCL(0);
	I2C_DELAY;
}

/*-----------------------------------------------------------------------
 * Send 8 bits and look for an acknowledgement.
 */
static int write_byte(uchar data)
{
	I2C_SOFT_DECLARATIONS	/* intentional without ';' */
	int j;
	int nack;

	I2C_ACTIVE;
	for(j = 0; j < 8; j++) {
		I2C_SCL(0);
		I2C_DELAY;
		I2C_SDA(data & 0x80);
		I2C_DELAY;
		I2C_SCL(1);
		I2C_DELAY;
		I2C_DELAY;

		data <<= 1;
	}

	/*
	 * Look for an <ACK>(negative logic) and return it.
	 */
	I2C_SCL(0);
	I2C_DELAY;
	I2C_SDA(1);
	I2C_TRISTATE;
	I2C_DELAY;
	I2C_SCL(1);
	I2C_DELAY;
	I2C_DELAY;
	nack = I2C_READ;
	I2C_SCL(0);
	I2C_DELAY;
	I2C_ACTIVE;

	return(nack);	/* not a nack is an ack */
}

#if defined(CONFIG_I2C_MULTI_BUS)
/*
 * Functions for multiple I2C bus handling
 */
unsigned int i2c_get_bus_num(void)
{
	return i2c_bus_num;
}

int i2c_set_bus_num(unsigned int bus)
{
#if defined(CONFIG_I2C_MUX)
	if (bus < CONFIG_SYS_MAX_I2C_BUS) {
		i2c_bus_num = bus;
	} else {
		int	ret;

		ret = i2x_mux_select_mux(bus);
		i2c_init_board();
		if (ret == 0)
			i2c_bus_num = bus;
		else
			return ret;
	}
#else
	if (bus >= CONFIG_SYS_MAX_I2C_BUS)
		return -1;
	i2c_bus_num = bus;
#endif
	return 0;
}
#endif

/*-----------------------------------------------------------------------
 * if ack == I2C_ACK, ACK the byte so can continue reading, else
 * send I2C_NOACK to end the read.
 */
static uchar read_byte(int ack)
{
	I2C_SOFT_DECLARATIONS	/* intentional without ';' */
	int  data;
	int  j;

	/*
	 * Read 8 bits, MSB first.
	 */
	I2C_TRISTATE;
	I2C_SDA(1);
	data = 0;
	for(j = 0; j < 8; j++) {
		I2C_SCL(0);
		I2C_DELAY;
		I2C_SCL(1);
		I2C_DELAY;
		data <<= 1;
		data |= I2C_READ;
		I2C_DELAY;
	}
	send_ack(ack);

	return(data);
}

/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/

/*-----------------------------------------------------------------------
 * Initialization
 */
void i2c_init (int speed, int slaveaddr)
{
#if defined(CONFIG_SYS_I2C_INIT_BOARD)
	/* call board specific i2c bus reset routine before accessing the   */
	/* environment, which might be in a chip on that bus. For details   */
	/* about this problem see doc/I2C_Edge_Conditions.                  */
	i2c_init_board();
#else
	/*
	 * WARNING: Do NOT save speed in a static variable: if the
	 * I2C routines are called before RAM is initialized (to read
	 * the DIMM SPD, for instance), RAM won't be usable and your
	 * system will crash.
	 */
	send_reset ();
#endif
}

/*-----------------------------------------------------------------------
 * Probe to see if a chip is present.  Also good for checking for the
 * completion of EEPROM writes since the chip stops responding until
 * the write completes (typically 10mSec).
 */
int i2c_probe(uchar addr)
{
	int rc;

	/*
	 * perform 1 byte write transaction with just address byte
	 * (fake write)
	 */
	send_start();
	rc = write_byte ((addr << 1) | 0);
	send_stop();

	return (rc ? 1 : 0);
}

/*-----------------------------------------------------------------------
 * Read bytes
 */
int  i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int shift;
	PRINTD("i2c_read: chip %02X addr %02X alen %d buffer %p len %d\n",
		chip, addr, alen, buffer, len);

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones
	 * like Catalyst 24WC04/08/16 which has 9/10/11 bits of
	 * address and the extra bits end up in the "chip address"
	 * bit slots. This makes a 24WC08 (1Kbyte) chip look like
	 * four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to
	 * still be one byte because the extra address bits are
	 * hidden in the chip address.
	 */
	chip |= ((addr >> (alen * 8)) & CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);

	PRINTD("i2c_read: fix addr_overflow: chip %02X addr %02X\n",
		chip, addr);
#endif

	/*
	 * Do the addressing portion of a write cycle to set the
	 * chip's address pointer.  If the address length is zero,
	 * don't do the normal write cycle to set the address pointer,
	 * there is no address pointer in this chip.
	 */
	send_start();
	if(alen > 0) {
		if(write_byte(chip << 1)) {	/* write cycle */
			send_stop();
			PRINTD("i2c_read, no chip responded %02X\n", chip);
			return(1);
		}
		shift = (alen-1) * 8;
		while(alen-- > 0) {
			if(write_byte(addr >> shift)) {
				PRINTD("i2c_read, address not <ACK>ed\n");
				return(1);
			}
			shift -= 8;
		}

		/* Some I2C chips need a stop/start sequence here,
		 * other chips don't work with a full stop and need
		 * only a start.  Default behaviour is to send the
		 * stop/start sequence.
		 */
#ifdef CONFIG_SOFT_I2C_READ_REPEATED_START
		send_start();
#else
		send_stop();
		send_start();
#endif
	}
	/*
	 * Send the chip address again, this time for a read cycle.
	 * Then read the data.  On the last byte, we do a NACK instead
	 * of an ACK(len == 0) to terminate the read.
	 */
	write_byte((chip << 1) | 1);	/* read cycle */
	while(len-- > 0) {
		*buffer++ = read_byte(len == 0);
	}
	send_stop();
	return(0);
}

/*-----------------------------------------------------------------------
 * Write bytes
 */
int  i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int shift, failures = 0;

	PRINTD("i2c_write: chip %02X addr %02X alen %d buffer %p len %d\n",
		chip, addr, alen, buffer, len);

	send_start();
	if(write_byte(chip << 1)) {	/* write cycle */
		send_stop();
		PRINTD("i2c_write, no chip responded %02X\n", chip);
		return(1);
	}
	shift = (alen-1) * 8;
	while(alen-- > 0) {
		if(write_byte(addr >> shift)) {
			PRINTD("i2c_write, address not <ACK>ed\n");
			return(1);
		}
		shift -= 8;
	}

	while(len-- > 0) {
		if(write_byte(*buffer++)) {
			failures++;
		}
	}
	send_stop();
	return(failures);
}
