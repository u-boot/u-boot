/*
 * PLB2800 internal switch ethernet driver.
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(CONFIG_NET_MULTI) \
	&& defined(CONFIG_PLB2800_ETHER)

#include <malloc.h>
#include <net.h>
#include <asm/addrspace.h>


#define NUM_RX_DESC	PKTBUFSRX
#define TOUT_LOOP	1000000

#define LONG_REF(addr) (*((volatile unsigned long*)addr))

#define CMAC_CRX_CTRL	LONG_REF(0xb800c870)
#define CMAC_CTX_CTRL	LONG_REF(0xb800c874)
#define SYS_MAC_ADDR_0	LONG_REF(0xb800c878)
#define SYS_MAC_ADDR_1	LONG_REF(0xb800c87c)
#define MIPS_H_MASK	LONG_REF(0xB800C810)

#define MA_LEARN	LONG_REF(0xb8008004)
#define DA_LOOKUP	LONG_REF(0xb8008008)

#define CMAC_CRX_CTRL_PD	0x00000001
#define CMAC_CRX_CTRL_CG	0x00000002
#define CMAC_CRX_CTRL_PL_SHIFT	2
#define CMAC_CRIT		0x0
#define CMAC_NON_CRIT		0x1
#define MBOX_STAT_ID_SHF	28
#define MBOX_STAT_CP		0x80000000
#define MBOX_STAT_MB		0x00000001
#define EN_MA_LEARN		0x02000000
#define EN_DA_LKUP		0x01000000
#define MA_DEST_SHF		11
#define DA_DEST_SHF		11
#define DA_STATE_SHF		19
#define TSTAMP_MS		0x00000000
#define SW_H_MBOX4_MASK		0x08000000
#define SW_H_MBOX3_MASK		0x04000000
#define SW_H_MBOX2_MASK		0x02000000
#define SW_H_MBOX1_MASK		0x01000000

typedef volatile struct {
  unsigned int stat;
  unsigned int cmd;
  unsigned int cnt;
  unsigned int adr;
} mailbox_t;

#define MBOX_REG(mb) ((mailbox_t*)(0xb800c830+(mb<<4)))

typedef volatile struct {
  unsigned int word0;
  unsigned int word1;
  unsigned int word2;
} mbhdr_t;

#define MBOX_MEM(mb) ((void*)(0xb800a000+((3-mb)<<11)))


static int plb2800_eth_init(struct eth_device *dev, bd_t * bis);
static int plb2800_eth_send(struct eth_device *dev, volatile void *packet,
						  int length);
static int plb2800_eth_recv(struct eth_device *dev);
static void plb2800_eth_halt(struct eth_device *dev);

static void plb2800_set_mac_addr(struct eth_device *dev, unsigned char * addr);
static unsigned char * plb2800_get_mac_addr(void);

static int rx_new;
static int mac_addr_set = 0;


int plb2800_eth_initialize(bd_t * bis)
{
	struct eth_device *dev;
	ulong temp;

#ifdef DEBUG
	printf("Entered plb2800_eth_initialize()\n");
#endif

	if (!(dev = (struct eth_device *) malloc (sizeof *dev)))
	{
		printf("Failed to allocate memory\n");
		return 0;
	}
	memset(dev, 0, sizeof(*dev));

	sprintf(dev->name, "PLB2800 Switch");
	dev->init = plb2800_eth_init;
	dev->halt = plb2800_eth_halt;
	dev->send = plb2800_eth_send;
	dev->recv = plb2800_eth_recv;

	eth_register(dev);

	/* bug fix */
	*(ulong *)0xb800e800 = 0x838;

	/* Set MBOX ownership */
	temp = CMAC_CRIT << MBOX_STAT_ID_SHF;
	MBOX_REG(0)->stat = temp;
	MBOX_REG(1)->stat = temp;

	temp = CMAC_NON_CRIT << MBOX_STAT_ID_SHF;
	MBOX_REG(2)->stat = temp;
	MBOX_REG(3)->stat = temp;

	plb2800_set_mac_addr(dev, plb2800_get_mac_addr());

	/* Disable all Mbox interrupt */
	temp = MIPS_H_MASK;
	temp &= ~ (SW_H_MBOX1_MASK | SW_H_MBOX2_MASK | SW_H_MBOX3_MASK | SW_H_MBOX4_MASK) ;
	MIPS_H_MASK = temp;

#ifdef DEBUG
	printf("Leaving plb2800_eth_initialize()\n");
#endif

	return 1;
}

static int plb2800_eth_init(struct eth_device *dev, bd_t * bis)
{
#ifdef DEBUG
	printf("Entering plb2800_eth_init()\n");
#endif

	plb2800_set_mac_addr(dev, dev->enetaddr);

	rx_new = 0;

#ifdef DEBUG
	printf("Leaving plb2800_eth_init()\n");
#endif

	return 0;
}


