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
#if defined(CONFIG_KM82XX)
#include <mpc8260.h>
#endif
#include <ioports.h>
#include <command.h>
#include <malloc.h>
#include <hush.h>
#include <net.h>
#include <netdev.h>
#include <asm/io.h>
#include <linux/ctype.h>

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#endif

#include "../common/common.h"
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
#include <i2c.h>

static void i2c_write_start_seq(void);
static int i2c_make_abort(void);
DECLARE_GLOBAL_DATA_PTR;

int ivm_calc_crc(unsigned char *buf, int len)
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

/*
 * Set Keymile specific environment variables
 * Currently only some memory layout variables are calculated here
 * ... ------------------------------------------------
 * ... |@rootfsaddr |@pnvramaddr |@varaddr |@reserved |@END_OF_RAM
 * ... |<------------------- pram ------------------->|
 * ... ------------------------------------------------
 * @END_OF_RAM: denotes the RAM size
 * @pnvramaddr: Startadress of pseudo non volatile RAM in hex
 * @pram      : preserved ram size in k
 * @varaddr   : startadress for /var mounted into RAM
 */
int set_km_env(void)
{
	uchar buf[32];
	unsigned int pnvramaddr;
	unsigned int pram;
	unsigned int varaddr;

	pnvramaddr = gd->ram_size - CONFIG_KM_RESERVED_PRAM - CONFIG_KM_PHRAM
			- CONFIG_KM_PNVRAM;
	sprintf((char *)buf, "0x%x", pnvramaddr);
	setenv("pnvramaddr", (char *)buf);

	pram = (CONFIG_KM_RESERVED_PRAM + CONFIG_KM_PHRAM + CONFIG_KM_PNVRAM) /
		0x400;
	sprintf((char *)buf, "0x%x", pram);
	setenv("pram", (char *)buf);

	varaddr = gd->ram_size - CONFIG_KM_RESERVED_PRAM - CONFIG_KM_PHRAM;
	sprintf((char *)buf, "0x%x", varaddr);
	setenv("varaddr", (char *)buf);
	return 0;
}

static int ivm_set_value(char *name, char *value)
{
	char tempbuf[256];

	if (value != NULL) {
		sprintf(tempbuf, "%s=%s", name, value);
		return set_local_var(tempbuf, 0);
	} else {
		unset_local_var(name);
	}
	return 0;
}

static int ivm_get_value(unsigned char *buf, int len, char *name, int off,
				int check)
{
	unsigned short	val;
	unsigned char	valbuf[30];

	if ((buf[off + 0] != buf[off + 2]) &&
	    (buf[off + 2] != buf[off + 4])) {
		printf("%s Error corrupted %s\n", __func__, name);
		val = -1;
	} else {
		val = buf[off + 0] + (buf[off + 1] << 8);
		if ((val == 0) && (check == 1))
			val = -1;
	}
	sprintf((char *)valbuf, "%x", val);
	ivm_set_value(name, (char *)valbuf);
	return val;
}

#define INV_BLOCKSIZE		0x100
#define INV_DATAADDRESS		0x21
#define INVENTORYDATASIZE	(INV_BLOCKSIZE - INV_DATAADDRESS - 3)

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

static char convert_char(char c)
{
	return (c < ' ' || c > '~') ? '.' : c;
}

static int ivm_findinventorystring(int type,
					unsigned char* const string,
					unsigned long maxlen,
					unsigned char *buf)
{
	int xcode = 0;
	unsigned long cr = 0;
	unsigned long addr = INV_DATAADDRESS;
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

	/*
	 * the expected number of CR was found until the end of the IVM
	 *  content --> fill string
	 */
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

		/*
		 * copy phase is done: check if everything is ok. If not,
		 * the inventory data is most probably corrupted: tell
		 * the world there is a problem!
		 */
		if (addr == INVENTORYDATASIZE) {
			xcode = -1;
			printf("Error end of string not found\n");
		} else if ((size >= (maxlen - 1)) &&
			   (buf[addr] != '\r')) {
			xcode = -1;
			printf("string too long till next CR\n");
		}
	} else {
		/*
		 * some CR are missing...
		 * the inventory data is most probably corrupted
		 */
		xcode = -1;
		printf("not enough cr found\n");
	}
	return xcode;
}

