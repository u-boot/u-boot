/******************************************************************************
 *
 * Name:    skproc.c
 * Project:	GEnesis, PCI Gigabit Ethernet Adapter
 * Version:	$Revision: 1.4 $
 * Date:    $Date: 2003/02/25 14:16:37 $
 * Purpose:	Funktions to display statictic data
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2003 SysKonnect GmbH.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	Created 22-Nov-2000
 *	Author: Mirko Lindner (mlindner@syskonnect.de)
 *
 *	The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/
/******************************************************************************
 *
 * History:
 *
 *	$Log: skproc.c,v $
 *	Revision 1.4  2003/02/25 14:16:37  mlindner
 *	Fix: Copyright statement
 *
 *	Revision 1.3  2002/10/02 12:59:51  mlindner
 *	Add: Support for Yukon
 *	Add: Speed check and setup
 *	Add: Merge source for kernel 2.2.x and 2.4.x
 *	Add: Read sensor names directly from VPD
 *	Fix: Volt values
 *
 *	Revision 1.2.2.7  2002/01/14 12:45:15  mlindner
 *	Fix: Editorial changes
 *
 *	Revision 1.2.2.6  2001/12/06 15:26:07  mlindner
 *	Fix: Return value of proc_read
 *
 *	Revision 1.2.2.5  2001/12/06 09:57:39  mlindner
 *	New ProcFs entries
 *
 *	Revision 1.2.2.4  2001/09/05 12:16:02  mlindner
 *	Add: New ProcFs entries
 *	Fix: Counter Errors (Jumbo == to long errors)
 *	Fix: Kernel error compilation
 *	Fix: too short counters
 *
 *	Revision 1.2.2.3  2001/06/25 07:26:26  mlindner
 *	Add: More error messages
 *
 *	Revision 1.2.2.2  2001/03/15 12:50:13  mlindner
 *	fix: ProcFS owner protection
 *
 *	Revision 1.2.2.1  2001/03/12 16:43:48  mlindner
 *	chg: 2.4 requirements for procfs
 *
 *	Revision 1.1  2001/01/22 14:15:31  mlindner
 *	added ProcFs functionality
 *	Dual Net functionality integrated
 *	Rlmt networks added
 *
 *
 ******************************************************************************/

#include <config.h>

#ifdef CONFIG_SK98

#include <linux/proc_fs.h>

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#define ZEROPAD		1		/* pad with zero */
#define SIGN		2		/* unsigned/signed long */
#define PLUS		4		/* show plus */
#define SPACE		8		/* space if plus */
#define LEFT		16		/* left justified */
#define SPECIALX	32		/* 0x */
#define LARGE		64

extern SK_AC *pACList;
extern struct net_device *SkGeRootDev;

extern char *SkNumber (char *str,
		       long long num,
		       int base,
		       int size,
		       int precision,
		       int type);


/*****************************************************************************
 *
 *	proc_read - print "summaries" entry
 *
 * Description:
 *  This function fills the proc entry with statistic data about
 *  the ethernet device.
 *
 *
 * Returns: buffer with statistic data
 *
 */
