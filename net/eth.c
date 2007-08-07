/*
 * (C) Copyright 2001-2004
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <miiphy.h>

#if defined(CONFIG_CMD_NET) && defined(CONFIG_NET_MULTI)

#ifdef CFG_GT_6426x
extern int gt6426x_eth_initialize(bd_t *bis);
#endif

extern int au1x00_enet_initialize(bd_t*);
extern int dc21x4x_initialize(bd_t*);
extern int e1000_initialize(bd_t*);
extern int eepro100_initialize(bd_t*);
extern int eth_3com_initialize(bd_t*);
extern int fec_initialize(bd_t*);
extern int inca_switch_initialize(bd_t*);
extern int mpc5xxx_fec_initialize(bd_t*);
extern int mpc512x_fec_initialize(bd_t*);
extern int mpc8220_fec_initialize(bd_t*);
extern int mv6436x_eth_initialize(bd_t *);
extern int mv6446x_eth_initialize(bd_t *);
extern int natsemi_initialize(bd_t*);
extern int ns8382x_initialize(bd_t*);
extern int pcnet_initialize(bd_t*);
extern int plb2800_eth_initialize(bd_t*);
extern int ppc_4xx_eth_initialize(bd_t *);
extern int rtl8139_initialize(bd_t*);
extern int rtl8169_initialize(bd_t*);
extern int scc_initialize(bd_t*);
extern int skge_initialize(bd_t*);
extern int tsi108_eth_initialize(bd_t*);
extern int tsec_initialize(bd_t*, int, char *);
extern int npe_initialize(bd_t *);
extern int uec_initialize(int);
extern int bfin_EMAC_initialize(bd_t *);
extern int atstk1000_eth_initialize(bd_t *);

static struct eth_device *eth_devices, *eth_current;

struct eth_device *eth_get_dev(void)
{
	return eth_current;
}

struct eth_device *eth_get_dev_by_name(char *devname)
{
	struct eth_device *dev, *target_dev;

	if (!eth_devices)
		return NULL;

	dev = eth_devices;
	target_dev = NULL;
	do {
		if (strcmp(devname, dev->name) == 0) {
			target_dev = dev;
			break;
		}
		dev = dev->next;
	} while (dev != eth_devices);

	return target_dev;
}

int eth_get_dev_index (void)
{
	struct eth_device *dev;
	int num = 0;

	if (!eth_devices) {
		return (-1);
	}

	for (dev = eth_devices; dev; dev = dev->next) {
		if (dev == eth_current)
			break;
		++num;
	}

	if (dev) {
		return (num);
	}

	return (0);
}

int eth_register(struct eth_device* dev)
{
	struct eth_device *d;

	if (!eth_devices) {
		eth_current = eth_devices = dev;
#ifdef CONFIG_NET_MULTI
		/* update current ethernet name */
		{
			char *act = getenv("ethact");
			if (act == NULL || strcmp(act, eth_current->name) != 0)
				setenv("ethact", eth_current->name);
		}
#endif
	} else {
		for (d=eth_devices; d->next!=eth_devices; d=d->next);
		d->next = dev;
	}

	dev->state = ETH_STATE_INIT;
	dev->next  = eth_devices;

	return 0;
}

