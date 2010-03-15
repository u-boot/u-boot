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
#if defined(CONFIG_MGCOGE)
#include <mpc8260.h>
#endif
#include <ioports.h>
#include <malloc.h>
#include <hush.h>
#include <net.h>
#include <asm/io.h>

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif

#include "../common/common.h"
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
#include <i2c.h>

extern int i2c_soft_read_pin (void);

int ivm_calc_crc (unsigned char *buf, int len)
{
	const unsigned short crc_tab[16] = {
		0x0000, 0xCC01, 0xD801, 0x1400,
		0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401,
		0x5000, 0x9C01, 0x8801, 0x4400};

	unsigned short crc     = 0;   /* final result */
	unsigned short r1      = 0;   /* temp */
	unsigned char  byte    = 0;   /* input buffer */
	int	i;

	/* calculate CRC from array data */
	for (i = 0; i < len; i++) {
		byte = buf[i];

		/* lower 4 bits */
		r1 = crc_tab[crc & 0xF];
		crc = ((crc) >> 4) & 0x0FFF;
		crc = crc ^ r1 ^ crc_tab[byte & 0xF];

		/* upper 4 bits */
		r1 = crc_tab[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ r1 ^ crc_tab[(byte >> 4) & 0xF];
	}
	return crc;
}

static int  ivm_set_value (char *name, char *value)
{
	char tempbuf[256];

	if (value != NULL) {
		sprintf (tempbuf, "%s=%s", name, value);
		return set_local_var (tempbuf, 0);
	} else {
		unset_local_var (name);
	}
	return 0;
}

static int ivm_get_value (unsigned char *buf, int len, char *name, int off,
				int check)
{
	unsigned short	val;
	unsigned char	valbuf[30];

	if ((buf[off + 0] != buf[off + 2]) &&
	    (buf[off + 2] != buf[off + 4])) {
		printf ("%s Error corrupted %s\n", __FUNCTION__, name);
		val = -1;
	} else {
		val = buf[off + 0] + (buf[off + 1] << 8);
		if ((val == 0) && (check == 1))
			val = -1;
	}
	sprintf ((char *)valbuf, "%x", val);
	ivm_set_value (name, (char *)valbuf);
	return val;
}

#define INVENTORYBLOCKSIZE	0x100
#define INVENTORYDATAADDRESS	0x21
#define INVENTORYDATASIZE	(INVENTORYBLOCKSIZE - INVENTORYDATAADDRESS - 3)

#define IVM_POS_SHORT_TEXT		0
#define IVM_POS_MANU_ID			1
#define IVM_POS_MANU_SERIAL		2
#define IVM_POS_PART_NUMBER		3
#define IVM_POS_BUILD_STATE		4
#define IVM_POS_SUPPLIER_PART_NUMBER	5
#define IVM_POS_DELIVERY_DATE		6
#define IVM_POS_SUPPLIER_BUILD_STATE	7
#define IVM_POS_CUSTOMER_ID		8
#define IVM_POS_CUSTOMER_PROD_ID	9
#define IVM_POS_HISTORY			10
#define IVM_POS_SYMBOL_ONLY		11

static char convert_char (char c)
{
	return (c < ' ' || c > '~') ? '.' : c;
}

static int ivm_findinventorystring (int type,
					unsigned char* const string,
					unsigned long maxlen,
					unsigned char *buf)
{
	int xcode = 0;
	unsigned long cr = 0;
	unsigned long addr = INVENTORYDATAADDRESS;
	unsigned long size = 0;
	unsigned long nr = type;
	int stop = 0; 	/* stop on semicolon */

	memset(string, '\0', maxlen);
	switch (type) {
		case IVM_POS_SYMBOL_ONLY:
			nr = 0;
			stop= 1;
		break;
		default:
			nr = type;
			stop = 0;
	}

	/* Look for the requested number of CR. */
	while ((cr != nr) && (addr < INVENTORYDATASIZE)) {
		if ((buf[addr] == '\r')) {
			cr++;
		}
		addr++;
	}

	/* the expected number of CR was found until the end of the IVM
	 *  content --> fill string */
	if (addr < INVENTORYDATASIZE) {
		/* Copy the IVM string in the corresponding string */
		for (; (buf[addr] != '\r')			&&
			((buf[addr] != ';') ||  (!stop))	&&
			(size < (maxlen - 1)			&&
			(addr < INVENTORYDATASIZE)); addr++)
		{
			size += sprintf((char *)string + size, "%c",
						convert_char (buf[addr]));
		}

		/* copy phase is done: check if everything is ok. If not,
		 * the inventory data is most probably corrupted: tell
		 * the world there is a problem! */
		if (addr == INVENTORYDATASIZE) {
			xcode = -1;
			printf ("Error end of string not found\n");
		} else if ((size >= (maxlen - 1)) &&
			   (buf[addr] != '\r')) {
			xcode = -1;
			printf ("string too long till next CR\n");
		}
	} else {
		/* some CR are missing...
		 * the inventory data is most probably corrupted */
		xcode = -1;
		printf ("not enough cr found\n");
	}
	return xcode;
}

#define GET_STRING(name, which, len) \
	if (ivm_findinventorystring (which, valbuf, len, buf) == 0) { \
		ivm_set_value (name, (char *)valbuf); \
	}

static int ivm_check_crc (unsigned char *buf, int block)
{
	unsigned long	crc;
	unsigned long	crceeprom;

	crc = ivm_calc_crc (buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN - 2);
	crceeprom = (buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN - 1] + \
			buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN - 2] * 256);
	if (crc != crceeprom) {
		if (block == 0)
			printf ("Error CRC Block: %d EEprom: calculated: \
			%lx EEprom: %lx\n", block, crc, crceeprom);
		return -1;
	}
	return 0;
}

