/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#if defined(VXWORKS)
#include <stdio.h>
#include <string.h>
#define CONFIG_SYS_DEF_EEPROM_ADDR 0xa0
extern char iicReadByte(char, char);
extern ulong_t crc32(unsigned char *, unsigned long);
#else
#include <common.h>
#endif

#include "vpd.h"

/*
 * vpd_reader() - reads VPD data from I2C EEPROMS.
 *                returns pointer to buffer or NULL.
 */
static unsigned char *vpd_reader(unsigned char *buf, unsigned dev_addr,
				 unsigned off, unsigned count)
{
	unsigned offset = off;	/* Calculated offset */

	/*
	 * The main board EEPROM contains
	 * SDRAM SPD in the first 128 bytes,
	 * so skew the offset.
	 */
	if (dev_addr == CONFIG_SYS_DEF_EEPROM_ADDR)
		offset += SDRAM_SPD_DATA_SIZE;

	/* Try to read the I2C EEPROM */
#if defined(VXWORKS)
	{
		int i;

		for (i = 0; i < count; ++i)
			buf[i] = iicReadByte(dev_addr, offset + i);
	}
#else
	if (eeprom_read(dev_addr, offset, buf, count)) {
		printf("Failed to read %d bytes from VPD EEPROM 0x%x @ 0x%x\n",
			count, dev_addr, offset);
		return NULL;
	}
#endif

	return buf;
}


/*
 * vpd_get_packet() - returns next VPD packet or NULL.
 */
static vpd_packet_t *vpd_get_packet(vpd_packet_t * vpd_packet)
{
	vpd_packet_t *packet = vpd_packet;

	if (packet != NULL) {
		if (packet->identifier == VPD_PID_TERM)
			return NULL;
		else
			packet = (vpd_packet_t *) ((char *) packet +
						   packet->size + 2);
	}

	return packet;
}


/*
 * vpd_find_packet() - Locates and returns the specified
 *		       VPD packet or NULL on error.
 */
static vpd_packet_t *vpd_find_packet(vpd_t * vpd, unsigned char ident)
{
	vpd_packet_t *packet = (vpd_packet_t *) &vpd->packets;

	/* Guaranteed illegal */
	if (ident == VPD_PID_GI)
		return NULL;

	/* Scan tuples looking for a match */
	while ((packet->identifier != ident) &&
	       (packet->identifier != VPD_PID_TERM))
		packet = vpd_get_packet(packet);

	/* Did we find it? */
	if ((packet->identifier) && (packet->identifier != ident))
		return NULL;
	return packet;
}


/*
 * vpd_is_valid() - Validates contents of VPD data
 *		    in I2C EEPROM.  Returns 1 for
 *		    success or 0 for failure.
 */
static int vpd_is_valid(unsigned dev_addr, unsigned char *buf)
{
	unsigned num_bytes;
	vpd_packet_t *packet;
	vpd_t *vpd = (vpd_t *) buf;
	unsigned short stored_crc16, calc_crc16 = 0xffff;

	/* Check Eyecatcher */
	if (strncmp
	    ((char *) (vpd->header.eyecatcher), VPD_EYECATCHER,
	     VPD_EYE_SIZE) != 0) {
		unsigned offset = 0;

		if (dev_addr == CONFIG_SYS_DEF_EEPROM_ADDR)
			offset += SDRAM_SPD_DATA_SIZE;
		printf("Error: VPD EEPROM 0x%x corrupt @ 0x%x\n", dev_addr,
		       offset);

		return 0;
	}

	/* Check Length */
	if (vpd->header.size > VPD_MAX_EEPROM_SIZE) {
		printf("Error: VPD EEPROM 0x%x contains bad size 0x%x\n",
		       dev_addr, vpd->header.size);
		return 0;
	}

	/* Now find the termination packet */
	packet = vpd_find_packet(vpd, VPD_PID_TERM);
	if (packet == NULL) {
		printf("Error: VPD EEPROM 0x%x missing termination packet\n",
		       dev_addr);
		return 0;
	}

	/* Calculate data size */
	num_bytes = (unsigned long) ((unsigned char *) packet -
				     (unsigned char *) vpd +
				     sizeof(vpd_packet_t));

	/* Find stored CRC and clear it */
	packet = vpd_find_packet(vpd, VPD_PID_CRC);
	if (packet == NULL) {
		printf("Error: VPD EEPROM 0x%x missing CRC\n", dev_addr);
		return 0;
	}
	memcpy(&stored_crc16, packet->data, sizeof(ushort));
	memset(packet->data, 0, sizeof(ushort));

	/* OK, lets calculate the CRC and check it */
#if defined(VXWORKS)
	calc_crc16 = (0xffff & crc32(buf, num_bytes));
#else
	calc_crc16 = (0xffff & crc32(0, buf, num_bytes));
#endif
	/* Now restore the CRC */
	memcpy(packet->data, &stored_crc16, sizeof(ushort));
	if (stored_crc16 != calc_crc16) {
		printf("Error: VPD EEPROM 0x%x has bad CRC 0x%x\n",
		       dev_addr, stored_crc16);
		return 0;
	}

	return 1;
}