#define GET_STRING(name, which, len) \
	if (ivm_findinventorystring(which, valbuf, len, buf) == 0) { \
		ivm_set_value(name, (char *)valbuf); \
	}

static int ivm_check_crc(unsigned char *buf, int block)
{
	unsigned long	crc;
	unsigned long	crceeprom;

	crc = ivm_calc_crc(buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN - 2);
	crceeprom = (buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN - 1] + \
			buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN - 2] * 256);
	if (crc != crceeprom) {
		if (block == 0)
			printf("Error CRC Block: %d EEprom: calculated: \
			%lx EEprom: %lx\n", block, crc, crceeprom);
		return -1;
	}
	return 0;
}

static int ivm_analyze_block2(unsigned char *buf, int len)
{
	unsigned char	valbuf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN];
	unsigned long	count;

	/* IVM_MacAddress */
	sprintf((char *)valbuf, "%pM", buf);
	ivm_set_value("IVM_MacAddress", (char *)valbuf);
	/* if an offset is defined, add it */
#if defined(CONFIG_PIGGY_MAC_ADRESS_OFFSET)
	if (CONFIG_PIGGY_MAC_ADRESS_OFFSET > 0) {
		unsigned long val = (buf[4] << 16) + (buf[5] << 8) + buf[6];

		val += CONFIG_PIGGY_MAC_ADRESS_OFFSET;
		buf[4] = (val >> 16) & 0xff;
		buf[5] = (val >> 8) & 0xff;
		buf[6] = val & 0xff;
		sprintf((char *)valbuf, "%pM", buf);
	}
#endif
	setenv((char *)"ethaddr", (char *)valbuf);

	/* IVM_MacCount */
	count = (buf[10] << 24) +
		   (buf[11] << 16) +
		   (buf[12] << 8)  +
		    buf[13];
	if (count == 0xffffffff)
		count = 1;
	sprintf((char *)valbuf, "%lx", count);
	ivm_set_value("IVM_MacCount", (char *)valbuf);
	return 0;
}

int ivm_analyze_eeprom(unsigned char *buf, int len)
{
	unsigned short	val;
	unsigned char	valbuf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN];
	unsigned char	*tmp;

	if (ivm_check_crc(buf, 0) != 0)
		return -1;

	ivm_get_value(buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN,
			"IVM_BoardId", 0, 1);
	val = ivm_get_value(buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN,
			"IVM_HWKey", 6, 1);
	if (val != 0xffff) {
		sprintf((char *)valbuf, "%x", ((val / 100) % 10));
		ivm_set_value("IVM_HWVariant", (char *)valbuf);
		sprintf((char *)valbuf, "%x", (val % 100));
		ivm_set_value("IVM_HWVersion", (char *)valbuf);
	}
	ivm_get_value(buf, CONFIG_SYS_IVM_EEPROM_PAGE_LEN,
		"IVM_Functions", 12, 0);

	GET_STRING("IVM_Symbol", IVM_POS_SYMBOL_ONLY, 8)
	GET_STRING("IVM_DeviceName", IVM_POS_SHORT_TEXT, 64)
	tmp = (unsigned char *) getenv("IVM_DeviceName");
	if (tmp) {
		int	len = strlen((char *)tmp);
		int	i = 0;

		while (i < len) {
			if (tmp[i] == ';') {
				ivm_set_value("IVM_ShortText",
					(char *)&tmp[i + 1]);
				break;
			}
			i++;
		}
		if (i >= len)
			ivm_set_value("IVM_ShortText", NULL);
	} else {
		ivm_set_value("IVM_ShortText", NULL);
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

	if (ivm_check_crc(&buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN * 2], 2) != 0)
		return 0;
	ivm_analyze_block2(&buf[CONFIG_SYS_IVM_EEPROM_PAGE_LEN * 2],
		CONFIG_SYS_IVM_EEPROM_PAGE_LEN);

	return 0;
}

