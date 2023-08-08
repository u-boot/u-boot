// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <i2c.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/clock.h>

#if defined(CONFIG_ARCH_OCTEONTX)
# define TWSI_THP		24
#else
# define TWSI_THP		3
#endif

/**
 * Slave address to use for Thunder when accessed by another master
 */
#ifndef CONFIG_SYS_I2C_OCTEONTX_SLAVE_ADDR
# define CONFIG_SYS_I2C_OCTEONTX_SLAVE_ADDR	0x77
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_0
# ifdef CONFIG_SYS_I2C_SPEED
#  define CONFIG_SYS_I2C_OCTEONTX_SPEED_0	CONFIG_SYS_I2C_SPEED
# else
#  define CONFIG_SYS_I2C_SPEED			100000
#  define CONFIG_SYS_I2C_OCTEONTX_SPEED_0	CONFIG_SYS_I2C_SPEED
# endif
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_1
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_1	CONFIG_SYS_I2C_OCTEONTX_SPEED_0
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_2
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_2	CONFIG_SYS_I2C_OCTEONTX_SPEED_1
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_3
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_3	CONFIG_SYS_I2C_OCTEONTX_SPEED_2
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_4
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_4	CONFIG_SYS_I2C_OCTEONTX_SPEED_3
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_5
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_5	CONFIG_SYS_I2C_OCTEONTX_SPEED_4
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_6
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_6	CONFIG_SYS_I2C_OCTEONTX_SPEED_5
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_7
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_7	CONFIG_SYS_I2C_OCTEONTX_SPEED_6
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_8
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_8	CONFIG_SYS_I2C_OCTEONTX_SPEED_7
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_9
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_9	CONFIG_SYS_I2C_OCTEONTX_SPEED_8
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_10
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_10	CONFIG_SYS_I2C_OCTEONTX_SPEED_9
#endif

#ifndef CONFIG_SYS_I2C_OCTEONTX_SPEED_11
# define CONFIG_SYS_I2C_OCTEONTX_SPEED_11	CONFIG_SYS_I2C_OCTEONTX_SPEED_10
#endif

#define TWSI_SW_TWSI		0x1000
#define TWSI_TWSI_SW		0x1008
#define TWSI_INT		0x1010
#define TWSI_SW_TWSI_EXT	0x1018

union twsx_sw_twsi {
	u64 u;
	struct {
		u64 data:32;
		u64 eop_ia:3;
		u64 ia:5;
		u64 addr:10;
		u64 scr:2;
		u64 size:3;
		u64 sovr:1;
		u64 r:1;
		u64 op:4;
		u64 eia:1;
		u64 slonly:1;
		u64 v:1;
	} s;
};

union twsx_sw_twsi_ext {
	u64 u;
	struct {
		u64 data:32;
		u64 ia:8;
		u64 rsvd:24;
	} s;
};

union twsx_int {
	u64 u;
	struct {
		u64 st_int:1;	/** TWSX_SW_TWSI register update int */
		u64 ts_int:1;	/** TWSX_TWSI_SW register update int */
		u64 core_int:1;	/** TWSI core interrupt, ignored for HLC */
		u64 rsvd1:5;		/** Reserved */
		u64 sda_ovr:1;	/** SDA testing override */
		u64 scl_ovr:1;	/** SCL testing override */
		u64 sda:1;		/** SDA signal */
		u64 scl:1;		/** SCL signal */
		u64 rsvd2:52;		/** Reserved */
	} s;
};

enum {
	TWSI_OP_WRITE	= 0,
	TWSI_OP_READ	= 1,
};

enum {
	TWSI_EOP_SLAVE_ADDR = 0,
	TWSI_EOP_CLK_CTL = 3,
	TWSI_SW_EOP_IA   = 6,
};

enum {
	TWSI_SLAVEADD     = 0,
	TWSI_DATA         = 1,
	TWSI_CTL          = 2,
	TWSI_CLKCTL       = 3,
	TWSI_STAT         = 3,
	TWSI_SLAVEADD_EXT = 4,
	TWSI_RST          = 7,
};

enum {
	TWSI_CTL_AAK	= BIT(2),
	TWSI_CTL_IFLG	= BIT(3),
	TWSI_CTL_STP	= BIT(4),
	TWSI_CTL_STA	= BIT(5),
	TWSI_CTL_ENAB	= BIT(6),
	TWSI_CTL_CE	= BIT(7),
};

