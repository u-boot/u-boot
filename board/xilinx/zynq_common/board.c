/*
 * Just to satisfy init routines..
 */

#include <common.h>
#include <asm/arch/mmc.h>
#include <asm/arch/xparameters.h>
#include <netdev.h>
#include <zynqpl.h>
#include "ps7_init_hw.h"

#define PARPORT_CRTL_BASEADDR                   XPSS_CRTL_PARPORT_BASEADDR
#define NOR_FLASH_BASEADDR                      XPSS_PARPORT0_BASEADDR

#define PARPORT_MC_DIRECT_CMD                   0x010
#define PARPORT_MC_SET_CYCLES                   0x014
#define PARPORT_MC_SET_OPMODE                   0x018

#define BOOT_MODE_REG     (XPSS_SYS_CTRL_BASEADDR + 0x25C)
#define BOOT_MODES_MASK    0x0000000F
#define QSPI_MODE         (0x00000001)            /**< QSPI */
#define NOR_FLASH_MODE    (0x00000002)            /**< NOR  */
#define NAND_FLASH_MODE   (0x00000004)            /**< NAND */
#define SD_MODE           (0x00000005)            /**< Secure Digital card */
#define JTAG_MODE	  (0x00000000)		  /**< JTAG */

DECLARE_GLOBAL_DATA_PTR;

#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")

static void Out32(u32 OutAddress, u32 Value)
{
    *(volatile u32 *) OutAddress = Value;
    dmb();
}

static u32 In32(u32 InAddress)
{
    volatile u32 temp = *(volatile u32 *)InAddress;
    dmb();
    return temp;
}

static inline void Out8(u32 OutAddress, u8 Value)
{
    *(volatile u8 *) OutAddress = Value;
}

static inline u8 In8(u32 InAddress)
{
    return *(u8 *) InAddress;
}

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

#ifndef CONFIG_SYS_NO_FLASH
/*
 * init_nor_flash init the parameters of pl353 for the M29EW Flash
 */
void init_nor_flash(void)
{
  /* Init variables */

   /* Write timing info to set_cycles registers */
  u32 set_cycles_reg = (0x0 << 20) | /* Set_t6 or we_time from sram_cycles */
                       (0x1 << 17) | /* Set_t5 or t_tr from sram_cycles */
                       (0x2 << 14) | /* Set_t4 or t_pc from sram_cycles */
                       (0x5 << 11) | /* Set_t3 or t_wp from sram_cycles */
                       (0x2 << 8) |  /* Set_t2 t_ceoe from sram_cycles */
                       (0x7 << 4) |  /* Set_t1 t_wc from sram_cycles */
                       (0x7);        /* Set_t0 t_rc from sram_cycles */

  Out32(PARPORT_CRTL_BASEADDR + PARPORT_MC_SET_CYCLES, set_cycles_reg);

  /* write operation mode to set_opmode registers */
  u32 set_opmode_reg = (0x1 << 13) | /* set_burst_align, see to 32 beats */
                       (0x1 << 12) | /* set_bls, set to default */
                       (0x0 << 11) | /* set_adv bit, set to default */
                       (0x0 << 10) | /* set_baa, I guess we don't use baa_n */
                       (0x0 << 7) |  /* set_wr_bl, write brust length, set to 0 */
                       (0x0 << 6) |  /* set_wr_sync, set to 0 */
                       (0x0 << 3) |  /* set_rd_bl, read brust lenght, set to 0 */
                       (0x0 << 2) |  /* set_rd_sync, set to 0 */
                       (0x0 );       /* set_mw, memory width, 16bits width*/
  Out32(PARPORT_CRTL_BASEADDR + PARPORT_MC_SET_OPMODE, set_opmode_reg);

  /*
   * Issue a direct_cmd by writing to direct_cmd register
   * This is needed becuase the UpdatesReg flag in direct_cmd updates the
   * state of SMC
   */
  u32 direct_cmd_reg = (0x0 << 23) | /* chip 1 from interface 0 */
                       (0x2 << 21) | /* UpdateRegs operation, to update the two reg we wrote earlier*/
                       (0x0 << 20) | /* cre */
                       (0x0);        /* addr, not use in UpdateRegs */
  Out32(PARPORT_CRTL_BASEADDR + PARPORT_MC_DIRECT_CMD, direct_cmd_reg);

  /* reset the flash itself so that it's ready to be accessed */

  Out8(NOR_FLASH_BASEADDR + 0xAAA, 0xAA);
  Out8(NOR_FLASH_BASEADDR + 0x555, 0x55);
  Out8(NOR_FLASH_BASEADDR,         0xF0);
}
#endif

