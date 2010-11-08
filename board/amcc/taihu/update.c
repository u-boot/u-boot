/*
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

#include <config.h>
#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <i2c.h>

#define PCI_M66EN 0x10

static uchar buf_33[] =
{
	0xb5,	/* 0x00:hce =1, bss = 0, pae=1, ppdv= 0b10,spe = 1,ebw=0b01*/
	0x80,	/* 0x01~0x03:ptm1ms =0x80000001 */
	0x00,
	0x00,
	0x00,	/* 0x04~0x06:ptm1la = 0x00000000 */
	0x00,
	0x00,
	0x00,	/* 0x07~0x09:ptm2ma = 0x00000000 */
	0x00,
	0x00,
	0x00,	/* 0x0a~0x0c:ptm2la = 0x00000000 */
	0x00,
	0x00,
	0x10,	/* 0x0d~0x0e:vendor id 0x1014*/
	0x14,
	0x00,	/* 0x0f~0x10:device id 0x0000*/
	0x00,
	0x00,	/* 0x11:revision 0x00 */
	0x00,	/* 0x12~0x14:class 0x000000 */
	0x00,
	0x00,
	0x10,	/* 0x15~0x16:subsystem vendor id */
	0xe8,
	0x00,	/* 0x17~0x18:subsystem device id */
	0x00,
	0x61,	/* 0x19: opdv=0b01,cbdv=0b10,ccdv=0b00,ptm2ms_ena=0, ptm1ms_ena=1 */
	0x68,	/* 0x1a: rpci=1,fbmul=0b1010,epdv=0b00 */
	0x2d,	/* 0x1b: fwdvb=0b101,fwdva=0b101 */
	0x82,	/* 0x1c: pllr=1,sscs=0,mpdv=0b00,tun[22-23]=0b10 */
	0xbe,	/* 0x1d: tun[24-31]=0xbe */
	0x00,
	0x00
};

static uchar buf_66[] =
{
	0xb5,	/* 0x00:hce =1, bss = 0, pae=1, ppdv= 0b10,spe = 1,ebw=0b01*/
	0x80,	/* 0x01~0x03:ptm1ms =0x80000001 */
	0x00,
	0x00,
	0x00,	/* 0x04~0x06:ptm1la = 0x00000000 */
	0x00,
	0x00,
	0x00,	/* 0x07~0x09:ptm2ma = 0x00000000 */
	0x00,
	0x00,
	0x00,	/* 0x0a~0x0c:ptm2la = 0x00000000 */
	0x00,
	0x00,
	0x10,	/* 0x0d~0x0e:vendor id 0x1014*/
	0x14,
	0x00,	/* 0x0f~0x10:device id 0x0000*/
	0x00,
	0x00,	/* 0x11:revision 0x00 */
	0x00,	/* 0x12~0x14:class 0x000000 */
	0x00,
	0x00,
	0x10,	/* 0x15~0x16:subsystem vendor id */
	0xe8,
	0x00,	/* 0x17~0x18:subsystem device id */
	0x00,
	0x61,	/* 0x19: opdv=0b01,cbdv=0b10,ccdv=0b00,ptm2ms_ena=0, ptm1ms_ena=1 */
	0x68,	/* 0x1a: rpci=1,fbmul=0b1010,epdv=0b00 */
	0x2d,	/* 0x1b: fwdvb=0b101,fwdva=0b101 */
	0x82,	/* 0x1c: pllr=1,sscs=0,mpdv=0b00,tun[22-23]=0b10 */
	0xbe,	/* 0x1d: tun[24-31]=0xbe */
	0x00,
	0x00
};

static int update_boot_eeprom(cmd_tbl_t* cmdtp, int flag, int argc, char * const argv[])
{
	ulong len = 0x20;
	uchar chip = CONFIG_SYS_I2C_EEPROM_ADDR;
	uchar *pbuf;
	uchar base;
	int i;

	if ((*(volatile char*)CPLD_REG0_ADDR & PCI_M66EN) != PCI_M66EN) {
		pbuf = buf_33;
		base = 0x00;
	} else {
		pbuf = buf_66;
		base = 0x40;
	}

	for (i = 0; i< len; i++, base++) {
		if (i2c_write(chip, base, 1, &pbuf[i],1)!= 0) {
			printf("i2c_write fail\n");
			return 1;
		}
		udelay(11000);
	}

	return 0;
}

U_BOOT_CMD (
	update_boot_eeprom, 1, 1, update_boot_eeprom,
	"update boot eeprom content",
	""
);