int proc_read(char *buffer,
char **buffer_location,
off_t offset,
int buffer_length,
int *eof,
void *data)
{
	int len = 0;
	int t;
	int i;
	DEV_NET			*pNet;
	SK_AC			*pAC;
	char			test_buf[100];
	char			sens_msg[50];
	unsigned long		Flags;
	unsigned int		Size;
	struct SK_NET_DEVICE	*next;
	struct SK_NET_DEVICE	*SkgeProcDev = SkGeRootDev;

	SK_PNMI_STRUCT_DATA	*pPnmiStruct;
	SK_PNMI_STAT		*pPnmiStat;
	struct proc_dir_entry *file = (struct proc_dir_entry*) data;

	while (SkgeProcDev) {
		pNet = (DEV_NET*) SkgeProcDev->priv;
		pAC = pNet->pAC;
		next = pAC->Next;
		pPnmiStruct = &pAC->PnmiStruct;
		/* NetIndex in GetStruct is now required, zero is only dummy */

		for (t=pAC->GIni.GIMacsFound; t > 0; t--) {
			if ((pAC->GIni.GIMacsFound == 2) && pAC->RlmtNets == 1)
				t--;

			spin_lock_irqsave(&pAC->SlowPathLock, Flags);
			Size = SK_PNMI_STRUCT_SIZE;
			SkPnmiGetStruct(pAC, pAC->IoBase,
				pPnmiStruct, &Size, t-1);
			spin_unlock_irqrestore(&pAC->SlowPathLock, Flags);

			if (strcmp(pAC->dev[t-1]->name, file->name) == 0) {
				pPnmiStat = &pPnmiStruct->Stat[0];
				len = sprintf(buffer,
					"\nDetailed statistic for device %s\n",
					pAC->dev[t-1]->name);
				len += sprintf(buffer + len,
					"=======================================\n");

				/* Board statistics */
				len += sprintf(buffer + len,
					"\nBoard statistics\n\n");
				len += sprintf(buffer + len,
					"Active Port                    %c\n",
					'A' + pAC->Rlmt.Net[t-1].Port[pAC->Rlmt.
					Net[t-1].PrefPort]->PortNumber);
				len += sprintf(buffer + len,
					"Preferred Port                 %c\n",
					'A' + pAC->Rlmt.Net[t-1].Port[pAC->Rlmt.
					Net[t-1].PrefPort]->PortNumber);

				len += sprintf(buffer + len,
					"Bus speed (MHz)                %d\n",
					pPnmiStruct->BusSpeed);

				len += sprintf(buffer + len,
					"Bus width (Bit)                %d\n",
					pPnmiStruct->BusWidth);
				len += sprintf(buffer + len,
					"Hardware revision              v%d.%d\n",
					(pAC->GIni.GIPciHwRev >> 4) & 0x0F,
					pAC->GIni.GIPciHwRev & 0x0F);

				/* Print sensor informations */
				for (i=0; i < pAC->I2c.MaxSens; i ++) {
					/* Check type */
					switch (pAC->I2c.SenTable[i].SenType) {
					case 1:
						strcpy(sens_msg, pAC->I2c.SenTable[i].SenDesc);
						strcat(sens_msg, " (C)");
						len += sprintf(buffer + len,
							"%-25s      %d.%02d\n",
							sens_msg,
							pAC->I2c.SenTable[i].SenValue / 10,
							pAC->I2c.SenTable[i].SenValue % 10);

						strcpy(sens_msg, pAC->I2c.SenTable[i].SenDesc);
						strcat(sens_msg, " (F)");
						len += sprintf(buffer + len,
							"%-25s      %d.%02d\n",
							sens_msg,
							((((pAC->I2c.SenTable[i].SenValue)
							*10)*9)/5 + 3200)/100,
							((((pAC->I2c.SenTable[i].SenValue)
							*10)*9)/5 + 3200) % 10);
						break;
					case 2:
						strcpy(sens_msg, pAC->I2c.SenTable[i].SenDesc);
						strcat(sens_msg, " (V)");
						len += sprintf(buffer + len,
							"%-25s      %d.%03d\n",
							sens_msg,
							pAC->I2c.SenTable[i].SenValue / 1000,
							pAC->I2c.SenTable[i].SenValue % 1000);
						break;
					case 3:
						strcpy(sens_msg, pAC->I2c.SenTable[i].SenDesc);
						strcat(sens_msg, " (rpm)");
						len += sprintf(buffer + len,
							"%-25s      %d\n",
							sens_msg,
							pAC->I2c.SenTable[i].SenValue);
						break;
					default:
						break;
					}
				}

				/*Receive statistics */
				len += sprintf(buffer + len,
				"\nReceive statistics\n\n");

				len += sprintf(buffer + len,
					"Received bytes                 %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxOctetsOkCts,
					10,0,-1,0));
				len += sprintf(buffer + len,
					"Received packets               %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxOkCts,
					10,0,-1,0));
#if 0
				if (pAC->GIni.GP[0].PhyType == SK_PHY_XMAC &&
					pAC->HWRevision < 12) {
					pPnmiStruct->InErrorsCts = pPnmiStruct->InErrorsCts -
						pPnmiStat->StatRxShortsCts;
					pPnmiStat->StatRxShortsCts = 0;
				}
#endif
				if (pNet->Mtu > 1500)
					pPnmiStruct->InErrorsCts = pPnmiStruct->InErrorsCts -
						pPnmiStat->StatRxTooLongCts;

				len += sprintf(buffer + len,
					"Receive errors                 %s\n",
					SkNumber(test_buf, pPnmiStruct->InErrorsCts,
					10,0,-1,0));
				len += sprintf(buffer + len,
					"Receive drops                  %s\n",
					SkNumber(test_buf, pPnmiStruct->RxNoBufCts,
					10,0,-1,0));
				len += sprintf(buffer + len,
					"Received multicast             %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxMulticastOkCts,
					10,0,-1,0));
				len += sprintf(buffer + len,
					"Receive error types\n");
				len += sprintf(buffer + len,
					"   length                      %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxRuntCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   buffer overflow             %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxFifoOverflowCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   bad crc                     %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxFcsCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   framing                     %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxFramingCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   missed frames               %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxMissedCts,
					10, 0, -1, 0));

				if (pNet->Mtu > 1500)
					pPnmiStat->StatRxTooLongCts = 0;

				len += sprintf(buffer + len,
					"   too long                    %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxTooLongCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   carrier extension           %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxCextCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   too short                   %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxShortsCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   symbol                      %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxSymbolCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   LLC MAC size                %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxIRLengthCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   carrier event               %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxCarrierCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   jabber                      %s\n",
					SkNumber(test_buf, pPnmiStat->StatRxJabberCts,
					10, 0, -1, 0));


				/*Transmit statistics */
				len += sprintf(buffer + len,
				"\nTransmit statistics\n\n");

				len += sprintf(buffer + len,
					"Transmited bytes               %s\n",
					SkNumber(test_buf, pPnmiStat->StatTxOctetsOkCts,
					10,0,-1,0));
				len += sprintf(buffer + len,
					"Transmited packets             %s\n",
					SkNumber(test_buf, pPnmiStat->StatTxOkCts,
					10,0,-1,0));
				len += sprintf(buffer + len,
					"Transmit errors                %s\n",
					SkNumber(test_buf, pPnmiStat->StatTxSingleCollisionCts,
					10,0,-1,0));
				len += sprintf(buffer + len,
					"Transmit dropped               %s\n",
					SkNumber(test_buf, pPnmiStruct->TxNoBufCts,
					10,0,-1,0));
				len += sprintf(buffer + len,
					"Transmit collisions            %s\n",
					SkNumber(test_buf, pPnmiStat->StatTxSingleCollisionCts,
					10,0,-1,0));
				len += sprintf(buffer + len,
					"Transmit errors types\n");
				len += sprintf(buffer + len,
					"   excessive collision         %ld\n",
					pAC->stats.tx_aborted_errors);
				len += sprintf(buffer + len,
					"   carrier                     %s\n",
					SkNumber(test_buf, pPnmiStat->StatTxCarrierCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   fifo underrun               %s\n",
					SkNumber(test_buf, pPnmiStat->StatTxFifoUnderrunCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   heartbeat                   %s\n",
					SkNumber(test_buf, pPnmiStat->StatTxCarrierCts,
					10, 0, -1, 0));
				len += sprintf(buffer + len,
					"   window                      %ld\n",
					pAC->stats.tx_window_errors);

			}
		}
		SkgeProcDev = next;
	}
	if (offset >= len) {
		*eof = 1;
		return 0;
	}

	*buffer_location = buffer + offset;
	if (buffer_length >= len - offset) {
		*eof = 1;
	}
	return (min_t(int, buffer_length, len - offset));
}


