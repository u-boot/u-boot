/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __SL28_H
#define __SL28_H

#include <asm/arch/stream_id_lsch3.h>
#include <asm/arch/config.h>
#include <asm/arch/soc.h>

/* we don't use hwconfig but this has to be defined.. */
#define HWCONFIG_BUFFER_SIZE 256

/* we don't have secure memory unless we have a BL31 */
#ifndef CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT
#undef CONFIG_SYS_MEM_RESERVE_SECURE
#endif

/* DDR */
#define CONFIG_MEM_INIT_VALUE		0xdeadbeef

#define CONFIG_VERY_BIG_RAM
#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000
#define CONFIG_SYS_FSL_DDR_SDRAM_BASE_PHY	0
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE
#define CONFIG_SYS_DDR_BLOCK2_BASE	0x2080000000ULL
#define CONFIG_SYS_FSL_DDR_MAIN_NUM_CTRLS	1

/* early stack pointer */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_FSL_OCRAM_BASE + 0xeff0)

/* SMP */
#define CPU_RELEASE_ADDR		secondary_boot_addr

/* generic timer */

/* early heap for SPL DM */
#define CONFIG_MALLOC_F_ADDR		CONFIG_SYS_FSL_OCRAM_BASE

/* serial port */
#define CONFIG_SYS_NS16550_CLK          (get_bus_freq(0) / 2)

#define COUNTER_FREQUENCY_REAL		(get_board_sys_clk() / 4)

/* SPL */
#define CONFIG_SPL_BSS_START_ADDR	0x80100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x00100000
#define CONFIG_SPL_MAX_SIZE		0x20000
#define CONFIG_SPL_STACK		(CONFIG_SYS_FSL_OCRAM_BASE + 0x9ff0)

#define CONFIG_SYS_SPL_MALLOC_SIZE	0x00100000
#define CONFIG_SYS_SPL_MALLOC_START	0x80200000
#define CONFIG_SYS_MONITOR_LEN		(1024 * 1024)

/* GUID for capsule updatable firmware image */
#define KONTRON_SL28_FIT_IMAGE_GUID \
	EFI_GUID(0x86ebd44f, 0xfeb8, 0x466f, 0x8b, 0xb8, \
		 0x89, 0x06, 0x18, 0x45, 0x6d, 0x8b)

/* environment */
/* see include/configs/ti_armv7_common.h */
#define ENV_MEM_LAYOUT_SETTINGS \
	"loadaddr=0x82000000\0" \
	"kernel_addr_r=0x82000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"bootm_size=0x10000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"scriptaddr=0x80000000\0" \
	"ramdisk_addr_r=0x88080000\0"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(NVME, nvme, 0) \
	func(USB, usb, 0) \
	func(SCSI, scsi, 0) \
	func(DHCP, dhcp, 0) \
	func(PXE, pxe, 0)
#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	"env_addr=0x203e0004\0" \
	"envload=env import -d -b ${env_addr}\0" \
	"install_rcw=source 20200000\0" \
	"fdtfile=freescale/fsl-ls1028a-kontron-sl28.dtb\0" \
	"dfu_alt_info=sf 0:0=u-boot-bin raw 0x210000 0x1d0000;" \
			    "u-boot-env raw 0x3e0000 0x20000\0" \
	ENV_MEM_LAYOUT_SETTINGS \
	BOOTENV

#endif /* __SL28_H */
