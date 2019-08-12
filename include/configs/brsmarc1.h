/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * brsmarc1.h
 *
 * specific parts for B&R BRSMARC1 Motherboard
 *
 * Copyright (C) 2017 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * B&R Industrial Automation GmbH - http://www.br-automation.com
 *
 */

#ifndef __CONFIG_BRSMARC1_H__
#define __CONFIG_BRSMARC1_H__

#include <configs/bur_cfg_common.h>
#include <configs/bur_am335x_common.h>
/* ------------------------------------------------------------------------- */
#define CONFIG_BOARD_TYPES

/* memory */
#define CONFIG_SYS_MALLOC_LEN		(5 * 1024 * 1024)
#define CONFIG_SYS_BOOTM_LEN		(32 * 1024 * 1024)

/* Clock Defines */
#define V_OSCK				26000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_MACH_TYPE		3589

#ifndef CONFIG_SPL_BUILD

/* Default environment */
#define CONFIG_EXTRA_ENV_SETTINGS	\
BUR_COMMON_ENV \
"autoload=0\0" \
"scradr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
"cfgscr=mw ${dtbaddr} 0;" \
" sf probe && sf read ${scradr} 0xC0000 0x10000 && source ${scradr};" \
" fdt addr ${dtbaddr} || cp ${fdtcontroladdr} ${dtbaddr} 4000\0" \
"dtbaddr=0x84000000\0" \
"loadaddr=0x82000000\0" \
"b_break=0\0" \
"b_tgts_std=mmc0 mmc1 def net usb0\0" \
"b_tgts_rcy=def net usb0\0" \
"b_tgts_pme=net usb0 mmc0 mmc1\0" \
"b_deftgts=if test ${b_mode} = 12; then setenv b_tgts ${b_tgts_pme};" \
" elif test ${b_mode} = 0; then setenv b_tgts ${b_tgts_rcy};" \
" else setenv b_tgts ${b_tgts_std}; fi\0" \
"b_mmc0=load mmc 1 ${scradr} bootscr.img && source ${scradr}\0" \
"b_mmc1=load mmc 1 ${loadaddr} arimg.ugz && run startsys\0" \
"b_def=sf read ${loadaddr} 100000 700000; run startsys\0" \
"b_net=tftp ${scradr} netscript.img && source ${scradr}\0" \
"b_usb0=usb start && load usb 0 ${scradr} bootscr.img && source ${scradr}\0" \
"b_default=run b_deftgts; for target in ${b_tgts};"\
" do run b_${target}; if test ${b_break} = 1; then; exit; fi; done\0" \
"vxargs=setenv bootargs cpsw(0,0)host:vxWorks h=${serverip}" \
" e=${ipaddr}:${netmask} g=${gatewayip} u=vxWorksFTP pw=vxWorks\0" \
"vxfdt=fdt addr ${dtbaddr}; fdt resize 0x8000;" \
" fdt boardsetup\0" \
"startsys=run vxargs && mw 0x80001100 0 && run vxfdt &&" \
" bootm ${loadaddr} - ${dtbaddr}\0"
#endif /* !CONFIG_SPL_BUILD*/

/* undefine command which we not need here */
#undef CONFIG_BOOTM_NETBSD
#undef CONFIG_BOOTM_PLAN9
#undef CONFIG_BOOTM_RTEMS

/* Support both device trees and ATAGs. */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/* SPI Flash */
#define CONFIG_SYS_SPI_U_BOOT_OFFS		0x40000

/* Environment */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_OFFSET_REDUND		(CONFIG_ENV_OFFSET + \
						 CONFIG_ENV_SECT_SIZE)

#define CONFIG_CONS_INDEX			1
#endif	/* __CONFIG_BRSMARC1_H__ */
