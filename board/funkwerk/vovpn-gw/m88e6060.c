/*
 * (C) Copyright 2004
 * Elmeg Communications Systems GmbH, Juergen Selent (j.selent@elmeg.de)
 *
 * Support for the Elmeg VoVPN Gateway Module
 * ------------------------------------------
 * Initialize Marvell M88E6060 Switch
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
#include <ioports.h>
#include <mpc8260.h>
#include <asm/m8260_pci.h>
#include <net.h>
#include <miiphy.h>

#include "m88e6060.h"

#if (CONFIG_COMMANDS & CFG_CMD_NET) || defined(CONFIG_CMD_NET)
static int		prtTab[M88X_PRT_CNT] = { 8, 9, 10, 11, 12, 13 };
static int		phyTab[M88X_PHY_CNT] = { 0, 1, 2, 3, 4 };

static m88x_regCfg_t	prtCfg0[] = {
	{  4, 0x3e7c, 0x8000 },
	{  4, 0x3e7c, 0x8003 },
	{  6, 0x0fc0, 0x001e },
	{ -1, 0xffff, 0x0000 }
};

static m88x_regCfg_t	prtCfg1[] = {
	{  4, 0x3e7c, 0x8000 },
	{  4, 0x3e7c, 0x8003 },
	{  6, 0x0fc0, 0x001d },
	{ -1, 0xffff, 0x0000 }
};

static m88x_regCfg_t	prtCfg2[] = {
	{  4, 0x3e7c, 0x8000 },
	{  4, 0x3e7c, 0x8003 },
	{  6, 0x0fc0, 0x001b },
	{ -1, 0xffff, 0x0000 }
};

static m88x_regCfg_t	prtCfg3[] = {
	{  4, 0x3e7c, 0x8000 },
	{  4, 0x3e7c, 0x8003 },
	{  6, 0x0fc0, 0x0017 },
	{ -1, 0xffff, 0x0000 }
};

static m88x_regCfg_t	prtCfg4[] = {
	{  4, 0x3e7c, 0x8000 },
	{  4, 0x3e7c, 0x8003 },
	{  6, 0x0fc0, 0x000f },
	{ -1, 0xffff, 0x0000 }
};

static m88x_regCfg_t	*prtCfg[M88X_PRT_CNT] = {
	prtCfg0,prtCfg1,prtCfg2,prtCfg3,prtCfg4,NULL
};

static m88x_regCfg_t	phyCfgX[] = {
	{  4, 0xfa1f, 0x01e0 },
	{  0, 0x213f, 0x1200 },
	{ 24, 0x81ff, 0x1200 },
	{ -1, 0xffff, 0x0000 }
};

static m88x_regCfg_t	*phyCfg[M88X_PHY_CNT] = {
	phyCfgX,phyCfgX,phyCfgX,phyCfgX,NULL
};

#if 0
static void
m88e6060_dump( int devAddr )
{
	int		i, j;
	unsigned short	val[6];

	printf( "M88E6060 Register Dump\n" );
	printf( "====================================\n" );
	printf( "PortNo    0    1    2    3    4    5\n" );
	for (i=0; i<6; i++)
		miiphy_read( devAddr+prtTab[i],M88X_PRT_STAT,&val[i] );
	printf( "STAT   %04hx %04hx %04hx %04hx %04hx %04hx\n",
		val[0],val[1],val[2],val[3],val[4],val[5] );

	for (i=0; i<6; i++)
		miiphy_read( devAddr+prtTab[i],M88X_PRT_ID,&val[i] );
	printf( "ID     %04hx %04hx %04hx %04hx %04hx %04hx\n",
		val[0],val[1],val[2],val[3],val[4],val[5] );

	for (i=0; i<6; i++)
		miiphy_read( devAddr+prtTab[i],M88X_PRT_CNTL,&val[i] );
	printf( "CNTL   %04hx %04hx %04hx %04hx %04hx %04hx\n",
		val[0],val[1],val[2],val[3],val[4],val[5] );

	for (i=0; i<6; i++)
		miiphy_read( devAddr+prtTab[i],M88X_PRT_VLAN,&val[i] );
	printf( "VLAN   %04hx %04hx %04hx %04hx %04hx %04hx\n",
		val[0],val[1],val[2],val[3],val[4],val[5] );

	for (i=0; i<6; i++)
		miiphy_read( devAddr+prtTab[i],M88X_PRT_PAV,&val[i] );
	printf( "PAV    %04hx %04hx %04hx %04hx %04hx %04hx\n",
		val[0],val[1],val[2],val[3],val[4],val[5] );

	for (i=0; i<6; i++)
		miiphy_read( devAddr+prtTab[i],M88X_PRT_RX,&val[i] );
	printf( "RX     %04hx %04hx %04hx %04hx %04hx %04hx\n",
		val[0],val[1],val[2],val[3],val[4],val[5] );

	for (i=0; i<6; i++)
		miiphy_read( devAddr+prtTab[i],M88X_PRT_TX,&val[i] );
	printf( "TX     %04hx %04hx %04hx %04hx %04hx %04hx\n",
		val[0],val[1],val[2],val[3],val[4],val[5] );

	printf( "------------------------------------\n" );
	printf( "PhyNo     0    1    2    3    4\n" );
	for (i=0; i<9; i++) {
		for (j=0; j<5; j++) {
			miiphy_read( devAddr+phyTab[j],i,&val[j] );
		}
		printf( "0x%02x   %04hx %04hx %04hx %04hx %04hx\n",
			i,val[0],val[1],val[2],val[3],val[4] );
	}
	for (i=0x10; i<0x1d; i++) {
		for (j=0; j<5; j++) {
			miiphy_read( devAddr+phyTab[j],i,&val[j] );
		}
		printf( "0x%02x   %04hx %04hx %04hx %04hx %04hx\n",
			i,val[0],val[1],val[2],val[3],val[4] );
	}
}
#endif

int
m88e6060_initialize( int devAddr )
{
	static char	*_f = "m88e6060_initialize:";
	m88x_regCfg_t	*p;
	int		err;
	int		i;
	unsigned short	val;

	/*** reset all phys into powerdown ************************************/
	for (i=0, err=0; i<M88X_PHY_CNT; i++) {
		err += bb_miiphy_read(NULL, devAddr+phyTab[i],M88X_PHY_CNTL,&val );
		/* keep SpeedLSB, Duplex */
		val &= 0x2100;
		/* set SWReset, AnegEn, PwrDwn, RestartAneg */
		val |= 0x9a00;
		err += bb_miiphy_write(NULL, devAddr+phyTab[i],M88X_PHY_CNTL,val );
	}
	if (err) {
		printf( "%s [ERR] reset phys\n",_f );
		return( -1 );
	}

	/*** disable all ports ************************************************/
	for (i=0, err=0; i<M88X_PRT_CNT; i++) {
		err += bb_miiphy_read(NULL, devAddr+prtTab[i],M88X_PRT_CNTL,&val );
		val &= 0xfffc;
		err += bb_miiphy_write(NULL, devAddr+prtTab[i],M88X_PRT_CNTL,val );
	}
	if (err) {
		printf( "%s [ERR] disable ports\n",_f );
		return( -1 );
	}

	/*** initialize switch ************************************************/
	/* set switch mac addr */
