/*
 * Internal Definitions
 */
#include <linux/stringify.h>
#define BOOTFLASH_START	0xF0000000

/*
 * DDR Setup
 */
#define CONFIG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory */
#define CONFIG_SYS_SDRAM_BASE2	(CONFIG_SYS_SDRAM_BASE + 0x10000000) /* +256M */

#define CONFIG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
					DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)

#define CFG_83XX_DDR_USES_CS0

/*
 * Manually set up DDR parameters
 */
#define CONFIG_SYS_SDRAM_SIZE		0x80000000 /* 2048 MiB */

/*
 * The reserved memory
 */
#define CONFIG_SYS_FLASH_BASE		0xF0000000

/* Reserve 768 kB for Mon */
#define CONFIG_SYS_MONITOR_LEN		(768 * 1024)

/*
 * Initial RAM Base Address Setup
 */
#define CONFIG_SYS_INIT_RAM_LOCK
#define CONFIG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000 /* End of used area in RAM */
/*
 * Init Local Bus Memory Controller:
 *
 * Bank Bus     Machine PortSz  Size  Device
 * ---- ---     ------- ------  -----  ------
 *  0   Local   GPCM    16 bit  256MB FLASH
 *  1   Local   GPCM     8 bit  128MB GPIO/PIGGY
 *
 */

/*
 * FLASH on the Local Bus
 */
#define CONFIG_SYS_FLASH_SIZE		256 /* max FLASH size is 256M */

#define CONFIG_SYS_FLASH_BANKS_LIST { CONFIG_SYS_FLASH_BASE }

/* I2C */
#define CONFIG_SYS_NUM_I2C_BUSES	4
#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_I2C_BUSES	{{0, {I2C_NULL_HOP} }, \
		{0, {{I2C_MUX_PCA9547, 0x70, 2} } }, \
		{0, {{I2C_MUX_PCA9547, 0x70, 1} } }, \
		{1, {I2C_NULL_HOP} } }

#if defined(CONFIG_CMD_NAND)
#define CONFIG_NAND_KMETER1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		CONFIG_SYS_KMBEC_FPGA_BASE
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CONFIG_SYS_BOOTMAPSZ		(8 << 20)

/*
 * Environment
 */

/*
 * Environment Configuration
 */
#ifndef CONFIG_KM_DEF_ENV		/* if not set by keymile-common.h */
#define CONFIG_KM_DEF_ENV "km-common=empty\0"
#endif

#ifndef CONFIG_KM_DEF_ARCH
#define CONFIG_KM_DEF_ARCH	"arch=ppc_82xx\0"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_KM_DEF_ENV						 \
	CONFIG_KM_DEF_ARCH						 \
	"newenv="							 \
		"prot off " __stringify(CONFIG_ENV_ADDR) " +0x40000 && " \
		"era " __stringify(CONFIG_ENV_ADDR) " +0x40000\0"	 \
	"unlock=yes\0"							 \
	""

/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
