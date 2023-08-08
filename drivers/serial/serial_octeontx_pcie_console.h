/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0+    BSD-3-Clause
 */
#ifndef __SERIAL_OCTEONTX_PCIE_CONSOLE_H__
#define __SERIAL_OCTEONTX_PCIE_CONSOLE_H__

/** CONSOLE! converted to little-endian */
#define OCTEONTX_PCIE_CONSOLE_MAGIC		0x21454C4F534E4F43

/** CONDSCR! converted to little-endian */
#define OCTEONTX_PCIE_CONSOLE_NEXUS_MAGIC	0x21524353444E4F43

#define OCTEONTX_PCIE_CONSOLE_MAJOR		1
#define OCTEONTX_PCIE_CONSOLE_MINOR		0

#define TURN_HOST				0
#define TURN_TARGET				1

/** Maximum number of supported consoles */
#define OCTEONTX_PCIE_MAX_CONSOLES		16

/** Set if writes should not block if the buffer is full */
#define OCTEONTX_PCIE_CONSOLE_FLAG_NONBLOCK	BIT(0)

/** Set if the PCIE console cannot be shared */
#define OCTEONTX_PCIE_CONSOLE_FLAG_EXCLUSIVE	BIT(1)

#define OCTEONTX_PCIE_CONSOLE_NAME_LEN		16

#define OCTEONTX_PCIE_CONSOLE_NAME		"U-Boot"

#define OCTEONTX_PCIE_CONSOLE_OWNER_UNUSED	0
#define OCTEONTX_PCIE_CONSOLE_OWNER_UBOOT	1

/**
 * Use Peterson's algorithm since atomics aren't supported by
 * most PCIe hosts (i.e. X86)
 *
 * @param	turn		Set to whoever is modifying it
 * @param	host_lock	Set if the host has it
 * @param	target_lock	Set if the target has it
 * @param	pad		padding
 *
 * NOTE: This data structure is 64-bits for endian purposes and alignment.
 */
struct octeontx_pcie_lock {
#ifdef __LITTLE_ENDIAN
	volatile u8	turn;
	volatile u8	host_lock;
	volatile u8	target_lock;
	u8		pad[5];
#else
	u8		pad[5];
	volatile u8	target_lock;
	volatile u8	host_lock;
	volatile u8	turn;
#endif
} __aligned(8);

/**
 * Structure that defines a single console.
 *
 * Note: when read_index == write_index, the buffer is empty.
 * The actual usable size  of each console is console_buf_size -1;
 *
 * There are two different types of locks.  pcie_lock is for locking
 * between the host and target.  excl_lock should always be acquired
 * before pcie_lock is acquired and released after pcie_lock is released.
 *
 * excl_lock is a spinlock held between different tasks, such as u-boot
 * and atf or the atf and the Linux kernel.  It should be held whenever
 * any of the indices are changed or when the pcie_lock is held.
 *
 * @param magic		console magic number OCTEONTX_PCIE_CONSOLE_MAGIC
 * @param name		name assigned to the console, i.e. "ATF" or "U-Boot"
 * @param flags		flags associated with console, see
 *			OCTEONTX_PCIE_CONSOLE_FLAG_...
 * @param owner_id		owning task id of last user, 0 if unused.
 * @param input_buf_size	Input buffer size in bytes
 * @param output_buf_size	Output buffer size in bytes
 * @param input_base_addr	Base address of input buffer
 * @param input_read_index	index target begins reading data from
 * @param input_write_index	index host starts writing from
 * @param output_base_addr	Base address of output buffer
 * @param host_console_connected	non-zero if host console is connected
 * @param output_read_index	index host reads from
 * @param output_write_index	index target writes to
 * @param pcie_lock		lock held whenever the indices are updated
 *				using Peterson's algorithm.  Use
 *				octeontx_pcie_target_lock() and
 *				octeontx_pcie_target_unlock() to lock and
 *				unlock this data structure.
 * @param user			User-defined pointer
 *				(octeontx_pcie_console_priv *) for U-Boot
 * @param excl_lock		cpu core lock.  This lock should be held
 *				whenever this data structure is updated by
 *				the target since it can be shared by multiple
 *				targets.
 * @param pad			pads header to 128 bytes
 *
 * Typically the input and output buffers immediately follow this data
 * structure, however, this is not a requirement.
 *
 * Note that the host_console_connected and output_read_index MUST be
 * next to each other and should be 64-bit aligned. This is due to the
 * fact that if the output buffer fills up and no host is connected that
 * the read pointer must be modified atomically in case the host should
 * connect within that window.
 */
struct octeontx_pcie_console {
	__le64			magic;
	char			name[OCTEONTX_PCIE_CONSOLE_NAME_LEN];
	volatile __le32		flags;
	volatile __le32		owner_id;
	__le32			input_buf_size;
	__le32			output_buf_size;
	__le64			input_base_addr;
	__le32			input_read_index;
	volatile __le32		input_write_index;
	__le64			output_base_addr;
	volatile __le32		host_console_connected;
	volatile __le32		output_read_index;
	__le32			output_write_index;
	octeontx_spinlock_t	excl_lock;
	void			*user;
	struct octeontx_pcie_lock	pcie_lock;
	u32			pad[8];
} __packed;