int eth_initialize(bd_t *bis)
{
	char enetvar[32], env_enetaddr[6];
	int i, eth_number = 0;
	char *tmp, *end;

	eth_devices = NULL;
	eth_current = NULL;

	show_boot_progress (64);
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	miiphy_init();
#endif

#if defined(CONFIG_DB64360) || defined(CONFIG_CPCI750)
	mv6436x_eth_initialize(bis);
#endif
#if defined(CONFIG_DB64460) || defined(CONFIG_P3Mx)
	mv6446x_eth_initialize(bis);
#endif
#if defined(CONFIG_4xx) && !defined(CONFIG_IOP480) && !defined(CONFIG_AP1000)
	ppc_4xx_eth_initialize(bis);
#endif
#ifdef CONFIG_INCA_IP_SWITCH
	inca_switch_initialize(bis);
#endif
#ifdef CONFIG_PLB2800_ETHER
	plb2800_eth_initialize(bis);
#endif
#ifdef SCC_ENET
	scc_initialize(bis);
#endif
#if defined(CONFIG_MPC5xxx_FEC)
	mpc5xxx_fec_initialize(bis);
#endif
#if defined(CONFIG_MPC512x_FEC)
	mpc512x_fec_initialize (bis);
#endif
#if defined(CONFIG_MPC8220_FEC)
	mpc8220_fec_initialize(bis);
#endif
#if defined(CONFIG_SK98)
	skge_initialize(bis);
#endif
#if defined(CONFIG_TSEC1)
	tsec_initialize(bis, 0, CONFIG_TSEC1_NAME);
#endif
#if defined(CONFIG_TSEC2)
	tsec_initialize(bis, 1, CONFIG_TSEC2_NAME);
#endif
#if defined(CONFIG_MPC85XX_FEC)
	tsec_initialize(bis, 2, CONFIG_MPC85XX_FEC_NAME);
#else
#    if defined(CONFIG_TSEC3)
	tsec_initialize(bis, 2, CONFIG_TSEC3_NAME);
#    endif
#    if defined(CONFIG_TSEC4)
	tsec_initialize(bis, 3, CONFIG_TSEC4_NAME);
#    endif
#endif
#if defined(CONFIG_UEC_ETH1)
	uec_initialize(0);
#endif
#if defined(CONFIG_UEC_ETH2)
	uec_initialize(1);
#endif

#if defined(FEC_ENET) || defined(CONFIG_ETHER_ON_FCC)
	fec_initialize(bis);
#endif
#if defined(CONFIG_AU1X00)
	au1x00_enet_initialize(bis);
#endif
#if defined(CONFIG_IXP4XX_NPE)
	npe_initialize(bis);
#endif
#ifdef CONFIG_E1000
	e1000_initialize(bis);
#endif
#ifdef CONFIG_EEPRO100
	eepro100_initialize(bis);
#endif
#ifdef CONFIG_TULIP
	dc21x4x_initialize(bis);
#endif
#ifdef CONFIG_3COM
	eth_3com_initialize(bis);
#endif
#ifdef CONFIG_PCNET
	pcnet_initialize(bis);
#endif
#ifdef CFG_GT_6426x
	gt6426x_eth_initialize(bis);
#endif
#ifdef CONFIG_NATSEMI
	natsemi_initialize(bis);
#endif
#ifdef CONFIG_NS8382X
	ns8382x_initialize(bis);
#endif
#if defined(CONFIG_TSI108_ETH)
	tsi108_eth_initialize(bis);
#endif
#if defined(CONFIG_RTL8139)
	rtl8139_initialize(bis);
#endif
#if defined(CONFIG_RTL8169)
	rtl8169_initialize(bis);
#endif
#if defined(CONFIG_BF537)
	bfin_EMAC_initialize(bis);
#endif
#if defined(CONFIG_ATSTK1000)
	atstk1000_eth_initialize(bis);
#endif

	if (!eth_devices) {
		puts ("No ethernet found.\n");
		show_boot_progress (-64);
	} else {
		struct eth_device *dev = eth_devices;
		char *ethprime = getenv ("ethprime");

		show_boot_progress (65);
		do {
			if (eth_number)
				puts (", ");

			printf("%s", dev->name);

			if (ethprime && strcmp (dev->name, ethprime) == 0) {
				eth_current = dev;
				puts (" [PRIME]");
			}

			sprintf(enetvar, eth_number ? "eth%daddr" : "ethaddr", eth_number);
			tmp = getenv (enetvar);

			for (i=0; i<6; i++) {
				env_enetaddr[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
				if (tmp)
					tmp = (*end) ? end+1 : end;
			}

			if (memcmp(env_enetaddr, "\0\0\0\0\0\0", 6)) {
				if (memcmp(dev->enetaddr, "\0\0\0\0\0\0", 6) &&
				    memcmp(dev->enetaddr, env_enetaddr, 6))
				{
					printf ("\nWarning: %s MAC addresses don't match:\n",
						dev->name);
					printf ("Address in SROM is         "
					       "%02X:%02X:%02X:%02X:%02X:%02X\n",
					       dev->enetaddr[0], dev->enetaddr[1],
					       dev->enetaddr[2], dev->enetaddr[3],
					       dev->enetaddr[4], dev->enetaddr[5]);
					printf ("Address in environment is  "
					       "%02X:%02X:%02X:%02X:%02X:%02X\n",
					       env_enetaddr[0], env_enetaddr[1],
					       env_enetaddr[2], env_enetaddr[3],
					       env_enetaddr[4], env_enetaddr[5]);
				}

				memcpy(dev->enetaddr, env_enetaddr, 6);
			}

			eth_number++;
			dev = dev->next;
		} while(dev != eth_devices);

#ifdef CONFIG_NET_MULTI
		/* update current ethernet name */
		if (eth_current) {
			char *act = getenv("ethact");
			if (act == NULL || strcmp(act, eth_current->name) != 0)
				setenv("ethact", eth_current->name);
		} else
			setenv("ethact", NULL);
#endif

		putc ('\n');
	}

	return eth_number;
}

void eth_set_enetaddr(int num, char *addr) {
	struct eth_device *dev;
	unsigned char enetaddr[6];
	char *end;
	int i;

	debug ("eth_set_enetaddr(num=%d, addr=%s)\n", num, addr);

	if (!eth_devices)
		return;

	for (i=0; i<6; i++) {
		enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
		if (addr)
			addr = (*end) ? end+1 : end;
	}

	dev = eth_devices;
	while(num-- > 0) {
		dev = dev->next;

		if (dev == eth_devices)
			return;
	}

	debug ( "Setting new HW address on %s\n"
		"New Address is             %02X:%02X:%02X:%02X:%02X:%02X\n",
		dev->name,
		enetaddr[0], enetaddr[1],
		enetaddr[2], enetaddr[3],
		enetaddr[4], enetaddr[5]);

	memcpy(dev->enetaddr, enetaddr, 6);
}

int eth_init(bd_t *bis)
{
	struct eth_device* old_current;

	if (!eth_current)
		return 0;

	old_current = eth_current;
	do {
		debug ("Trying %s\n", eth_current->name);

		if (eth_current->init(eth_current, bis)) {
			eth_current->state = ETH_STATE_ACTIVE;

			return 1;
		}
		debug  ("FAIL\n");

		eth_try_another(0);
	} while (old_current != eth_current);

	return 0;
}

void eth_halt(void)
{
	if (!eth_current)
		return;

	eth_current->halt(eth_current);

	eth_current->state = ETH_STATE_PASSIVE;
}

int eth_send(volatile void *packet, int length)
{
	if (!eth_current)
		return -1;

	return eth_current->send(eth_current, packet, length);
}

int eth_rx(void)
{
	if (!eth_current)
		return -1;

	return eth_current->recv(eth_current);
}

void eth_try_another(int first_restart)
{
	static struct eth_device *first_failed = NULL;

	if (!eth_current)
		return;

	if (first_restart) {
		first_failed = eth_current;
	}

	eth_current = eth_current->next;

#ifdef CONFIG_NET_MULTI
	/* update current ethernet name */
	{
		char *act = getenv("ethact");
		if (act == NULL || strcmp(act, eth_current->name) != 0)
			setenv("ethact", eth_current->name);
	}
#endif

	if (first_failed == eth_current) {
		NetRestartWrap = 1;
	}
}

#ifdef CONFIG_NET_MULTI
void eth_set_current(void)
{
	char *act;
	struct eth_device* old_current;

	if (!eth_current)	/* XXX no current */
		return;

	act = getenv("ethact");
	if (act != NULL) {
		old_current = eth_current;
		do {
			if (strcmp(eth_current->name, act) == 0)
				return;
			eth_current = eth_current->next;
		} while (old_current != eth_current);
	}

	setenv("ethact", eth_current->name);
}
#endif

char *eth_get_name (void)
{
	return (eth_current ? eth_current->name : "unknown");
}
#elif defined(CONFIG_CMD_NET) && !defined(CONFIG_NET_MULTI)

extern int at91rm9200_miiphy_initialize(bd_t *bis);
extern int emac4xx_miiphy_initialize(bd_t *bis);
extern int mcf52x2_miiphy_initialize(bd_t *bis);
extern int ns7520_miiphy_initialize(bd_t *bis);

int eth_initialize(bd_t *bis)
{
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	miiphy_init();
#endif

#if defined(CONFIG_AT91RM9200)
	at91rm9200_miiphy_initialize(bis);
#endif
#if defined(CONFIG_4xx) && !defined(CONFIG_IOP480) \
	&& !defined(CONFIG_AP1000) && !defined(CONFIG_405)
	emac4xx_miiphy_initialize(bis);
#endif
#if defined(CONFIG_MCF52x2)
	mcf52x2_miiphy_initialize(bis);
#endif
#if defined(CONFIG_NETARM)
	ns7520_miiphy_initialize(bis);
#endif
	return 0;
}
#endif
