/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2023-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Max Merchel
 */

enum tqma6ul_som_type {
	/* unknown */
	tqma6ul_som_type_unknown,
	/* connector module */
	tqma6ul_som_type_ca,
	/* LGA Variant */
	tqma6ul_som_type_lga,
};

/**
 * Checks configuration for TQMa6UL SoM module variant
 */
enum tqma6ul_som_type check_tqma6ul_variant(void);

/**
 * Adjusts device tree name based on CPU variant.
 */
enum tqma6ul_som_type set_tqma6ul_dt_name(char *dt, size_t dtsize, const char *mb);
