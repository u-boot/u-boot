/*
 * Harvested from Cadence driver.
 */

#ifndef __XZYNQETH_H__
#define __XZYNQETH_H__

#define XZYNQETH_RX_BUF_SIZE 128

#define XZYNQETH_RBQ_LENGTH 128
#define XZYNQETH_TBQ_LENGTH 128

#define XZYNQETH_MDIO_ENABLED (GEM_MDIO_EN)

#define XZYNQETH_DEF_PCLK_DIV (MDC_DIV_32)

#define XZYNQETH_DEF_AHB_WIDTH (AMBA_AHB_32)

#define XZYNQETH_DEF_DUPLEX (GEM_FULL_DUPLEX)

#define XZYNQETH_DEF_SPEED (SPEED_100M)

#define XZYNQETH_DEF_LOOP (LB_NONE)

#define XZYNQETH_READ_SNAP       (1<<14)     /* Read snapshot register */
#define XZYNQETH_TAKE_SNAP       (1<<13)     /* Take a snapshot */
/* Transmit zero quantum pause frame */
#define XZYNQETH_TX_0Q_PAUSE     (1<<12)
#define XZYNQETH_TX_PAUSE        (1<<11)     /* Transmit pause frame */
/* Halt transmission after curr frame */
#define XZYNQETH_TX_HALT         (1<<10)
#define XZYNQETH_TX_START        (1<<9)      /* Start tx (tx_go) */
/* Enable writing to stat registers */
#define XZYNQETH_STATS_WR_EN     (1<<7)
#define XZYNQETH_STATS_INC       (1<<6)      /* Increment statistic registers */
#define XZYNQETH_STATS_CLR       (1<<5)      /* Clear statistic registers */
#define XZYNQETH_MDIO_EN         (1<<4)      /* Enable MDIO port */
#define XZYNQETH_TX_EN           (1<<3)      /* Enable transmit circuits */
#define XZYNQETH_RX_EN           (1<<2)      /* Enable receive circuits */
#define XZYNQETH_LB_MAC          (1<<1)      /* Perform local loopback at MAC */
/* Perform ext loopback through PHY */
#define XZYNQETH_LB_PHY          (1<<0)

/* Do not copy pause frames to memory */
#define XZYNQETH_RX_NO_PAUSE     (1<<23)
#define XZYNQETH_AHB_WIDTH1      (1<<22)     /* Bit 1 for defining AHB width */
#define XZYNQETH_AHB_WIDTH0      (1<<21)     /* Bit 0 for defining AHB width */
#define XZYNQETH_MDC_DIV2        (1<<20)     /* PCLK divisor for MDC, bit 2 */
#define XZYNQETH_MDC_DIV1        (1<<19)     /* PCLK divisor for MDC, bit 1 */
#define XZYNQETH_MDC_DIV0        (1<<18)     /* PCLK divisor for MDC, bit 0 */
/* Discard FCS from received frames. */
#define XZYNQETH_RX_NO_FCS       (1<<17)
#define XZYNQETH_RX_LEN_CHK      (1<<16)     /* Receive length check. */
/* Pos of LSB for rx buffer offsets. */
#define XZYNQETH_RX_OFFSET_BASE  14
/* RX offset bit 1 */
#define XZYNQETH_RX_OFFSET1      (1<<(GEM_RX_OFFSET_BASE + 1))
/* RX offset bit 0 */
#define XZYNQETH_RX_OFFSET0      (1<<GEM_RX_OFFSET_BASE)
#define XZYNQETH_RX_PAUSE_EN     (1<<13)     /* Enable pause reception */
/* Retry test for speeding up debug */
#define XZYNQETH_RETRY_TEST      (1<<12)
#define XZYNQETH_PCS_SEL         (1<<11)     /* Select PCS */
#define XZYNQETH_GIG_MODE        (1<<10)     /* Gigabit mode enable */
#define XZYNQETH_EAM_EN          (1<<9)      /* External address match enable */
/* Enable 1536 byte frames reception */
#define XZYNQETH_FRAME_1536      (1<<8)
#define XZYNQETH_UNICAST_EN      (1<<7)      /* Receive unicast hash frames */
#define XZYNQETH_MULTICAST_EN    (1<<6)      /* Receive multicast hash frames */
/* Do not receive broadcast frames */
#define XZYNQETH_NO_BROADCAST    (1<<5)
#define XZYNQETH_COPY_ALL        (1<<4)      /* Copy all frames */
#define XZYNQETH_RX_JUMBO        (1<<3)      /* Allow jumbo frame reception */
#define XZYNQETH_VLAN_ONLY       (1<<2)      /* Receive only VLAN frames */
#define XZYNQETH_FULL_DUPLEX     (1<<1)      /* Enable full duplex */
#define XZYNQETH_SPEED_100       (1<<0)      /* Set to 100Mb mode */

