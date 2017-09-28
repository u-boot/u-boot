/******************************************************************************
*
* (c) Copyright 2010-2014 Xilinx, Inc. All rights reserved.
*
* SPDX-License-Identifier: GPL-2.0+
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#define OPCODE_EXIT       0U
#define OPCODE_CLEAR      1U
#define OPCODE_WRITE      2U
#define OPCODE_MASKWRITE  3U
#define OPCODE_MASKPOLL   4U
#define OPCODE_MASKDELAY  5U

/* Encode number of arguments in last nibble */
#define EMIT_EXIT()                   ((OPCODE_EXIT      << 4) | 0)
#define EMIT_WRITE(addr, val)          ((OPCODE_WRITE     << 4) | 2) , addr, val
#define EMIT_MASKWRITE(addr, mask, val) ((OPCODE_MASKWRITE << 4) | 3) ,\
					addr, mask, val
#define EMIT_MASKPOLL(addr, mask)      ((OPCODE_MASKPOLL  << 4) | 2) ,\
					addr, mask
#define EMIT_MASKDELAY(addr, mask)      ((OPCODE_MASKDELAY << 4) | 2) ,\
					addr, mask

/* Returns codes  of PS7_Init */
#define PS7_INIT_SUCCESS   (0)
#define PS7_INIT_CORRUPT   (1)
#define PS7_INIT_TIMEOUT   (2)
#define PS7_POLL_FAILED_DDR_INIT (3)
#define PS7_POLL_FAILED_DMA      (4)
#define PS7_POLL_FAILED_PLL      (5)

/* Freq of all peripherals */

#define APU_FREQ  650000000
#define DDR_FREQ  525000000
#define DCI_FREQ  10096154
#define QSPI_FREQ  200000000
#define SMC_FREQ  10000000
#define ENET0_FREQ  125000000
#define ENET1_FREQ  10000000
#define USB0_FREQ  60000000
#define USB1_FREQ  60000000
#define SDIO_FREQ  100000000
#define UART_FREQ  100000000
#define SPI_FREQ  10000000
#define I2C_FREQ  108333336
#define WDT_FREQ  108333336
#define TTC_FREQ  50000000
#define CAN_FREQ  10000000
#define PCAP_FREQ  200000000
#define TPIU_FREQ  200000000
#define FPGA0_FREQ  50000000
#define FPGA1_FREQ  10000000
#define FPGA2_FREQ  10000000
#define FPGA3_FREQ  10000000


/* For delay calculation using global registers*/
#define SCU_GLOBAL_TIMER_COUNT_L32	0xF8F00200
#define SCU_GLOBAL_TIMER_COUNT_U32	0xF8F00204
#define SCU_GLOBAL_TIMER_CONTROL	0xF8F00208
#define SCU_GLOBAL_TIMER_AUTO_INC	0xF8F00218

int ps7_config(unsigned long *);
int ps7_init(void);
int ps7_post_config(void);
int ps7_debug(void);

void perf_start_clock(void);
void perf_disable_clock(void);
void perf_reset_clock(void);
void perf_reset_and_start_timer(void);
int get_number_of_cycles_for_delay(unsigned int delay);
#ifdef __cplusplus
}
#endif
