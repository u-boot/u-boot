/*
 * ATAPI Masks
 */

#ifndef __BFIN_PERIPHERAL_PATA__
#define __BFIN_PERIPHERAL_PATA__

/* Bit masks for ATAPI_CONTROL */
#define                 PIO_START  0x1        /* Start PIO/Reg Op */
#define               MULTI_START  0x2        /* Start Multi-DMA Op */
#define               ULTRA_START  0x4        /* Start Ultra-DMA Op */
#define                  XFER_DIR  0x8        /* Transfer Direction */
#define                  IORDY_EN  0x10       /* IORDY Enable */
#define                FIFO_FLUSH  0x20       /* Flush FIFOs */
#define                  SOFT_RST  0x40       /* Soft Reset */
#define                   DEV_RST  0x80       /* Device Reset */
#define                TFRCNT_RST  0x100      /* Trans Count Reset */
#define               END_ON_TERM  0x200      /* End/Terminate Select */
#define               PIO_USE_DMA  0x400      /* PIO-DMA Enable */
#define          UDMAIN_FIFO_THRS  0xf000     /* Ultra DMA-IN FIFO Threshold */

/* Bit masks for ATAPI_STATUS */
#define               PIO_XFER_ON  0x1        /* PIO transfer in progress */
#define             MULTI_XFER_ON  0x2        /* Multi-word DMA transfer in progress */
#define             ULTRA_XFER_ON  0x4        /* Ultra DMA transfer in progress */
#define               ULTRA_IN_FL  0xf0       /* Ultra DMA Input FIFO Level */

/* Bit masks for ATAPI_DEV_ADDR */
#define                  DEV_ADDR  0x1f       /* Device Address */

/* Bit masks for ATAPI_INT_MASK */
#define        ATAPI_DEV_INT_MASK  0x1        /* Device interrupt mask */
#define             PIO_DONE_MASK  0x2        /* PIO transfer done interrupt mask */
#define           MULTI_DONE_MASK  0x4        /* Multi-DMA transfer done interrupt mask */
#define          UDMAIN_DONE_MASK  0x8        /* Ultra-DMA in transfer done interrupt mask */
#define         UDMAOUT_DONE_MASK  0x10       /* Ultra-DMA out transfer done interrupt mask */
#define       HOST_TERM_XFER_MASK  0x20       /* Host terminate current transfer interrupt mask */
#define           MULTI_TERM_MASK  0x40       /* Device terminate Multi-DMA transfer interrupt mask */
#define          UDMAIN_TERM_MASK  0x80       /* Device terminate Ultra-DMA-in transfer interrupt mask */
#define         UDMAOUT_TERM_MASK  0x100      /* Device terminate Ultra-DMA-out transfer interrupt mask */

/* Bit masks for ATAPI_INT_STATUS */
#define             ATAPI_DEV_INT  0x1        /* Device interrupt status */
#define              PIO_DONE_INT  0x2        /* PIO transfer done interrupt status */
#define            MULTI_DONE_INT  0x4        /* Multi-DMA transfer done interrupt status */
#define           UDMAIN_DONE_INT  0x8        /* Ultra-DMA in transfer done interrupt status */
#define          UDMAOUT_DONE_INT  0x10       /* Ultra-DMA out transfer done interrupt status */
#define        HOST_TERM_XFER_INT  0x20       /* Host terminate current transfer interrupt status */
#define            MULTI_TERM_INT  0x40       /* Device terminate Multi-DMA transfer interrupt status */
#define           UDMAIN_TERM_INT  0x80       /* Device terminate Ultra-DMA-in transfer interrupt status */
#define          UDMAOUT_TERM_INT  0x100      /* Device terminate Ultra-DMA-out transfer interrupt status */

/* Bit masks for ATAPI_LINE_STATUS */
#define                ATAPI_INTR  0x1        /* Device interrupt to host line status */
#define                ATAPI_DASP  0x2        /* Device dasp to host line status */
#define                ATAPI_CS0N  0x4        /* ATAPI chip select 0 line status */
#define                ATAPI_CS1N  0x8        /* ATAPI chip select 1 line status */
#define                ATAPI_ADDR  0x70       /* ATAPI address line status */
#define              ATAPI_DMAREQ  0x80       /* ATAPI DMA request line status */
#define             ATAPI_DMAACKN  0x100      /* ATAPI DMA acknowledge line status */
#define               ATAPI_DIOWN  0x200      /* ATAPI write line status */
#define               ATAPI_DIORN  0x400      /* ATAPI read line status */
#define               ATAPI_IORDY  0x800      /* ATAPI IORDY line status */

/* Bit masks for ATAPI_SM_STATE */
#define                PIO_CSTATE  0xf        /* PIO mode state machine current state */
#define                DMA_CSTATE  0xf0       /* DMA mode state machine current state */
#define             UDMAIN_CSTATE  0xf00      /* Ultra DMA-In mode state machine current state */
#define            UDMAOUT_CSTATE  0xf000     /* ATAPI IORDY line status */