#define ea eth_get_dev()->enetaddr
	val = (ea[4] <<  8) | ea[5];
	err = bb_miiphy_write(NULL, devAddr+15,M88X_GLB_MAC45,val );
	val = (ea[2] <<  8) | ea[3];
	err += bb_miiphy_write(NULL, devAddr+15,M88X_GLB_MAC23,val );
	val = (ea[0] <<  8) | ea[1];
#undef ea
	val &= 0xfeff;		/* clear DiffAddr */
	err += bb_miiphy_write(NULL, devAddr+15,M88X_GLB_MAC01,val );
	if (err) {
		printf( "%s [ERR] switch mac address register\n",_f );
		return( -1 );
	}

	/* !DiscardExcessive, MaxFrameSize, CtrMode */
	err = bb_miiphy_read(NULL, devAddr+15,M88X_GLB_CNTL,&val );
	val &= 0xd870;
	val |= 0x0500;
	err += bb_miiphy_write(NULL, devAddr+15,M88X_GLB_CNTL,val );
	if (err) {
		printf( "%s [ERR] switch global control register\n",_f );
		return( -1 );
	}

	/* LernDis off, ATUSize 1024, AgeTime 5min */
	err = bb_miiphy_read(NULL, devAddr+15,M88X_ATU_CNTL,&val );
	val &= 0x000f;
	val |= 0x2130;
	err += bb_miiphy_write(NULL, devAddr+15,M88X_ATU_CNTL,val );
	if (err) {
		printf( "%s [ERR] atu control register\n",_f );
		return( -1 );
	}

	/*** initialize ports *************************************************/
	for (i=0; i<M88X_PRT_CNT; i++) {
		if ((p = prtCfg[i]) == NULL) {
			continue;
		}
		while (p->reg != -1) {
			err = 0;
			err += bb_miiphy_read(NULL, devAddr+prtTab[i],p->reg,&val );
			val &= p->msk;
			val |= p->val;
			err += bb_miiphy_write(NULL, devAddr+prtTab[i],p->reg,val );
			if (err) {
				printf( "%s [ERR] config port %d register %d\n",_f,i,p->reg );
				/* XXX what todo */
			}
			p++;
		}
	}

	/*** initialize phys **************************************************/
	for (i=0; i<M88X_PHY_CNT; i++) {
		if ((p = phyCfg[i]) == NULL) {
			continue;
		}
		while (p->reg != -1) {
			err = 0;
			err += bb_miiphy_read(NULL, devAddr+phyTab[i],p->reg,&val );
			val &= p->msk;
			val |= p->val;
			err += bb_miiphy_write(NULL, devAddr+phyTab[i],p->reg,val );
			if (err) {
				printf( "%s [ERR] config phy %d register %d\n",_f,i,p->reg );
				/* XXX what todo */
			}
			p++;
		}
	}
	udelay(100000);
	return( 0 );
}
#endif