enum {
	/** Bus error */
	TWSI_STAT_BUS_ERROR		= 0x00,
	/** Start condition transmitted */
	TWSI_STAT_START			= 0x08,
	/** Repeat start condition transmitted */
	TWSI_STAT_RSTART		= 0x10,
	/** Address + write bit transmitted, ACK received */
	TWSI_STAT_TXADDR_ACK		= 0x18,
	/** Address + write bit transmitted, /ACK received */
	TWSI_STAT_TXADDR_NAK		= 0x20,
	/** Data byte transmitted in master mode, ACK received */
	TWSI_STAT_TXDATA_ACK		= 0x28,
	/** Data byte transmitted in master mode, ACK received */
	TWSI_STAT_TXDATA_NAK		= 0x30,
	/** Arbitration lost in address or data byte */
	TWSI_STAT_TX_ARB_LOST		= 0x38,
	/** Address + read bit transmitted, ACK received */
	TWSI_STAT_RXADDR_ACK		= 0x40,
	/** Address + read bit transmitted, /ACK received */
	TWSI_STAT_RXADDR_NAK		= 0x48,
	/** Data byte received in master mode, ACK transmitted */
	TWSI_STAT_RXDATA_ACK_SENT	= 0x50,
	/** Data byte received, NACK transmitted */
	TWSI_STAT_RXDATA_NAK_SENT	= 0x58,
	/** Slave address received, sent ACK */
	TWSI_STAT_SLAVE_RXADDR_ACK	= 0x60,
	/**
	 * Arbitration lost in address as master, slave address + write bit
	 * received, ACK transmitted
	 */
	TWSI_STAT_TX_ACK_ARB_LOST	= 0x68,
	/** General call address received, ACK transmitted */
	TWSI_STAT_RX_GEN_ADDR_ACK	= 0x70,
	/**
	 * Arbitration lost in address as master, general call address
	 * received, ACK transmitted
	 */
	TWSI_STAT_RX_GEN_ADDR_ARB_LOST	= 0x78,
	/** Data byte received after slave address received, ACK transmitted */
	TWSI_STAT_SLAVE_RXDATA_ACK	= 0x80,
	/** Data byte received after slave address received, /ACK transmitted */
	TWSI_STAT_SLAVE_RXDATA_NAK	= 0x88,
	/**
	 * Data byte received after general call address received, ACK
	 * transmitted
	 */
	TWSI_STAT_GEN_RXADDR_ACK	= 0x90,
	/**
	 * Data byte received after general call address received, /ACK
	 * transmitted
	 */
	TWSI_STAT_GEN_RXADDR_NAK	= 0x98,
	/** STOP or repeated START condition received in slave mode */
	TWSI_STAT_STOP_MULTI_START	= 0xA0,
	/** Slave address + read bit received, ACK transmitted */
	TWSI_STAT_SLAVE_RXADDR2_ACK	= 0xA8,
	/**
	 * Arbitration lost in address as master, slave address + read bit
	 * received, ACK transmitted
	 */
	TWSI_STAT_RXDATA_ACK_ARB_LOST	= 0xB0,
	/** Data byte transmitted in slave mode, ACK received */
	TWSI_STAT_SLAVE_TXDATA_ACK	= 0xB8,
	/** Data byte transmitted in slave mode, /ACK received */
	TWSI_STAT_SLAVE_TXDATA_NAK	= 0xC0,
	/** Last byte transmitted in slave mode, ACK received */
	TWSI_STAT_SLAVE_TXDATA_END_ACK	= 0xC8,
	/** Second address byte + write bit transmitted, ACK received */
	TWSI_STAT_TXADDR2DATA_ACK	= 0xD0,
	/** Second address byte + write bit transmitted, /ACK received */
	TWSI_STAT_TXADDR2DATA_NAK	= 0xD8,
	/** No relevant status information */
	TWSI_STAT_IDLE		= 0xF8
};

struct octeontx_twsi {
	int			id;
	int			speed;
	void			*baseaddr;
};

/** Array of bus speeds */
static unsigned int speeds[] = {
	CONFIG_SYS_I2C_OCTEONTX_SPEED_0,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_1,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_2,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_3,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_4,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_5,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_6,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_7,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_8,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_9,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_10,
	CONFIG_SYS_I2C_OCTEONTX_SPEED_11,
};

