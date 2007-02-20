/******************************************************************************
*
*     Author: Xilinx, Inc.
*
*
*     This program is free software; you can redistribute it and/or modify it
*     under the terms of the GNU General Public License as published by the
*     Free Software Foundation; either version 2 of the License, or (at your
*     option) any later version.
*
*
*     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
*     COURTESY TO YOU. BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
*     ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION OR STANDARD,
*     XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION IS FREE
*     FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE FOR OBTAINING
*     ANY THIRD PARTY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
*     XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
*     THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY
*     WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM
*     CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND
*     FITNESS FOR A PARTICULAR PURPOSE.
*
*
*     Xilinx hardware products are not intended for use in life support
*     appliances, devices, or systems. Use in such applications is
*     expressly prohibited.
*
*
*     (c) Copyright 2002-2004 Xilinx Inc.
*     All rights reserved.
*
*
*     You should have received a copy of the GNU General Public License along
*     with this program; if not, write to the Free Software Foundation, Inc.,
*     675 Mass Ave, Cambridge, MA 02139, USA.
*
******************************************************************************/

#include <config.h>
#include <common.h>
#include <environment.h>
#include <net.h>

#ifdef CFG_ENV_IS_IN_EEPROM
#include <i2c.h>
#include "xiic_l.h"

#define IIC_DELAY     5000

static u8 envStep = 0;		/* 0 means crc has not been read */
const u8 hex[] = "0123456789ABCDEF"; /* lookup table for ML300 CRC */

/************************************************************************
 * Use Xilinx provided driver to send data to EEPROM using iic bus.
 */
static void
send(u32 adr, u8 * data, u32 len)
{
	u8 sendBuf[34];		/* first 2-bit is address and others are data */
	u32 pos, wlen;
	u32 ret;

	wlen = 32;
	for (pos = 0; pos < len; pos += 32) {
		if ((len - pos) < 32)
			wlen = len - pos;

		/* Put address and data bits together */
		sendBuf[0] = (u8) ((adr + pos) >> 8);
		sendBuf[1] = (u8) (adr + pos);
		memcpy(&sendBuf[2], &data[pos], wlen);

		/* Send to EEPROM through iic bus */
		ret = XIic_Send(XPAR_IIC_0_BASEADDR, CFG_I2C_EEPROM_ADDR >> 1,
				sendBuf, wlen + 2);

		udelay(IIC_DELAY);
	}
}

/************************************************************************
 * Use Xilinx provided driver to read data from EEPROM using the iic bus.
 */
static void
receive(u32 adr, u8 * data, u32 len)
{
	u8 address[2];
	u32 ret;

	address[0] = (u8) (adr >> 8);
	address[1] = (u8) adr;

	/* Provide EEPROM address */
	ret =
	    XIic_Send(XPAR_IIC_0_BASEADDR, CFG_I2C_EEPROM_ADDR >> 1, address,
		      2);
	/* Receive data from EEPROM */
	ret =
	    XIic_Recv(XPAR_IIC_0_BASEADDR, CFG_I2C_EEPROM_ADDR >> 1, data, len);
}

/************************************************************************
 * Convert a hexadecimal string to its equivalent integer value.
 */
static u8
axtoi(u8 * hexStg)
{
	u8 n;			/* position in string */
	u8 m;			/* position in digit[] to shift */
	u8 count;		/* loop index */
	u8 intValue;		/* integer value of hex string */
	u8 digit[2];		/* hold values to convert */

	for (n = 0; n < 2; n++) {
		if (hexStg[n] == '\0')
			break;
		if (hexStg[n] > 0x29 && hexStg[n] < 0x40)
			digit[n] = hexStg[n] & 0x0f;
		else if (hexStg[n] >= 'a' && hexStg[n] <= 'f')
			digit[n] = (hexStg[n] & 0x0f) + 9;
		else if (hexStg[n] >= 'A' && hexStg[n] <= 'F')
			digit[n] = (hexStg[n] & 0x0f) + 9;
		else
			break;
	}

	intValue = 0;
	count = n;
	m = n - 1;
	n = 0;
	while (n < count) {
		intValue = intValue | (digit[n] << (m << 2));
		m--;		/* adjust the position to set */
		n++;		/* next digit to process */
	}

	return (intValue);
}

/************************************************************************
 * Convert an integer string to its equivalent value.
 */
static u8
atoi(uchar * string)
{
	u8 res = 0;
	while (*string >= '0' && *string <= '9') {
		res *= 10;
		res += *string - '0';
		string++;
	}

	return res;
}

/************************************************************************
 * Key-value pairs are separated by "=" sign.
 */
static void
findKey(uchar * buffer, int *loc, u8 len)
{
	u32 i;

	for (i = 0; i < len; i++)
		if (buffer[i] == '=') {
			*loc = i;
			return;
		}

	/* return -1 is no "=" sign found */
	*loc = -1;
}

