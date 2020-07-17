/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Core ACPI (Advanced Configuration and Power Interface) support
 *
 * Copyright 2019 Google LLC
 *
 * Modified from coreboot file acpigen.h
 */

#ifndef __ACPI_ACPIGEN_H
#define __ACPI_ACPIGEN_H

#include <linux/types.h>

struct acpi_ctx;
struct acpi_gen_regaddr;
struct acpi_gpio;

/* Top 4 bits of the value used to indicate a three-byte length value */
#define ACPI_PKG_LEN_3_BYTES	0x80

#define ACPI_METHOD_NARGS_MASK		0x7
#define ACPI_METHOD_SERIALIZED_MASK	BIT(3)

#define ACPI_END_TAG			0x79

/* ACPI Op/Prefix codes */
enum {
	ZERO_OP			= 0x00,
	ONE_OP			= 0x01,
	NAME_OP			= 0x08,
	BYTE_PREFIX		= 0x0a,
	WORD_PREFIX		= 0x0b,
	DWORD_PREFIX		= 0x0c,
	STRING_PREFIX		= 0x0d,
	QWORD_PREFIX		= 0x0e,
	SCOPE_OP		= 0x10,
	BUFFER_OP		= 0x11,
	PACKAGE_OP		= 0x12,
	METHOD_OP		= 0x14,
	SLEEP_OP		= 0x22,
	DUAL_NAME_PREFIX	= 0x2e,
	MULTI_NAME_PREFIX	= 0x2f,
	DEBUG_OP		= 0x31,
	EXT_OP_PREFIX		= 0x5b,
	ROOT_PREFIX		= 0x5c,
	LOCAL0_OP		= 0x60,
	LOCAL1_OP		= 0x61,
	LOCAL2_OP		= 0x62,
	LOCAL3_OP		= 0x63,
	LOCAL4_OP		= 0x64,
	LOCAL5_OP		= 0x65,
	LOCAL6_OP		= 0x66,
	LOCAL7_OP		= 0x67,
	STORE_OP		= 0x70,
	AND_OP			= 0x7b,
	OR_OP			= 0x7d,
	NOT_OP			= 0x80,
	DEVICE_OP		= 0x82,
	POWER_RES_OP		= 0x84,
	RETURN_OP		= 0xa4,
};

/**
 * acpigen_get_current() - Get the current ACPI code output pointer
 *
 * @ctx: ACPI context pointer
 * @return output pointer
 */
u8 *acpigen_get_current(struct acpi_ctx *ctx);

/**
 * acpigen_emit_byte() - Emit a byte to the ACPI code
 *
 * @ctx: ACPI context pointer
 * @data: Value to output
 */
void acpigen_emit_byte(struct acpi_ctx *ctx, uint data);

/**
 * acpigen_emit_word() - Emit a 16-bit word to the ACPI code
 *
 * @ctx: ACPI context pointer
 * @data: Value to output
 */
void acpigen_emit_word(struct acpi_ctx *ctx, uint data);

/**
 * acpigen_emit_dword() - Emit a 32-bit 'double word' to the ACPI code
 *
 * @ctx: ACPI context pointer
 * @data: Value to output
 */
void acpigen_emit_dword(struct acpi_ctx *ctx, uint data);

/**
 * acpigen_emit_stream() - Emit a stream of bytes
 *
 * @ctx: ACPI context pointer
 * @data: Data to output
 * @size: Size of data in bytes
 */
void acpigen_emit_stream(struct acpi_ctx *ctx, const char *data, int size);

/**
 * acpigen_emit_string() - Emit a string
 *
 * Emit a string with a null terminator
 *
 * @ctx: ACPI context pointer
 * @str: String to output, or NULL for an empty string
 */
void acpigen_emit_string(struct acpi_ctx *ctx, const char *str);

/**
 * acpigen_write_len_f() - Write a 'forward' length placeholder
 *
 * This adds space for a length value in the ACPI stream and pushes the current
 * position (before the length) on the stack. After calling this you can write
 * some data and then call acpigen_pop_len() to update the length value.
 *
 * Usage:
 *
 *    acpigen_write_len_f() ------\
 *    acpigen_write...()          |
 *    acpigen_write...()          |
 *      acpigen_write_len_f() --\ |
 *      acpigen_write...()      | |
 *      acpigen_write...()      | |
 *      acpigen_pop_len() ------/ |
 *    acpigen_write...()          |
 *    acpigen_pop_len() ----------/
 *
 * See ACPI 6.3 section 20.2.4 Package Length Encoding
 *
 * This implementation always uses a 3-byte packet length for simplicity. It
 * could be adjusted to support other lengths.
 *
 * @ctx: ACPI context pointer
 */
