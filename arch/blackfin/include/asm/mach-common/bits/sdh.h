/*
 * SDH Masks
 */

#ifndef __BFIN_PERIPHERAL_SDH__
#define __BFIN_PERIPHERAL_SDH__

/* Bit masks for SDH_COMMAND */
#define                   CMD_IDX  0x3f       /* Command Index */
#define                   CMD_RSP  0x40       /* Response */
#define                 CMD_L_RSP  0x80       /* Long Response */
#define                 CMD_INT_E  0x100      /* Command Interrupt */
#define                CMD_PEND_E  0x200      /* Command Pending */
#define                     CMD_E  0x400      /* Command Enable */
#ifdef RSI_BLKSZ
#define           CMD_CRC_CHECK_D  0x800      /* CRC Check is disabled */
#define            CMD_DATA0_BUSY  0x1000     /* Check Busy State on DATA0 */
#endif

/* Bit masks for SDH_PWR_CTL */
#ifndef RSI_BLKSZ
#define                    PWR_ON  0x3        /* Power On */
#define                 SD_CMD_OD  0x40       /* Open Drain Output */
#define                   ROD_CTL  0x80       /* Rod Control */
#endif

/* Bit masks for SDH_CLK_CTL */
#define                    CLKDIV  0xff       /* MC_CLK Divisor */
#define                     CLK_E  0x100      /* MC_CLK Bus Clock Enable */
#define                  PWR_SV_E  0x200      /* Power Save Enable */
#define             CLKDIV_BYPASS  0x400      /* Bypass Divisor */
#define             BUS_MODE_MASK  0x1800     /* Bus Mode Mask */
#define                 STD_BUS_1  0x000      /* Standard Bus 1 bit mode */
#define                WIDE_BUS_4  0x800      /* Wide Bus 4 bit mode */
#define                BYTE_BUS_8  0x1000     /* Byte Bus 8 bit mode */
#ifdef RSI_BLKSZ
#define            CARD_TYPE_MASK  0xe000     /* Card type mask */
#define          CARD_TYPE_OFFSET  13         /* Card type offset */
#define            CARD_TYPE_SDIO  0
#define            CARD_TYPE_eMMC  1
#define              CARD_TYPE_SD  2
#define           CARD_TYPE_CEATA  3
#endif

/* Bit masks for SDH_RESP_CMD */
#define                  RESP_CMD  0x3f       /* Response Command */

/* Bit masks for SDH_DATA_CTL */
#define                     DTX_E  0x1        /* Data Transfer Enable */
#define                   DTX_DIR  0x2        /* Data Transfer Direction */
#define                  DTX_MODE  0x4        /* Data Transfer Mode */
#define                 DTX_DMA_E  0x8        /* Data Transfer DMA Enable */
#ifndef RSI_BLKSZ
#define              DTX_BLK_LGTH  0xf0       /* Data Transfer Block Length */
#else

/* Bit masks for SDH_BLK_SIZE */
#define              DTX_BLK_LGTH  0x1fff     /* Data Transfer Block Length */
#endif

/* Bit masks for SDH_STATUS */
#define              CMD_CRC_FAIL  0x1        /* CMD CRC Fail */
#define              DAT_CRC_FAIL  0x2        /* Data CRC Fail */
#define              CMD_TIME_OUT  0x4        /* CMD Time Out */
#define              DAT_TIME_OUT  0x8        /* Data Time Out */
#define               TX_UNDERRUN  0x10       /* Transmit Underrun */
#define                RX_OVERRUN  0x20       /* Receive Overrun */
#define              CMD_RESP_END  0x40       /* CMD Response End */
#define                  CMD_SENT  0x80       /* CMD Sent */
#define                   DAT_END  0x100      /* Data End */
#define             START_BIT_ERR  0x200      /* Start Bit Error */
#define               DAT_BLK_END  0x400      /* Data Block End */
#define                   CMD_ACT  0x800      /* CMD Active */
#define                    TX_ACT  0x1000     /* Transmit Active */
#define                    RX_ACT  0x2000     /* Receive Active */
#define              TX_FIFO_STAT  0x4000     /* Transmit FIFO Status */
#define              RX_FIFO_STAT  0x8000     /* Receive FIFO Status */
#define              TX_FIFO_FULL  0x10000    /* Transmit FIFO Full */
#define              RX_FIFO_FULL  0x20000    /* Receive FIFO Full */
#define              TX_FIFO_ZERO  0x40000    /* Transmit FIFO Empty */
#define               RX_DAT_ZERO  0x80000    /* Receive FIFO Empty */
#define                TX_DAT_RDY  0x100000   /* Transmit Data Available */
#define               RX_FIFO_RDY  0x200000   /* Receive Data Available */