#define Xil_Out32 Out32
#define Xil_In32 In32

#if 1
/* PLL divisor */
#define ARM_PLL_FDIV		48	/* 800 MHz CPU */ 
#define DDR_PLL_FDIV		24
#define	IO_PLL_FDIV		30

#define ARM_PLL_RES		2
#define ARM_PLL_CP		2
#define ARM_PLL_LOCK_CNT	250

#define DDR_PLL_RES		2
#define DDR_PLL_CP		2
#define DDR_PLL_LOCK_CNT	300

#define IO_PLL_RES		2
#define IO_PLL_CP		12
#define IO_PLL_LOCK_CNT		325

/* CPU */
#define CPU_SRCSEL		CPU_ARM_PLL
#define CPU_DIVISOR		2
/* DDR */
#define DDR_3XCLK_DIVISOR	2
#define DDR_2XCLK_DIVISOR	6
/* DCI */
#define DCI_DIVISOR0		2
#define DCI_DIVISOR1		40
/* USB0 */
#define USB0_SRCSEL		IO_PLL
#define USB0_DIVISOR0		15
#define USB0_DIVISOR1		1
/* GEM0 RX */
#define GEM0_RX_SRCSEL		GEM0_MIO_RX_CLK
/* GEM0 */
#define GEM0_SRCSEL		IO_PLL
#define GEM0_DIVISOR0		40 // was 8 JHL
#define GEM0_DIVISOR1		1
/* SMC */
#define SMC_SRCSEL		IO_PLL
#define SMC_DIVISOR		40
/* LQSPI */
#define LQSPI_SRCSEL		IO_PLL
#define LQSPI_DIVISOR		10
/* SDIO */
#define SDIO0_CD_SEL		0
#define SDIO0_WP_SEL		15
#define SDIO_SRCSEL		IO_PLL
#define SDIO_DIVISOR		20
/* UART */
#define UART_SRCSEL		IO_PLL
#define UART_DIVISOR		20
/* SPI */
#define SPI_SRCSEL		IO_PLL
#define SPI_DIVISOR		20
/* CAN */
#define CAN_SRCSEL		IO_PLL
#define CAN_DIVISOR0		42
#define CAN_DIVISOR1		1
/* FPGA0 */
#define FPGA0_SRCSEL		IO_PLL
#define FPGA0_DIVISOR0		40 // 15
#define FPGA0_DIVISOR1		1
/* FPGA1 */
#define FPGA1_SRCSEL		IO_PLL
#define FPGA1_DIVISOR0		15
#define FPGA1_DIVISOR1		1
/* FPGA2 */
#define FPGA2_SRCSEL		IO_PLL
#define FPGA2_DIVISOR0		15
#define FPGA2_DIVISOR1		1
/* FPGA3 */
#define FPGA3_SRCSEL		IO_PLL
#define FPGA3_DIVISOR0		15
#define FPGA3_DIVISOR1		1
/* PCAP */
#define PCAP_SRCSEL		IO_PLL
#define PCAP_DIVISOR		15
#endif
void memtest_mio_init(void)
{
	unsigned int RegVal=0;

	/* SLCR unlock */
	Xil_Out32(SLCR_UNLOCK, 0xDF0D);

	/* LSPI */
	RegVal = (LVCMOS18 | FAST_CMOS | MIO_LQSPI | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO1, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_LQSPI | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO2, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_LQSPI | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO3, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_LQSPI | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO4, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_LQSPI | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO5, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_LQSPI | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO6, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_LQSPI | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO8, RegVal);

	/* GEM0 */
	RegVal = (DISABLE_RCVR | LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO16, RegVal);

	RegVal = (DISABLE_RCVR | LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO17, RegVal);

	RegVal = (DISABLE_RCVR | LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO18, RegVal);

	RegVal = (DISABLE_RCVR | LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO19, RegVal);

	RegVal = (DISABLE_RCVR | LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO20, RegVal);

	RegVal = (DISABLE_RCVR | LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO21, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO22, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO23, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO24, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO25, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO26, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_GEM | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO27, RegVal);

	/* MDIO0 */
	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_MDIO0 | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO52, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_MDIO0 | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO53, RegVal);

	/* USB0 */
	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_GPIO | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO7, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO28, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO29, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO30, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO31, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO32, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO33, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO34, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO35, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO36, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO37, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO38, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_USB | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO39, RegVal);

	/* SPI1 */
	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_SPI | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO10, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_SPI | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO11, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_SPI | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO12, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_SPI | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO13, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_SPI | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO14, RegVal);

	/* SDIO0 */
	RegVal =  (LVCMOS18 | SLOW_CMOS | MIO_GPIO | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO0, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_GPIO | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO15, RegVal);

	RegVal = ((SDIO0_CD_SEL << SDIO0_CD_SEL_SHIFT) | SDIO0_WP_SEL);
	Xil_Out32(SLCR_SDIO0_WP_CD, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_SDIO | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO40, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_SDIO | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO41, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_SDIO | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO42, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_SDIO | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO43, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_SDIO | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO44, RegVal);

	RegVal = (LVCMOS18 | FAST_CMOS | MIO_SDIO | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO45, RegVal);

	/* CAN0 */
	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_GPIO | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO9, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_CAN | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO46, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_CAN | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO47, RegVal);

	/* UART1 */
	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_UART | TRI_ENABLE_OUT);
	Xil_Out32(SLCR_MIO48, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_UART | TRI_ENABLE_IN);
	Xil_Out32(SLCR_MIO49, RegVal);

	/* I2C0 */
	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_I2C | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO50, RegVal);

	RegVal = (LVCMOS18 | SLOW_CMOS | MIO_I2C | TRI_ENABLE_IN_OUT);
	Xil_Out32(SLCR_MIO51, RegVal);

	Xil_Out32(SLCR_LOCK, 0x767B);

}