void acpigen_write_len_f(struct acpi_ctx *ctx);

/**
 * acpigen_pop_len() - Update the previously stacked length placeholder
 *
 * Call this after the data for the block has been written. It updates the
 * top length value in the stack and pops it off.
 *
 * @ctx: ACPI context pointer
 */
void acpigen_pop_len(struct acpi_ctx *ctx);

/**
 * acpigen_write_package() - Start writing a package
 *
 * A package collects together a number of elements in the ACPI code. To write
 * a package use:
 *
 * acpigen_write_package(ctx, 3);
 * ...write things
 * acpigen_pop_len()
 *
 * If you don't know the number of elements in advance, acpigen_write_package()
 * returns a pointer to the value so you can update it later:
 *
 * char *num_elements = acpigen_write_package(ctx, 0);
 * ...write things
 * *num_elements += 1;
 * ...write things
 * *num_elements += 1;
 * acpigen_pop_len()
 *
 * @ctx: ACPI context pointer
 * @nr_el: Number of elements (0 if not known)
 * @returns pointer to the number of elements, which can be updated by the
 *	caller if needed
 */
char *acpigen_write_package(struct acpi_ctx *ctx, int nr_el);

/**
 * acpigen_write_byte() - Write a byte
 *
 * @ctx: ACPI context pointer
 * @data: Value to write
 */
void acpigen_write_byte(struct acpi_ctx *ctx, unsigned int data);

/**
 * acpigen_write_word() - Write a word
 *
 * @ctx: ACPI context pointer
 * @data: Value to write
 */
void acpigen_write_word(struct acpi_ctx *ctx, unsigned int data);

/**
 * acpigen_write_dword() - Write a dword
 *
 * @ctx: ACPI context pointer
 * @data: Value to write
 */
void acpigen_write_dword(struct acpi_ctx *ctx, unsigned int data);

/**
 * acpigen_write_qword() - Write a qword
 *
 * @ctx: ACPI context pointer
 * @data: Value to write
 */
void acpigen_write_qword(struct acpi_ctx *ctx, u64 data);

/**
 * acpigen_write_zero() - Write zero
 *
 * @ctx: ACPI context pointer
 */
void acpigen_write_zero(struct acpi_ctx *ctx);

/**
 * acpigen_write_one() - Write one
 *
 * @ctx: ACPI context pointer
 */
void acpigen_write_one(struct acpi_ctx *ctx);

/**
 * acpigen_write_integer() - Write an integer
 *
 * This writes an operation (BYTE_OP, WORD_OP, DWORD_OP, QWORD_OP depending on
 * the integer size) and an integer value. Note that WORD means 16 bits in ACPI.
 *
 * @ctx: ACPI context pointer
 * @data: Integer to write
 */
void acpigen_write_integer(struct acpi_ctx *ctx, u64 data);

/**
 * acpigen_write_name_zero() - Write a named zero value
 *
 * @ctx: ACPI context pointer
 * @name: Name of the value
 */
void acpigen_write_name_zero(struct acpi_ctx *ctx, const char *name);

/**
 * acpigen_write_name_one() - Write a named one value
 *
 * @ctx: ACPI context pointer
 * @name: Name of the value
 */
void acpigen_write_name_one(struct acpi_ctx *ctx, const char *name);

/**
 * acpigen_write_name_byte() - Write a named byte value
 *
 * @ctx: ACPI context pointer
 * @name: Name of the value
 * @val: Value to write
 */
void acpigen_write_name_byte(struct acpi_ctx *ctx, const char *name, uint val);

/**
 * acpigen_write_name_word() - Write a named word value
 *
 * @ctx: ACPI context pointer
 * @name: Name of the value
 * @val: Value to write
 */
void acpigen_write_name_word(struct acpi_ctx *ctx, const char *name, uint val);

/**
 * acpigen_write_name_dword() - Write a named dword value
 *
 * @ctx: ACPI context pointer
 * @name: Name of the value
 * @val: Value to write
 */
void acpigen_write_name_dword(struct acpi_ctx *ctx, const char *name, uint val);

/**
 * acpigen_write_name_qword() - Write a named qword value
 *
 * @ctx: ACPI context pointer
 * @name: Name of the value
 * @val: Value to write
 */
void acpigen_write_name_qword(struct acpi_ctx *ctx, const char *name, u64 val);

/**
 * acpigen_write_name_integer() - Write a named integer value
 *
 * @ctx: ACPI context pointer
 * @name: Name of the value
 * @val: Value to write
 */
void acpigen_write_name_integer(struct acpi_ctx *ctx, const char *name,
				u64 val);

/**
 * acpigen_write_name_string() - Write a named string value
 *
 * @ctx: ACPI context pointer
 * @name: Name of the value
 * @string: String to write
 */