static int plb2800_eth_send(struct eth_device *dev, volatile void *packet,
						  int length)
{
	int                    i;
	int                    res         = -1;
	u32                    temp;
	mailbox_t *            mb          = MBOX_REG(0);
	char      *            mem         = MBOX_MEM(0);

#ifdef DEBUG
	printf("Entered plb2800_eth_send()\n");
#endif

	if (length <= 0)
	{
		printf ("%s: bad packet size: %d\n", dev->name, length);
		goto Done;
	}

	if (length < 64)
	{
		length = 64;
	}

	temp = CMAC_CRX_CTRL_CG | ((length + 4) << CMAC_CRX_CTRL_PL_SHIFT);

#ifdef DEBUG
	printf("0 mb->stat = 0x%x\n",  mb->stat);
#endif

	for(i = 0; mb->stat & (MBOX_STAT_CP | MBOX_STAT_MB); i++)
	{
		if (i >= TOUT_LOOP)
		{
			printf("%s: tx buffer not ready\n", dev->name);
			printf("1 mb->stat = 0x%x\n",  mb->stat);
			goto Done;
		}
	}

		/* For some strange reason, memcpy doesn't work, here!
		 */
	do
	{
		int words = (length >> 2) + 1;
		unsigned int* dst = (unsigned int*)(mem);
		unsigned int* src = (unsigned int*)(packet);
		for (i = 0; i < words; i++)
		{
			*dst = *src;
			dst++;
			src++;
		};
	} while(0);

	CMAC_CRX_CTRL = temp;
	mb->cmd = MBOX_STAT_CP;

#ifdef DEBUG
	printf("2 mb->stat = 0x%x\n",  mb->stat);
#endif

	res = length;
Done:

#ifdef DEBUG
	printf("Leaving plb2800_eth_send()\n");
#endif

	return res;
}


static int plb2800_eth_recv(struct eth_device *dev)
{
	int                    length  = 0;
	mailbox_t            * mbox    = MBOX_REG(3);
	unsigned char        * hdr     = MBOX_MEM(3);
	unsigned int           stat;

#ifdef DEBUG
	printf("Entered plb2800_eth_recv()\n");
#endif

	for (;;)
	{
		stat = mbox->stat;

		if (!(stat & MBOX_STAT_CP))
		{
			break;
		}

		length = ((*(hdr + 6) & 0x3f) << 8) + *(hdr + 7);
		memcpy((void *)NetRxPackets[rx_new], hdr + 12, length);

		stat &= ~MBOX_STAT_CP;
		mbox->stat = stat;
#ifdef DEBUG
		{
			int i;
			for (i=0;i<length - 4;i++)
			{
				if (i % 16 == 0) printf("\n%04x: ", i);
				printf("%02X ", NetRxPackets[rx_new][i]);
			}
			printf("\n");
		}
#endif

		if (length)
		{
#ifdef DEBUG
			printf("Received %d bytes\n", length);
#endif
			NetReceive((void*)(NetRxPackets[rx_new]),
			            length - 4);
		}
		else
		{
#if 1
			printf("Zero length!!!\n");
#endif
		}

		rx_new = (rx_new + 1) % NUM_RX_DESC;
	}

#ifdef DEBUG
	printf("Leaving plb2800_eth_recv()\n");
#endif

	return length;
}


static void plb2800_eth_halt(struct eth_device *dev)
{
#ifdef DEBUG
	printf("Entered plb2800_eth_halt()\n");
#endif

#ifdef DEBUG
	printf("Leaving plb2800_eth_halt()\n");
#endif
}

static void plb2800_set_mac_addr(struct eth_device *dev, unsigned char * addr)
{
	char packet[60];
	ulong temp;
	int ix;

	if (mac_addr_set ||
	    NULL == addr || memcmp(addr, "\0\0\0\0\0\0", 6) == 0)
	{
		return;
	}

	/* send one packet through CPU port
	 * in order to learn system MAC address
	 */

	/* Set DA_LOOKUP register */
	temp = EN_MA_LEARN | (0 << DA_STATE_SHF) | (63 << DA_DEST_SHF);
	DA_LOOKUP = temp;

	/* Set MA_LEARN register */
	temp = 50 << MA_DEST_SHF; 	/* static entry */
	MA_LEARN = temp;

	/* set destination address */
	for (ix=0;ix<6;ix++)
		packet[ix] = 0xff;

	/* set source address = system MAC address */
	for (ix=0;ix<6;ix++)
		packet[6+ix] = addr[ix];

	/* set type field */
	packet[12]=0xaa;
	packet[13]=0x55;

	/* set data field */
	for(ix=14;ix<60;ix++)
		packet[ix] = 0x00;

#ifdef DEBUG
	for (ix=0;ix<6;ix++)
		printf("mac_addr[%d]=%02X\n", ix, (unsigned char)packet[6+ix]);
#endif

	/* set one packet */
	plb2800_eth_send(dev, packet, sizeof(packet));

	/* delay for a while */
	for(ix=0;ix<65535;ix++)
		temp = ~temp;

	/* Set CMAC_CTX_CTRL register */
	temp = TSTAMP_MS;	/* no autocast */
	CMAC_CTX_CTRL = temp;

	/* Set DA_LOOKUP register */
	temp = EN_DA_LKUP;
	DA_LOOKUP = temp;

	mac_addr_set = 1;
}

static unsigned char * plb2800_get_mac_addr(void)
{
	static unsigned char addr[6];
	char *tmp, *end;
	int i;

	tmp = getenv ("ethaddr");
	if (NULL == tmp) return NULL;

	for (i=0; i<6; i++) {
		addr[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
		if (tmp)
			tmp = (*end) ? end+1 : end;
	}

	return addr;
}

#endif /* CONFIG_PLB2800_ETHER */
