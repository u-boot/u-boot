// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2008 Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 */

#include <common.h>
#include "rtl8169.h"

static unsigned char *PCI_MEMR;

static void mac_delay(unsigned int cnt)
{
	udelay(cnt);
}

static void mac_pci_setup(void)
{
	unsigned long pci_data;

	PCI_PAR = 0x00000010;
	PCI_PDR = 0x00001000;
	PCI_PAR = 0x00000004;
	pci_data = PCI_PDR;
	PCI_PDR = pci_data | 0x00000007;
	PCI_PAR = 0x00000010;

	PCI_MEMR = (unsigned char *)((PCI_PDR | 0xFE240050) & 0xFFFFFFF0);
}

static void EECS(int level)
{
	unsigned char data = *PCI_MEMR;

	if (level)
		*PCI_MEMR = data | 0x08;
	else
		*PCI_MEMR = data & 0xf7;
}

static void EECLK(int level)
{
	unsigned char data = *PCI_MEMR;

	if (level)
		*PCI_MEMR = data | 0x04;
	else
		*PCI_MEMR = data & 0xfb;
}

static void EEDI(int level)
{
	unsigned char data = *PCI_MEMR;

	if (level)
		*PCI_MEMR = data | 0x02;
	else
		*PCI_MEMR = data & 0xfd;
}

static inline void sh7785lcr_bitset(unsigned short bit)
{
	if (bit)
		EEDI(HIGH);
	else
		EEDI(LOW);

	EECLK(LOW);
	mac_delay(TIME1);
	EECLK(HIGH);
	mac_delay(TIME1);
	EEDI(LOW);
}

static inline unsigned char sh7785lcr_bitget(void)
{
	unsigned char bit;

	EECLK(LOW);
	mac_delay(TIME1);
	bit = *PCI_MEMR & 0x01;
	EECLK(HIGH);
	mac_delay(TIME1);

	return bit;
}

static inline void sh7785lcr_setcmd(unsigned char command)
{
	sh7785lcr_bitset(BIT_DUMMY);
	switch (command) {
	case MAC_EEP_READ:
		sh7785lcr_bitset(1);
		sh7785lcr_bitset(1);
		sh7785lcr_bitset(0);
		break;
	case MAC_EEP_WRITE:
		sh7785lcr_bitset(1);
		sh7785lcr_bitset(0);
		sh7785lcr_bitset(1);
		break;
	case MAC_EEP_ERACE:
		sh7785lcr_bitset(1);
		sh7785lcr_bitset(1);
		sh7785lcr_bitset(1);
		break;
	case MAC_EEP_EWEN:
		sh7785lcr_bitset(1);
		sh7785lcr_bitset(0);
		sh7785lcr_bitset(0);
		break;
	case MAC_EEP_EWDS:
		sh7785lcr_bitset(1);
		sh7785lcr_bitset(0);
		sh7785lcr_bitset(0);
		break;
	default:
		break;
	}
}

static inline unsigned short sh7785lcr_getdt(void)
{
	unsigned short data = 0;
	int i;

	sh7785lcr_bitget();			/* DUMMY */
	for (i = 0 ; i < 16 ; i++) {
		data <<= 1;
		data |= sh7785lcr_bitget();
	}
	return data;
}

static inline void sh7785lcr_setadd(unsigned short address)
{
	sh7785lcr_bitset(address & 0x0020);	/* A5 */
	sh7785lcr_bitset(address & 0x0010);	/* A4 */
	sh7785lcr_bitset(address & 0x0008);	/* A3 */
	sh7785lcr_bitset(address & 0x0004);	/* A2 */
	sh7785lcr_bitset(address & 0x0002);	/* A1 */
	sh7785lcr_bitset(address & 0x0001);	/* A0 */
}

static inline void sh7785lcr_setdata(unsigned short data)
{
	sh7785lcr_bitset(data & 0x8000);
	sh7785lcr_bitset(data & 0x4000);
	sh7785lcr_bitset(data & 0x2000);
	sh7785lcr_bitset(data & 0x1000);
	sh7785lcr_bitset(data & 0x0800);
	sh7785lcr_bitset(data & 0x0400);
	sh7785lcr_bitset(data & 0x0200);
	sh7785lcr_bitset(data & 0x0100);
	sh7785lcr_bitset(data & 0x0080);
	sh7785lcr_bitset(data & 0x0040);
	sh7785lcr_bitset(data & 0x0020);
	sh7785lcr_bitset(data & 0x0010);
	sh7785lcr_bitset(data & 0x0008);
	sh7785lcr_bitset(data & 0x0004);
	sh7785lcr_bitset(data & 0x0002);
	sh7785lcr_bitset(data & 0x0001);
}

