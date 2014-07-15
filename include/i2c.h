/*
 * Copyright (C) 2009 Sergey Kubushyn <ksi@koi8.net>
 * Copyright (C) 2009 - 2013 Heiko Schocher <hs@denx.de>
 * Changes for multibus/multiadapter I2C support.
 *
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

#if !defined(CONFIG_SYS_I2C_MAX_HOPS)
/* no muxes used bus = i2c adapters */
#define CONFIG_SYS_I2C_DIRECT_BUS	1
#define CONFIG_SYS_I2C_MAX_HOPS		0
#define CONFIG_SYS_NUM_I2C_BUSES	ll_entry_count(struct i2c_adapter, i2c)
#else
/* we use i2c muxes */
#undef CONFIG_SYS_I2C_DIRECT_BUS
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

struct i2c_adapter {
	void		(*init)(struct i2c_adapter *adap, int speed,
				int slaveaddr);
	int		(*probe)(struct i2c_adapter *adap, uint8_t chip);
	int		(*read)(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len);
	int		(*write)(struct i2c_adapter *adap, uint8_t chip,
				uint addr, int alen, uint8_t *buffer,
				int len);
	uint		(*set_bus_speed)(struct i2c_adapter *adap,
				uint speed);
	int		speed;
	int		waitdelay;
	int		slaveaddr;
	int		init_done;
	int		hwadapnr;
	char		*name;
};

#define U_BOOT_I2C_MKENT_COMPLETE(_init, _probe, _read, _write, \
		_set_speed, _speed, _slaveaddr, _hwadapnr, _name) \
	{ \
		.init		=	_init, \
		.probe		=	_probe, \
		.read		=	_read, \
		.write		=	_write, \
		.set_bus_speed	=	_set_speed, \
		.speed		=	_speed, \
		.slaveaddr	=	_slaveaddr, \
		.init_done	=	0, \
		.hwadapnr	=	_hwadapnr, \
		.name		=	#_name \
};

#define U_BOOT_I2C_ADAP_COMPLETE(_name, _init, _probe, _read, _write, \
			_set_speed, _speed, _slaveaddr, _hwadapnr) \
	ll_entry_declare(struct i2c_adapter, _name, i2c) = \
	U_BOOT_I2C_MKENT_COMPLETE(_init, _probe, _read, _write, \
		 _set_speed, _speed, _slaveaddr, _hwadapnr, _name);

struct i2c_adapter *i2c_get_adapter(int index);

#ifndef CONFIG_SYS_I2C_DIRECT_BUS
struct i2c_mux {
	int	id;
	char	name[16];
};

struct i2c_next_hop {
	struct i2c_mux		mux;
	uint8_t		chip;
	uint8_t		channel;
};

struct i2c_bus_hose {
	int	adapter;
	struct i2c_next_hop	next_hop[CONFIG_SYS_I2C_MAX_HOPS];
};
#define I2C_NULL_HOP	{{-1, ""}, 0, 0}
extern struct i2c_bus_hose	i2c_bus[];

#define I2C_ADAPTER(bus)	i2c_bus[bus].adapter
#else
#define I2C_ADAPTER(bus)	bus
#endif
#define	I2C_BUS			gd->cur_i2c_bus

#define	I2C_ADAP_NR(bus)	i2c_get_adapter(I2C_ADAPTER(bus))
#define	I2C_ADAP		I2C_ADAP_NR(gd->cur_i2c_bus)
#define I2C_ADAP_HWNR		(I2C_ADAP->hwadapnr)

#ifndef CONFIG_SYS_I2C_DIRECT_BUS
#define I2C_MUX_PCA9540_ID	1
#define I2C_MUX_PCA9540		{I2C_MUX_PCA9540_ID, "PCA9540B"}
#define I2C_MUX_PCA9542_ID	2
#define I2C_MUX_PCA9542		{I2C_MUX_PCA9542_ID, "PCA9542A"}
#define I2C_MUX_PCA9544_ID	3
#define I2C_MUX_PCA9544		{I2C_MUX_PCA9544_ID, "PCA9544A"}
#define I2C_MUX_PCA9547_ID	4
#define I2C_MUX_PCA9547		{I2C_MUX_PCA9547_ID, "PCA9547A"}
#define I2C_MUX_PCA9548_ID	5
#define I2C_MUX_PCA9548		{I2C_MUX_PCA9548_ID, "PCA9548"}
#endif