/************************************************************************
 * Compute a new ML300 CRC when user calls the saveenv command.
 * Also update EEPROM with new CRC value.
 */
static u8
update_crc(u32 len, uchar * data)
{
	uchar temp[6] = { 'C', '=', 0x00, 0x00, 0x00, 0x00 };
	u32 crc;		/* new crc value */
	u32 i;

	crc = 0;

	/* calculate new CRC */
	for (i = 0; i < len; i++)
		crc += data[i];

	/* CRC includes key for check sum */
	crc += 'C' + '=';

	/* compose new CRC to be updated */
	temp[2] = hex[(crc >> 4) & 0xf];
	temp[3] = hex[crc & 0xf];

	/* check to see if env size exceeded */
	if (len + 6 > ENV_SIZE) {
		printf("ERROR: not enough space to store CRC on EEPROM");
		return 1;
	}

	memcpy(data + len, temp, 6);
	return 0;
}

/************************************************************************
 * Read out ML300 CRC and compare it with a runtime calculated ML300 CRC.
 * If equal, then pass back a u-boot CRC value, otherwise pass back
 * junk to indicate CRC error.
*/
static void
read_crc(uchar * buffer, int len)
{
	u32 addr, n;
	u32 crc;		/* runtime crc */
	u8 old[2] = { 0xff, 0xff };	/* current CRC in EEPROM */
	u8 stop;		/* indication of end of env data */
	u8 pre;			/* previous EEPROM data bit */
	int i, loc;

	addr = CFG_ENV_OFFSET;	/* start from first env address */
	n = 0;
	pre = 1;
	stop = 1;
	crc = 0;

	/* calculate runtime CRC according to ML300 and read back
	   old CRC stored in the EEPROM */
	while (n < CFG_ENV_SIZE) {
		receive(addr, buffer, len);

		/* found two null chars, end of env */
		if ((pre || buffer[0]) == 0)
			break;

		findKey(buffer, &loc, len);

		/* found old check sum, read and store old CRC */
		if ((loc == 0 && pre == 'C')
		    || (loc > 0 && buffer[loc - 1] == 'C'))
			receive(addr + loc + 1, old, 2);

		pre = buffer[len - 1];

		/* calculate runtime ML300 CRC */
		crc += buffer[0];
		i = 1;
		do {
			crc += buffer[i];
			stop = buffer[i] || buffer[i - 1];
			i++;
		} while (stop && (i < len));

		if (stop == 0)
			break;

		n += len;
		addr += len;
	}

	/* exclude old CRC from runtime calculation */
	crc -= (old[0] + old[1]);

	/* match CRC values, send back u-boot CRC */
	if ((old[0] == hex[(crc >> 4) & 0xf])
	    && (old[1] == hex[crc & 0xf])) {
		crc = 0;
		n = 0;
		addr =
		    CFG_ENV_OFFSET - offsetof(env_t, crc) + offsetof(env_t,
								     data);
		/* calculate u-boot crc */
		while (n < ENV_SIZE) {
			receive(addr, buffer, len);
			crc = crc32(crc, buffer, len);
			n += len;
			addr += len;
		}

		memcpy(buffer, &crc, 4);
	}
}

/************************************************************************
 * Convert IP address to hexadecimals.
 */
static void
ip_ml300(uchar * s, uchar * res)
{
	char temp[2];
	u8 i;

	res[0] = 0x00;

	for (i = 0; i < 4; i++) {
		sprintf(temp, "%02x", atoi(s));
		s = (uchar *)strchr((char *)s, '.') + 1;
		strcat((char *)res, temp);
	}
}

/************************************************************************
 * Change 0xff (255), a dummy null char to 0x00.
 */
static void
change_null(uchar * s)
{
	if (s != NULL) {
		change_null((uchar *)strchr((char *)s + 1, 255));
		*(strchr((char *)s, 255)) = '\0';
	}
}

/************************************************************************
 * Update environment variable name and values to u-boot standard.
 */
void
convert_env(void)
{
	char *s;		/* pointer to env value */
	char temp[20];		/* temp storage for addresses */

	/* E -> ethaddr */
	s = getenv("E");
	if (s != NULL) {
		sprintf(temp, "%c%c.%c%c.%c%c.%c%c.%c%c.%c%c",
			s[0], s[1],  s[ 2], s[ 3],
			s[4], s[5],  s[ 6], s[ 7],
			s[8], s[9],  s[10], s[11] );
		setenv("ethaddr", temp);
		setenv("E", NULL);
	}

	/* L -> serial# */
	s = getenv("L");
	if (s != NULL) {
		setenv("serial#", s);
		setenv("L", NULL);
	}

	/* I -> ipaddr */
	s = getenv("I");
	if (s != NULL) {
		sprintf(temp, "%d.%d.%d.%d", axtoi((u8 *)s), axtoi((u8 *)(s + 2)),
			axtoi((u8 *)(s + 4)), axtoi((u8 *)(s + 6)));
		setenv("ipaddr", temp);
		setenv("I", NULL);
	}

	/* S -> serverip */
	s = getenv("S");
	if (s != NULL) {
		sprintf(temp, "%d.%d.%d.%d", axtoi((u8 *)s), axtoi((u8 *)(s + 2)),
			axtoi((u8 *)(s + 4)), axtoi((u8 *)(s + 6)));
		setenv("serverip", temp);
		setenv("S", NULL);
	}

	/* A -> bootargs */
	s = getenv("A");
	if (s != NULL) {
		setenv("bootargs", s);
		setenv("A", NULL);
	}

	/* F -> bootfile */
	s = getenv("F");
	if (s != NULL) {
		setenv("bootfile", s);
		setenv("F", NULL);
	}

	/* M -> bootcmd */
	s = getenv("M");
	if (s != NULL) {
		setenv("bootcmd", s);
		setenv("M", NULL);
	}

	/* Don't include C (CRC) */
	setenv("C", NULL);
}