/** Last i2c id assigned */
static int last_id;

static void twsi_unblock(void *baseaddr);
static int twsi_stop(void *baseaddr);

/**
 * Converts the i2c status to a meaningful string
 *
 * @param	status	status to convert
 *
 * @return	string representation of the status
 */
static const char *twsi_i2c_status_str(uint8_t status)
{
	switch (status) {
	case TWSI_STAT_BUS_ERROR:
		return "Bus error";
	case TWSI_STAT_START:
		return "START condition transmitted";
	case TWSI_STAT_RSTART:
		return "Repeated START condition transmitted";
	case TWSI_STAT_TXADDR_ACK:
		return "Address + write bit transmitted, ACK received";
	case TWSI_STAT_TXADDR_NAK:
		return "Address + write bit transmitted, NAK received";
	case TWSI_STAT_TXDATA_ACK:
		return "Data byte transmitted in master mode, ACK received";
	case TWSI_STAT_TXDATA_NAK:
		return "Data byte transmitted in master mode, NAK received";
	case TWSI_STAT_TX_ARB_LOST:
		return "Arbitration lost in address or data byte";
	case TWSI_STAT_RXADDR_ACK:
		return "Address + read bit transmitted, ACK received";
	case TWSI_STAT_RXADDR_NAK:
		return "Address + read bit transmitted, NAK received";
	case TWSI_STAT_RXDATA_ACK_SENT:
		return "Data byte received in master mode, ACK transmitted";
	case TWSI_STAT_RXDATA_NAK_SENT:
		return "Data byte received in master mode, NAK transmitted";
	case TWSI_STAT_SLAVE_RXADDR_ACK:
		return "Slave address + write bit received, ACK transmitted";
	case TWSI_STAT_TX_ACK_ARB_LOST:
		return "Arbitration lost in address as master, slave address + write bit received, ACK transmitted";
	case TWSI_STAT_RX_GEN_ADDR_ACK:
		return "General call address received, ACK transmitted";
	case TWSI_STAT_RX_GEN_ADDR_ARB_LOST:
		return "Arbitration lost in address as master, general call address received, ACK transmitted";
	case TWSI_STAT_SLAVE_RXDATA_ACK:
		return "Data byte received after slave address received, ACK transmitted";
	case TWSI_STAT_SLAVE_RXDATA_NAK:
		return "Data byte received after slave address received, NAK transmitted";
	case TWSI_STAT_GEN_RXADDR_ACK:
		return "Data byte received after general call address received, ACK transmitted";
	case TWSI_STAT_GEN_RXADDR_NAK:
		return "Data byte received after general call address received, NAK transmitted";
	case TWSI_STAT_STOP_MULTI_START:
		return "STOP or repeated START condition received in slave mode";
	case TWSI_STAT_SLAVE_RXADDR2_ACK:
		return "Slave address + read bit received, ACK transmitted";
	case TWSI_STAT_RXDATA_ACK_ARB_LOST:
		return "Arbitration lost in address as master, slave address + read bit received, ACK transmitted";
	case TWSI_STAT_SLAVE_TXDATA_ACK:
		return "Data byte transmitted in slave mode, ACK received";
	case TWSI_STAT_SLAVE_TXDATA_NAK:
		return "Data byte transmitted in slave mode, NAK received";
	case TWSI_STAT_SLAVE_TXDATA_END_ACK:
		return "Last byte transmitted in slave mode, ACK received";
	case TWSI_STAT_TXADDR2DATA_ACK:
		return "Second address byte + write bit transmitted, ACK received";
	case TWSI_STAT_TXADDR2DATA_NAK:
		return "Second address byte + write bit transmitted, NAK received";
	case TWSI_STAT_IDLE:
		return "Idle";
	default:
		return "Unknown status code";
	}
}

/**
 * Returns true if we lost arbitration
 *
 * @param	code		status code
 * @param	final_read	true if this is the final read operation
 *
 * @return	true if arbitration has been lost, false if it hasn't been lost.
 */
