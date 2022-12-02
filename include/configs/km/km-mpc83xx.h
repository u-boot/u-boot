/*
 * Internal Definitions
 */
#include <linux/stringify.h>
#define BOOTFLASH_START	0xF0000000

/*
 * DDR Setup
 */
#define CFG_SYS_SDRAM_BASE		0x00000000 /* DDR is system memory */

#define CFG_SYS_DDR_SDRAM_CLK_CNTL	(DDR_SDRAM_CLK_CNTL_SS_EN | \
					DDR_SDRAM_CLK_CNTL_CLK_ADJUST_05)

#define CFG_83XX_DDR_USES_CS0

/*
 * Manually set up DDR parameters
 */
#define CFG_SYS_SDRAM_SIZE		0x80000000 /* 2048 MiB */

/*
 * The reserved memory
 */
#define CFG_SYS_FLASH_BASE		0xF0000000

/* Reserve 768 kB for Mon */

/*
 * Initial RAM Base Address Setup
 */
#define CFG_SYS_INIT_RAM_ADDR	0xE6000000 /* Initial RAM address */
#define CFG_SYS_INIT_RAM_SIZE	0x1000 /* End of used area in RAM */
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
#define CFG_SYS_FLASH_SIZE		256 /* max FLASH size is 256M */

#define CFG_SYS_FLASH_BANKS_LIST { CFG_SYS_FLASH_BASE }

#if defined(CONFIG_CMD_NAND)
#define CFG_SYS_NAND_BASE		CFG_SYS_KMBEC_FPGA_BASE
#endif

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_SYS_BOOTMAPSZ		(8 << 20)

/*
 * QE UEC ethernet configuration
 */
#define CONFIG_UEC_ETH