int ivm_read_eeprom(void)
{
#if defined(CONFIG_I2C_MUX)
	I2C_MUX_DEVICE *dev = NULL;
#endif
	uchar i2c_buffer[CONFIG_SYS_IVM_EEPROM_MAX_LEN];
	uchar	*buf;
	unsigned dev_addr = CONFIG_SYS_IVM_EEPROM_ADR;
	int ret;

#if defined(CONFIG_I2C_MUX)
	/* First init the Bus, select the Bus */
#if defined(CONFIG_SYS_I2C_IVM_BUS)
	dev = i2c_mux_ident_muxstring((uchar *)CONFIG_SYS_I2C_IVM_BUS);
#else
	buf = (unsigned char *) getenv("EEprom_ivm");
	if (buf != NULL)
		dev = i2c_mux_ident_muxstring(buf);
#endif
	if (dev == NULL) {
		printf("Error couldnt add Bus for IVM\n");
		return -1;
	}
	i2c_set_bus_num(dev->busid);
#endif

	buf = (unsigned char *) getenv("EEprom_ivm_addr");
	if (buf != NULL)
		dev_addr = simple_strtoul((char *)buf, NULL, 16);

	/* add deblocking here */
	i2c_make_abort();

	ret = i2c_read(dev_addr, 0, 1, i2c_buffer,
		CONFIG_SYS_IVM_EEPROM_MAX_LEN);
	if (ret != 0) {
		printf ("Error reading EEprom\n");
		return -2;
	}

	return ivm_analyze_eeprom(i2c_buffer, CONFIG_SYS_IVM_EEPROM_MAX_LEN);
}

#if defined(CONFIG_SYS_I2C_INIT_BOARD)
#define DELAY_ABORT_SEQ		62  /* @200kHz 9 clocks = 44us, 62us is ok */
#define DELAY_HALF_PERIOD	(500 / (CONFIG_SYS_I2C_SPEED / 1000))

#if defined(CONFIG_KM_82XX)
#define SDA_MASK	0x00010000
#define SCL_MASK	0x00020000
void set_pin(int state, unsigned long mask)
{
	ioport_t *iop = ioport_addr((immap_t *)CONFIG_SYS_IMMR, 3);

	if (state)
		setbits_be32(&iop->pdat, mask);
	else
		clrbits_be32(&iop->pdat, mask);

	setbits_be32(&iop->pdir, mask);
}

static int get_pin(unsigned long mask)
{
	ioport_t *iop = ioport_addr((immap_t *)CONFIG_SYS_IMMR, 3);

	clrbits_be32(&iop->pdir, mask);
	return 0 != (in_be32(&iop->pdat) & mask);
}

static void set_sda(int state)
{
	set_pin(state, SDA_MASK);
}

static void set_scl(int state)
{
	set_pin(state, SCL_MASK);
}

static int get_sda(void)
{
	return get_pin(SDA_MASK);
}

static int get_scl(void)
{
	return get_pin(SCL_MASK);
}

#if defined(CONFIG_HARD_I2C)
static void setports(int gpio)
{
	ioport_t *iop = ioport_addr((immap_t *)CONFIG_SYS_IMMR, 3);

	if (gpio) {
		clrbits_be32(&iop->ppar, (SDA_MASK | SCL_MASK));
		clrbits_be32(&iop->podr, (SDA_MASK | SCL_MASK));
	} else {
		setbits_be32(&iop->ppar, (SDA_MASK | SCL_MASK));
		clrbits_be32(&iop->pdir, (SDA_MASK | SCL_MASK));
		setbits_be32(&iop->podr, (SDA_MASK | SCL_MASK));
	}
}
#endif
#endif

#if !defined(CONFIG_MPC83xx)
static void i2c_write_start_seq(void)
{
	set_sda(1);
	udelay(DELAY_HALF_PERIOD);
	set_scl(1);
	udelay(DELAY_HALF_PERIOD);
	set_sda(0);
	udelay(DELAY_HALF_PERIOD);
	set_scl(0);
	udelay(DELAY_HALF_PERIOD);
}

