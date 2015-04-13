/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 *
 * Copyright 2014 Red Hat, Inc.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _CONFIG_CMD_DISTRO_BOOTCMD_H
#define _CONFIG_CMD_DISTRO_BOOTCMD_H

/*
 * A note on error handling: It is possible for BOOT_TARGET_DEVICES to
 * reference a device that is not enabled in the U-Boot configuration, e.g.
 * it may include MMC in the list without CONFIG_CMD_MMC being enabled. Given
 * that BOOT_TARGET_DEVICES is a macro that's expanded by the C pre-processor
 * at compile time, it's not  possible to detect and report such problems via
 * a simple #ifdef/#error combination. Still, the code needs to report errors.
 * The best way I've found to do this is to make BOOT_TARGET_DEVICES expand to
 * reference a non-existent symbol, and have the name of that symbol encode
 * the error message. Consequently, this file contains references to e.g.
 * BOOT_TARGET_DEVICES_references_MMC_without_CONFIG_CMD_MMC. Given the
 * prevalence of capitals here, this looks like a pre-processor macro and
 * hence seems like it should be all capitals, but it's really an error
 * message that includes some other pre-processor symbols in the text.
 */

/* We need the part command */
#define CONFIG_PARTITION_UUIDS
#define CONFIG_CMD_PART

#define BOOTENV_SHARED_BLKDEV_BODY(devtypel) \
		"if " #devtypel " dev ${devnum}; then " \
			"setenv devtype " #devtypel "; " \
			"run scan_dev_for_boot_part; " \
		"fi\0"

#define BOOTENV_SHARED_BLKDEV(devtypel) \
	#devtypel "_boot=" \
	BOOTENV_SHARED_BLKDEV_BODY(devtypel)

#define BOOTENV_DEV_BLKDEV(devtypeu, devtypel, instance) \
	"bootcmd_" #devtypel #instance "=" \
		"setenv devnum " #instance "; " \
		"run " #devtypel "_boot\0"

#define BOOTENV_DEV_NAME_BLKDEV(devtypeu, devtypel, instance) \
	#devtypel #instance " "

#ifdef CONFIG_SANDBOX
#define BOOTENV_SHARED_HOST	BOOTENV_SHARED_BLKDEV(host)
#define BOOTENV_DEV_HOST	BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_HOST	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_SHARED_HOST
#define BOOTENV_DEV_HOST \
	BOOT_TARGET_DEVICES_references_HOST_without_CONFIG_SANDBOX
#define BOOTENV_DEV_NAME_HOST \
	BOOT_TARGET_DEVICES_references_HOST_without_CONFIG_SANDBOX
#endif

#ifdef CONFIG_CMD_MMC
#define BOOTENV_SHARED_MMC	BOOTENV_SHARED_BLKDEV(mmc)
#define BOOTENV_DEV_MMC		BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_MMC	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_SHARED_MMC
#define BOOTENV_DEV_MMC \
	BOOT_TARGET_DEVICES_references_MMC_without_CONFIG_CMD_MMC
#define BOOTENV_DEV_NAME_MMC \
	BOOT_TARGET_DEVICES_references_MMC_without_CONFIG_CMD_MMC
#endif

#ifdef CONFIG_CMD_SATA
#define BOOTENV_SHARED_SATA	BOOTENV_SHARED_BLKDEV(sata)
#define BOOTENV_DEV_SATA	BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_SATA	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_SHARED_SATA
#define BOOTENV_DEV_SATA \
	BOOT_TARGET_DEVICES_references_SATA_without_CONFIG_CMD_SATA
#define BOOTENV_DEV_NAME_SATA \
	BOOT_TARGET_DEVICES_references_SATA_without_CONFIG_CMD_SATA
#endif

#ifdef CONFIG_CMD_SCSI
#define BOOTENV_RUN_SCSI_INIT "run scsi_init; "
#define BOOTENV_SET_SCSI_NEED_INIT "setenv scsi_need_init; "
#define BOOTENV_SHARED_SCSI \
	"scsi_init=" \
		"if ${scsi_need_init}; then " \
			"setenv scsi_need_init false; " \
			"scsi scan; " \
		"fi\0" \
	\
	"scsi_boot=" \
		BOOTENV_RUN_SCSI_INIT \
		BOOTENV_SHARED_BLKDEV_BODY(scsi)
#define BOOTENV_DEV_SCSI	BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_SCSI	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_RUN_SCSI_INIT
#define BOOTENV_SET_SCSI_NEED_INIT
#define BOOTENV_SHARED_SCSI
#define BOOTENV_DEV_SCSI \
	BOOT_TARGET_DEVICES_references_SCSI_without_CONFIG_CMD_SCSI
#define BOOTENV_DEV_NAME_SCSI \
	BOOT_TARGET_DEVICES_references_SCSI_without_CONFIG_CMD_SCSI
#endif

#ifdef CONFIG_CMD_IDE
#define BOOTENV_SHARED_IDE	BOOTENV_SHARED_BLKDEV(ide)
#define BOOTENV_DEV_IDE		BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_IDE	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_SHARED_IDE
#define BOOTENV_DEV_IDE \
	BOOT_TARGET_DEVICES_references_IDE_without_CONFIG_CMD_IDE
#define BOOTENV_DEV_NAME_IDE \
	BOOT_TARGET_DEVICES_references_IDE_without_CONFIG_CMD_IDE
#endif

#ifdef CONFIG_CMD_USB
#define BOOTENV_RUN_USB_INIT "usb start; "
#define BOOTENV_SHARED_USB \
	"usb_boot=" \
		BOOTENV_RUN_USB_INIT \
		BOOTENV_SHARED_BLKDEV_BODY(usb)
