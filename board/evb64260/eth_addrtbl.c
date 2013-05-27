#include <common.h>
#include <malloc.h>
#include <galileo/gt64260R.h>
#include <galileo/core.h>
#include <asm/cache.h>
#include "eth.h"
#include "eth_addrtbl.h"

#define PRINTF printf

#ifdef CONFIG_GT_USE_MAC_HASH_TABLE

static u32 addressTableHashMode[GAL_ETH_DEVS] = { 0, };
static u32 addressTableHashSize[GAL_ETH_DEVS] = { 0, };
static addrTblEntry *addressTableBase[GAL_ETH_DEVS] = { 0, };
static void *realAddrTableBase[GAL_ETH_DEVS] = { 0, };

static const u32 hashLength[2] = {
	(0x8000),		/* 8K * 4 entries */
	(0x8000 / 16),		/* 512 * 4 entries */
};

/* Initialize the address table for a port, if needed */
unsigned int initAddressTable (u32 port, u32 hashMode, u32 hashSizeSelector)
{
	unsigned int tableBase;

	if (port < 0 || port >= GAL_ETH_DEVS) {
		printf ("%s: Invalid port number %d\n", __FUNCTION__, port);
		return 0;
	}

	if (hashMode > 1) {
		printf ("%s: Invalid Hash Mode %d\n", __FUNCTION__, port);
		return 0;
	}

	if (realAddrTableBase[port] &&
	    (addressTableHashSize[port] != hashSizeSelector)) {
		/* we have been here before,
		 * but now we want a different sized table
		 */
		free (realAddrTableBase[port]);
		realAddrTableBase[port] = 0;
		addressTableBase[port] = 0;

	}

	tableBase = (unsigned int) addressTableBase[port];
	/* we get called for every probe, so only do this once */
	if (!tableBase) {
		int bytes =
			hashLength[hashSizeSelector] * sizeof (addrTblEntry);

		realAddrTableBase[port] =
			malloc (bytes + 64);
		tableBase = (unsigned int)realAddrTableBase;

		if (!tableBase) {
			printf ("%s: alloc memory failed \n", __FUNCTION__);
			return 0;
		}

		/* align to octal byte */
		if (tableBase & 63)
			tableBase = (tableBase + 63) & ~63;

		addressTableHashMode[port] = hashMode;
		addressTableHashSize[port] = hashSizeSelector;
		addressTableBase[port] = (addrTblEntry *) tableBase;

		memset ((void *) tableBase, 0, bytes);
	}

	return tableBase;
}

/*
 * ----------------------------------------------------------------------------
 * This function will calculate the hash function of the address.
 * depends on the hash mode and hash size.
 * Inputs
 * macH             - the 2 most significant bytes of the MAC address.
 * macL             - the 4 least significant bytes of the MAC address.
 * hashMode         - hash mode 0 or hash mode 1.
 * hashSizeSelector - indicates number of hash table entries (0=0x8000,1=0x800)
 * Outputs
 * return the calculated entry.
 */