/* Bit masks for SDH_STATUS_CLR */
#define         CMD_CRC_FAIL_STAT  0x1        /* CMD CRC Fail Status */
#define         DAT_CRC_FAIL_STAT  0x2        /* Data CRC Fail Status */
#define          CMD_TIMEOUT_STAT  0x4        /* CMD Time Out Status */
#define          DAT_TIMEOUT_STAT  0x8        /* Data Time Out status */
#define          TX_UNDERRUN_STAT  0x10       /* Transmit Underrun Status */
#define           RX_OVERRUN_STAT  0x20       /* Receive Overrun Status */
#define         CMD_RESP_END_STAT  0x40       /* CMD Response End Status */
#define             CMD_SENT_STAT  0x80       /* CMD Sent Status */
#define              DAT_END_STAT  0x100      /* Data End Status */
#define        START_BIT_ERR_STAT  0x200      /* Start Bit Error Status */
#define          DAT_BLK_END_STAT  0x400      /* Data Block End Status */

/* Bit masks for SDH_MASK0 */
#define         CMD_CRC_FAIL_MASK  0x1        /* CMD CRC Fail Mask */
#define         DAT_CRC_FAIL_MASK  0x2        /* Data CRC Fail Mask */
#define          CMD_TIMEOUT_MASK  0x4        /* CMD Time Out Mask */
#define          DAT_TIMEOUT_MASK  0x8        /* Data Time Out Mask */
#define          TX_UNDERRUN_MASK  0x10       /* Transmit Underrun Mask */
#define           RX_OVERRUN_MASK  0x20       /* Receive Overrun Mask */
#define         CMD_RESP_END_MASK  0x40       /* CMD Response End Mask */
#define             CMD_SENT_MASK  0x80       /* CMD Sent Mask */
#define              DAT_END_MASK  0x100      /* Data End Mask */
#define        START_BIT_ERR_MASK  0x200      /* Start Bit Error Mask */
#define          DAT_BLK_END_MASK  0x400      /* Data Block End Mask */
#define              CMD_ACT_MASK  0x800      /* CMD Active Mask */
#define               TX_ACT_MASK  0x1000     /* Transmit Active Mask */
#define               RX_ACT_MASK  0x2000     /* Receive Active Mask */
#define         TX_FIFO_STAT_MASK  0x4000     /* Transmit FIFO Status Mask */
#define         RX_FIFO_STAT_MASK  0x8000     /* Receive FIFO Status Mask */
#define         TX_FIFO_FULL_MASK  0x10000    /* Transmit FIFO Full Mask */
#define         RX_FIFO_FULL_MASK  0x20000    /* Receive FIFO Full Mask */
#define         TX_FIFO_ZERO_MASK  0x40000    /* Transmit FIFO Empty Mask */
#define          RX_DAT_ZERO_MASK  0x80000    /* Receive FIFO Empty Mask */
#define           TX_DAT_RDY_MASK  0x100000   /* Transmit Data Available Mask */
#define          RX_FIFO_RDY_MASK  0x200000   /* Receive Data Available Mask */

/* Bit masks for SDH_FIFO_CNT */
#define                FIFO_COUNT  0x7fff     /* FIFO Count */

/* Bit masks for SDH_E_STATUS */
#define              SDIO_INT_DET  0x2        /* SDIO Int Detected */
#define               SD_CARD_DET  0x10       /* SD Card Detect */
#define          SD_CARD_BUSYMODE  0x80000000 /* Card is in Busy mode */
#define           SD_CARD_SLPMODE  0x40000000 /* Card in Sleep Mode */
#define             SD_CARD_READY  0x00020000 /* Card Ready */

/* Bit masks for SDH_E_MASK */
#define                  SDIO_MSK  0x2        /* Mask SDIO Int Detected */
#define                   SCD_MSK  0x10       /* Mask Card Detect */

/* Bit masks for SDH_CFG */
#define                   CLKS_EN  0x1        /* Clocks Enable */
#define                      SD4E  0x4        /* SDIO 4-Bit Enable */
#define                       MWE  0x8        /* Moving Window Enable */
#define                    SD_RST  0x10       /* SDMMC Reset */
#define                 PUP_SDDAT  0x20       /* Pull-up SD_DAT */
#define                PUP_SDDAT3  0x40       /* Pull-up SD_DAT3 */
#ifndef RSI_BLKSZ
#define                 PD_SDDAT3  0x80       /* Pull-down SD_DAT3 */
#else
#define                    PWR_ON  0x600      /* Power On */
#define                 SD_CMD_OD  0x800      /* Open Drain Output */
#define                   BOOT_EN  0x1000     /* Boot Enable */
#define                 BOOT_MODE  0x2000     /* Alternate Boot Mode */
#define               BOOT_ACK_EN  0x4000     /* Boot ACK is expected */
#endif

/* Bit masks for SDH_RD_WAIT_EN */
#define                       RWR  0x1        /* Read Wait Request */

#endif