static int twsi_i2c_lost_arb(u8 code, int final_read)
{
	switch (code) {
	/* Arbitration lost */
	case TWSI_STAT_TX_ARB_LOST:
	case TWSI_STAT_TX_ACK_ARB_LOST:
	case TWSI_STAT_RX_GEN_ADDR_ARB_LOST:
	case TWSI_STAT_RXDATA_ACK_ARB_LOST:
		return -EAGAIN;

	/* Being addressed as slave, should back off and listen */
	case TWSI_STAT_SLAVE_RXADDR_ACK:
	case TWSI_STAT_RX_GEN_ADDR_ACK:
	case TWSI_STAT_GEN_RXADDR_ACK:
	case TWSI_STAT_GEN_RXADDR_NAK:
		return -EIO;

	/* Core busy as slave */
	case TWSI_STAT_SLAVE_RXDATA_ACK:
	case TWSI_STAT_SLAVE_RXDATA_NAK:
	case TWSI_STAT_STOP_MULTI_START:
	case TWSI_STAT_SLAVE_RXADDR2_ACK:
	case TWSI_STAT_SLAVE_TXDATA_ACK:
	case TWSI_STAT_SLAVE_TXDATA_NAK:
	case TWSI_STAT_SLAVE_TXDATA_END_ACK:
		return  -EIO;

	/* Ack allowed on pre-terminal bytes only */
	case TWSI_STAT_RXDATA_ACK_SENT:
		if (!final_read)
			return 0;
		return -EAGAIN;

	/* NAK allowed on terminal byte only */
	case TWSI_STAT_RXDATA_NAK_SENT:
		if (!final_read)
			return 0;
		return -EAGAIN;

	case TWSI_STAT_TXDATA_NAK:
	case TWSI_STAT_TXADDR_NAK:
	case TWSI_STAT_RXADDR_NAK:
	case TWSI_STAT_TXADDR2DATA_NAK:
		return -EAGAIN;
	}
	return 0;
}

#define RST_BOOT_PNR_MUL(val)  (((val) >> 33) & 0x1F)

/**
 * Writes to the MIO_TWS(0..5)_SW_TWSI register
 *
 * @param	baseaddr	Base address of i2c registers
 * @param	sw_twsi		value to write
 *
 * @return	0 for success, otherwise error
 */
static u64 twsi_write_sw(void *baseaddr, union twsx_sw_twsi sw_twsi)
{
	unsigned long start = get_timer(0);

	sw_twsi.s.r = 0;
	sw_twsi.s.v = 1;

	debug("%s(%p, 0x%llx)\n", __func__, baseaddr, sw_twsi.u);
	writeq(sw_twsi.u, baseaddr + TWSI_SW_TWSI);
	do {
		sw_twsi.u = readq(baseaddr + TWSI_SW_TWSI);
	} while (sw_twsi.s.v != 0 && get_timer(start) < 50);

	if (sw_twsi.s.v)
		debug("%s: timed out\n", __func__);
	return sw_twsi.u;
}

/**
 * Reads the MIO_TWS(0..5)_SW_TWSI register
 *
 * @param	baseaddr	Base address of i2c registers
 * @param	sw_twsi		value for eia and op, etc. to read
 *
 * @return	value of the register
 */
static u64 twsi_read_sw(void *baseaddr, union twsx_sw_twsi sw_twsi)
{
	unsigned long start = get_timer(0);

	sw_twsi.s.r = 1;
	sw_twsi.s.v = 1;

	debug("%s(%p, 0x%llx)\n", __func__, baseaddr, sw_twsi.u);
	writeq(sw_twsi.u, baseaddr + TWSI_SW_TWSI);

	do {
		sw_twsi.u = readq(baseaddr + TWSI_SW_TWSI);
	} while (sw_twsi.s.v != 0 && get_timer(start) < 50);

	if (sw_twsi.s.v)
		debug("%s: Error writing 0x%llx\n", __func__, sw_twsi.u);

	debug("%s: Returning 0x%llx\n", __func__, sw_twsi.u);
	return sw_twsi.u;
}

/**
 * Write control register
 *
 * @param	baseaddr	Base address for i2c registers
 * @param	data		data to write
 */
static void twsi_write_ctl(void *baseaddr, u8 data)
{
	union twsx_sw_twsi twsi_sw;

	debug("%s(%p, 0x%x)\n", __func__, baseaddr, data);
	twsi_sw.u = 0;

	twsi_sw.s.op	 = TWSI_SW_EOP_IA;
	twsi_sw.s.eop_ia = TWSI_CTL;
	twsi_sw.s.data	 = data;

	twsi_write_sw(baseaddr, twsi_sw);
}