#define XZYNQETH_PHY_IDLE        (1<<2)      /* PHY management is idle */
#define XZYNQETH_MDIO_IN         (1<<1)      /* Status of mdio_in pin */
#define XZYNQETH_LINK_STATUS     (1<<0)      /* Status of link pin */

#define XZYNQETH_TX_HRESP        (1<<8)      /* Transmit hresp not OK */
#define XZYNQETH_LATE_COL        (1<<7)      /* Late collision */
#define XZYNQETH_TX_URUN         (1<<6)      /* Transmit underrun occurred */
#define XZYNQETH_TX_COMPLETE     (1<<5)      /* Transmit completed OK */
/* Transmit buffs exhausted mid frame */
#define XZYNQETH_TX_BUF_EXH      (1<<4)
/* Status of tx_go internal variable */
#define XZYNQETH_TX_GO           (1<<3)
#define XZYNQETH_TX_RETRY_EXC    (1<<2)      /* Retry limit exceeded */
/* Collision occurred during frame tx */
#define XZYNQETH_TX_COL          (1<<1)
#define XZYNQETH_TX_USED         (1<<0)      /* Used bit read in tx buffer */

#define XZYNQETH_RX_HRESP        (1<<3)      /* Receive hresp not OK */
#define XZYNQETH_RX_ORUN         (1<<2)      /* Receive overrun occurred */
#define XZYNQETH_RX_DONE         (1<<1)      /* Frame successfully received */
#define XZYNQETH_RX_BUF_USED     (1<<0)      /* Receive buffer used bit read */

#define XZYNQETH_IRQ_PCS_AN      (1<<16)     /* PCS autonegotiation complete */
/* External interrupt pin triggered */
#define XZYNQETH_IRQ_EXT_INT     (1<<15)
#define XZYNQETH_IRQ_PAUSE_TX    (1<<14)     /* Pause frame transmitted */
#define XZYNQETH_IRQ_PAUSE_0     (1<<13)     /* Pause time has reached zero */
#define XZYNQETH_IRQ_PAUSE_RX    (1<<12)     /* Pause frame received */
#define XZYNQETH_IRQ_HRESP       (1<<11)     /* hresp not ok */
#define XZYNQETH_IRQ_RX_ORUN     (1<<10)     /* Receive overrun occurred */
#define XZYNQETH_IRQ_PCS_LINK    (1<<9)      /* Status of PCS link changed */
#define XZYNQETH_IRQ_TX_DONE     (1<<7)      /* Frame transmitted ok */
/* Transmit err occurred or no buffers*/
#define XZYNQETH_IRQ_TX_ERROR    (1<<6)
#define XZYNQETH_IRQ_RETRY_EXC   (1<<5)      /* Retry limit exceeded */
#define XZYNQETH_IRQ_TX_URUN     (1<<4)      /* Transmit underrun occurred */
#define XZYNQETH_IRQ_TX_USED     (1<<3)      /* Tx buffer used bit read */
#define XZYNQETH_IRQ_RX_USED     (1<<2)      /* Rx buffer used bit read */
#define XZYNQETH_IRQ_RX_DONE     (1<<1)      /* Frame received ok */
/* PHY management operation complete */
#define XZYNQETH_IRQ_MAN_DONE    (1<<0)
#define XZYNQETH_IRQ_ALL         (0xFFFFFFFF)/* Everything! */

#define XZYNQETH_TBQE_USED       (1<<31)     /* Used bit. */
#define XZYNQETH_TBQE_WRAP       (1<<30)     /* Wrap bit */
#define XZYNQETH_TBQE_RETRY_EXC  (1<<29)     /* Retry limit exceeded. */
#define XZYNQETH_TBQE_URUN       (1<<28)     /* Transmit underrun occurred. */
#define XZYNQETH_TBQE_BUF_EXH    (1<<27)     /* Buffers exhausted mid frame. */
#define XZYNQETH_TBQE_LATE_COL   (1<<26)     /* Late collision. */
#define XZYNQETH_TBQE_NO_CRC     (1<<16)     /* No CRC */
#define XZYNQETH_TBQE_LAST_BUF   (1<<15)     /* Last buffer */
#define XZYNQETH_TBQE_LEN_MASK   (0x3FFF)    /* Mask for length field */
#define XZYNQETH_TX_MAX_LEN      (0x3FFF)    /* Maximum transmit length value */
/* Dummy value to check for free buffer*/
#define XZYNQETH_TBQE_DUMMY      (0x8000BFFF)

