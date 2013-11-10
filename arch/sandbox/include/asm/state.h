/*
 * Copyright (c) 2011-2012 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SANDBOX_STATE_H
#define __SANDBOX_STATE_H

#include <config.h>
#include <stdbool.h>

/* How we exited U-Boot */
enum exit_type_id {
	STATE_EXIT_NORMAL,
	STATE_EXIT_COLD_REBOOT,
	STATE_EXIT_POWER_OFF,
};

struct sandbox_spi_info {
	const char *spec;
	const struct sandbox_spi_emu_ops *ops;
};

/* The complete state of the test system */
struct sandbox_state {
	const char *cmd;		/* Command to execute */
	bool interactive;		/* Enable cmdline after execute */
	const char *fdt_fname;		/* Filename of FDT binary */
	enum exit_type_id exit_type;	/* How we exited U-Boot */
	const char *parse_err;		/* Error to report from parsing */
	int argc;			/* Program arguments */
	char **argv;
	uint8_t *ram_buf;		/* Emulated RAM buffer */
	unsigned int ram_size;		/* Size of RAM buffer */
	const char *ram_buf_fname;	/* Filename to use for RAM buffer */
	bool write_ram_buf;		/* Write RAM buffer on exit */

	/* Pointer to information for each SPI bus/cs */
	struct sandbox_spi_info spi[CONFIG_SANDBOX_SPI_MAX_BUS]
					[CONFIG_SANDBOX_SPI_MAX_CS];
};

/**
 * Record the exit type to be reported by the test program.
 *
 * @param exit_type	Exit type to record
 */
void state_record_exit(enum exit_type_id exit_type);

/**
 * Gets a pointer to the current state.
 *
 * @return pointer to state
 */
struct sandbox_state *state_get_current(void);

/**
 * Initialize the test system state
 */
int state_init(void);

/**
 * Uninitialize the test system state, writing out state if configured to
 * do so.
 *
 * @return 0 if OK, -ve on error
 */
int state_uninit(void);

#endif
