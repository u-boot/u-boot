/*
 * (C) Copyright 2008
 * Niklaus Giger, niklaus.giger@member.fsf.org
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _VXWORKS_H_
#define _VXWORKS_H_

int do_bootvx(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);
void boot_prep_vxworks(bootm_headers_t *images);
void boot_jump_vxworks(bootm_headers_t *images);
void do_bootvx_fdt(bootm_headers_t *images);

#endif