/**
 * This is the main container structure that contains all the information
 * about all PCI consoles.  The address of this structure is passed to
 * various routines that operation on PCI consoles.
 *
 * @param magic		console descriptor magic number
 * @param major_version	major version of console data structure
 * @param minor_version	minor version of console data structure
 * @param flags		flags applied to all consoles
 * @param num_consoles	number of console data structures available
 * @param excl_lock	lock between cores for this data structure
 * @param in_use	Set if the console is claimed by anyone (shared or not)
 * @param exclusive	bitmap of consoles exclusively used
 * @param pad		padding for header for future versions
 * @param console_addr	array of addresses for each console, 0 if unavailable.
 *
 * Note that in_use and exclusive need to be next to each other and that
 * they must remain in the same cacheline due to the need for atomic operations.
 */
struct octeontx_pcie_console_nexus {
	__le64		magic;
	u8		major_version;
	u8		minor_version;
	u8		flags;
	u8		num_consoles;
	octeontx_spinlock_t excl_lock;
	volatile __le32	in_use;
	volatile __le32	exclusive;
	u64		pad[13];
	/* Starts at offset 128 */
	__le64		console_addr[OCTEONTX_PCIE_MAX_CONSOLES];
} __packed;

struct octeontx_pcie_console_priv {
	struct resource res;
	struct octeontx_pcie_console *console;
	struct octeontx_pcie_console_nexus *nexus;
	struct stdio_dev *sdev;
	int console_num;
};

/**
 * Platform data
 *
 * @param addr	PCIe console descriptor address
 * @param size	PCIe console descriptor size (including all consoles)
 * @param base	user base address (if different) but usually the same as addr
 * @param desc	PCIe console descriptor pointer
 * @param console U-Boot's pcie console
 */
struct octeontx_pcie_console_plat_data {
	fdt_addr_t addr;
	fdt_addr_t size;
	void *base;
	struct octeontx_pcie_console_nexus *nexus;
	struct octeontx_pcie_console *console;	/* Only one console in U-Boot */
	struct resource res;
};

struct octeontx_pcie_console_nexus_priv {
	struct octeontx_pcie_console *console;
	struct octeontx_pcie_console_nexus *nexus;
	struct resource res;
	ofnode console_node;
};

/**
 * Peterson's algorithm for locking of data structures shared with PCIe host.
 * See https://en.wikipedia.org/wiki/Peterson%27s_algorithm
 *
 * The two flags are instead called target_lock and host_lock.  Since it's
 * possible that multiple CPU cores (target) may want to acquire this lock,
 * use octeontx_spin_lock() and octeontx_spin_unlock() around any PCIe locks.
 *
 * @param	lock	pointer to PCIe lock datastructure
 *
 */
static inline void octeontx_pcie_init_target_lock(
				struct octeontx_pcie_lock *lock)
{
	lock->host_lock = 0;
	lock->target_lock = 0;
	lock->turn = TURN_TARGET;

}

/**
 * Peterson's algorithm for locking of data structures shared with PCIe host.
 * See https://en.wikipedia.org/wiki/Peterson%27s_algorithm
 *
 * The two flags are instead called target_lock and host_lock.  Since it's
 * possible that multiple CPU cores (target) may want to acquire this lock,
 * use octeontx_spin_lock() and octeontx_spin_unlock() around any PCIe locks.
 *
 * @param	lock	pointer to PCIe lock datastructure
 *
 */
static inline void octeontx_pcie_target_lock(struct octeontx_pcie_lock *lock)
{
	lock->target_lock = 1;
	__iowmb();
	lock->turn = TURN_TARGET;
	__iowmb();
	while (lock->host_lock && lock->turn == TURN_TARGET)
		__iormb();
}

/**
 * Peterson's algorithm for unlocking of data structures shared with PCIe host
 *
 * @param	lock	pointer to PCIe lock datastructure
 *
 */
static inline void octeontx_pcie_target_unlock(struct octeontx_pcie_lock *lock)
{
	lock->target_lock = 0;
	__iowmb();
}

/**
 * This is basically the algorithm that should be called on the PCIe host
 */
static inline void octeontx_pcie_host_lock(struct octeontx_pcie_lock *lock)
{
	lock->host_lock = 1;
	__iowmb();
	lock->turn = TURN_HOST;
	__iowmb();
}

/**
 * This is basically the algorithm that should be called on the PCIe host
 */
static inline void octeontx_pcie_host_unlock(struct octeontx_pcie_lock *lock)
{
	lock->host_lock = 0;
	__iowmb();
}

#endif /* __SERIAL_OCTEONTX_PCIE_CONSOLE_H__ */