void memtest_arm_pll_init(void)
{
	unsigned int RegVal=0;

	/* Upadte arm pll configuration */
	Xil_Out32(SLCR_ARM_PLL_CFG, ((ARM_PLL_LOCK_CNT << PLL_LOCK_CNT_SHIFT) | (ARM_PLL_CP << PLL_CP_SHIFT) | ( ARM_PLL_RES << PLL_RES_SHIFT)));

	/* Update slcr_pll_fbdiv value */
	RegVal = Xil_In32(SLCR_ARM_PLL_CTRL);
	RegVal &= ~(0x7F << PLL_FDIV_SHIFT);
	RegVal |= (ARM_PLL_FDIV << PLL_FDIV_SHIFT);
	Xil_Out32(SLCR_ARM_PLL_CTRL, RegVal);

	/* Put PLL in bypass mode */
	RegVal = Xil_In32(SLCR_ARM_PLL_CTRL);
	RegVal |= PLL_BYPASS_FORCE;
	Xil_Out32(SLCR_ARM_PLL_CTRL, RegVal);

	/* Assert reset to PLL */
	RegVal = Xil_In32(SLCR_ARM_PLL_CTRL);
	RegVal |= PLL_RESET;
	Xil_Out32(SLCR_ARM_PLL_CTRL, RegVal);

	/* Deassert reset to PLL */
	RegVal = Xil_In32(SLCR_ARM_PLL_CTRL);
	RegVal &= ~PLL_RESET;
	Xil_Out32(SLCR_ARM_PLL_CTRL, RegVal);

	/* Check PLL is locked */
	while(!(Xil_In32(SLCR_PLL_STATUS) & 0x1));

	/* Remove PLL bypass mode */
	RegVal = Xil_In32(SLCR_ARM_PLL_CTRL);
	RegVal &= ~PLL_BYPASS_FORCE;
	Xil_Out32(SLCR_ARM_PLL_CTRL, RegVal);

}

