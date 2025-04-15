#ifndef _ASM_X86_E820_H
#define _ASM_X86_E820_H

#define E820MAX		128	/* number of entries in E820MAP */

#ifdef __ASSEMBLY__

#define E820_RAM	1
#define E820_RESERVED	2
#define E820_ACPI	3
#define E820_NVS	4
#define E820_UNUSABLE	5
#define E820_COUNT	6	/* Number of types */

#else

#include <linux/types.h>

/* Available e820 memory-region types */
enum e820_type {
	E820_RAM	= 1,
	E820_RESERVED,
	E820_ACPI,
	E820_NVS,
	E820_UNUSABLE,

	E820_COUNT,
};

struct e820_entry {
	__u64 addr;	/* start of memory segment */
	__u64 size;	/* size of memory segment */
	__u32 type;	/* type of memory segment */
} __attribute__((packed));

#define ISA_START_ADDRESS	0xa0000
#define ISA_END_ADDRESS		0x100000

/**
 * Context to use for e820_add()
 *
 * @entries: Table being filled in
 * @addr: Current address we are up to
 * @count: Number of entries added to @entries so far
 * @max_entries: Maximum number of entries allowed
 */
struct e820_ctx {
	struct e820_entry *entries;
	u64 addr;
	int count;
	int max_entries;
};

/**
 * e820_init() - Start setting up an e820 table
 *
 * @ctx: Context to set up
 * @entries: Place to put entries
 * @max_entries: Maximum size of @entries
 */
void e820_init(struct e820_ctx *ctx, struct e820_entry *entries,
	       int max_entries);

/**
 * e820_add() - Add an entry to the table
 *
 * @ctx: Context
 * @type: Type of entry
 * @addr: Start address of entry
 * @size Size of entry
 */
void e820_add(struct e820_ctx *ctx, enum e820_type type, u64 addr, u64 size);

/**
 * e820_to_addr() - Add an entry that covers the space up to a given address
 *
 * @ctx: Context
 * @type: Type of entry
 * @end_addr: Address where the entry should finish
 */
void e820_to_addr(struct e820_ctx *ctx, enum e820_type type, u64 end_addr);

/**
 * e820_next() - Add an entry that carries on from the last one
 *
 * @ctx: Context
 * @type: Type of entry
 * @size Size of entry
 */
void e820_next(struct e820_ctx *ctx, enum e820_type type, u64 size);

/**
 * e820_finish() - Finish the table
 *
 * Checks the table is not too large, panics if so
 *
 * @ctx: Context
 * Returns: Number of entries
 */
int e820_finish(struct e820_ctx *ctx);

/* Implementation-defined function to install an e820 map */
unsigned int install_e820_map(unsigned int max_entries,
			      struct e820_entry *);

/**
 * e820_dump() - Dump the e820 table
 *
 * @entries: Pointer to start of table
 * @count: Number of entries in the table
 */
void e820_dump(struct e820_entry *entries, uint count);

/**
 * cb_install_e820_map() - Install e820 map provided by coreboot sysinfo
 *
 * This should be used when booting from coreboot, since in that case the
 * memory areas are provided by coreboot in its sysinfo.
 *
 * @max_entries: Maximum number of entries to write
 * @entries: Place to put entires
 * Return: number of entries written
 */
unsigned int cb_install_e820_map(unsigned int max_entries,
				 struct e820_entry *entries);

/**
 * e820_dump() - Dump an e820 table
 *
 * @entries: Pointer to first entry
 * @count: Number of entries in the table
 */
void e820_dump(struct e820_entry *entries, uint count);

#endif /* __ASSEMBLY__ */

#endif /* _ASM_X86_E820_H */
