/*
 * (C) Copyright 2001, 2002
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

#if (CONFIG_COMMANDS & CFG_CMD_NET) && defined(CONFIG_NET_MULTI)

#ifdef CFG_GT_6426x
extern int gt6426x_eth_initialize(bd_t *bis);
#endif

extern int eepro100_initialize(bd_t*);
extern int natsemi_initialize(bd_t*);
extern int ns8382x_initialize(bd_t*);
extern int dc21x4x_initialize(bd_t*);
extern int eth_3com_initialize(bd_t*);
extern int pcnet_initialize(bd_t*);
extern int fec_initialize(bd_t*);
extern int scc_initialize(bd_t*);

static struct eth_device *eth_devices, *eth_current;

struct eth_device *eth_get_dev(void)
{
	return eth_current;
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
	unsigned char enetvar[32], env_enetaddr[6];
	int i, eth_number = 0;
	char *tmp, *end;

	eth_devices = NULL;
	eth_current = NULL;

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
#ifdef SCC_ENET
	scc_initialize(bis);
#endif
#ifdef FEC_ENET
	fec_initialize(bis);
#endif

	if (!eth_devices) {
		puts ("No ethernet found.\n");
	} else {
		struct eth_device *dev = eth_devices;
		char *ethprime = getenv ("ethprime");

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
					printf("\nWarning: %s HW address don't match:\n", dev->name);
					printf("Address in SROM is         "
					       "%02X:%02X:%02X:%02X:%02X:%02X\n",
					       dev->enetaddr[0], dev->enetaddr[1],
					       dev->enetaddr[2], dev->enetaddr[3],
					       dev->enetaddr[4], dev->enetaddr[5]);
					printf("Address in environment is  "
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

		putc ('\n');
	}

	return eth_number;
}

void eth_set_enetaddr(int num, char *addr) {
	struct eth_device *dev;
	unsigned char enetaddr[6];
	char *end;
	int i;

#ifdef DEBUG
	printf("eth_set_enetaddr(num=%d, addr=%s)\n", num, addr);
#endif
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

#ifdef DEBUG
	printf("Setting new HW address on %s\n", dev->name);
	printf("New Address is             "
	       "%02X:%02X:%02X:%02X:%02X:%02X\n",
	       dev->enetaddr[0], dev->enetaddr[1],
	       dev->enetaddr[2], dev->enetaddr[3],
	       dev->enetaddr[4], dev->enetaddr[5]);
#endif

	memcpy(dev->enetaddr, enetaddr, 6);
}

int eth_init(bd_t *bis)
{
	struct eth_device* old_current;

	if (!eth_current)
		return 0;

	old_current = eth_current;
	do {
#ifdef DEBUG
		printf("Trying %s\n", eth_current->name);
#endif

		if (eth_current->init(eth_current, bis)) {
			eth_current->state = ETH_STATE_ACTIVE;

			return 1;
		}

#ifdef DEBUG
		puts ("FAIL\n");
#endif

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

	if (first_restart)
	{
		first_failed = eth_current;
	}

	eth_current = eth_current->next;

	if (first_failed == eth_current)
	{
		NetRestartWrap = 1;
	}
}

#endif
