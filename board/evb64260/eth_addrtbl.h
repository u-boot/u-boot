#ifndef _ADDRESS_TABLE_H
#define _ADDRESS_TABLE_H 1

/*
 * ----------------------------------------------------------------------------
 * addressTable.h - this file has all the declarations of the address table
 */

#define _8K_TABLE                           0
#define ADDRESS_TABLE_ALIGNMENT             8
#define HASH_DEFAULT_MODE                   14
#define HASH_MODE                           13
#define HASH_SIZE                           12
#define HOP_NUMBER                          12
#define MAC_ADDRESS_STRING_SIZE             12
#define MAC_ENTRY_SIZE                      sizeof(addrTblEntry)
#define MAX_NUMBER_OF_ADDRESSES_TO_STORE    1000
#define PROMISCUOUS_MODE                    0
#define SKIP                                1<<1
#define SKIP_BIT                            1
#define VALID                               1

/*
 * ----------------------------------------------------------------------------
 * XXX_MIKE - potential sign-extension bugs lurk here...
 */
#define NIBBLE_SWAPPING_32_BIT(X) ( (((X) & 0xf0f0f0f0) >> 4) \
				  | (((X) & 0x0f0f0f0f) << 4) )

#define NIBBLE_SWAPPING_16_BIT(X) ( (((X) & 0x0000f0f0) >> 4) \
				  | (((X) & 0x00000f0f) << 4) )

#define FLIP_4_BITS(X)  ( (((X) & 0x01) << 3) | (((X) & 0x002) << 1) \
			| (((X) & 0x04) >> 1) | (((X) & 0x008) >> 3) )

#define FLIP_6_BITS(X)  ( (((X) & 0x01) << 5) | (((X) & 0x020) >> 5) \
			| (((X) & 0x02) << 3) | (((X) & 0x010) >> 3) \
			| (((X) & 0x04) << 1) | (((X) & 0x008) >> 1) )

#define FLIP_9_BITS(X)  ( (((X) & 0x01) << 8) | (((X) & 0x100) >> 8) \
			| (((X) & 0x02) << 6) | (((X) & 0x080) >> 6) \
			| (((X) & 0x04) << 4) | (((X) & 0x040) >> 4) \
	 | ((X) & 0x10) | (((X) & 0x08) << 2) | (((X) & 0x020) >> 2) )

/*
 * V: value we're operating on
 * O: offset of rightmost bit in field
 * W: width of field to shift
 * S: distance to shift left
 */
#define MASK( fieldWidth )                            ((1 << (fieldWidth)) - 1)
#define leftShiftedBitfield( V,O,W,S)        (((V) & (MASK(W) << (O)))  << (S))
#define rightShiftedBitfield(V,O,W,S)  (((u32)((V) & (MASK(W) << (O)))) >> (S))


/*
 * Push to main memory all cache lines associated with
 * the specified range of virtual memory addresses
 *
 * A: Address of first byte in range to flush
 * N: Number of bytes to flush
 * Note - flush_dcache_range() does a "sync", does NOT invalidate
 */
#define DCACHE_FLUSH_N_SYNC( A, N )        flush_dcache_range( (A), ((A)+(N)) )


typedef struct addressTableEntryStruct  {
    u32 hi;
    u32 lo;
} addrTblEntry;

u32
uncachedPages( u32 pages  );
u32
hashTableFunction( u32 macH, u32 macL, u32 HashSize, u32 hash_mode );

unsigned int
initAddressTable( u32 port, u32 hashMode, u32 hashSize );

int
addAddressTableEntry( u32 port, u32 macH, u32 macL, u32 rd, u32 skip          );

#endif                                           /* #ifndef _ADDRESS_TABLE_H */