/*****************************************************************************
 *
 * SkDoDiv - convert 64bit number
 *
 * Description:
 *	This function "converts" a long long number.
 *
 * Returns:
 *	remainder of division
 */
static long SkDoDiv (long long Dividend, int Divisor, long long *pErg)
{
	long Rest;
	long long Ergebnis;
	long Akku;


	Akku = Dividend >> 32;

	Ergebnis = ((long long) (Akku / Divisor)) << 32;
	Rest = Akku % Divisor;

	Akku = Rest << 16;
	Akku |= ((Dividend & 0xFFFF0000) >> 16);


	Ergebnis += ((long long) (Akku / Divisor)) << 16;
	Rest = Akku % Divisor;

	Akku = Rest << 16;
	Akku |= (Dividend & 0xFFFF);

	Ergebnis += (Akku / Divisor);
	Rest = Akku % Divisor;

	*pErg = Ergebnis;
	return (Rest);
}


#if 0
#define do_div(n,base) ({ \
long long __res; \
__res = ((unsigned long long) n) % (unsigned) base; \
n = ((unsigned long long) n) / (unsigned) base; \
__res; })

#endif


/*****************************************************************************
 *
 * SkNumber - Print results
 *
 * Description:
 *	This function converts a long long number into a string.
 *
 * Returns:
 *	number as string
 */
char * SkNumber(char * str, long long num, int base, int size, int precision
	,int type)
{
	char c,sign,tmp[66], *strorg = str;
	const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
	int i;

	if (type & LARGE)
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIALX) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num != 0)
		tmp[i++] = digits[SkDoDiv(num,base, &num)];

	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIALX) {
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;
	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';

	str[0] = '\0';

	return strorg;
}

#endif /* CONFIG_SK98 */
