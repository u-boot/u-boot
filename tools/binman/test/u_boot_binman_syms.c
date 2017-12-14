/*
 * Copyright (c) 2017 Google, Inc
 *
 * SPDX-License-Identifier:     GPL-2.0+
 *
 * Simple program to create some binman symbols. This is used by binman tests.
 */

#define CONFIG_BINMAN
#include <binman_sym.h>

binman_sym_declare(unsigned long, u_boot_spl, pos);
binman_sym_declare(unsigned long long, u_boot_spl2, pos);
binman_sym_declare(unsigned long, u_boot_any, pos);