void acpigen_write_name_string(struct acpi_ctx *ctx, const char *name,
			       const char *string);

/**
 * acpigen_write_string() - Write a string
 *
 * This writes a STRING_PREFIX followed by a null-terminated string
 *
 * @ctx: ACPI context pointer
 * @str: String to write
 */
void acpigen_write_string(struct acpi_ctx *ctx, const char *str);

/**
 * acpigen_emit_namestring() - Emit an ACPI name
 *
 * This writes out an ACPI name or path in the required special format. It does
 * not add the NAME_OP prefix.
 *
 * @ctx: ACPI context pointer
 * @namepath: Name / path to emit
 */
void acpigen_emit_namestring(struct acpi_ctx *ctx, const char *namepath);

/**
 * acpigen_write_name() - Write out an ACPI name
 *
 * This writes out an ACPI name or path in the required special format with a
 * NAME_OP prefix.
 *
 * @ctx: ACPI context pointer
 * @namepath: Name / path to emit
 */
void acpigen_write_name(struct acpi_ctx *ctx, const char *namepath);

/**
 * acpigen_write_scope() - Write a scope
 *
 * @ctx: ACPI context pointer
 * @scope: Scope to write (e.g. "\\_SB.ABCD")
 */
void acpigen_write_scope(struct acpi_ctx *ctx, const char *scope);

/**
 * acpigen_write_uuid() - Write a UUID
 *
 * This writes out a UUID in the format used by ACPI, with a BUFFER_OP prefix.
 *
 * @ctx: ACPI context pointer
 * @uuid: UUID to write in the form aabbccdd-eeff-gghh-iijj-kkllmmnnoopp
 * @return 0 if OK, -EINVAL if the format is incorrect
 */
int acpigen_write_uuid(struct acpi_ctx *ctx, const char *uuid);

/**
 * acpigen_emit_ext_op() - Emit an extended op with the EXT_OP_PREFIX prefix
 *
 * @ctx: ACPI context pointer
 * @op: Operation code (e.g. SLEEP_OP)
 */
void acpigen_emit_ext_op(struct acpi_ctx *ctx, uint op);

/**
 * acpigen_write_method() - Write a method header
 *
 * @ctx: ACPI context pointer
 * @name: Method name (4 characters)
 * @nargs: Number of method arguments (0 if none)
 */
void acpigen_write_method(struct acpi_ctx *ctx, const char *name, int nargs);

/**
 * acpigen_write_method_serialized() - Write a method header
 *
 * This sets the 'serialized' flag so that the method is thread-safe
 *
 * @ctx: ACPI context pointer
 * @name: Method name (4 characters)
 * @nargs: Number of method arguments (0 if none)
 */
void acpigen_write_method_serialized(struct acpi_ctx *ctx, const char *name,
				     int nargs);

/**
 * acpigen_write_device() - Write an ACPI device
 *
 * @ctx: ACPI context pointer
 * @name: Device name to write
 */
void acpigen_write_device(struct acpi_ctx *ctx, const char *name);

/**
 * acpigen_write_sta() - Write a _STA method
 *
 * @ctx: ACPI context pointer
 * @status: Status value to return
 */
void acpigen_write_sta(struct acpi_ctx *ctx, uint status);

/**
 * acpigen_write_resourcetemplate_header() - Write a ResourceTemplate header
 *
 * @ctx: ACPI context pointer
 */
void acpigen_write_resourcetemplate_header(struct acpi_ctx *ctx);

/**
 * acpigen_write_resourcetemplate_footer() - Write a ResourceTemplate footer
 *
 * @ctx: ACPI context pointer
 */
void acpigen_write_resourcetemplate_footer(struct acpi_ctx *ctx);

/**
 * acpigen_write_register_resource() - Write a register resource
 *
 * This writes a header, the address information and a footer
 *
 * @ctx: ACPI context pointer
 * @addr: Address to write
 */
void acpigen_write_register_resource(struct acpi_ctx *ctx,
				     const struct acpi_gen_regaddr *addr);

/**
 * acpigen_write_sleep() - Write a sleep operation
 *
 * @ctx: ACPI context pointer
 * @sleep_ms: Number of milliseconds to sleep for
 */
void acpigen_write_sleep(struct acpi_ctx *ctx, u64 sleep_ms);

/**
 * acpigen_write_store() - Write a store operation
 *
 * @ctx: ACPI context pointer
 */
void acpigen_write_store(struct acpi_ctx *ctx);

/**
 * acpigen_write_debug_string() - Write a debug string
 *
 * This writes a debug operation with an associated string
 *
 * @ctx: ACPI context pointer
 * @str: String to write
 */
