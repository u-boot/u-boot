/*
 * Hacked together,
 * hopefully functional.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARMV7 /* CPU */
#define CONFIG_ZYNQ /* SoC */

#include "../board/petalogix/arm-auto/xparameters.h"

#define CONFIG_CPU_FREQ_HZ	XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ

#if defined(XILINX_PS7_GEM_BASEADDR)
# if (XILINX_PS7_GEM_BASEADDR == 0xE000B000)
#  define CONFIG_ZYNQ_GEM0
#  define CONFIG_ZYNQ_GEM_PHY_ADDR0	-1
# else
#  define CONFIG_ZYNQ_GEM1
#  define CONFIG_ZYNQ_GEM_PHY_ADDR1	-1
# endif
#endif

#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_CACHELINE_SIZE	32 /* Assuming bytes? */

/* Keep L2 Cache Disabled */
#define CONFIG_SYS_L2CACHE_OFF

#ifndef CONFIG_SYS_L2CACHE_OFF
# define CONFIG_SYS_L2_PL310
# define CONFIG_SYS_PL310_BASE  0xf8f02000
#endif

#define CONFIG_NR_DRAM_BANKS		1

/*-----------------------------------------------------------------------
 * Main Memory
 */

/* MALLOC */
#define CONFIG_SYS_MALLOC_LEN		0x400000

/* global pointer */
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* size of global data */
/* start of global data */
//#define	CONFIG_SYS_GBL_DATA_OFFSET	(CONFIG_SYS_SDRAM_SIZE - CONFIG_SYS_GBL_DATA_SIZE)


/* Memory test handling */
#define	CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define	CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_SDRAM_BASE + 0x1000)

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_SYS_INIT_RAM_ADDR	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_INIT_RAM_SIZE	CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
						CONFIG_SYS_INIT_RAM_SIZE - \
						GENERATED_GBL_DATA_SIZE)

/* Enable the PL to be downloaded */
#define CONFIG_FPGA
#define CONFIG_FPGA_XILINX
#define CONFIG_FPGA_ZYNQPL
#define CONFIG_CMD_FPGA

/* Get common PetaLinux board setup */
#include <configs/petalinux-auto-board.h>

/* Enable phylib */
# define CONFIG_MII
# define CONFIG_SYS_FAULT_ECHO_LINK_DOWN
# define CONFIG_PHYLIB
# define CONFIG_PHY_MARVELL

#endif /* __CONFIG_H */