static void sh7785lcr_datawrite(const unsigned short *data, unsigned short address,
			 unsigned int count)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		EECS(HIGH);
		EEDI(LOW);
		mac_delay(TIME1);

		sh7785lcr_setcmd(MAC_EEP_WRITE);
		sh7785lcr_setadd(address++);
		sh7785lcr_setdata(*(data + i));

		EECLK(LOW);
		EEDI(LOW);
		EECS(LOW);
		mac_delay(TIME2);
	}
}

static void sh7785lcr_macerase(void)
{
	unsigned int i;
	unsigned short pci_address = 7;

	for (i = 0; i < 3; i++) {
		EECS(HIGH);
		EEDI(LOW);
		mac_delay(TIME1);
		sh7785lcr_setcmd(MAC_EEP_ERACE);
		sh7785lcr_setadd(pci_address++);
		mac_delay(TIME1);
		EECLK(LOW);
		EEDI(LOW);
		EECS(LOW);
	}

	mac_delay(TIME2);

	printf("\n\nErace End\n");
	for (i = 0; i < 10; i++)
		mac_delay(TIME2);
}

static void sh7785lcr_macwrite(unsigned short *data)
{
	sh7785lcr_macerase();

	sh7785lcr_datawrite(EEPROM_W_Data_8169_A, 0x0000, 7);
	sh7785lcr_datawrite(data, PCI_EEP_ADDRESS, PCI_MAC_ADDRESS_SIZE);
	sh7785lcr_datawrite(EEPROM_W_Data_8169_B, 0x000a, 54);
}

void sh7785lcr_macdtrd(unsigned char *buf, unsigned short address, unsigned int count)
{
	unsigned int i;
	unsigned short wk;

	for (i = 0 ; i < count; i++) {
		EECS(HIGH);
		EEDI(LOW);
		mac_delay(TIME1);
		sh7785lcr_setcmd(MAC_EEP_READ);
		sh7785lcr_setadd(address++);
		wk = sh7785lcr_getdt();

		*buf++ = (unsigned char)(wk & 0xff);
		*buf++ = (unsigned char)((wk >> 8) & 0xff);
		EECLK(LOW);
		EEDI(LOW);
		EECS(LOW);
	}
}

static void sh7785lcr_macadrd(unsigned char *buf)
{
	*PCI_MEMR = PCI_PROG;

	sh7785lcr_macdtrd(buf, PCI_EEP_ADDRESS, PCI_MAC_ADDRESS_SIZE);
}

static void sh7785lcr_eepewen(void)
{
	*PCI_MEMR = PCI_PROG;
	mac_delay(TIME1);
	EECS(LOW);
	EECLK(LOW);
	EEDI(LOW);
	EECS(HIGH);
	mac_delay(TIME1);

	sh7785lcr_setcmd(MAC_EEP_EWEN);
	sh7785lcr_bitset(1);
	sh7785lcr_bitset(1);
	sh7785lcr_bitset(BIT_DUMMY);
	sh7785lcr_bitset(BIT_DUMMY);
	sh7785lcr_bitset(BIT_DUMMY);
	sh7785lcr_bitset(BIT_DUMMY);

	EECLK(LOW);
	EEDI(LOW);
	EECS(LOW);
	mac_delay(TIME1);
}

void mac_write(unsigned short *data)
{
	mac_pci_setup();
	sh7785lcr_eepewen();
	sh7785lcr_macwrite(data);
}

void mac_read(void)
{
	unsigned char data[6];

	mac_pci_setup();
	sh7785lcr_macadrd(data);
	printf("Mac = %02x:%02x:%02x:%02x:%02x:%02x\n",
		data[0], data[1], data[2], data[3], data[4], data[5]);
}

int do_set_mac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	unsigned char mac[6];
	char *s, *e;

	if (argc != 2)
		return cmd_usage(cmdtp);

	s = argv[1];

	for (i = 0; i < 6; i++) {
		mac[i] = s ? simple_strtoul(s, &e, 16) : 0;
		if (s)
			s = (*e) ? e + 1 : e;
	}
	mac_write((unsigned short *)mac);

	return 0;
}

U_BOOT_CMD(
	setmac,	2,	1,	do_set_mac,
	"write MAC address for RTL8110SCL",
	"\n"
	"setmac <mac address> - write MAC address for RTL8110SCL"
);

int do_print_mac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 1)
		return cmd_usage(cmdtp);

	mac_read();

	return 0;
}

U_BOOT_CMD(
	printmac,	1,	1,	do_print_mac,
	"print MAC address for RTL8110",
	"\n"
	"    - print MAC address for RTL8110"
);