/*
 * size_ok() - Check to see if packet size matches
 *	       size of data we want. Returns 1 for
 *	       good match or 0 for failure.
 */
static int size_ok(vpd_packet_t *packet, unsigned long size)
{
	if (packet->size != size) {
		printf("VPD Packet 0x%x corrupt.\n", packet->identifier);
		return 0;
	}
	return 1;
}


/*
 * strlen_ok() - Check to see if packet size matches
 *		 strlen of the string we want to populate.
 *		 Returns 1 for valid length or 0 for failure.
 */
static int strlen_ok(vpd_packet_t *packet, unsigned long length)
{
	if (packet->size >= length) {
		printf("VPD Packet 0x%x corrupt.\n", packet->identifier);
		return 0;
	}
	return 1;
}


/*
 * get_vpd_data() - populates the passed VPD structure 'vpdInfo'
 *		    with data obtained from the specified
 *		    I2C EEPROM 'dev_addr'.  Returns 0 for
 *		    success or 1 for failure.
 */
int vpd_get_data(unsigned char dev_addr, VPD *vpdInfo)
{
	unsigned char buf[VPD_EEPROM_SIZE];
	vpd_t *vpd = (vpd_t *) buf;
	vpd_packet_t *packet;

	if (vpdInfo == NULL)
		return 1;

	/*
	 * Fill vpdInfo with 0s to blank out
	 * unused fields, fill vpdInfo->ethAddrs
	 * with all 0xffs so that other's code can
	 * determine how many real Ethernet addresses
	 * there are.  OUIs starting with 0xff are
	 * broadcast addresses, and would never be
	 * permantely stored.
	 */
	memset((void *) vpdInfo, 0, sizeof(VPD));
	memset((void *) &vpdInfo->ethAddrs, 0xff, sizeof(vpdInfo->ethAddrs));
	vpdInfo->_devAddr = dev_addr;

	/* Read the minimum size first */
	if (vpd_reader(buf, dev_addr, 0, VPD_EEPROM_SIZE) == NULL)
		return 1;

	/* Check validity of VPD data */
	if (!vpd_is_valid(dev_addr, buf)) {
		printf("VPD Data is INVALID!\n");
		return 1;
	}

	/*
	 * Walk all the packets and populate
	 * the VPD info structure.
	 */
	packet = (vpd_packet_t *) &vpd->packets;
	do {
		switch (packet->identifier) {
		case VPD_PID_GI:
			printf("Error: Illegal VPD value\n");
			break;
		case VPD_PID_PID:
			if (strlen_ok(packet, MAX_PROD_ID)) {
				strncpy(vpdInfo->productId,
					(char *) (packet->data),
					packet->size);
			}
			break;
		case VPD_PID_REV:
			if (size_ok(packet, sizeof(char)))
				vpdInfo->revisionId = *packet->data;
			break;
		case VPD_PID_SN:
			if (size_ok(packet, sizeof(unsigned long))) {
				memcpy(&vpdInfo->serialNum,
					packet->data,
					sizeof(unsigned long));
			}
			break;
		case VPD_PID_MANID:
			if (size_ok(packet, sizeof(unsigned char)))
				vpdInfo->manuID = *packet->data;
			break;
		case VPD_PID_PCO:
			if (size_ok(packet, sizeof(unsigned long))) {
				memcpy(&vpdInfo->configOpt,
					packet->data,
					sizeof(unsigned long));
			}
			break;
		case VPD_PID_SYSCLK:
			if (size_ok(packet, sizeof(unsigned long)))
				memcpy(&vpdInfo->sysClk,
					packet->data,
					sizeof(unsigned long));
			break;
		case VPD_PID_SERCLK:
			if (size_ok(packet, sizeof(unsigned long)))
				memcpy(&vpdInfo->serClk,
					packet->data,
					sizeof(unsigned long));
			break;
		case VPD_PID_FLASH:
			if (size_ok(packet, 9)) {	/* XXX - hardcoded,
							   padding in struct */
				memcpy(&vpdInfo->flashCfg, packet->data, 9);
			}
			break;
		case VPD_PID_ETHADDR:
			memcpy(vpdInfo->ethAddrs, packet->data, packet->size);
			break;
		case VPD_PID_POTS:
			if (size_ok(packet, sizeof(char)))
				vpdInfo->numPOTS = (unsigned) *packet->data;
			break;
		case VPD_PID_DS1:
			if (size_ok(packet, sizeof(char)))
				vpdInfo->numDS1 = (unsigned) *packet->data;
		case VPD_PID_GAL:
		case VPD_PID_CRC:
		case VPD_PID_TERM:
			break;
		default:
			printf("Warning: Found unknown VPD packet ID 0x%x\n",
			       packet->identifier);
			break;
		}
	} while ((packet = vpd_get_packet(packet)));

	return 0;
}


