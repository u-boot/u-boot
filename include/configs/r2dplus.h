#ifndef __CONFIG_H
#define __CONFIG_H

#define __LITTLE_ENDIAN__	1

/* SCIF */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x8C000000
#define CONFIG_SYS_SDRAM_SIZE		0x04000000

/* Address of u-boot image in Flash */
#define CONFIG_SYS_MONITOR_LEN		(256 * 1024)
#define CONFIG_SYS_BOOTMAPSZ		(8 * 1024 * 1024)

/*
 * NOR Flash ( Spantion S29GL256P )
 */
#define CONFIG_SYS_FLASH_BASE		(0xA0000000)
#define CONFIG_SYS_FLASH_BANKS_LIST	{ CONFIG_SYS_FLASH_BASE }

/*
 * SuperH Clock setting
 */
#define	CONFIG_SYS_PLL_SETTLING_TIME	100/* in us */

#endif /* __CONFIG_H */