/*
 * I2C is a synchronous protocol and resets of the processor in the middle
 * of an access can block the I2C Bus until a powerdown of the full unit is
 * done. This function toggles the SCL until the SCL and SCA line are
 * released, but max. 16 times, after this a I2C start-sequence is sent.
 * This I2C Deblocking mechanism was developed by Keymile in association
 * with Anatech and Atmel in 1998.
 */
static int i2c_make_abort(void)
{

#if defined(CONFIG_HARD_I2C) && !defined(MACH_TYPE_KM_KIRKWOOD)
	immap_t *immap = (immap_t *)CONFIG_SYS_IMMR ;
	i2c8260_t *i2c	= (i2c8260_t *)&immap->im_i2c;

	/*
	 * disable I2C controller first, otherwhise it thinks we want to
	 * talk to the slave port...
	 */
	clrbits_8(&i2c->i2c_i2mod, 0x01);

	/* Set the PortPins to GPIO */
	setports(1);
#endif

	int	scl_state = 0;
	int	sda_state = 0;
	int	i = 0;
	int	ret = 0;

	if (!get_sda()) {
		ret = -1;
		while (i < 16) {
			i++;
			set_scl(0);
			udelay(DELAY_ABORT_SEQ);
			set_scl(1);
			udelay(DELAY_ABORT_SEQ);
			scl_state = get_scl();
			sda_state = get_sda();
			if (scl_state && sda_state) {
				ret = 0;
				break;
			}
		}
	}
	if (ret == 0)
		for (i = 0; i < 5; i++)
			i2c_write_start_seq();

	/* respect stop setup time */
	udelay(DELAY_ABORT_SEQ);
	set_scl(1);
	udelay(DELAY_ABORT_SEQ);
	set_sda(1);
	get_sda();

#if defined(CONFIG_HARD_I2C)
	/* Set the PortPins back to use for I2C */
	setports(0);
#endif
	return ret;
}
#endif

#if defined(CONFIG_MPC83xx)
static void i2c_write_start_seq(void)
{
	struct fsl_i2c *dev;
	dev = (struct fsl_i2c *) (CONFIG_SYS_IMMR + CONFIG_SYS_I2C_OFFSET);
	udelay(DELAY_ABORT_SEQ);
	out_8(&dev->cr, (I2C_CR_MEN | I2C_CR_MSTA));
	udelay(DELAY_ABORT_SEQ);
	out_8(&dev->cr, (I2C_CR_MEN));
}

static int i2c_make_abort(void)
{
	struct fsl_i2c *dev;
	dev = (struct fsl_i2c *) (CONFIG_SYS_IMMR + CONFIG_SYS_I2C_OFFSET);
	uchar	dummy;
	uchar   last;
	int     nbr_read = 0;
	int     i = 0;
	int	    ret = 0;

	/* wait after each operation to finsh with a delay */
	out_8(&dev->cr, (I2C_CR_MSTA));
	udelay(DELAY_ABORT_SEQ);
	out_8(&dev->cr, (I2C_CR_MEN | I2C_CR_MSTA));
	udelay(DELAY_ABORT_SEQ);
	dummy = in_8(&dev->dr);
	udelay(DELAY_ABORT_SEQ);
	last = in_8(&dev->dr);
	nbr_read++;

	/*
	 * do read until the last bit is 1, but stop if the full eeprom is
	 * read.
	 */
	while (((last & 0x01) != 0x01) &&
		(nbr_read < CONFIG_SYS_IVM_EEPROM_MAX_LEN)) {
		udelay(DELAY_ABORT_SEQ);
		last = in_8(&dev->dr);
		nbr_read++;
	}
	if ((last & 0x01) != 0x01)
		ret = -2;
	if ((last != 0xff) || (nbr_read > 1))
		printf("[INFO] i2c abort after %d bytes (0x%02x)\n",
			nbr_read, last);
	udelay(DELAY_ABORT_SEQ);
	out_8(&dev->cr, (I2C_CR_MEN));
	udelay(DELAY_ABORT_SEQ);
	/* clear status reg */
	out_8(&dev->sr, 0);

	for (i = 0; i < 5; i++)
		i2c_write_start_seq();
	if (ret != 0)
		printf("[ERROR] i2c abort failed after %d bytes (0x%02x)\n",
			nbr_read, last);

	return ret;
}
#endif

