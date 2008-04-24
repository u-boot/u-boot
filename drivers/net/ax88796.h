/*
 * AX88796L(NE2000) support
 *
 * (c) 2007 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
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
 */

#ifndef __DRIVERS_AX88796L_H__
#define __DRIVERS_AX88796L_H__

#define DP_DATA		(0x10 << 1)
#define START_PG	0x40	/* First page of TX buffer */
#define START_PG2	0x48
#define STOP_PG		0x80	/* Last page +1 of RX ring */
#define TX_PAGES	12
#define RX_START	(START_PG+TX_PAGES)
#define RX_END		STOP_PG

#define AX88796L_BASE_ADDRESS	CONFIG_DRIVER_NE2000_BASE
#define AX88796L_BYTE_ACCESS	0x00001000
#define AX88796L_OFFSET		0x00000400
#define AX88796L_ADDRESS_BYTE	AX88796L_BASE_ADDRESS + \
		AX88796L_BYTE_ACCESS + AX88796L_OFFSET
#define AX88796L_REG_MEMR	AX88796L_ADDRESS_BYTE + (0x14<<1)
#define AX88796L_REG_CR		AX88796L_ADDRESS_BYTE + (0x00<<1)

#define AX88796L_CR		(*(vu_short *)(AX88796L_REG_CR))
#define AX88796L_MEMR		(*(vu_short *)(AX88796L_REG_MEMR))

#define EECS_HIGH		(AX88796L_MEMR |= 0x10)
#define EECS_LOW		(AX88796L_MEMR &= 0xef)
#define EECLK_HIGH		(AX88796L_MEMR |= 0x80)
#define EECLK_LOW		(AX88796L_MEMR &= 0x7f)
#define EEDI_HIGH		(AX88796L_MEMR |= 0x20)
#define EEDI_LOW		(AX88796L_MEMR &= 0xdf)
#define EEDO			((AX88796L_MEMR & 0x40)>>6)

#define PAGE0_SET		(AX88796L_CR &= 0x3f)
#define PAGE1_SET		(AX88796L_CR = (AX88796L_CR & 0x3f) | 0x40)

#define BIT_DUMMY	0
#define MAC_EEP_READ	1
#define MAC_EEP_WRITE	2
#define MAC_EEP_ERACE	3
#define MAC_EEP_EWEN	4
#define MAC_EEP_EWDS	5

/* R7780MP Specific code */
#if defined(CONFIG_R7780MP)
#define ISA_OFFSET	0x1400
#define DP_IN(_b_, _o_, _d_)	(_d_) = \
	*( (vu_short *) ((_b_) + ((_o_) * 2) + ISA_OFFSET))
#define DP_OUT(_b_, _o_, _d_) \
	*((vu_short *)((_b_) + ((_o_) * 2) + ISA_OFFSET)) = (_d_)
#define DP_IN_DATA(_b_, _d_)	(_d_) = *( (vu_short *) ((_b_) + ISA_OFFSET))
#define DP_OUT_DATA(_b_, _d_)	*( (vu_short *) ((_b_)+ISA_OFFSET)) = (_d_)
#else
/* Please change for your target boards */
#define ISA_OFFSET	0x0000
#define DP_IN(_b_, _o_, _d_)	(_d_) = *( (vu_short *)((_b_)+(_o_ )+ISA_OFFSET))
#define DP_OUT(_b_, _o_, _d_)	*((vu_short *)((_b_)+(_o_)+ISA_OFFSET)) = (_d_)
#define DP_IN_DATA(_b_, _d_)	(_d_) = *( (vu_short *) ((_b_)+ISA_OFFSET))
#define DP_OUT_DATA(_b_, _d_)	*( (vu_short *) ((_b_)+ISA_OFFSET)) = (_d_)
#endif


/*
 * Set 1 bit data
 */
static void ax88796_bitset(u32 bit)
{
	/* DATA1 */
	if( bit )
		EEDI_HIGH;
	else
		EEDI_LOW;

	EECLK_LOW;
	udelay(1000);
	EECLK_HIGH;
	udelay(1000);
	EEDI_LOW;
}

/*
 * Get 1 bit data
 */
static u8 ax88796_bitget(void)
{
	u8 bit;

	EECLK_LOW;
	udelay(1000);
	/* DATA */
	bit = EEDO;
	EECLK_HIGH;
	udelay(1000);

	return bit;
}

/*
 * Send COMMAND to EEPROM
 */
static void ax88796_eep_cmd(u8 cmd)
{
	ax88796_bitset(BIT_DUMMY);
	switch(cmd){
		case MAC_EEP_READ:
			ax88796_bitset(1);
			ax88796_bitset(1);
			ax88796_bitset(0);
			break;

		case MAC_EEP_WRITE:
			ax88796_bitset(1);
			ax88796_bitset(0);
			ax88796_bitset(1);
			break;

		case MAC_EEP_ERACE:
			ax88796_bitset(1);
			ax88796_bitset(1);
			ax88796_bitset(1);
			break;

		case MAC_EEP_EWEN:
			ax88796_bitset(1);
			ax88796_bitset(0);
			ax88796_bitset(0);
			break;

		case MAC_EEP_EWDS:
			ax88796_bitset(1);
			ax88796_bitset(0);
			ax88796_bitset(0);
			break;
		default:
			break;
	}
}

static void ax88796_eep_setaddr(u16 addr)
{
	int i ;
	for( i = 7 ; i >= 0 ; i-- )
		ax88796_bitset(addr & (1 << i));
}

/*
 * Get data from EEPROM
 */
static u16 ax88796_eep_getdata(void)
{
	ushort data = 0;
	int i;

	ax88796_bitget();	/* DUMMY */
	for( i = 0 ; i < 16 ; i++ ){
		data <<= 1;
		data |= ax88796_bitget();
	}
	return data;
}

static void ax88796_mac_read(u8 *buff)
{
	int i ;
	u16 data, addr = 0;

	for( i = 0 ; i < 3; i++ )
	{
		EECS_HIGH;
		EEDI_LOW;
		udelay(1000);
		/* READ COMMAND */
		ax88796_eep_cmd(MAC_EEP_READ);
		/* ADDRESS */
		ax88796_eep_setaddr(addr++);
		/* GET DATA */
		data = ax88796_eep_getdata();
		*buff++ = (uchar)(data & 0xff);
		*buff++ = (uchar)((data >> 8) & 0xff);
		EECLK_LOW;
		EEDI_LOW;
		EECS_LOW;
	}
}

int get_prom(u8* mac_addr)
{
	u8 prom[32];
	int i;

	ax88796_mac_read(prom);
	for (i = 0; i < 6; i++){
		mac_addr[i] = prom[i];
	}
	return 1;
}

#endif /* __DRIVERS_AX88796L_H__ */