static int ivm_analyze_block2 (unsigned char *buf, int len)
{
	unsigned char	valbuf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN];
	unsigned long	count;

	/* IVM_MacAddress */
	sprintf ((char *)valbuf, "%02X:%02X:%02X:%02X:%02X:%02X",
			buf[1],
			buf[2],
			buf[3],
			buf[4],
			buf[5],
			buf[6]);
	ivm_set_value ("IVM_MacAddress", (char *)valbuf);
	if (getenv ("ethaddr") == NULL)
		setenv ((char *)"ethaddr", (char *)valbuf);
	/* IVM_MacCount */
	count = (buf[10] << 24) +
		   (buf[11] << 16) +
		   (buf[12] << 8)  +
		    buf[13];
	if (count == 0xffffffff)
		count = 1;
	sprintf ((char *)valbuf, "%lx", count);
	ivm_set_value ("IVM_MacCount", (char *)valbuf);
	return 0;
}

int ivm_analyze_eeprom (unsigned char *buf, int len)
{
	unsigned short	val;
	unsigned char	valbuf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN];
	unsigned char	*tmp;

	if (ivm_check_crc (buf, 0) != 0)
		return -1;

	ivm_get_value (buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN, "IVM_BoardId", 0, 1);
	val = ivm_get_value (buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN, "IVM_HWKey", 6, 1);
	if (val != 0xffff) {
		sprintf ((char *)valbuf, "%x", ((val /100) % 10));
		ivm_set_value ("IVM_HWVariant", (char *)valbuf);
		sprintf ((char *)valbuf, "%x", (val % 100));
		ivm_set_value ("IVM_HWVersion", (char *)valbuf);
	}
	ivm_get_value (buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN, "IVM_Functions", 12, 0);

	GET_STRING("IVM_Symbol", IVM_POS_SYMBOL_ONLY, 8)
	GET_STRING("IVM_DeviceName", IVM_POS_SHORT_TEXT, 64)
	tmp = (unsigned char *) getenv("IVM_DeviceName");
	if (tmp) {
		int	len = strlen ((char *)tmp);
		int	i = 0;

		while (i < len) {
			if (tmp[i] == ';') {
				ivm_set_value ("IVM_ShortText", (char *)&tmp[i + 1]);
				break;
			}
			i++;
		}
		if (i >= len)
			ivm_set_value ("IVM_ShortText", NULL);
	} else {
		ivm_set_value ("IVM_ShortText", NULL);
	}
	GET_STRING("IVM_ManufacturerID", IVM_POS_MANU_ID, 32)
	GET_STRING("IVM_ManufacturerSerialNumber", IVM_POS_MANU_SERIAL, 20)
	GET_STRING("IVM_ManufacturerPartNumber", IVM_POS_PART_NUMBER, 32)
	GET_STRING("IVM_ManufacturerBuildState", IVM_POS_BUILD_STATE, 32)
	GET_STRING("IVM_SupplierPartNumber", IVM_POS_SUPPLIER_PART_NUMBER, 32)
	GET_STRING("IVM_DelieveryDate", IVM_POS_DELIVERY_DATE, 32)
	GET_STRING("IVM_SupplierBuildState", IVM_POS_SUPPLIER_BUILD_STATE, 32)
	GET_STRING("IVM_CustomerID", IVM_POS_CUSTOMER_ID, 32)
	GET_STRING("IVM_CustomerProductID", IVM_POS_CUSTOMER_PROD_ID, 32)

	if (ivm_check_crc (&buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN * 2], 2) != 0)
		return 0;
	ivm_analyze_block2 (&buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN * 2], CONFIG_SYS_IVM_EEPROM_PAGE_LEN);

	return 0;
}