/**
 * i2c_init_board - reset i2c bus. When the board is powercycled during a
 * bus transfer it might hang; for details see doc/I2C_Edge_Conditions.
 */
void i2c_init_board(void)
{
	/* Now run the AbortSequence() */
	i2c_make_abort();
}
#endif
#endif

#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
int fdt_set_node_and_value(void *blob,
				char *nodename,
				char *regname,
				void *var,
				int size)
{
	int ret = 0;
	int nodeoffset = 0;

	nodeoffset = fdt_path_offset(blob, nodename);
	if (nodeoffset >= 0) {
		ret = fdt_setprop(blob, nodeoffset, regname, var,
					size);
		if (ret < 0)
			printf("ft_blob_update(): cannot set %s/%s "
				"property err:%s\n", nodename, regname,
				fdt_strerror(ret));
	} else {
		printf("ft_blob_update(): cannot find %s node "
			"err:%s\n", nodename, fdt_strerror(nodeoffset));
	}
	return ret;
}

int fdt_get_node_and_value(void *blob,
				char *nodename,
				char *propname,
				void **var)
{
	int len;
	int nodeoffset = 0;

	nodeoffset = fdt_path_offset(blob, nodename);
	if (nodeoffset >= 0) {
		*var = (void *)fdt_getprop(blob, nodeoffset, propname, &len);
		if (len == 0) {
			/* no value */
			printf("%s no value\n", __func__);
			return -1;
		} else if (len > 0) {
			return len;
		} else {
			printf("libfdt fdt_getprop(): %s\n",
				fdt_strerror(len));
			return -2;
		}
	} else {
		printf("%s: cannot find %s node err:%s\n", __func__,
			nodename, fdt_strerror(nodeoffset));
		return -3;
	}
}
#endif

#if !defined(MACH_TYPE_KM_KIRKWOOD)
int ethernet_present(void)
{
	struct km_bec_fpga *base =
		(struct km_bec_fpga *)CONFIG_SYS_KMBEC_FPGA_BASE;

	return in_8(&base->bprth) & PIGGY_PRESENT;
}
#endif

int board_eth_init(bd_t *bis)
{
	if (ethernet_present())
		return cpu_eth_init(bis);

	return -1;
}

/*
 * do_setboardid command
 * read out the board id and the hw key from the intventory EEPROM and set
 * this values as environment variables.
 */
static int do_setboardid(cmd_tbl_t *cmdtp, int flag, int argc,
				char *const argv[])
{
	unsigned char buf[32];
	char *p;

	p = get_local_var("IVM_BoardId");
	if (p == NULL) {
		printf("can't get the IVM_Boardid\n");
		return 1;
	}
	sprintf((char *)buf, "%s", p);
	setenv("boardid", (char *)buf);

	p = get_local_var("IVM_HWKey");
	if (p == NULL) {
		printf("can't get the IVM_HWKey\n");
		return 1;
	}
	sprintf((char *)buf, "%s", p);
	setenv("hwkey", (char *)buf);

	return 0;
}

U_BOOT_CMD(km_setboardid, 1, 0, do_setboardid, "setboardid", "read out bid and "
				 "hwkey from IVM and set in environment");

/*
 * command km_checkbidhwk
 *	if "boardid" and "hwkey" are not already set in the environment, do:
 *		if a "boardIdListHex" exists in the environment:
 *			- read ivm data for boardid and hwkey
 *			- compare each entry of the boardIdListHex with the
 *				IVM data:
 *			if match:
 *				set environment variables boardid, boardId,
 *				hwkey, hwKey to	the found values
 *				both (boardid and boardId) are set because
 *				they might be used differently in the
 *				application and in the init scripts (?)
 *	return 0 in case of match, 1 if not match or error
 */