#define BOOTENV_DEV_USB		BOOTENV_DEV_BLKDEV
#define BOOTENV_DEV_NAME_USB	BOOTENV_DEV_NAME_BLKDEV
#else
#define BOOTENV_RUN_USB_INIT
#define BOOTENV_SHARED_USB
#define BOOTENV_DEV_USB \
	BOOT_TARGET_DEVICES_references_USB_without_CONFIG_CMD_USB
#define BOOTENV_DEV_NAME_USB \
	BOOT_TARGET_DEVICES_references_USB_without_CONFIG_CMD_USB
#endif

#if defined(CONFIG_CMD_DHCP)
#define BOOTENV_DEV_DHCP(devtypeu, devtypel, instance) \
	"bootcmd_dhcp=" \
		BOOTENV_RUN_USB_INIT \
		"if dhcp ${scriptaddr} ${boot_script_dhcp}; then " \
			"source ${scriptaddr}; " \
		"fi\0"
#define BOOTENV_DEV_NAME_DHCP(devtypeu, devtypel, instance) \
	"dhcp "
#else
#define BOOTENV_DEV_DHCP \
	BOOT_TARGET_DEVICES_references_DHCP_without_CONFIG_CMD_DHCP
#define BOOTENV_DEV_NAME_DHCP \
	BOOT_TARGET_DEVICES_references_DHCP_without_CONFIG_CMD_DHCP
#endif

#if defined(CONFIG_CMD_DHCP) && defined(CONFIG_CMD_PXE)
#define BOOTENV_DEV_PXE(devtypeu, devtypel, instance) \
	"bootcmd_pxe=" \
		BOOTENV_RUN_USB_INIT \
		"dhcp; " \
		"if pxe get; then " \
			"pxe boot; " \
		"fi\0"
#define BOOTENV_DEV_NAME_PXE(devtypeu, devtypel, instance) \
	"pxe "
#else
#define BOOTENV_DEV_PXE \
	BOOT_TARGET_DEVICES_references_PXE_without_CONFIG_CMD_DHCP_or_PXE
#define BOOTENV_DEV_NAME_PXE \
	BOOT_TARGET_DEVICES_references_PXE_without_CONFIG_CMD_DHCP_or_PXE
#endif

#define BOOTENV_DEV_NAME(devtypeu, devtypel, instance) \
	BOOTENV_DEV_NAME_##devtypeu(devtypeu, devtypel, instance)
#define BOOTENV_BOOT_TARGETS \
	"boot_targets=" BOOT_TARGET_DEVICES(BOOTENV_DEV_NAME) "\0"

#define BOOTENV_DEV(devtypeu, devtypel, instance) \
	BOOTENV_DEV_##devtypeu(devtypeu, devtypel, instance)
#define BOOTENV \
	BOOTENV_SHARED_HOST \
	BOOTENV_SHARED_MMC \
	BOOTENV_SHARED_USB \
	BOOTENV_SHARED_SATA \
	BOOTENV_SHARED_SCSI \
	BOOTENV_SHARED_IDE \
	"boot_prefixes=/ /boot/\0" \
	"boot_scripts=boot.scr.uimg boot.scr\0" \
	"boot_script_dhcp=boot.scr.uimg\0" \
	BOOTENV_BOOT_TARGETS \
	\
	"boot_extlinux="                                                  \
		"sysboot ${devtype} ${devnum}:${bootpart} any "           \
			"${scriptaddr} ${prefix}extlinux/extlinux.conf\0" \
	\
	"scan_dev_for_extlinux="                                          \
		"if test -e ${devtype} ${devnum}:${bootpart} "            \
				"${prefix}extlinux/extlinux.conf; then "  \
			"echo Found ${prefix}extlinux/extlinux.conf; "    \
			"run boot_extlinux; "                             \
			"echo SCRIPT FAILED: continuing...; "             \
		"fi\0"                                                    \
	\
	"boot_a_script="                                                  \
		"load ${devtype} ${devnum}:${bootpart} "                  \
			"${scriptaddr} ${prefix}${script}; "              \
		"source ${scriptaddr}\0"                                  \
	\
	"scan_dev_for_scripts="                                           \
		"for script in ${boot_scripts}; do "                      \
			"if test -e ${devtype} ${devnum}:${bootpart} "    \
					"${prefix}${script}; then "       \
				"echo Found U-Boot script "               \
					"${prefix}${script}; "            \
				"run boot_a_script; "                     \
				"echo SCRIPT FAILED: continuing...; "     \
			"fi; "                                            \
		"done\0"                                                  \
	\
	"scan_dev_for_boot="                                              \
		"echo Scanning ${devtype} ${devnum}:${bootpart}...; "     \
		"for prefix in ${boot_prefixes}; do "                     \
			"run scan_dev_for_extlinux; "                     \
			"run scan_dev_for_scripts; "                      \
		"done\0"                                                  \
	\
	"scan_dev_for_boot_part="                                         \
		"part list ${devtype} ${devnum} -bootable devplist; "     \
		"env exists devplist || setenv devplist 1; "              \
		"for bootpart in ${devplist}; do "                        \
			"if fstype ${devtype} ${devnum}:${bootpart} "     \
					"bootfstype; then "               \
				"run scan_dev_for_boot; "                 \
			"fi; "                                            \
		"done\0"                                                  \
	\
	BOOT_TARGET_DEVICES(BOOTENV_DEV)                                  \
	\
	"distro_bootcmd=" BOOTENV_SET_SCSI_NEED_INIT                      \
		"for target in ${boot_targets}; do "                      \
			"run bootcmd_${target}; "                         \
		"done\0"

#ifndef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND "run distro_bootcmd"
#endif

#endif  /* _CONFIG_CMD_DISTRO_BOOTCMD_H */
