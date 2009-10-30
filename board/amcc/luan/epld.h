#define EPLD0_FSEL_FB2		0x80
#define EPLD0_BOOT_SMALL_FLASH	0x40	/* 0 boot from large flash, 1 from small flash */
#define EPLD0_RAW_CARD_BIT0	0x20	/* raw card EC level */
#define EPLD0_RAW_CARD_BIT1	0x10
#define EPLD0_RAW_CARD_BIT2	0x08
#define EPLD0_EXT_ARB_SEL_N	0x04	/* 0 select on-board ext PCI-X, 1 internal arbiter */
#define EPLD0_FLASH_ONBRD_N	0x02	/* 0 small flash/SRAM active, 1 block access */
#define EPLD0_FLASH_SRAM_SEL_N	0x01	/* 0 SRAM at mem top, 1 small flash at mem top */

#define EPLD1_CLK_CNTL0		0x80	/* FSEL-FB1 of MPC9772 */
#define EPLD1_PCIL0_CNTL1	0x40	/* S*0 of 9531 */
#define EPLD1_PCIL0_CNTL2	0x20	/* S*1 of 9531 */
#define EPLD1_CLK_CNTL3		0x10	/* FSEL-B1 of MPC9772 */
#define EPLD1_CLK_CNTL4		0x08	/* FSEL-B0 of MPC9772 */
#define EPLD1_MASTER_CLOCK6	0x04	/* clock source select 6 */
#define EPLD1_MASTER_CLOCK7	0x02	/* clock source select 7 */
#define EPLD1_MASTER_CLOCK8	0x01	/* clock source select 8 */

#define EPLD2_ETH_MODE_10	0x80	/* Ethernet mode 10   (default = 1) */
#define EPLD2_ETH_MODE_100	0x40	/* Ethernet mode 100  (default = 1) */
#define EPLD2_ETH_MODE_1000	0x20	/* Ethernet mode 1000 (default = 1) */
#define EPLD2_ETH_DUPLEX_MODE	0x10	/* Ethernet force full duplex mode */
#define EPLD2_RESET_ETH_N	0x08	/* Ethernet reset (default = 1) */
#define EPLD2_ETH_AUTO_NEGO	0x04	/* Ethernet auto negotiation */
#define EPLD2_DEFAULT_UART_N	0x01	/* 0 select DSR DTR for UART1 */

#define EPLD3_STATUS_LED4	0x08	/* status LED 8 (1 = LED on) */
#define EPLD3_STATUS_LED3	0x04	/* status LED 4 (1 = LED on) */
#define EPLD3_STATUS_LED2	0x02	/* status LED 2 (1 = LED on) */
#define EPLD3_STATUS_LED1	0x01	/* status LED 1 (1 = LED on) */

#define EPLD4_PCIL0_VTH1	0x80	/* PCI-X 0 VTH1 status */
#define EPLD4_PCIL0_VTH2	0x40	/* PCI-X 0 VTH2 status */
#define EPLD4_PCIL0_VTH3	0x20	/* PCI-X 0 VTH3 status */
#define EPLD4_PCIL0_VTH4	0x10	/* PCI-X 0 VTH4 status */
#define EPLD4_PCIX1_VTH1	0x08	/* PCI-X 1 VTH1 status */
#define EPLD4_PCIX1_VTH2	0x04	/* PCI-X 1 VTH2 status */
#define EPLD4_PCIX1_VTH3	0x02	/* PCI-X 1 VTH3 status */
#define EPLD4_PCIX1_VTH4	0x01	/* PCI-X 1 VTH4 status */

#define EPLD5_PCIL0_INT0	0x80	/* PCIX0 INT0 status, write 0 to reset */
#define EPLD5_PCIL0_INT1	0x40	/* PCIX0 INT1 status, write 0 to reset */
#define EPLD5_PCIL0_INT2	0x20	/* PCIX0 INT2 status, write 0 to reset */
#define EPLD5_PCIL0_INT3	0x10	/* PCIX0 INT3 status, write 0 to reset */
#define EPLD5_PCIX1_INT0	0x08	/* PCIX1 INT0 status, write 0 to reset */
#define EPLD5_PCIX1_INT1	0x04	/* PCIX1 INT1 status, write 0 to reset */
#define EPLD5_PCIX1_INT2	0x02	/* PCIX1 INT2 status, write 0 to reset */
#define EPLD5_PCIX1_INT3	0x01	/* PCIX1 INT3 status, write 0 to reset */

#define EPLD6_PCIL0_RESET_CTL	0x80	/* 0=enable slot reset, 1=disable slot reset */
#define EPLD6_PCIX1_RESET_CTL	0x40	/* 0=enable slot reset, 1=disable slot reset */
#define EPLD6_ETH_INT_MODE	0x20	/* 0=IRQ5 recv's external eth int */
#define EPLD6_PCIX2_RESET_CTL	0x10	/* 0=enable slot reset, 1=disable slot reset */
#define EPLD6_PCI1_CLKCNTL1	0x80	/* PCI1 clock control S*0 of 9531 */
#define EPLD6_PCI1_CLKCNTL2	0x40	/* PCI1 clock control S*1 of 9531 */
#define EPLD6_PCI2_CLKCNTL1	0x20	/* PCI2 clock control S*0 of 9531 */
#define EPLD6_PCI2_CLKCNTL2	0x10	/* PCI2 clock control S*1 of 9531 */

#define EPLD7_VTH1		0x80	/* PCI2 VTH1 status */
#define EPLD7_VTH2		0x40	/* PCI2 VTH2 status */
#define EPLD7_VTH3		0x20	/* PCI2 VTH3 status */
#define EPLD7_VTH4		0x10	/* PCI2 VTH4 status */
#define EPLD7_INTA_MODE		0x80	/* see S5 on SW2 for details */
#define EPLD7_PCI_INT_MODE_N	0x40	/* see S1 on SW2 for details */
#define EPLD7_WRITE_ENABLE_GPIO	0x20	/* see S2 on SW2 for details */
#define EPLD7_WRITE_ENABLE_INT	0x10	/* see S3 on SW2 for details */


typedef struct {
    unsigned char  status;		/* misc status */
    unsigned char  clock;		/* clock status, PCI-X clock control */
    unsigned char  ethuart;		/* Ethernet, UART status */
    unsigned char  leds;		/* LED register */
    unsigned char  vth01;		/* PCI0, PCI1 VTH register */
    unsigned char  pciints;		/* PCI0, PCI1 interrupts */
    unsigned char  pci2;		/* PCI2 interrupts, clock control */
    unsigned char  vth2;		/* PCI2 VTH register */
    unsigned char  filler1[4096-8];
    unsigned char  gpio00;		/* GPIO bits  0-7 */
    unsigned char  gpio08;		/* GPIO bits  8-15 */
    unsigned char  gpio16;		/* GPIO bits 16-23 */
    unsigned char  gpio24;		/* GPIO bits 24-31 */
    unsigned char  filler2[4096-4];
    unsigned char  version;		/* EPLD version */
} epld_t;