void memtest_ddr_pll_init(void)
{
	unsigned int RegVal=0;

	/* Upadte ddr pll configuration */
	Xil_Out32(SLCR_DDR_PLL_CFG, ((DDR_PLL_LOCK_CNT << PLL_LOCK_CNT_SHIFT) | (DDR_PLL_CP << PLL_CP_SHIFT) | (DDR_PLL_RES << PLL_RES_SHIFT)));

	/* Update slcr_pll_fbdiv value */
	RegVal = Xil_In32(SLCR_DDR_PLL_CTRL);
	RegVal &= ~(0x7F << PLL_FDIV_SHIFT);
	RegVal |= (DDR_PLL_FDIV << PLL_FDIV_SHIFT);
	Xil_Out32(SLCR_DDR_PLL_CTRL, RegVal);

	/* Put PLL in bypass mode */
	RegVal = Xil_In32(SLCR_DDR_PLL_CTRL);
	RegVal |= PLL_BYPASS_FORCE;
	Xil_Out32(SLCR_DDR_PLL_CTRL, RegVal);

	/* Assert reset to PLL */
	RegVal = Xil_In32(SLCR_DDR_PLL_CTRL);
	RegVal |= PLL_RESET;
	Xil_Out32(SLCR_DDR_PLL_CTRL, RegVal);

	/* Deassert reset to PLL */
	RegVal = Xil_In32(SLCR_DDR_PLL_CTRL);
	RegVal &= ~PLL_RESET;
	Xil_Out32(SLCR_DDR_PLL_CTRL, RegVal);

	/* Check PLL is locked */
	while(!(Xil_In32(SLCR_PLL_STATUS) & 0x2));

	/* Remove PLL bypass mode */
	RegVal = Xil_In32(SLCR_DDR_PLL_CTRL);
	RegVal &= ~PLL_BYPASS_FORCE;
	Xil_Out32(SLCR_DDR_PLL_CTRL, RegVal);
}


void memtest_io_pll_init(void)
{
	unsigned int RegVal=0;

	/* Upadte io pll configuration */
	Xil_Out32(SLCR_IO_PLL_CFG, ((IO_PLL_LOCK_CNT << PLL_LOCK_CNT_SHIFT) | (IO_PLL_CP << PLL_CP_SHIFT) | (IO_PLL_RES << PLL_RES_SHIFT)));

	/* Update slcr_pll_fbdiv value */
	RegVal = Xil_In32(SLCR_IO_PLL_CTRL);
	RegVal &= ~(0x7F << PLL_FDIV_SHIFT);
	RegVal |= (IO_PLL_FDIV << PLL_FDIV_SHIFT);
	Xil_Out32(SLCR_IO_PLL_CTRL, RegVal);

	/* Put PLL in bypass mode */
	RegVal = Xil_In32(SLCR_IO_PLL_CTRL);
	RegVal |= PLL_BYPASS_FORCE;
	Xil_Out32(SLCR_IO_PLL_CTRL, RegVal);

	/* Assert reset to PLL */
	RegVal = Xil_In32(SLCR_IO_PLL_CTRL);
	RegVal |= PLL_RESET;
	Xil_Out32(SLCR_IO_PLL_CTRL, RegVal);

	/* Deassert reset to PLL */
	RegVal = Xil_In32(SLCR_IO_PLL_CTRL);
	RegVal &= ~PLL_RESET;
	Xil_Out32(SLCR_IO_PLL_CTRL, RegVal);

	/* Check PLL is locked */
	while(!(Xil_In32(SLCR_PLL_STATUS) & 0x4));

	/* Remove PLL bypass mode */
	RegVal = Xil_In32(SLCR_IO_PLL_CTRL);
	RegVal &= ~PLL_BYPASS_FORCE;
	Xil_Out32(SLCR_IO_PLL_CTRL, RegVal);
}


