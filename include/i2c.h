/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
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
 * The original I2C interface was
 *   (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
 *   AIRVENT SAM s.p.a - RIMINI(ITALY)
 * but has been changed substantially.
 */

#ifndef _I2C_H_
#define _I2C_H_

/*
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 * The implementation MUST NOT use static or global variables if the
 * I2C routines are used to read SDRAM configuration information
 * because this is done before the memories are initialized. Limited
 * use of stack-based variables are OK (the initial stack size is
 * limited).
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 */

/*
 * Configuration items.
 */
#define I2C_RXTX_LEN	128	/* maximum tx/rx buffer length */

#ifdef	CONFIG_I2C_MULTI_BUS
#define	MAX_I2C_BUS			2
#define	I2C_MULTI_BUS			1
#else
#define	MAX_I2C_BUS			1
#define	I2C_MULTI_BUS			0
#endif

#if !defined(CONFIG_SYS_MAX_I2C_BUS)
#define CONFIG_SYS_MAX_I2C_BUS		MAX_I2C_BUS
#endif

/* define the I2C bus number for RTC and DTT if not already done */
#if !defined(CONFIG_SYS_RTC_BUS_NUM)
#define CONFIG_SYS_RTC_BUS_NUM		0
#endif
#if !defined(CONFIG_SYS_DTT_BUS_NUM)
#define CONFIG_SYS_DTT_BUS_NUM		0
#endif
#if !defined(CONFIG_SYS_SPD_BUS_NUM)
#define CONFIG_SYS_SPD_BUS_NUM		0
#endif

#ifndef I2C_SOFT_DECLARATIONS
# if defined(CONFIG_MPC8260)
#  define I2C_SOFT_DECLARATIONS volatile ioport_t *iop = ioport_addr((immap_t *)CONFIG_SYS_IMMR, I2C_PORT);
# elif defined(CONFIG_8xx)
#  define I2C_SOFT_DECLARATIONS	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

# elif (defined(CONFIG_AT91RM9200) || \
	defined(CONFIG_AT91SAM9260) ||  defined(CONFIG_AT91SAM9261) || \
	defined(CONFIG_AT91SAM9263)) && !defined(CONFIG_AT91_LEGACY)
#  define I2C_SOFT_DECLARATIONS	at91_pio_t *pio	= (at91_pio_t *) ATMEL_BASE_PIOA;
# else
#  define I2C_SOFT_DECLARATIONS
# endif
#endif

#ifdef CONFIG_8xx
/* Set default value for the I2C bus speed on 8xx. In the
 * future, we'll define these in all 8xx board config files.
 */
#ifndef	CONFIG_SYS_I2C_SPEED
#define	CONFIG_SYS_I2C_SPEED	50000
#endif
#endif

/*
 * Many boards/controllers/drivers don't support an I2C slave interface so
 * provide a default slave address for them for use in common code.  A real
 * value for CONFIG_SYS_I2C_SLAVE should be defined for any board which does
 * support a slave interface.
 */
#ifndef	CONFIG_SYS_I2C_SLAVE
#define	CONFIG_SYS_I2C_SLAVE	0xfe
#endif

/*
 * Initialization, must be called once on start up, may be called
 * repeatedly to change the speed and slave addresses.
 */
void i2c_init(int speed, int slaveaddr);
void i2c_init_board(void);
#ifdef CONFIG_SYS_I2C_BOARD_LATE_INIT
void i2c_board_late_init(void);
#endif

#if defined(CONFIG_I2C_MUX)

typedef struct _mux {
	uchar	chip;
	uchar	channel;
	char	*name;
	struct _mux	*next;
} I2C_MUX;

typedef struct _mux_device {
	int	busid;
	I2C_MUX	*mux;	/* List of muxes, to reach the device */
	struct _mux_device	*next;
} I2C_MUX_DEVICE;

I2C_MUX_DEVICE	*i2c_mux_search_device(int id);
I2C_MUX_DEVICE *i2c_mux_ident_muxstring (uchar *buf);
int i2x_mux_select_mux(int bus);
int i2c_mux_ident_muxstring_f (uchar *buf);
#endif

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uchar chip);

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);

/*
 * Utility routines to read/write registers.
 */
static inline u8 i2c_reg_read(u8 addr, u8 reg)
{
	u8 buf;

#ifdef CONFIG_8xx
	/* MPC8xx needs this.  Maybe one day we can get rid of it. */
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

#ifdef DEBUG
	printf("%s: addr=0x%02x, reg=0x%02x\n", __func__, addr, reg);
#endif

	i2c_read(addr, reg, 1, &buf, 1);

	return buf;
}

static inline void i2c_reg_write(u8 addr, u8 reg, u8 val)
{
#ifdef CONFIG_8xx
	/* MPC8xx needs this.  Maybe one day we can get rid of it. */
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
#endif

#ifdef DEBUG
	printf("%s: addr=0x%02x, reg=0x%02x, val=0x%02x\n",
	       __func__, addr, reg, val);
#endif

	i2c_write(addr, reg, 1, &val, 1);
}

/*
 * Functions for setting the current I2C bus and its speed
 */

/*
 * i2c_set_bus_num:
 *
 *  Change the active I2C bus.  Subsequent read/write calls will
 *  go to this one.
 *
 *	bus - bus index, zero based
 *
 *	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_num(unsigned int bus);

/*
 * i2c_get_bus_num:
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */

unsigned int i2c_get_bus_num(void);

/*
 * i2c_set_bus_speed:
 *
 *  Change the speed of the active I2C bus
 *
 *	speed - bus speed in Hz
 *
 *	Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_speed(unsigned int);

/*
 * i2c_get_bus_speed:
 *
 *  Returns speed of currently active I2C bus in Hz
 */

unsigned int i2c_get_bus_speed(void);

/* NOTE: These two functions MUST be always_inline to avoid code growth! */
static inline unsigned int I2C_GET_BUS(void) __attribute__((always_inline));
static inline unsigned int I2C_GET_BUS(void)
{
	return I2C_MULTI_BUS ? i2c_get_bus_num() : 0;
}

static inline void I2C_SET_BUS(unsigned int bus) __attribute__((always_inline));
static inline void I2C_SET_BUS(unsigned int bus)
{
	if (I2C_MULTI_BUS)
		i2c_set_bus_num(bus);
}

#endif	/* _I2C_H_ */