#define XZYNQETH_RBQE_BROADCAST  (1<<31)     /* Broadcast frame */
#define XZYNQETH_RBQE_MULTICAST  (1<<30)     /* Multicast hashed frame */
#define XZYNQETH_RBQE_UNICAST    (1<<29)     /* Unicast hashed frame */
#define XZYNQETH_RBQE_EXT_ADDR   (1<<28)     /* External address match */
#define XZYNQETH_RBQE_SPEC_MATCH (1<<27)     /* Specific address matched */
/* Pos for base of specific match */
#define XZYNQETH_RBQE_SPEC_BASE  (25)
#define XZYNQETH_RBQE_SPEC_MAT1  (1<<(RBQE_SPEC_BASE + 1))
#define XZYNQETH_RBQE_SPEC_MAT0  (1<<RBQE_SPEC_BASE)
#define XZYNQETH_RBQE_TYPE_MATCH (1<<24)     /* Type ID matched */
/* Position for base of type id match */
#define XZYNQETH_RBQE_TYPE_BASE  (22)
#define XZYNQETH_RBQE_TYPE_MAT1  (1<<(RBQE_TYPE_BASE + 1))
#define XZYNQETH_RBQE_TYPE_MAT0  (1<<RBQE_TYPE_BASE)
#define XZYNQETH_RBQE_VLAN       (1<<21)     /* VLAN tagged */
#define XZYNQETH_RBQE_PRIORITY   (1<<20)     /* Priority tagged */
/* Position for base of VLAN priority */
#define XZYNQETH_RBQE_VLAN_BASE  (17)
#define XZYNQETH_RBQE_VLAN_P2    (1<<(RBQE_VLAN_BASE+2))
#define XZYNQETH_RBQE_VLAN_P1    (1<<(RBQE_VLAN_BASE+1))
#define XZYNQETH_RBQE_VLAN_P0    (1<<RBQE_VLAN_BASE)
#define XZYNQETH_RBQE_CFI        (1<<16)     /* CFI frame */
#define XZYNQETH_RBQE_EOF        (1<<15)     /* End of frame. */
#define XZYNQETH_RBQE_SOF        (1<<14)     /* Start of frame. */
#define XZYNQETH_RBQE_LEN_MASK   (0x3FFF)    /* Mask for the length field. */
#define XZYNQETH_RBQE_WRAP       (1<<1)      /* Wrap bit.. */
#define XZYNQETH_RBQE_USED       (1<<0)      /* Used bit.. */
#define XZYNQETH_RBQE_ADD_MASK   (0xFFFFFFFC)/* Mask for address */

#define XZYNQETH_REV_ID_MODEL_MASK   (0x000F0000)    /* Model ID */
#define XZYNQETH_REV_ID_MODEL_BASE   (16)            /* For Shifting */
#define XZYNQETH_REV_ID_REG_MODEL    (0x00020000)    /* GEM module ID */
#define XZYNQETH_REV_ID_REV_MASK     (0x0000FFFF)    /* Revision ID */

#define XZYNQETH_NET_CONTROL         (0x00)
#define XZYNQETH_NET_CONFIG          (0x04)
#define XZYNQETH_NET_STATUS          (0x08)
#define XZYNQETH_USER_IO             (0x0C)
#define XZYNQETH_TX_STATUS           (0x14)
#define XZYNQETH_RX_QPTR             (0x18)
#define XZYNQETH_TX_QPTR             (0x1C)
#define XZYNQETH_RX_STATUS           (0x20)
#define XZYNQETH_IRQ_STATUS          (0x24)
#define XZYNQETH_IRQ_ENABLE          (0x28)
#define XZYNQETH_IRQ_DISABLE         (0x2C)
#define XZYNQETH_IRQ_MASK            (0x30)
#define XZYNQETH_PHY_MAN             (0x34)
#define XZYNQETH_RX_PAUSE_TIME       (0x38)
#define XZYNQETH_TX_PAUSE_QUANT      (0x3C)