/************************************************************************
 * Save user modified environment values back to EEPROM.
 */
static void
save_env(void)
{
	char eprom[ENV_SIZE];	/* buffer to be written back to EEPROM */
	char *s, temp[20];
	char ff[] = { 0xff, 0x00 };	/* dummy null value */
	u32 len;		/* length of env to be written to EEPROM */

	eprom[0] = 0x00;

	/* ethaddr -> E */
	s = getenv("ethaddr");
	if (s != NULL) {
		strcat(eprom, "E=");
		sprintf(temp, "%c%c%c%c%c%c%c%c%c%c%c%c",
			*s, *(s + 1), *(s + 3), *(s + 4), *(s + 6), *(s + 7),
			*(s + 9), *(s + 10), *(s + 12), *(s + 13), *(s + 15),
			*(s + 16));
		strcat(eprom, temp);
		strcat(eprom, ff);
	}

	/* serial# -> L */
	s = getenv("serial#");
	if (s != NULL) {
		strcat(eprom, "L=");
		strcat(eprom, s);
		strcat(eprom, ff);
	}

	/* ipaddr -> I */
	s = getenv("ipaddr");
	if (s != NULL) {
		strcat(eprom, "I=");
		ip_ml300((uchar *)s, (uchar *)temp);
		strcat(eprom, temp);
		strcat(eprom, ff);
	}

	/* serverip -> S */
	s = getenv("serverip");
	if (s != NULL) {
		strcat(eprom, "S=");
		ip_ml300((uchar *)s, (uchar *)temp);
		strcat(eprom, temp);
		strcat(eprom, ff);
	}

	/* bootargs -> A */
	s = getenv("bootargs");
	if (s != NULL) {
		strcat(eprom, "A=");
		strcat(eprom, s);
		strcat(eprom, ff);
	}

	/* bootfile -> F */
	s = getenv("bootfile");
	if (s != NULL) {
		strcat(eprom, "F=");
		strcat(eprom, s);
		strcat(eprom, ff);
	}

	/* bootcmd -> M */
	s = getenv("bootcmd");
	if (s != NULL) {
		strcat(eprom, "M=");
		strcat(eprom, s);
		strcat(eprom, ff);
	}

	len = strlen(eprom);	/* find env length without crc */
	change_null((uchar *)eprom);	/* change 0xff to 0x00 */

	/* update EEPROM env values if there is enough space */
	if (update_crc(len, (uchar *)eprom) == 0)
		send(CFG_ENV_OFFSET, (uchar *)eprom, len + 6);
}

/************************************************************************
 * U-boot call for EEPROM read associated activities.
 */
int
i2c_read(uchar chip, uint addr, int alen, uchar * buffer, int len)
{

	if (envStep == 0) {
		/* first read call is for crc */
		read_crc(buffer, len);
		++envStep;
		return 0;
	} else if (envStep == 1) {
		/* then read out EEPROM content for runtime u-boot CRC calculation */
		receive(addr, buffer, len);

		if (addr + len - CFG_ENV_OFFSET == CFG_ENV_SIZE)
			/* end of runtime crc read */
			++envStep;
		return 0;
	}

	if (len < 2) {
		/* when call getenv_r */
		receive(addr, buffer, len);
	} else if (addr + len < CFG_ENV_OFFSET + CFG_ENV_SIZE) {
		/* calling env_relocate(), but don't read out
		   crc value from EEPROM */
		receive(addr, buffer + 4, len);
	} else {
		receive(addr, buffer + 4, len - 4);
	}

	return 0;

}

/************************************************************************
 * U-boot call for EEPROM write acativities.
 */
int
i2c_write(uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	/* save env on last page write called by u-boot */
	if (addr + len >= CFG_ENV_OFFSET + CFG_ENV_SIZE)
		save_env();

	return 0;
}

/************************************************************************
 * Dummy function.
 */
int
i2c_probe(uchar chip)
{
	return 1;
}

#endif