/* Bit masks for ATAPI_TERMINATE */
#define           ATAPI_HOST_TERM  0x1        /* Host terminationation */

/* Bit masks for ATAPI_REG_TIM_0 */
#define                    T2_REG  0xff       /* End of cycle time for register access transfers */
#define                  TEOC_REG  0xff00     /* Selects DIOR/DIOW pulsewidth */

/* Bit masks for ATAPI_PIO_TIM_0 */
#define                    T1_REG  0xf        /* Time from address valid to DIOR/DIOW */
#define                T2_REG_PIO  0xff0      /* DIOR/DIOW pulsewidth */
#define                    T4_REG  0xf000     /* DIOW data hold */

/* Bit masks for ATAPI_PIO_TIM_1 */
#define              TEOC_REG_PIO  0xff       /* End of cycle time for PIO access transfers. */

/* Bit masks for ATAPI_MULTI_TIM_0 */
#define                        TD  0xff       /* DIOR/DIOW asserted pulsewidth */
#define                        TM  0xff00     /* Time from address valid to DIOR/DIOW */

/* Bit masks for ATAPI_MULTI_TIM_1 */
#define                       TKW  0xff       /* Selects DIOW negated pulsewidth */
#define                       TKR  0xff00     /* Selects DIOR negated pulsewidth */

/* Bit masks for ATAPI_MULTI_TIM_2 */
#define                        TH  0xff       /* Selects DIOW data hold */
#define                      TEOC  0xff00     /* Selects end of cycle for DMA */

/* Bit masks for ATAPI_ULTRA_TIM_0 */
#define                      TACK  0xff       /* Selects setup and hold times for TACK */
#define                      TENV  0xff00     /* Selects envelope time */

/* Bit masks for ATAPI_ULTRA_TIM_1 */
#define                      TDVS  0xff       /* Selects data valid setup time */
#define                 TCYC_TDVS  0xff00     /* Selects cycle time - TDVS time */

/* Bit masks for ATAPI_ULTRA_TIM_2 */
#define                       TSS  0xff       /* Selects time from STROBE edge to negation of DMARQ or assertion of STOP */
#define                      TMLI  0xff00     /* Selects interlock time */

/* Bit masks for ATAPI_ULTRA_TIM_3 */
#define                      TZAH  0xff       /* Selects minimum delay required for output */
#define               READY_PAUSE  0xff00     /* Selects ready to pause */

/* Bit masks for ATAPI_CONTROL */
#define                 PIO_START  0x1        /* Start PIO/Reg Op */
#define               MULTI_START  0x2        /* Start Multi-DMA Op */
#define               ULTRA_START  0x4        /* Start Ultra-DMA Op */
#define                  XFER_DIR  0x8        /* Transfer Direction */
#define                  IORDY_EN  0x10       /* IORDY Enable */
#define                FIFO_FLUSH  0x20       /* Flush FIFOs */
#define                  SOFT_RST  0x40       /* Soft Reset */
#define                   DEV_RST  0x80       /* Device Reset */
#define                TFRCNT_RST  0x100      /* Trans Count Reset */
#define               END_ON_TERM  0x200      /* End/Terminate Select */
#define               PIO_USE_DMA  0x400      /* PIO-DMA Enable */
#define          UDMAIN_FIFO_THRS  0xf000     /* Ultra DMA-IN FIFO Threshold */

/* Bit masks for ATAPI_STATUS */
#define               PIO_XFER_ON  0x1        /* PIO transfer in progress */
#define             MULTI_XFER_ON  0x2        /* Multi-word DMA transfer in progress */
#define             ULTRA_XFER_ON  0x4        /* Ultra DMA transfer in progress */
#define               ULTRA_IN_FL  0xf0       /* Ultra DMA Input FIFO Level */

/* Bit masks for ATAPI_DEV_ADDR */
#define                  DEV_ADDR  0x1f       /* Device Address */

/* Bit masks for ATAPI_INT_MASK */
#define        ATAPI_DEV_INT_MASK  0x1        /* Device interrupt mask */
#define             PIO_DONE_MASK  0x2        /* PIO transfer done interrupt mask */
#define           MULTI_DONE_MASK  0x4        /* Multi-DMA transfer done interrupt mask */
#define          UDMAIN_DONE_MASK  0x8        /* Ultra-DMA in transfer done interrupt mask */
#define         UDMAOUT_DONE_MASK  0x10       /* Ultra-DMA out transfer done interrupt mask */
#define       HOST_TERM_XFER_MASK  0x20       /* Host terminate current transfer interrupt mask */
#define           MULTI_TERM_MASK  0x40       /* Device terminate Multi-DMA transfer interrupt mask */
#define          UDMAIN_TERM_MASK  0x80       /* Device terminate Ultra-DMA-in transfer interrupt mask */
#define         UDMAOUT_TERM_MASK  0x100      /* Device terminate Ultra-DMA-out transfer interrupt mask */