/**
 * Reads the TWSI Control Register
 *
 * @param[in]	baseaddr	Base address for i2c
 *
 * @return	8-bit TWSI control register
 */
static u32 twsi_read_ctl(void *baseaddr)
{
	union twsx_sw_twsi sw_twsi;

	sw_twsi.u	 = 0;
	sw_twsi.s.op	 = TWSI_SW_EOP_IA;
	sw_twsi.s.eop_ia = TWSI_CTL;

	sw_twsi.u = twsi_read_sw(baseaddr, sw_twsi);
	debug("%s(%p): 0x%x\n", __func__, baseaddr, sw_twsi.s.data);
	return sw_twsi.s.data;
}

/**
 * Read i2c status register
 *
 * @param	baseaddr	Base address of i2c registers
 *
 * @return	value of status register
 */
static u8 twsi_read_status(void *baseaddr)
{
	union twsx_sw_twsi twsi_sw;

	twsi_sw.u	= 0;
	twsi_sw.s.op	= TWSI_SW_EOP_IA;
	twsi_sw.s.eop_ia = TWSI_STAT;

	return twsi_read_sw(baseaddr, twsi_sw);
}

/**
 * Waits for an i2c operation to complete
 *
 * @param	baseaddr	Base address of registers
 *
 * @return	0 for success, 1 if timeout
 */
static int twsi_wait(void *baseaddr)
{
	unsigned long start = get_timer(0);
	u8 twsi_ctl;

	debug("%s(%p)\n", __func__, baseaddr);
	do {
		twsi_ctl = twsi_read_ctl(baseaddr);
		twsi_ctl &= TWSI_CTL_IFLG;
	} while (!twsi_ctl && get_timer(start) < 50);

	debug("  return: %u\n", !twsi_ctl);
	return !twsi_ctl;
}

/**
 * Unsticks the i2c bus
 *
 * @param	baseaddr	base address of registers
 */
static int twsi_start_unstick(void *baseaddr)
{
	twsi_stop(baseaddr);

	twsi_unblock(baseaddr);

	return 0;
}

/**
 * Sends an i2c start condition
 *
 * @param	baseaddr	base address of registers
 *
 * @return	0 for success, otherwise error
 */
static int twsi_start(void *baseaddr)
{
	int result;
	u8 stat;

	debug("%s(%p)\n", __func__, baseaddr);
	twsi_write_ctl(baseaddr, TWSI_CTL_STA | TWSI_CTL_ENAB);
	result = twsi_wait(baseaddr);
	if (result) {
		stat = twsi_read_status(baseaddr);
		debug("%s: result: 0x%x, status: 0x%x\n", __func__,
		      result, stat);
		switch (stat) {
		case TWSI_STAT_START:
		case TWSI_STAT_RSTART:
			return 0;
		case TWSI_STAT_RXADDR_ACK:
		default:
			return twsi_start_unstick(baseaddr);
		}
	}
	debug("%s: success\n", __func__);
	return 0;
}

/**
 * Sends an i2c stop condition
 *
 * @param	baseaddr	register base address
 *
 * @return	0 for success, -1 if error
 */
static int twsi_stop(void *baseaddr)
{
	u8 stat;

	twsi_write_ctl(baseaddr, TWSI_CTL_STP | TWSI_CTL_ENAB);

	stat = twsi_read_status(baseaddr);
	if (stat != TWSI_STAT_IDLE) {
		debug("%s: Bad status on bus@%p\n", __func__, baseaddr);
		return -1;
	}
	return 0;
}

/**
 * Writes data to the i2c bus
 *
 * @param	baseraddr	register base address
 * @param	slave_addr	address of slave to write to
 * @param	buffer		Pointer to buffer to write
 * @param	length		Number of bytes in buffer to write
 *
 * @return	0 for success, otherwise error
 */