#define XZYNQETH_HASH_BOT            (0x80)
#define XZYNQETH_HASH_TOP            (0x84)
#define XZYNQETH_LADDR1_BOT          (0x88)
#define XZYNQETH_LADDR1_TOP          (0x8C)
#define XZYNQETH_LADDR2_BOT          (0x90)
#define XZYNQETH_LADDR2_TOP          (0x94)
#define XZYNQETH_LADDR3_BOT          (0x98)
#define XZYNQETH_LADDR3_TOP          (0x9C)
#define XZYNQETH_LADDR4_BOT          (0xA0)
#define XZYNQETH_LADDR4_TOP          (0xA4)
#define XZYNQETH_ID_CHECK1           (0xA8)
#define XZYNQETH_ID_CHECK2           (0xAC)
#define XZYNQETH_ID_CHECK3           (0xB0)
#define XZYNQETH_ID_CHECK4           (0xB4)
#define XZYNQETH_REV_ID              (0xFC)

#define XZYNQETH_OCT_TX_BOT          (0x100)
#define XZYNQETH_OCT_TX_TOP          (0x104)
#define XZYNQETH_STATS_FRAMES_TX     (0x108)
#define XZYNQETH_BROADCAST_TX        (0x10C)
#define XZYNQETH_MULTICAST_TX        (0x110)
#define XZYNQETH_STATS_PAUSE_TX      (0x114)
#define XZYNQETH_FRAME64_TX          (0x118)
#define XZYNQETH_FRAME65_TX          (0x11C)
#define XZYNQETH_FRAME128_TX         (0x120)
#define XZYNQETH_FRAME256_TX         (0x124)
#define XZYNQETH_FRAME512_TX         (0x128)
#define XZYNQETH_FRAME1024_TX        (0x12C)
#define XZYNQETH_FRAME1519_TX        (0x130)
#define XZYNQETH_STATS_TX_URUN       (0x134)
#define XZYNQETH_STATS_SINGLE_COL    (0x138)
#define XZYNQETH_STATS_MULTI_COL     (0x13C)
#define XZYNQETH_STATS_EXCESS_COL    (0x140)
#define XZYNQETH_STATS_LATE_COL      (0x144)
#define XZYNQETH_STATS_DEF_TX        (0x148)
#define XZYNQETH_STATS_CRS_ERRORS    (0x14C)
#define XZYNQETH_OCT_RX_BOT          (0x150)
#define XZYNQETH_OCT_RX_TOP          (0x154)
#define XZYNQETH_STATS_FRAMES_RX     (0x158)
#define XZYNQETH_BROADCAST_RX        (0x15C)
#define XZYNQETH_MULTICAST_RX        (0x160)
#define XZYNQETH_STATS_PAUSE_RX      (0x164)
#define XZYNQETH_FRAME64_RX          (0x168)
#define XZYNQETH_FRAME65_RX          (0x16C)
#define XZYNQETH_FRAME128_RX         (0x170)
#define XZYNQETH_FRAME256_RX         (0x174)
#define XZYNQETH_FRAME512_RX         (0x178)
#define XZYNQETH_FRAME1024_RX        (0x17C)
#define XZYNQETH_FRAME1519_RX        (0x180)
#define XZYNQETH_STATS_USIZE_FRAMES  (0x184)
#define XZYNQETH_STATS_EXCESS_LEN    (0x188)
#define XZYNQETH_STATS_JABBERS       (0x18C)
#define XZYNQETH_STATS_FCS_ERRORS    (0x190)
#define XZYNQETH_STATS_LENGTH_ERRORS (0x194)
#define XZYNQETH_STATS_RX_SYM_ERR    (0x198)
#define XZYNQETH_STATS_ALIGN_ERRORS  (0x19C)
#define XZYNQETH_STATS_RX_RES_ERR    (0x1a0)
#define XZYNQETH_STATS_RX_ORUN       (0x1a4)

#define XZYNQETH_REG_TOP             (0x23C)

#define gem_get_tbq_end(mac)    ((mac)->tbq_end)
#define gem_get_tbq_current(mac)    ((mac)->tbq_current)

#define gem_get_rbq_end(mac)    ((mac)->rbq_end)
#define gem_get_rbq_current(mac)    ((mac)->rbq_current)

#define gem_check_free_tbq_buf(mac) \

#define gem_check_a_free_tbq_buf(mac, index) \

#define gem_get_free_tbq_num(mac, free_tbq_num) \

