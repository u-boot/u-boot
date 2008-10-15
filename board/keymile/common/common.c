/*
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
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

#include <common.h>
#include <mpc8260.h>
#include <ioports.h>
#include <malloc.h>

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif

#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
#include <i2c.h>
#endif

extern int i2c_soft_read_pin (void);

#if defined(CFG_I2C_INIT_BOARD)
#define DELAY_ABORT_SEQ		62
#define DELAY_HALF_PERIOD	(500 / (CFG_I2C_SPEED / 1000))

#if defined(CONFIG_MGCOGE)
#define SDA_MASK	0x00010000
#define SCL_MASK	0x00020000
static void set_pin (int state, unsigned long mask)
{
	volatile ioport_t *iop = ioport_addr ((immap_t *)CFG_IMMR, 3);

	if (state)
		iop->pdat |= (mask);
	else
		iop->pdat &= ~(mask);

	iop->pdir |= (mask);
}

static int get_pin (unsigned long mask)
{
	volatile ioport_t *iop = ioport_addr ((immap_t *)CFG_IMMR, 3);

	iop->pdir &= ~(mask);
	return (0 != (iop->pdat & (mask)));
}

static void set_sda (int state)
{
	set_pin (state, SDA_MASK);
}

static void set_scl (int state)
{
	set_pin (state, SCL_MASK);
}

static int get_sda (void)
{
	return get_pin (SDA_MASK);
}

static int get_scl (void)
{
	return get_pin (SCL_MASK);
}

#if defined(CONFIG_HARD_I2C)
static void setports (int gpio)
{
	volatile ioport_t *iop = ioport_addr ((immap_t *)CFG_IMMR, 3);

	if (gpio) {
		iop->ppar &= ~(SDA_MASK | SCL_MASK);
		iop->podr &= ~(SDA_MASK | SCL_MASK);
	} else {
		iop->ppar |= (SDA_MASK | SCL_MASK);
		iop->pdir &= ~(SDA_MASK | SCL_MASK);
		iop->podr |= (SDA_MASK | SCL_MASK);
	}
}
#endif
#endif

#if defined(CONFIG_MGSUVD)
static void set_sda (int state)
{
	I2C_SDA(state);
}

static void set_scl (int state)
{
	I2C_SCL(state);
}

static int get_sda (void)
{
	return i2c_soft_read_pin ();
}

static int get_scl (void)
{
	int	val;

	*(unsigned short *)(I2C_BASE_DIR) &=  ~SCL_CONF;
	udelay (1);
	val = *(unsigned char *)(I2C_BASE_PORT);

	return ((val & SCL_BIT) == SCL_BIT);
}

#endif

static void writeStartSeq (void)
{
	set_sda (1);
	udelay (DELAY_HALF_PERIOD);
	set_scl (1);
	udelay (DELAY_HALF_PERIOD);
	set_sda (0);
	udelay (DELAY_HALF_PERIOD);
	set_scl (0);
	udelay (DELAY_HALF_PERIOD);
}

/* I2C is a synchronous protocol and resets of the processor in the middle
   of an access can block the I2C Bus until a powerdown of the full unit is
   done. This function toggles the SCL until the SCL and SCA line are
   released, but max. 16 times, after this a I2C start-sequence is sent.
   This I2C Deblocking mechanism was developed by Keymile in association
   with Anatech and Atmel in 1998.
 */
static int i2c_make_abort (void)
{
	int	scl_state = 0;
	int	sda_state = 0;
	int	i = 0;
	int	ret = 0;

	if (!get_sda ()) {
		ret = -1;
		while (i < 16) {
			i++;
			set_scl (0);
			udelay (DELAY_ABORT_SEQ);
			set_scl (1);
			udelay (DELAY_ABORT_SEQ);
			scl_state = get_scl ();
			sda_state = get_sda ();
			if (scl_state && sda_state) {
				ret = 0;
				break;
			}
		}
	}
	if (ret == 0) {
		for (i =0; i < 5; i++) {
			writeStartSeq ();
		}
	}
	get_sda ();
	return ret;
}

/**
 * i2c_init_board - reset i2c bus. When the board is powercycled during a
 * bus transfer it might hang; for details see doc/I2C_Edge_Conditions.
 */
void i2c_init_board(void)
{
#if defined(CONFIG_HARD_I2C)
	volatile immap_t *immap = (immap_t *)CFG_IMMR ;
	volatile i2c8260_t *i2c	= (i2c8260_t *)&immap->im_i2c;

	/* disable I2C controller first, otherwhise it thinks we want to    */
	/* talk to the slave port...                                        */
	i2c->i2c_i2mod &= ~0x01;

	/* Set the PortPins to GPIO */
	setports (1);
#endif

	/* Now run the AbortSequence() */
	i2c_make_abort ();

#if defined(CONFIG_HARD_I2C)
	/* Set the PortPins back to use for I2C */
	setports (0);
#endif
}
#endif
