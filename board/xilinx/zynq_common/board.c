/*
 * Just to satisfy init routines..
 */

#include <common.h>
#include <asm/arch/mmc.h>
#include <asm/arch/nand.h>
#include <netdev.h>
#include <zynqpl.h>

#define BOOT_MODE_REG     (XPSS_SYS_CTRL_BASEADDR + 0x25C)
#define BOOT_MODES_MASK    0x0000000F
#define QSPI_MODE         (0x00000001)            /**< QSPI */
#define NOR_FLASH_MODE    (0x00000002)            /**< NOR  */
#define NAND_FLASH_MODE   (0x00000004)            /**< NAND */
#define SD_MODE           (0x00000005)            /**< Secure Digital card */
#define JTAG_MODE	  (0x00000000)		  /**< JTAG */

DECLARE_GLOBAL_DATA_PTR;

/* Common IO for xgmac and xnand */
/* Data Memory Barrier */
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#define SYNCHRONIZE_IO dmb()

void XIo_Out32(u32 OutAddress, u32 Value)
{
	*(volatile u32 *) OutAddress = Value;
	SYNCHRONIZE_IO;
}

u32 XIo_In32(u32 InAddress)
{
	volatile u32 temp = *(volatile u32 *)InAddress;
	SYNCHRONIZE_IO;
	return temp;
}

#define Xil_Out32
#define Xil_In32

#ifdef CONFIG_FPGA
Xilinx_desc fpga = XILINX_XC7Z020_DESC(0);
#endif

int board_init(void)
{
	/* temporary hack to clear pending irqs before Linux as it 
	   will hang Linux */

	Xil_Out32(0xe0001014, 0x26d);

	/* temporary hack to take USB out of reset til the is fixed
	   in Linux */

	Xil_Out32(0xe000a204, 0x80);
	Xil_Out32(0xe000a208, 0x80);
	Xil_Out32(0xe000a040, 0x80);
	Xil_Out32(0xe000a040, 0x00);
	Xil_Out32(0xe000a040, 0x80);

	icache_enable();

#ifdef CONFIG_FPGA
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif

	return 0;
}

int board_late_init (void)
{
	u32 boot_mode;

	boot_mode = (Xil_In32(BOOT_MODE_REG) & BOOT_MODES_MASK);
	switch(boot_mode) {
	case QSPI_MODE:
		setenv("modeboot", "run qspiboot");
		break;
	case NAND_FLASH_MODE:
		setenv("modeboot", "run nandboot");
		break;
	case NOR_FLASH_MODE:
		setenv("modeboot", "run norboot");
		break;
	case SD_MODE:
		setenv("modeboot", "run sdboot");
		break;
	case JTAG_MODE:
		setenv("modeboot", "run jtagboot");
		break;
	default:
		setenv("modeboot", "");
		break;
	}

	return 0;
}

#ifdef CONFIG_CMD_NET
int board_eth_init(bd_t *bis)
{
	return zynq_gem_initialize(bis);
}
#endif

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	return zynq_mmc_init(bd);
}
#endif

#ifdef CONFIG_CMD_NAND
int board_nand_init(struct nand_chip *nand_chip)
{
	return zynq_nand_init(nand_chip);
}
#endif

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;

	return 0;
}

/*
 * OK, and resets too.
 */
void reset_cpu(ulong addr)
{
	u32 *slcr_p;

	slcr_p = (u32*)XPSS_SYS_CTRL_BASEADDR;

	/* unlock SLCR */
	*(slcr_p + 2) = 0xDF0D;
	/* Tickle soft reset bit */
	*(slcr_p + 128) = 1;

	while(1) {;}
}