int do_checkboardidhwk(cmd_tbl_t *cmdtp, int flag, int argc,
			char *const argv[])
{
	unsigned long ivmbid = 0, ivmhwkey = 0;
	unsigned long envbid = 0, envhwkey = 0;
	char *p;
	int verbose = argc > 1 && *argv[1] == 'v';
	int rc = 0;

	/*
	 * first read out the real inventory values, these values are
	 * already stored in the local hush variables
	 */
	p = get_local_var("IVM_BoardId");
	if (p == NULL) {
		printf("can't get the IVM_Boardid\n");
		return 1;
	}
	rc = strict_strtoul(p, 16, &ivmbid);

	p = get_local_var("IVM_HWKey");
	if (p == NULL) {
		printf("can't get the IVM_HWKey\n");
		return 1;
	}
	rc = strict_strtoul(p, 16, &ivmhwkey);

	if (!ivmbid || !ivmhwkey) {
		printf("Error: IVM_BoardId and/or IVM_HWKey not set!\n");
		return rc;
	}

	/* now try to read values from environment if available */
	p = getenv("boardid");
	if (p != NULL)
		rc = strict_strtoul(p, 16, &envbid);
	p = getenv("hwkey");
	if (p != NULL)
		rc = strict_strtoul(p, 16, &envhwkey);

	if (rc != 0) {
		printf("strict_strtoul returns error: %d", rc);
		return rc;
	}

	if (!envbid || !envhwkey) {
		/*
		 * BoardId/HWkey not available in the environment, so try the
		 * environment variable for BoardId/HWkey list
		 */
		char *bidhwklist = getenv("boardIdListHex");

		if (bidhwklist) {
			int found = 0;
			char *rest = bidhwklist;
			char *endp;

			if (verbose) {
				printf("IVM_BoardId: %ld, IVM_HWKey=%ld\n",
					ivmbid, ivmhwkey);
				printf("boardIdHwKeyList: %s\n",
					bidhwklist);
			}
			while (!found) {
				/* loop over each bid/hwkey pair in the list */
				unsigned long bid   = 0;
				unsigned long hwkey = 0;

				while (*rest && !isxdigit(*rest))
					rest++;
				/*
				 * use simple_strtoul because we need &end and
				 * we know we got non numeric char at the end
				 */
				bid = simple_strtoul(rest, &endp, 16);
				/* BoardId and HWkey are separated with a "_" */
				if (*endp == '_') {
					rest  = endp + 1;
					/*
					 * use simple_strtoul because we need
					 * &end
					 */
					hwkey = simple_strtoul(rest, &endp, 16);
					rest  = endp;
					while (*rest && !isxdigit(*rest))
						rest++;
				}
				if ((!bid) || (!hwkey)) {
					/* end of list */
					break;
				}
				if (verbose) {
					printf("trying bid=0x%lX, hwkey=%ld\n",
						bid, hwkey);
				}
				/*
				 * Compare the values of the found entry in the
				 * list with the valid values which are stored
				 * in the inventory eeprom. If they are equal
				 * store the values in environment variables
				 * and save the environment.
				 * This can only happen once for the lifetime
				 * of a board, because once saved the function
				 * will never reach the while loop.
				 */
				if ((bid == ivmbid) && (hwkey == ivmhwkey)) {
					char buf[10];

					found = 1;
					envbid   = bid;
					envhwkey = hwkey;
					sprintf(buf, "%lx", bid);
					setenv("boardid", buf);
					sprintf(buf, "%lx", hwkey);
					setenv("hwkey", buf);
					saveenv();
				}
			} /* end while( ! found ) */
		}
	}

	/* compare now the values */
	if ((ivmbid == envbid) && (ivmhwkey == envhwkey)) {
		printf("boardid=0x%3lX, hwkey=%ld\n", envbid, envhwkey);
		rc = 0; /* match */
	} else {
		printf("Error: env bId=0x%3lX, hwKey=%ld\n", envbid, envhwkey);
		printf("       IVM bId=0x%3lX, hwKey=%ld\n", ivmbid, ivmhwkey);
		rc = 1; /* don't match */
	}
	return rc;
}

U_BOOT_CMD(km_checkbidhwk, 2, 0, do_checkboardidhwk,
		"check boardid and hwkey",
		"[v]\n  - check environment parameter "\
		"\"boardIdListHex\" against stored boardid and hwkey "\
		"from the IVM\n    v: verbose output"
);
