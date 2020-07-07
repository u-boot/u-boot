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

/* Top 4 bits of the value used to indicate a three-byte length value */
#define ACPI_PKG_LEN_3_BYTES	0x80

/* ACPI Op/Prefix codes */
enum {
	ZERO_OP			= 0x00,
	ONE_OP			= 0x01,
	BYTE_PREFIX		= 0x0a,
	WORD_PREFIX		= 0x0b,
	DWORD_PREFIX		= 0x0c,
	STRING_PREFIX		= 0x0d,
	QWORD_PREFIX		= 0x0e,
	PACKAGE_OP		= 0x12,
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
 * acpigen_write_string() - Write a string
 *
 * This writes a STRING_PREFIX followed by a null-terminated string
 *
 * @ctx: ACPI context pointer
 * @str: String to write
 */
void acpigen_write_string(struct acpi_ctx *ctx, const char *str);
#endif