int ivm_read_eeprom (void)
{
#if defined(CONFIG_I2C_MUX)
	I2C_MUX_DEVICE *dev = NULL;
#endif
	uchar i2c_buffer[CONFIG_SYS_IVM_EEPROM_MAX_LEN];
	uchar	*buf;
	unsigned dev_addr = CONFIG_SYS_IVM_EEPROM_ADR;

#if defined(CONFIG_I2C_MUX)
	/* First init the Bus, select the Bus */
#if defined(CONFIG_SYS_I2C_IVM_BUS)
	dev = i2c_mux_ident_muxstring ((uchar *)CONFIG_SYS_I2C_IVM_BUS);
#else
	buf = (unsigned char *) getenv ("EEprom_ivm");
	if (buf != NULL)
		dev = i2c_mux_ident_muxstring (buf);
#endif
	if (dev == NULL) {
		printf ("Error couldnt add Bus for IVM\n");
		return -1;
	}
	i2c_set_bus_num (dev->busid);
#endif

	buf = (unsigned char *) getenv ("EEprom_ivm_addr");
	if (buf != NULL)
		dev_addr = simple_strtoul ((char *)buf, NULL, 16);

	if (i2c_read(dev_addr, 0, 1, i2c_buffer, CONFIG_SYS_IVM_EEPROM_MAX_LEN) != 0) {
		printf ("Error reading EEprom\n");
		return -2;
	}

	return ivm_analyze_eeprom (i2c_buffer, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
}

#if defined(CONFIG_SYS_I2C_INIT_BOARD)
#define DELAY_ABORT_SEQ		62
#define DELAY_HALF_PERIOD	(500 / (CONFIG_SYS_I2C_SPEED / 1000))

#if defined(CONFIG_MGCOGE)
#define SDA_MASK	0x00010000
#define SCL_MASK	0x00020000
static void set_pin (int state, unsigned long mask)
{
	volatile ioport_t *iop = ioport_addr ((immap_t *)CONFIG_SYS_IMMR, 3);

	if (state)
		iop->pdat |= (mask);
	else
		iop->pdat &= ~(mask);

	iop->pdir |= (mask);
}

static int get_pin (unsigned long mask)
{
	volatile ioport_t *iop = ioport_addr ((immap_t *)CONFIG_SYS_IMMR, 3);

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
	volatile ioport_t *iop = ioport_addr ((immap_t *)CONFIG_SYS_IMMR, 3);

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

#if defined(CONFIG_KM8XX)
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
	return I2C_READ;
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

#if !defined(CONFIG_KMETER1)
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
#endif

/**
 * i2c_init_board - reset i2c bus. When the board is powercycled during a
 * bus transfer it might hang; for details see doc/I2C_Edge_Conditions.
 */
void i2c_init_board(void)
{
#if defined(CONFIG_KMETER1)
	struct fsl_i2c *dev;
	dev = (struct fsl_i2c *) (CONFIG_SYS_IMMR + CONFIG_SYS_I2C_OFFSET);
	uchar	dummy;

	out_8 (&dev->cr, (I2C_CR_MSTA));
	out_8 (&dev->cr, (I2C_CR_MEN | I2C_CR_MSTA));
	dummy = in_8(&dev->dr);
	dummy = in_8(&dev->dr);
	if (dummy != 0xff) {
		dummy = in_8(&dev->dr);
	}
	out_8 (&dev->cr, (I2C_CR_MEN));
	out_8 (&dev->cr, 0x00);
	out_8 (&dev->cr, (I2C_CR_MEN));

#else
#if defined(CONFIG_HARD_I2C) && !defined(CONFIG_MACH_SUEN3)
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR ;
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
#endif
}
#endif
#endif

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int fdt_set_node_and_value (void *blob,
				char *nodename,
				char *regname,
				void *var,
				int size)
{
	int ret = 0;
	int nodeoffset = 0;

	nodeoffset = fdt_path_offset (blob, nodename);
	if (nodeoffset >= 0) {
		ret = fdt_setprop (blob, nodeoffset, regname, var,
					size);
		if (ret < 0)
			printf("ft_blob_update(): cannot set %s/%s "
				"property err:%s\n", nodename, regname,
				fdt_strerror (ret));
	} else {
		printf("ft_blob_update(): cannot find %s node "
			"err:%s\n", nodename, fdt_strerror (nodeoffset));
	}
	return ret;
}
int fdt_get_node_and_value (void *blob,
				char *nodename,
				char *propname,
				void **var)
{
	int len;
	int nodeoffset = 0;

	nodeoffset = fdt_path_offset (blob, nodename);
	if (nodeoffset >= 0) {
		*var = (void *)fdt_getprop (blob, nodeoffset, propname, &len);
		if (len == 0) {
			/* no value */
			printf ("%s no value\n", __FUNCTION__);
			return -1;
		} else if (len > 0) {
			return len;
		} else {
			printf ("libfdt fdt_getprop(): %s\n",
				fdt_strerror(len));
			return -2;
		}
	} else {
		printf("%s: cannot find %s node err:%s\n", __FUNCTION__,
			nodename, fdt_strerror (nodeoffset));
		return -3;
	}
}
#endif

#if !defined(CONFIG_MACH_SUEN3)
int ethernet_present (void)
{
	return (in_8((u8 *)CONFIG_SYS_PIGGY_BASE + CONFIG_SYS_SLOT_ID_OFF) & 0x80);
}
#endif

int board_eth_init (bd_t *bis)
{
#ifdef CONFIG_KEYMILE_HDLC_ENET
	(void)keymile_hdlc_enet_initialize (bis);
#endif
	if (ethernet_present ()) {
		return -1;
	}
	return 0;
}