void memtest_pll_init(void)
{
	/* SLCR unlock */
	Xil_Out32(SLCR_UNLOCK, 0xDF0D);

	/* ARM PLL initialization */
	memtest_arm_pll_init();

	/* DDR PLL initialization */
//	memtest_ddr_pll_init();

	/* IO PLL initialization */
	memtest_io_pll_init();

	/* SLCR lock */
	Xil_Out32(SLCR_LOCK, 0x767B);

}

void memtest_clock_init(void)
{

	/* SLCR unlock */
	Xil_Out32(SLCR_UNLOCK, 0xDF0D);

#if 1
	/* ARM */
	Xil_Out32(SLCR_ARM_CLK_CTRL, (CPU_PERI_CLKACT_ENABLE | CPU_1XCLKACT_ENABLE | CPU_2XCLKACT_ENABLE | CPU_3OR2XCLKACT_ENABLE |
							CPU_6OR4XCLKACT_ENABLE | (CPU_DIVISOR << CPU_DIVISOR_SHIFT) | (CPU_SRCSEL << CPU_SRCSEL_SHIFT)));

	/* DDR */
	Xil_Out32(SLCR_DDR_CLK_CTRL, ((DDR_2XCLK_DIVISOR << DDR_2XCLK_DIVISOR_SHIFT) | (DDR_3XCLK_DIVISOR << DDR_3XCLK_DIVISOR_SHIFT) |
						DDR_2XCLKACT_ENABLE | DDR_3XCLKACT_ENABLE));

	/* QSPI */
	Xil_Out32(SLCR_LQSPI_CLK_CTRL, ((LQSPI_DIVISOR << LQSPI_DIVISOR_SHIFT) | (LQSPI_SRCSEL << LQSPI_SRCSEL_SHIFT) | LQSPI_CLKACT_ENABLE));
#endif
	/* GEM0 */
	Xil_Out32(SLCR_GEM0_RCLK_CTRL, ((GEM0_RX_SRCSEL << GEM0_RX_SRCSEL_SHIFT) | GEM0_RX_CLKACT_ENABLE));
	Xil_Out32(SLCR_GEM0_CLK_CTRL, ((GEM0_DIVISOR1 << GEM0_DIVISOR1_SHIFT) | (GEM0_DIVISOR0 << GEM0_DIVISOR0_SHIFT) | (GEM0_SRCSEL << GEM0_SRCSEL_SHIFT) | GEM0_CLKACT_ENABLE));

#if 1
	/* USB0 */
	/* USB Reference clock coming in externally hence no need to set */
	Xil_Out32(SLCR_USB0_CLK_CTRL, ((USB0_DIVISOR1 << USB0_DIVISOR1_SHIFT) | (USB0_DIVISOR0 << USB0_DIVISOR0_SHIFT) | (USB0_SRCSEL << USB0_SRCSEL_SHIFT) | USB0_CLKACT_ENABLE));

	/* SPI1 */
	Xil_Out32(SLCR_SPI_CLK_CTRL, ((SPI_DIVISOR << SPI_DIVISOR_SHIFT) | (SPI_SRCSEL << SPI_SRCSEL_SHIFT) | SPI1_CLKACT_ENABLE));

	/* SDIO0 */
	Xil_Out32(SLCR_SDIO_CLK_CTRL, ((SDIO_DIVISOR << SDIO_DIVISOR_SHIFT)| (SDIO_SRCSEL << SDIO_SRCSEL_SHIFT) | SDIO0_CLKACT_ENABLE));

	/* CAN0 */
	Xil_Out32(SLCR_CAN_CLK_CTRL, ((CAN_DIVISOR1 << CAN_DIVISOR1_SHIFT) | (CAN_DIVISOR0 << CAN_DIVISOR0_SHIFT) | (CAN_SRCSEL << CAN_SRCSEL_SHIFT) | CAN0_CLKACT_ENABLE));

	/* UART1 */ 
	Xil_Out32(SLCR_UART_CLK_CTRL, ((UART_DIVISOR << UART_DIVISOR_SHIFT) | (UART_SRCSEL << UART_SRCSEL_SHIFT) | UART1_CLKACT_ENABLE));

	/* FPGA0 */ 
	Xil_Out32(SLCR_FPGA0_CLK_CTRL, ((FPGA0_DIVISOR1 << FPGA0_DIVISOR1_SHIFT) | (FPGA0_DIVISOR0 << FPGA0_DIVISOR0_SHIFT) | (FPGA0_SRCSEL << FPGA0_SRCSEL_SHIFT)));

	/* FPGA1 */ 
	Xil_Out32(SLCR_FPGA1_CLK_CTRL, ((FPGA1_DIVISOR1 << FPGA1_DIVISOR1_SHIFT) | (FPGA1_DIVISOR0 << FPGA1_DIVISOR0_SHIFT) | (FPGA1_SRCSEL << FPGA1_SRCSEL_SHIFT)));

	/* FPGA2 */ 
	Xil_Out32(SLCR_FPGA2_CLK_CTRL, ((FPGA2_DIVISOR1 << FPGA2_DIVISOR1_SHIFT) | (FPGA2_DIVISOR0 << FPGA2_DIVISOR0_SHIFT) | (FPGA2_SRCSEL << FPGA2_SRCSEL_SHIFT)));

	/* FPGA3 */ 
	Xil_Out32(SLCR_FPGA3_CLK_CTRL, ((FPGA3_DIVISOR1 << FPGA3_DIVISOR1_SHIFT) | (FPGA3_DIVISOR0 << FPGA3_DIVISOR0_SHIFT) | (FPGA3_SRCSEL << FPGA3_SRCSEL_SHIFT)));

	/* PCAP */ 
	Xil_Out32(SLCR_PCAP_CLK_CTRL, ((PCAP_DIVISOR << PCAP_DIVISOR_SHIFT) | (PCAP_SRCSEL << PCAP_SRCSEL_SHIFT) | PCAP_CLKACT_ENABLE));

	/*IIC0*/
	/*MDIO0*/
	/*GPIO*/
	/* Enable interface clock */
	Xil_Out32(SLCR_APER_CLK_CTRL, (SLCR_DMA_CPU_2XCLKACT_ENABLE | SLCR_USB0_CPU_1XCLKACT_ENABLE | SLCR_GEM0_CPU_1XCLKACT_ENABLE |
						  SLCR_SDI0_CPU_1XCLKACT_ENABLE | SLCR_SPI1_CPU_1XCLKACT_ENABLE | SLCR_CAN0_CPU_1XCLKACT_ENABLE |
						  SLCR_I2C0_CPU_1XCLKACT_ENABLE | SLCR_UART1_CPU_1XCLKACT_ENABLE | SLCR_GPIO_CPU_1XCLKACT_ENABLE |
						  SLCR_LQSPI_CPU_1XCLKACT_ENABLE | SLCR_SMC_CPU_1XCLKACT_ENABLE));

	/* DCI */
	Xil_Out32(SLCR_DCI_CLK_CTRL, ((DCI_DIVISOR1 << DCI_DIVISOR1_SHIFT) | (DCI_DIVISOR0 << DCI_DIVISOR0_SHIFT) | DCI_CLKACT_ENABLE));
#endif
	/* SLCR lock */
	Xil_Out32(SLCR_LOCK, 0x767B);
}

void from_burst_main(void)
{
#ifdef CONFIG_ZYNQ_MIO_INIT
	memtest_mio_init();
#endif
#ifdef CONFIG_ZYNQ_PLL_INIT
	memtest_pll_init();
#endif
#ifdef CONFIG_ZYNQ_MIO_INIT
	memtest_clock_init();
#endif
}

#ifdef CONFIG_FPGA
Xilinx_desc fpga = XILINX_XC7Z020_DESC(0);
#endif

int board_init(void)
{
	/* taken from burst, some DDR/MIO/Clock setup hacked in */

	from_burst_main();

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
#ifndef CONFIG_SYS_NO_FLASH
	init_nor_flash();
#endif

#ifdef CONFIG_FPGA
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif

	return 0;
}

int board_late_init (void)
{
	u32 boot_mode;

	boot_mode = (In32(BOOT_MODE_REG) & BOOT_MODES_MASK);
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
	return Xgmac_register(bis);
}
#endif

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	return zynq_mmc_init(bd);
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