/* Bit masks for ATAPI_INT_STATUS */
#define             ATAPI_DEV_INT  0x1        /* Device interrupt status */
#define              PIO_DONE_INT  0x2        /* PIO transfer done interrupt status */
#define            MULTI_DONE_INT  0x4        /* Multi-DMA transfer done interrupt status */
#define           UDMAIN_DONE_INT  0x8        /* Ultra-DMA in transfer done interrupt status */
#define          UDMAOUT_DONE_INT  0x10       /* Ultra-DMA out transfer done interrupt status */
#define        HOST_TERM_XFER_INT  0x20       /* Host terminate current transfer interrupt status */
#define            MULTI_TERM_INT  0x40       /* Device terminate Multi-DMA transfer interrupt status */
#define           UDMAIN_TERM_INT  0x80       /* Device terminate Ultra-DMA-in transfer interrupt status */
#define          UDMAOUT_TERM_INT  0x100      /* Device terminate Ultra-DMA-out transfer interrupt status */

/* Bit masks for ATAPI_LINE_STATUS */
#define                ATAPI_INTR  0x1        /* Device interrupt to host line status */
#define                ATAPI_DASP  0x2        /* Device dasp to host line status */
#define                ATAPI_CS0N  0x4        /* ATAPI chip select 0 line status */
#define                ATAPI_CS1N  0x8        /* ATAPI chip select 1 line status */
#define                ATAPI_ADDR  0x70       /* ATAPI address line status */
#define              ATAPI_DMAREQ  0x80       /* ATAPI DMA request line status */
#define             ATAPI_DMAACKN  0x100      /* ATAPI DMA acknowledge line status */
#define               ATAPI_DIOWN  0x200      /* ATAPI write line status */
#define               ATAPI_DIORN  0x400      /* ATAPI read line status */
#define               ATAPI_IORDY  0x800      /* ATAPI IORDY line status */

/* Bit masks for ATAPI_SM_STATE */
#define                PIO_CSTATE  0xf        /* PIO mode state machine current state */
#define                DMA_CSTATE  0xf0       /* DMA mode state machine current state */
#define             UDMAIN_CSTATE  0xf00      /* Ultra DMA-In mode state machine current state */
#define            UDMAOUT_CSTATE  0xf000     /* ATAPI IORDY line status */

/* Bit masks for ATAPI_TERMINATE */
#define           ATAPI_HOST_TERM  0x1        /* Host terminationation */

/* Bit masks for ATAPI_REG_TIM_0 */
#define                    T2_REG  0xff       /* End of cycle time for register access transfers */
#define                  TEOC_REG  0xff00     /* Selects DIOR/DIOW pulsewidth */

/* Bit masks for ATAPI_PIO_TIM_0 */
#define                    T1_REG  0xf        /* Time from address valid to DIOR/DIOW */
#define                T2_REG_PIO  0xff0      /* DIOR/DIOW pulsewidth */
#define                    T4_REG  0xf000     /* DIOW data hold */

/* Bit masks for ATAPI_PIO_TIM_1 */
#define              TEOC_REG_PIO  0xff       /* End of cycle time for PIO access transfers. */

/* Bit masks for ATAPI_MULTI_TIM_0 */
#define                        TD  0xff       /* DIOR/DIOW asserted pulsewidth */
#define                        TM  0xff00     /* Time from address valid to DIOR/DIOW */

/* Bit masks for ATAPI_MULTI_TIM_1 */
#define                       TKW  0xff       /* Selects DIOW negated pulsewidth */
#define                       TKR  0xff00     /* Selects DIOR negated pulsewidth */

/* Bit masks for ATAPI_MULTI_TIM_2 */
#define                        TH  0xff       /* Selects DIOW data hold */
#define                      TEOC  0xff00     /* Selects end of cycle for DMA */

/* Bit masks for ATAPI_ULTRA_TIM_0 */
#define                      TACK  0xff       /* Selects setup and hold times for TACK */
#define                      TENV  0xff00     /* Selects envelope time */

/* Bit masks for ATAPI_ULTRA_TIM_1 */
#define                      TDVS  0xff       /* Selects data valid setup time */
#define                 TCYC_TDVS  0xff00     /* Selects cycle time - TDVS time */

/* Bit masks for ATAPI_ULTRA_TIM_2 */
#define                       TSS  0xff       /* Selects time from STROBE edge to negation of DMARQ or assertion of STOP */
#define                      TMLI  0xff00     /* Selects interlock time */

/* Bit masks for ATAPI_ULTRA_TIM_3 */
#define                      TZAH  0xff       /* Selects minimum delay required for output */
#define               READY_PAUSE  0xff00     /* Selects ready to pause */

#endif /* __BFIN_PERIPHERAL_PATA__ */
