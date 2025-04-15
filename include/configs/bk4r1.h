/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright 2016 3ADEV <http://3adev.com>
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * Configuration settings for BK4R1.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Define the BK4r1-specific env commands */
#define BK4_EXTRA_ENV_SETTINGS \
	"bootlimit=3\0" \
	"eraseuserdata=false\0" \
	"set_gpio103=mw 0x400ff0c4 0x0080; mw 0x4004819C 0x000011bf\0" \
	"set_gpio102=mw 0x400ff0c4 0x40; mw 0x40048198 0x000011bf\0" \
	"set_gpio96=mw 0x40048180 0x282; mw 0x400ff0c4 0x1\0"\
	"set_gpio122=mw 0x400481e8 0x0282; mw 0x400ff0c4 0x04000000\0"\
	"set_gpio6=mw 0x40048018 0x282; mw 0x400ff008 0x40\0"\
	"manage_userdata=" MANAGE_USERDATA "\0"\
	"ncenable=true\0"\
	"ncserverip=192.168.0.77\0"\
	"if_netconsole=ping $ncserverip\0"\
	"start_netconsole=setenv ncip $serverip; setenv bootdelay 10;" \
	     "setenv stdin nc; setenv stdout nc; setenv stderr nc; version;\0" \
	"preboot=" BK4_NET_INIT \
		"if ${ncenable}; then run if_netconsole start_netconsole; fi\0"

/* Enable PREBOOT variable */

/* BK4r1 net init sets GPIO122/PTE17 to enable Ethernet */
#define BK4_NET_INIT "run set_gpio122;"

/* Check if userdata volume shall be erased */
#define MANAGE_USERDATA "if ${eraseuserdata}; " \
						"then ubi part system; " \
						"ubi remove userdata; " \
						"ubi create userdata; " \
						"ubi detach; " \
						"setenv eraseuserdata false; " \
						"saveenv; " \
						"fi; "

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

/* NAND support */

#define IMX_FEC1_BASE			ENET1_BASE_ADDR

/* boot command, including the target-defined one if any */