void acpigen_write_debug_string(struct acpi_ctx *ctx, const char *str);

/**
 * acpigen_write_or() - Write a bitwise OR operation
 *
 * res = arg1 | arg2
 *
 * @ctx: ACPI context pointer
 * @arg1: ACPI opcode for operand 1 (e.g. LOCAL0_OP)
 * @arg2: ACPI opcode for operand 2 (e.g. LOCAL1_OP)
 * @res: ACPI opcode for result (e.g. LOCAL2_OP)
 */
void acpigen_write_or(struct acpi_ctx *ctx, u8 arg1, u8 arg2, u8 res);

/**
 * acpigen_write_and() - Write a bitwise AND operation
 *
 * res = arg1 & arg2
 *
 * @ctx: ACPI context pointer
 * @arg1: ACPI opcode for operand 1 (e.g. LOCAL0_OP)
 * @arg2: ACPI opcode for operand 2 (e.g. LOCAL1_OP)
 * @res: ACPI opcode for result (e.g. LOCAL2_OP)
 */
void acpigen_write_and(struct acpi_ctx *ctx, u8 arg1, u8 arg2, u8 res);

/**
 * acpigen_write_not() - Write a bitwise NOT operation
 *
 * res = ~arg1
 *
 * @ctx: ACPI context pointer
 * @arg: ACPI opcode for operand (e.g. LOCAL0_OP)
 * @res: ACPI opcode for result (e.g. LOCAL2_OP)
 */
void acpigen_write_not(struct acpi_ctx *ctx, u8 arg, u8 res);

/**
 * acpigen_write_power_res() - Write a power resource
 *
 * Name (_PRx, Package(One) { name })
 * ...
 * PowerResource (name, level, order)
 *
 * The caller should fill in the rest of the power resource and then call
 * acpigen_pop_len() to close it off
 *
 * @ctx: ACPI context pointer
 * @name: Name of power resource (e.g. "PRIC")
 * @level: Deepest sleep level that this resource must be kept on (0=S0, 3=S3)
 * @order: Order that this must be enabled/disabled (e.g. 0)
 * @dev_stats: List of states to define, e.g. {"_PR0", "_PR3"}
 * @dev_states_count: Number of dev states
 */
void acpigen_write_power_res(struct acpi_ctx *ctx, const char *name, uint level,
			     uint order, const char *const dev_states[],
			     size_t dev_states_count);

/**
 * acpigen_set_enable_tx_gpio() - Emit ACPI code to enable/disable a GPIO
 *
 * This emits code to either enable to disable a Tx GPIO. It takes account of
 * the GPIO polarity.
 *
 * The code needs access to the DW0 register for the pad being used. This is
 * provided by gpio->pin0_addr and ACPI methods must be defined for the board
 * which can read and write the pad's DW0 register given this address:
 *    @dw0_read: takes a single argument, the DW0 address
 *		 returns the DW0 value
 *    @dw0:write: takes two arguments, the DW0 address and the value to write
 *		 no return value
 *
 * Example code (-- means comment):
 *
 *	-- Get Pad Configuration DW0 register value
 *	Method (GPC0, 0x1, Serialized)
 *	{
 *		-- Arg0 - GPIO DW0 address
 *		Store (Arg0, Local0)
 *		OperationRegion (PDW0, SystemMemory, Local0, 4)
 *		Field (PDW0, AnyAcc, NoLock, Preserve) {
 *			TEMP, 32
 *		}
 *		Return (TEMP)
 *	}
 *
 *	-- Set Pad Configuration DW0 register value
 *	Method (SPC0, 0x2, Serialized)
 *	{
 *		-- Arg0 - GPIO DW0 address
 *		-- Arg1 - Value for DW0 register
 *		Store (Arg0, Local0)
 *		OperationRegion (PDW0, SystemMemory, Local0, 4)
 *		Field (PDW0, AnyAcc, NoLock, Preserve) {
 *			TEMP,32
 *		}
 *		Store (Arg1, TEMP)
 *	}
 *
 *
 * @ctx: ACPI context pointer
 * @tx_state_val: Mask to use to toggle the TX state on the GPIO pin, e,g.
 *	PAD_CFG0_TX_STATE
 * @dw0_read: Method name to use to read dw0, e.g. "\\_SB.GPC0"
 * @dw0_write: Method name to use to read dw0, e.g. "\\_SB.SPC0"
 * @gpio: GPIO to change
 * @enable: true to enable GPIO, false to disable
 * Returns 0 on success, -ve on error.
 */
int acpigen_set_enable_tx_gpio(struct acpi_ctx *ctx, u32 tx_state_val,
			       const char *dw0_read, const char *dw0_write,
			       struct acpi_gpio *gpio, bool enable);

#endif
