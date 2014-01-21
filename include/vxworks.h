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

/*
 * Use bootaddr to find the location in memory that VxWorks
 * will look for the bootline string. The default value for
 * PowerPC is LOCAL_MEM_LOCAL_ADRS + BOOT_LINE_OFFSET which
 * defaults to 0x4200
 */
#ifndef CONFIG_SYS_VXWORKS_BOOT_ADDR
#define CONFIG_SYS_VXWORKS_BOOT_ADDR 0x4200
#endif

#ifndef CONFIG_SYS_VXWORKS_BOOT_DEVICE
#if defined(CONFIG_4xx)
#define		CONFIG_SYS_VXWORKS_BOOT_DEVICE "emac(0,0)"
#else
#define		CONFIG_SYS_VXWORKS_BOOT_DEVICE "eth(0,0)"
#endif
#endif

#ifndef CONFIG_SYS_VXWORKS_SERVERNAME
#define CONFIG_SYS_VXWORKS_SERVERNAME	"srv"
#endif

#endif