static int twsi_write_data(void *baseaddr, u8  slave_addr,
			   u8 *buffer, unsigned int length)
{
	union twsx_sw_twsi twsi_sw;
	unsigned int curr = 0;
	int result;

	debug("%s(%p, 0x%x, %p, 0x%x)\n", __func__, baseaddr, slave_addr,
	      buffer, length);
	result = twsi_start(baseaddr);
	if (result) {
		debug("%s: Could not start BUS transaction\n", __func__);
		return -1;
	}

	result = twsi_wait(baseaddr);
	if (result) {
		debug("%s: wait failed\n", __func__);
		return result;
	}

	twsi_sw.u	 = 0;
	twsi_sw.s.op	 = TWSI_SW_EOP_IA;
	twsi_sw.s.eop_ia = TWSI_DATA;
	twsi_sw.s.data	 = (u32)(slave_addr << 1) | TWSI_OP_WRITE;

	twsi_write_sw(baseaddr, twsi_sw);
	twsi_write_ctl(baseaddr, TWSI_CTL_ENAB);

	debug("%s: Waiting\n", __func__);
	result = twsi_wait(baseaddr);
	if (result) {
		debug("%s: Timed out writing slave address 0x%x to target\n",
		      __func__, slave_addr);
		return result;
	}
	result = twsi_read_status(baseaddr);
	debug("%s: status: (%d) %s\n", __func__, result,
	      twsi_i2c_status_str(result));
	if (result != TWSI_STAT_TXADDR_ACK) {
		debug("%s: status: (%d) %s\n", __func__, result,
		      twsi_i2c_status_str(result));
		twsi_stop(baseaddr);
		return twsi_i2c_lost_arb(result, 0);
	}

	while (curr < length) {
		twsi_sw.u	 = 0;
		twsi_sw.s.op	 = TWSI_SW_EOP_IA;
		twsi_sw.s.eop_ia = TWSI_DATA;
		twsi_sw.s.data	 = buffer[curr++];

		twsi_write_sw(baseaddr, twsi_sw);
		twsi_write_ctl(baseaddr, TWSI_CTL_ENAB);

		debug("%s: Writing 0x%x\n", __func__, twsi_sw.s.data);

		result = twsi_wait(baseaddr);
		if (result) {
			debug("%s: Timed out writing data to 0x%x\n",
			      __func__, slave_addr);
			return result;
		}
		result = twsi_read_status(baseaddr);
		debug("%s: status: (%d) %s\n", __func__, result,
		      twsi_i2c_status_str(result));
	}

	debug("%s: Stopping\n", __func__);
	return twsi_stop(baseaddr);
}

/**
 * Manually clear the I2C bus and send a stop
 */
static void twsi_unblock(void *baseaddr)
{
	int i;
	union twsx_int	int_reg;

	int_reg.u = 0;
	for (i = 0; i < 9; i++) {
		int_reg.s.scl_ovr = 0;
		writeq(int_reg.u, baseaddr + TWSI_INT);
		udelay(5);
		int_reg.s.scl_ovr = 1;
		writeq(int_reg.u, baseaddr + TWSI_INT);
		udelay(5);
	}
	int_reg.s.sda_ovr = 1;
	writeq(int_reg.u, baseaddr + TWSI_INT);
	udelay(5);
	int_reg.s.scl_ovr = 0;
	writeq(int_reg.u, baseaddr + TWSI_INT);
	udelay(5);
	int_reg.u = 0;
	writeq(int_reg.u, baseaddr + TWSI_INT);
	udelay(5);
}

/**
 * Performs a read transaction on the i2c bus
 *
 * @param	baseaddr	Base address of twsi registers
 * @param	slave_addr	i2c bus address to read from
 * @param	buffer		buffer to read into
 * @param	length		number of bytes to read
 *
 * @return	0 for success, otherwise error
 */
static int twsi_read_data(void *baseaddr, u8 slave_addr,
			  u8 *buffer, unsigned int length)
{
	union twsx_sw_twsi twsi_sw;
	unsigned int curr = 0;
	int result;

	debug("%s(%p, 0x%x, %p, %u)\n", __func__, baseaddr, slave_addr,
	      buffer, length);
	result = twsi_start(baseaddr);
	if (result) {
		debug("%s: start failed\n", __func__);
		return result;
	}

	result = twsi_wait(baseaddr);
	if (result) {
		debug("%s: wait failed\n", __func__);
		return result;
	}

	twsi_sw.u	 = 0;
	twsi_sw.s.op	 = TWSI_SW_EOP_IA;
	twsi_sw.s.eop_ia = TWSI_DATA;

	twsi_sw.s.data  = (u32)(slave_addr << 1) | TWSI_OP_READ;

	twsi_write_sw(baseaddr, twsi_sw);
	twsi_write_ctl(baseaddr, TWSI_CTL_ENAB);

	result = twsi_wait(baseaddr);
	if (result) {
		debug("%s: waiting for sending addr failed\n", __func__);
		return result;
	}

	result = twsi_read_status(baseaddr);
	debug("%s: status: (%d) %s\n", __func__, result,
	      twsi_i2c_status_str(result));
	if (result != TWSI_STAT_RXADDR_ACK) {
		debug("%s: status: (%d) %s\n", __func__, result,
		      twsi_i2c_status_str(result));
		twsi_stop(baseaddr);
		return twsi_i2c_lost_arb(result, 0);
	}

	while (curr < length) {
		twsi_write_ctl(baseaddr, TWSI_CTL_ENAB |
				((curr < length - 1) ? TWSI_CTL_AAK : 0));

		result = twsi_wait(baseaddr);
		if (result) {
			debug("%s: waiting for data failed\n", __func__);
			return result;
		}

		twsi_sw.u = twsi_read_sw(baseaddr, twsi_sw);
		buffer[curr++] = twsi_sw.s.data;
	}

	twsi_stop(baseaddr);

	return 0;
}