/* Extra env settings (including the target-defined ones if any) */
#define CFG_EXTRA_ENV_SETTINGS \
	BK4_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"blimg_file=u-boot.vyb\0" \
	"blimg_addr=0x81000000\0" \
	"dtbkernel_file=fitImage\0" \
	"dtbkernel_addr=0x82000000\0" \
	"ram_file=uRamdisk\0" \
	"ram_addr=0x83000000\0" \
	"filesys=rootfs.ubifs\0" \
	"sys_addr=0x81000000\0" \
	"nfs_root=/path/to/nfs/root\0" \
	"tftptimeout=1000\0" \
	"tftptimeoutcountmax=1000000\0" \
	"ipaddr=192.168.0.60\0" \
	"serverip=192.168.0.1\0" \
	"bootargs_base=setenv bootargs rw " \
	"console=ttyLP1,115200n8\0" \
	"bootargs_sd=setenv bootargs ${bootargs} " \
		"root=/dev/mmcblk0p2 rootwait\0" \
	"bootargs_nand=setenv bootargs ${bootargs} " \
		"ubi.mtd=5 rootfstype=" \
				"ubifs root=ubi0:rootfs${active_workset}\0" \
	"bootargs_ram=setenv bootargs ${bootargs} " \
		"root=/dev/ram rw initrd=${ram_addr}\0" \
	"bootargs_mtd=setenv bootargs ${bootargs} ${mtdparts}\0" \
	"bootcmd_sd=run bootargs_base bootargs_sd bootargs_mtd; " \
		"fatload mmc 0:2 ${dtbkernel_addr} ${dtbkernel_file}; " \
		"bootm ${dtbkernel_addr}\0" \
	"bootcmd_nand=sf probe;run bootargs_base bootargs_nand bootargs_mtd; " \
		"ubi part dtbkernel; " \
		"ubi readvol ${dtbkernel_addr} dtbkernel${active_workset}; " \
		"led 0 on; " \
		"bootm ${dtbkernel_addr}\0" \
	"bootcmd_ram=run bootargs_base bootargs_ram bootargs_mtd; " \
		"nand read ${fdt_addr} dtb; " \
		"nand read ${kernel_addr} kernel; " \
		"nand read ${ram_addr} root; " \
		"bootz ${kernel_addr} ${ram_addr} ${fdt_addr}\0" \
	"update_bootloader_from_sd=if fatload mmc 0:2 ${blimg_addr} " \
		"${blimg_file}; " \
		"then sf probe; " \
		"mtdparts default; " \
		"nand erase.part bootloader; " \
		"nand write ${blimg_addr} bootloader ${filesize}; fi\0" \
	"update_bootloader_from_tftp=if tftp ${blimg_addr} "\
		"${tftpdir}${blimg_file}; "\
		"then sf probe; " \
		"mtdparts default; " \
		"nand erase.part bootloader; " \
		"nand write ${blimg_addr} bootloader ${filesize}; fi\0" \
	"update_dtbkernel_from_sd=if fatload mmc 0:2 ${dtbkernel_addr} " \
		"${dtbkernel_file}; " \
		"then sf probe; " \
		"ubi part dtbkernel; " \
		"ubi write ${dtbkernel_addr} dtbkernel${active_workset} " \
		"${filesize}; " \
		"ubi detach; fi\0" \
	"update_dtbkernel_from_tftp=if tftp ${dtbkernel_addr} " \
		"${tftpdir}${dtbkernel_file}; " \
		"then sf probe; " \
		"ubi part dtbkernel; " \
		"ubi write ${dtbkernel_addr} dtbkernel${active_workset} " \
		"${filesize}; " \
		"ubi detach; fi\0" \
	"update_ramdisk_from_sd=if fatload mmc 0:2 ${ram_addr} " \
		"${ram_file}; " \
		"then sf probe; " \
		"mtdparts default; " \
		"nand erase.part initrd; " \
		"nand write ${ram_addr} initrd ${filesize}; fi\0" \
	"update_ramdisk_from_tftp=if tftp ${ram_addr} ${tftpdir}${ram_file}; " \
		"then sf probe; " \
		"nand erase.part initrd; " \
		"nand write ${ram_addr} initrd ${filesize}; fi\0" \
	"update_rootfs_from_sd=if fatload mmc 0:2 ${sys_addr} " \
		"${filesys}; " \
		"then sf probe; " \
		"ubi part system; " \
		"ubi write ${sys_addr} rootfs${active_workset} ${filesize}; " \
		"ubi detach; fi\0" \
	"update_rootfs_from_tftp=if tftp ${sys_addr} ${tftpdir}${filesys}; " \
		"then sf probe; " \
		"ubi part system; " \
		"ubi write ${sys_addr} rootfs${active_workset} ${filesize}; " \
		"ubi detach; fi\0" \
	"setup_dtbkernel=nand erase.part dtbkernel; " \
		"ubi part dtbkernel; " \
		"ubi create dtbkernel1 972000 s; " \
		"ubi create dtbkernel2 972000 s; " \
		"ubi detach\0" \
	"setup_system=nand erase.part system; " \
		"ubi part system; " \
		"ubi create rootfs1 15E15000 d; " \
		"ubi create rootfs2 15E15000 d; " \
		"ubi create userdata; " \
		"ubi detach\0" \
	"setup_nor1=" BK4_NET_INIT \
		"if tftp ${sys_addr} ${tftpdir}ubinor1.img; " \
		"then sf probe 0:0; " \
		"sf erase 0 01000000; " \
		"mtdparts default; " \
		"ubi part nor; " \
		"ubi create nor1fs; " \
		"ubi write ${sys_addr} nor1fs ${filesize}; " \
		"ubi detach; fi\0" \
	"setup_nor2=" BK4_NET_INIT \
		"if tftp ${sys_addr} ${tftpdir}ubinor2.img; " \
		"then sf probe 0:1; " \
		"sf erase 0 01000000; " \
		"mtdparts default; " \
		"ubi part nor; " \
		"ubi create nor2fs; " \
		"ubi write ${sys_addr} nor2fs ${filesize}; " \
		"ubi detach; fi\0" \
	"prepare_install_bk4r1_envs=" \
		"echo 'Preparing envs for SD card recovery!';" \
		"setenv ipaddr 192.168.0.99;" \
		"setenv serverip 192.168.0.50;" \
		"\0" \
	"install_bk4r1rs="\
		"led 0 on; " \
		"nand erase.chip; mtdparts default; "\
		"led 1 on; "\
		"run setup_dtbkernel; " \
		"run setup_system; " \
		"led 2 on;" \
		"run update_bootloader_from_sd; "\
		"run update_dtbkernel_from_sd; "\
		"run update_rootfs_from_sd; "\
		"setenv bootcmd 'run bootcmd_nand'; "\
		"saveenv; " \
		"led 3 on; " \
		"echo Finished - Please Power off, REMOVE SDCARD and set boot" \
			"source to NAND\0" \
	"active_workset=1\0"

/* Physical memory map */
#define PHYS_SDRAM			(0x80000000)
#define PHYS_SDRAM_SIZE		(SZ_512M)

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#endif /* __CONFIG_H */