u32 hashTableFunction (u32 macH, u32 macL, u32 HashSize, u32 hash_mode)
{
	u32 hashResult;
	u32 addrH;
	u32 addrL;
	u32 addr0;
	u32 addr1;
	u32 addr2;
	u32 addr3;
	u32 addrHSwapped;
	u32 addrLSwapped;


	addrH = NIBBLE_SWAPPING_16_BIT (macH);
	addrL = NIBBLE_SWAPPING_32_BIT (macL);

	addrHSwapped = FLIP_4_BITS (addrH & 0xf)
		+ ((FLIP_4_BITS ((addrH >> 4) & 0xf)) << 4)
		+ ((FLIP_4_BITS ((addrH >> 8) & 0xf)) << 8)
		+ ((FLIP_4_BITS ((addrH >> 12) & 0xf)) << 12);

	addrLSwapped = FLIP_4_BITS (addrL & 0xf)
		+ ((FLIP_4_BITS ((addrL >> 4) & 0xf)) << 4)
		+ ((FLIP_4_BITS ((addrL >> 8) & 0xf)) << 8)
		+ ((FLIP_4_BITS ((addrL >> 12) & 0xf)) << 12)
		+ ((FLIP_4_BITS ((addrL >> 16) & 0xf)) << 16)
		+ ((FLIP_4_BITS ((addrL >> 20) & 0xf)) << 20)
		+ ((FLIP_4_BITS ((addrL >> 24) & 0xf)) << 24)
		+ ((FLIP_4_BITS ((addrL >> 28) & 0xf)) << 28);

	addrH = addrHSwapped;
	addrL = addrLSwapped;

	if (hash_mode == 0) {
		addr0 = (addrL >> 2) & 0x03f;
		addr1 = (addrL & 0x003) | ((addrL >> 8) & 0x7f) << 2;
		addr2 = (addrL >> 15) & 0x1ff;
		addr3 = ((addrL >> 24) & 0x0ff) | ((addrH & 1) << 8);
	} else {
		addr0 = FLIP_6_BITS (addrL & 0x03f);
		addr1 = FLIP_9_BITS (((addrL >> 6) & 0x1ff));
		addr2 = FLIP_9_BITS ((addrL >> 15) & 0x1ff);
		addr3 = FLIP_9_BITS ((((addrL >> 24) & 0x0ff) |
				      ((addrH & 0x1) << 8)));
	}

	hashResult = (addr0 << 9) | (addr1 ^ addr2 ^ addr3);

	if (HashSize == _8K_TABLE) {
		hashResult = hashResult & 0xffff;
	} else {
		hashResult = hashResult & 0x07ff;
	}

	return (hashResult);
}


/*
 * ----------------------------------------------------------------------------
 * This function will add an entry to the address table.
 * depends on the hash mode and hash size that was initialized.
 * Inputs
 * port - ETHERNET port number.
 * macH - the 2 most significant bytes of the MAC address.
 * macL - the 4 least significant bytes of the MAC address.
 * skip - if 1, skip this address.
 * rd   - the RD field in the address table.
 * Outputs
 * address table entry is added.
 * true if success.
 * false if table full
 */
int addAddressTableEntry (u32 port, u32 macH, u32 macL, u32 rd, u32 skip)
{
	addrTblEntry *entry;
	u32 newHi;
	u32 newLo;
	u32 i;

	newLo = (((macH >> 4) & 0xf) << 15)
		| (((macH >> 0) & 0xf) << 11)
		| (((macH >> 12) & 0xf) << 7)
		| (((macH >> 8) & 0xf) << 3)
		| (((macL >> 20) & 0x1) << 31)
		| (((macL >> 16) & 0xf) << 27)
		| (((macL >> 28) & 0xf) << 23)
		| (((macL >> 24) & 0xf) << 19)
		| (skip << SKIP_BIT) | (rd << 2) | VALID;

	newHi = (((macL >> 4) & 0xf) << 15)
		| (((macL >> 0) & 0xf) << 11)
		| (((macL >> 12) & 0xf) << 7)
		| (((macL >> 8) & 0xf) << 3)
		| (((macL >> 21) & 0x7) << 0);

	/*
	 * Pick the appropriate table, start scanning for free/reusable
	 * entries at the index obtained by hashing the specified MAC address
	 */
	entry = addressTableBase[port];
	entry += hashTableFunction (macH, macL, addressTableHashSize[port],
				    addressTableHashMode[port]);
	for (i = 0; i < HOP_NUMBER; i++, entry++) {
		if (!(entry->lo & VALID) /*|| (entry->lo & SKIP) */ ) {
			break;
		} else {	/* if same address put in same position */
			if (((entry->lo & 0xfffffff8) == (newLo & 0xfffffff8))
			    && (entry->hi == newHi)) {
				break;
			}
		}
	}

	if (i == HOP_NUMBER) {
		PRINTF ("addGT64260addressTableEntry: table section is full\n");
		return false;
	}

	/*
	 * Update the selected entry
	 */
	entry->hi = newHi;
	entry->lo = newLo;
	DCACHE_FLUSH_N_SYNC ((u32) entry, MAC_ENTRY_SIZE);
	return true;
}

#endif /* CONFIG_GT_USE_MAC_HASH_TABLE */