static void twsi_calc_div(unsigned int speed, int *m_div, int *n_div)
{
	int io_clock_hz;
	int tclk, fsamp;
	int ndiv, mdiv;

#if defined(CONFIG_ARCH_OCTEONTX)
	io_clock_hz = octeontx_get_io_clock();
	tclk = io_clock_hz / (2 * (TWSI_THP + 1));
#elif defined(CONFIG_ARCH_OCTEONTX2)
	/* Refclk src in mode register defaults to 100MHz clock */
	io_clock_hz = 100000000; /* 100 Mhz */
	tclk = io_clock_hz / (TWSI_THP + 2);
#endif
	debug("%s( io_clock %u tclk %u)\n", __func__, io_clock_hz, tclk);

	/* Set the TWSI clock to a conservative TWSI_BUS_FREQ.
	 * Compute the clocks M divider based on the SCLK.
	 *
	 * TWSI freq = (core freq) / (10 x (M+1) x 2 * (thp+1) x 2^N)
	 * M = ((core freq) / (10 x (TWSI freq) x 2 * (thp+1) x 2^N)) - 1
	 *
	 * For OcteonTX2 -
	 * TWSI freq = (core freq) / (10 x (M+1) x (thp+2) x 2^N)
	 * M = ((core freq) / (10 x (TWSI freq) x (thp+2) x 2^N)) - 1
	 */
	for (ndiv = 0; ndiv < 8; ndiv++) {
		fsamp = tclk / (1 << ndiv);
		mdiv = fsamp / speed / 10;
		mdiv -= 1;
		if (mdiv < 16)
			break;
	}
	*m_div = mdiv;
	*n_div = ndiv;
}

static int twsi_init(void *baseaddr, unsigned int speed, int slaveaddr)
{
	int n_div, m_div;
	union twsx_sw_twsi sw_twsi;

	debug("%s(%p, %u, 0x%x)\n", __func__, baseaddr, speed, slaveaddr);

	twsi_calc_div(speed, &m_div, &n_div);
	if (m_div >= 16)
		return -1;

	sw_twsi.u = 0;
	sw_twsi.s.v = 1;	/* Clear valid bit */
	sw_twsi.s.op = 0x6;	/* See EOP field */
	sw_twsi.s.r = 0;	/* Select CLKCTL when R = 0 */
	sw_twsi.s.eop_ia = 3;	/* R=0 selects CLKCTL, R=1 selects STAT */
	sw_twsi.s.data = ((m_div & 0xf) << 3) | ((n_div & 0x7) << 0);

	twsi_write_sw(baseaddr, sw_twsi);
	/* Only init non-slave ports */
	debug("%s: Writing 0x%llx to sw_twsi, m_div: 0x%x, n_div: 0x%x\n",
	      __func__, sw_twsi.u, m_div, n_div);

	sw_twsi.u = 0;
	sw_twsi.s.v = 1;
	sw_twsi.s.op = TWSI_SW_EOP_IA;
	sw_twsi.s.r = 0;
	sw_twsi.s.eop_ia = 0;
	sw_twsi.s.data = slaveaddr << 1;

	twsi_write_sw(baseaddr, sw_twsi);

	/* Set slave address */
	sw_twsi.u = 0;
	sw_twsi.s.v = 1;
	sw_twsi.s.op = TWSI_SW_EOP_IA;
	sw_twsi.s.r = 0;
	sw_twsi.s.eop_ia = TWSI_EOP_SLAVE_ADDR;
	sw_twsi.s.data = slaveaddr;
	twsi_write_sw(baseaddr, sw_twsi);

	return 0;
}