/*
 * vpd_init() - Initialize default VPD environment
 */
int vpd_init(unsigned char dev_addr)
{
	return 0;
}


/*
 * vpd_print() - Pretty print the VPD data.
 */
void vpd_print(VPD *vpdInfo)
{
	const char *const sp = "";
	const char *const sfmt = "%4s%-20s: \"%s\"\n";
	const char *const cfmt = "%4s%-20s: '%c'\n";
	const char *const dfmt = "%4s%-20s: %ld\n";
	const char *const hfmt = "%4s%-20s: %08lX\n";
	const char *const dsfmt = "%4s%-20s: %d\n";
	const char *const hsfmt = "%4s%-20s: %04X\n";
	const char *const dhfmt = "%4s%-20s: %ld (%lX)\n";

	printf("VPD read from I2C device: %02X\n", vpdInfo->_devAddr);

	if (vpdInfo->productId[0])
		printf(sfmt, sp, "Product ID", vpdInfo->productId);
	else
		printf(sfmt, sp, "Product ID", "UNKNOWN");

	if (vpdInfo->revisionId)
		printf(cfmt, sp, "Revision ID", vpdInfo->revisionId);

	if (vpdInfo->serialNum)
		printf(dfmt, sp, "Serial Number", vpdInfo->serialNum);

	if (vpdInfo->manuID)
		printf(dfmt, sp, "Manufacture ID", (long) vpdInfo->manuID);

	if (vpdInfo->configOpt)
		printf(hfmt, sp, "Configuration", vpdInfo->configOpt);

	if (vpdInfo->sysClk)
		printf(dhfmt, sp, "System Clock", vpdInfo->sysClk,
		       vpdInfo->sysClk);

	if (vpdInfo->serClk)
		printf(dhfmt, sp, "Serial Clock", vpdInfo->serClk,
		       vpdInfo->serClk);

	if (vpdInfo->numPOTS)
		printf(dfmt, sp, "Number of POTS lines", vpdInfo->numPOTS);

	if (vpdInfo->numDS1)
		printf(dfmt, sp, "Number of DS1s", vpdInfo->numDS1);

	/* Print Ethernet Addresses */
	if (vpdInfo->ethAddrs[0][0] != 0xff) {
		int i, j;

		printf("%4sEtherNet Address(es): ", sp);
		for (i = 0; i < MAX_ETH_ADDRS; i++) {
			if (vpdInfo->ethAddrs[i][0] != 0xff) {
				for (j = 0; j < 6; j++) {
					printf("%02X",
					       vpdInfo->ethAddrs[i][j]);
					if (((j + 1) % 6) != 0)
						printf(":");
					else
						printf(" ");
				}
				if (((i + 1) % 3) == 0)
					printf("\n%24s: ", sp);
			}
		}
		printf("\n");
	}

	if (vpdInfo->flashCfg.mfg && vpdInfo->flashCfg.dev) {
		printf("Main Flash Configuration:\n");
		printf(hsfmt, sp, "Manufacture ID", vpdInfo->flashCfg.mfg);
		printf(hsfmt, sp, "Device ID", vpdInfo->flashCfg.dev);
		printf(dsfmt, sp, "Device Width", vpdInfo->flashCfg.devWidth);
		printf(dsfmt, sp, "Num. Devices", vpdInfo->flashCfg.numDevs);
		printf(dsfmt, sp, "Num. Columns", vpdInfo->flashCfg.numCols);
		printf(dsfmt, sp, "Column Width", vpdInfo->flashCfg.colWidth);
		printf(dsfmt, sp, "WE Data Width",
		       vpdInfo->flashCfg.weDataWidth);
	}
}
