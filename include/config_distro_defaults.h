/*
 * Copyright 2013-2014 Red Hat, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _CONFIG_CMD_DISTRO_DEFAULTS_H
#define _CONFIG_CMD_DISTRO_DEFAULTS_H

/*
 * List of all commands and options that when defined enables support for
 * features required by distros to support boards in a standardised and
 * consistent manner.
 */

#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_PXE
#define CONFIG_BOOTP_SUBNETMASK

#if defined(__arm__) || defined(__aarch64__)
#define CONFIG_BOOTP_PXE_CLIENTARCH     0x100
#if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__)
#if !defined(CONFIG_BOOTP_VCI_STRING)
#define CONFIG_BOOTP_VCI_STRING         "U-Boot.armv7"
#endif
#elif defined(__aarch64__)
#if !defined(CONFIG_BOOTP_VCI_STRING)
#define CONFIG_BOOTP_VCI_STRING         "U-Boot.armv8"
#endif
#else
#if !defined(CONFIG_BOOTP_VCI_STRING)
#define CONFIG_BOOTP_VCI_STRING         "U-Boot.arm"
#endif
#endif
#elif defined(__i386__)
#define CONFIG_BOOTP_PXE_CLIENTARCH     0x0
#elif defined(__x86_64__)
#define CONFIG_BOOTP_PXE_CLIENTARCH     0x9
#endif

#ifdef CONFIG_ARM64
#define CONFIG_CMD_BOOTI
#endif
#define CONFIG_CMD_PXE

#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_BOOTDELAY     2
#define CONFIG_SYS_LONGHELP
#define CONFIG_MENU
#define CONFIG_DOS_PARTITION
#define CONFIG_EFI_PARTITION
#define CONFIG_ISO_PARTITION
#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_ENV_VARS_UBOOT_CONFIG

#endif	/* _CONFIG_CMD_DISTRO_DEFAULTS_H */