/**
 * Transfers data over the i2c bus
 *
 * @param	bus	i2c bus to transfer data over
 * @param	msg	Array of i2c messages
 * @param	nmsgs	Number of messages to send/receive
 *
 * @return	0 for success, otherwise error
 */
static int octeontx_i2c_xfer(struct udevice *bus, struct i2c_msg *msg,
			     int nmsgs)
{
	struct octeontx_twsi *twsi = dev_get_priv(bus);
	int result;

	debug("%s: %d messages\n", __func__, nmsgs);
	for (; nmsgs > 0; nmsgs--, msg++) {
		debug("%s: chip=0x%x, len=0x%x\n", __func__, msg->addr,
		      msg->len);
		if (msg->flags & I2C_M_RD) {
			debug("%s: Reading data\n", __func__);
			result = twsi_read_data(twsi->baseaddr, msg->addr,
						msg->buf, msg->len);
		} else {
			debug("%s: Writing data\n", __func__);
			result = twsi_write_data(twsi->baseaddr, msg->addr,
						 msg->buf, msg->len);
		}
		if (result) {
			debug("%s: error sending\n", __func__);
			return -EREMOTEIO;
		}
	}

	return 0;
}

static int octeontx_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct octeontx_twsi *twsi = dev_get_priv(bus);
	int m_div, n_div;
	union twsx_sw_twsi sw_twsi;
	void *baseaddr = twsi->baseaddr;

	debug("%s(%p, %u)\n", __func__, bus, speed);

	twsi_calc_div(speed, &m_div, &n_div);
	if (m_div >= 16)
		return -1;

	sw_twsi.u = 0;
	sw_twsi.s.v = 1;		/* Clear valid bit */
	sw_twsi.s.op = TWSI_SW_EOP_IA;	/* See EOP field */
	sw_twsi.s.r = 0;		/* Select CLKCTL when R = 0 */
	sw_twsi.s.eop_ia = TWSI_CLKCTL;	/* R=0 selects CLKCTL, R=1 selects STAT */
	sw_twsi.s.data = ((m_div & 0xf) << 3) | ((n_div & 0x7) << 0);

	/* Only init non-slave ports */
	writeq(sw_twsi.u, baseaddr + TWSI_SW_TWSI);

	debug("%s: Wrote 0x%llx to sw_twsi\n", __func__, sw_twsi.u);
	return 0;
}

static int octeontx_pci_i2c_probe(struct udevice *dev)
{
	struct octeontx_twsi *twsi = dev_get_priv(dev);
	pci_dev_t bdf = dm_pci_get_bdf(dev);

	debug("TWSI PCI device: %x\n", bdf);
	dev->req_seq = PCI_FUNC(bdf);

	twsi->baseaddr = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0,
					PCI_REGION_MEM);
	twsi->id = last_id++;

	debug("TWSI bus %d at %p\n", dev->seq, twsi->baseaddr);

	return twsi_init(twsi->baseaddr,
			 twsi->id < ARRAY_SIZE(speeds) ?
			 speeds[twsi->id] : CONFIG_SYS_I2C_SPEED,
			 CONFIG_SYS_I2C_OCTEONTX_SLAVE_ADDR);
}

static const struct dm_i2c_ops octeontx_i2c_ops = {
	.xfer		= octeontx_i2c_xfer,
	.set_bus_speed	= octeontx_i2c_set_bus_speed,
};

static const struct udevice_id octeontx_i2c_ids[] = {
	{ .compatible = "cavium,thunder-8890-twsi" },
	{ }
};

U_BOOT_DRIVER(octeontx_pci_twsi) = {
	.name	= "i2c_octeontx",
	.id	= UCLASS_I2C,
	.of_match = octeontx_i2c_ids,
	.probe	= octeontx_pci_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct octeontx_twsi),
	.ops	= &octeontx_i2c_ops,
};

static struct pci_device_id octeontx_twsi_supported[] = {
	{ PCI_VDEVICE(CAVIUM, 0xa012) },
	{ },
};

U_BOOT_PCI_DEVICE(octeontx_pci_twsi, octeontx_twsi_supported);