#ifndef I2C_SOFT_DECLARATIONS
# if defined(CONFIG_MPC8260)
#  define I2C_SOFT_DECLARATIONS volatile ioport_t *iop = ioport_addr((immap_t *)CONFIG_SYS_IMMR, I2C_PORT);
# elif defined(CONFIG_8xx)
#  define I2C_SOFT_DECLARATIONS	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

# elif (defined(CONFIG_AT91RM9200) || \
	defined(CONFIG_AT91SAM9260) ||  defined(CONFIG_AT91SAM9261) || \
	defined(CONFIG_AT91SAM9263))
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

#ifdef CONFIG_SYS_I2C
/*
 * i2c_get_bus_num:
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */
unsigned int i2c_get_bus_num(void);

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
 * i2c_init_all():
 *
 * Initializes all I2C adapters in the system. All i2c_adap structures must
 * be initialized beforehead with function pointers and data, including
 * speed and slaveaddr. Returns 0 on success, non-0 on failure.
 */
void i2c_init_all(void);

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uint8_t chip);

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
int i2c_read(uint8_t chip, unsigned int addr, int alen,
				uint8_t *buffer, int len);

int i2c_write(uint8_t chip, unsigned int addr, int alen,
				uint8_t *buffer, int len);

/*
 * Utility routines to read/write registers.
 */
uint8_t i2c_reg_read(uint8_t addr, uint8_t reg);

void i2c_reg_write(uint8_t addr, uint8_t reg, uint8_t val);

/*
 * i2c_set_bus_speed:
 *
 *  Change the speed of the active I2C bus
 *
 *	speed - bus speed in Hz
 *
 *	Returns: new bus speed
 *
 */
unsigned int i2c_set_bus_speed(unsigned int speed);

/*
 * i2c_get_bus_speed:
 *
 *  Returns speed of currently active I2C bus in Hz
 */

unsigned int i2c_get_bus_speed(void);

/*
 * i2c_reloc_fixup:
 *
 * Adjusts I2C pointers after U-Boot is relocated to DRAM
 */
void i2c_reloc_fixup(void);
#if defined(CONFIG_SYS_I2C_SOFT)
void i2c_soft_init(void);
void i2c_soft_active(void);
void i2c_soft_tristate(void);
int i2c_soft_read(void);
void i2c_soft_sda(int bit);
void i2c_soft_scl(int bit);
void i2c_soft_delay(void);
#endif
#else

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
#endif /* CONFIG_SYS_I2C */

/*
 * only for backwardcompatibility, should go away if we switched
 * completely to new multibus support.
 */
#if defined(CONFIG_SYS_I2C) || defined(CONFIG_I2C_MULTI_BUS)
# if !defined(CONFIG_SYS_MAX_I2C_BUS)
#  define CONFIG_SYS_MAX_I2C_BUS		2
# endif
# define I2C_MULTI_BUS				1
#else
# define CONFIG_SYS_MAX_I2C_BUS		1
# define I2C_MULTI_BUS				0
#endif

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

/* Multi I2C definitions */
enum {
	I2C_0, I2C_1, I2C_2, I2C_3, I2C_4, I2C_5, I2C_6, I2C_7,
	I2C_8, I2C_9, I2C_10,
};

/* Multi I2C busses handling */
#ifdef CONFIG_SOFT_I2C_MULTI_BUS
extern int get_multi_scl_pin(void);
extern int get_multi_sda_pin(void);
extern int multi_i2c_init(void);
#endif

/**
 * Get FDT values for i2c bus.
 *
 * @param blob  Device tree blbo
 * @return the number of I2C bus
 */
void board_i2c_init(const void *blob);

/**
 * Find the I2C bus number by given a FDT I2C node.
 *
 * @param blob  Device tree blbo
 * @param node  FDT I2C node to find
 * @return the number of I2C bus (zero based), or -1 on error
 */
int i2c_get_bus_num_fdt(int node);

/**
 * Reset the I2C bus represented by the given a FDT I2C node.
 *
 * @param blob  Device tree blbo
 * @param node  FDT I2C node to find
 * @return 0 if port was reset, -1 if not found
 */
int i2c_reset_port_fdt(const void *blob, int node);
#endif	/* _I2C_H_ */
