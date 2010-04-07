/*
 * Harvested from Cadence driver.
 */

#ifndef __XDFETH_H__
#define __XDFETH_H__

#define XDFETH_RX_BUF_SIZE 128

#define XDFETH_RBQ_LENGTH 128
#define XDFETH_TBQ_LENGTH 128

#define XDFETH_MDIO_ENABLED (GEM_MDIO_EN)

#define XDFETH_DEF_PCLK_DIV (MDC_DIV_32)

#define XDFETH_DEF_AHB_WIDTH (AMBA_AHB_32)

#define XDFETH_DEF_DUPLEX (GEM_FULL_DUPLEX)

#define XDFETH_DEF_SPEED (SPEED_100M)

#define XDFETH_DEF_LOOP (LB_NONE)

#define XDFETH_READ_SNAP       (1<<14)     /* Read snapshot register */
#define XDFETH_TAKE_SNAP       (1<<13)     /* Take a snapshot */
#define XDFETH_TX_0Q_PAUSE     (1<<12)     /* Transmit zero quantum pause frame */
#define XDFETH_TX_PAUSE        (1<<11)     /* Transmit pause frame */
#define XDFETH_TX_HALT         (1<<10)     /* Halt transmission after curr frame */
#define XDFETH_TX_START        (1<<9)      /* Start tx (tx_go) */
#define XDFETH_STATS_WR_EN     (1<<7)      /* Enable writing to stat registers */
#define XDFETH_STATS_INC       (1<<6)      /* Increment statistic registers */
#define XDFETH_STATS_CLR       (1<<5)      /* Clear statistic registers */
#define XDFETH_MDIO_EN         (1<<4)      /* Enable MDIO port */
#define XDFETH_TX_EN           (1<<3)      /* Enable transmit circuits */
#define XDFETH_RX_EN           (1<<2)      /* Enable receive circuits */
#define XDFETH_LB_MAC          (1<<1)      /* Perform local loopback at MAC */
#define XDFETH_LB_PHY          (1<<0)      /* Perform ext loopback through PHY */

#define XDFETH_RX_NO_PAUSE     (1<<23)     /* Do not copy pause frames to memory */
#define XDFETH_AHB_WIDTH1      (1<<22)     /* Bit 1 for defining AHB width */
#define XDFETH_AHB_WIDTH0      (1<<21)     /* Bit 0 for defining AHB width */
#define XDFETH_MDC_DIV2        (1<<20)     /* PCLK divisor for MDC, bit 2 */
#define XDFETH_MDC_DIV1        (1<<19)     /* PCLK divisor for MDC, bit 1 */
#define XDFETH_MDC_DIV0        (1<<18)     /* PCLK divisor for MDC, bit 0 */
#define XDFETH_RX_NO_FCS       (1<<17)     /* Discard FCS from received frames. */
#define XDFETH_RX_LEN_CHK      (1<<16)     /* Receive length check. */
#define XDFETH_RX_OFFSET_BASE  14          /* Pos of LSB for rx buffer offsets. */
#define XDFETH_RX_OFFSET1      (1<<(GEM_RX_OFFSET_BASE + 1)) /* RX offset bit 1 */
#define XDFETH_RX_OFFSET0      (1<<GEM_RX_OFFSET_BASE)       /* RX offset bit 0 */
#define XDFETH_RX_PAUSE_EN     (1<<13)     /* Enable pause reception */
#define XDFETH_RETRY_TEST      (1<<12)     /* Retry test for speeding up debug */
#define XDFETH_PCS_SEL         (1<<11)     /* Select PCS */
#define XDFETH_GIG_MODE        (1<<10)     /* Gigabit mode enable */
#define XDFETH_EAM_EN          (1<<9)      /* External address match enable */
#define XDFETH_FRAME_1536      (1<<8)      /* Enable 1536 byte frames reception */
#define XDFETH_UNICAST_EN      (1<<7)      /* Receive unicast hash frames */
#define XDFETH_MULTICAST_EN    (1<<6)      /* Receive multicast hash frames */
#define XDFETH_NO_BROADCAST    (1<<5)      /* Do not receive broadcast frames */
#define XDFETH_COPY_ALL        (1<<4)      /* Copy all frames */
#define XDFETH_RX_JUMBO        (1<<3)      /* Allow jumbo frame reception */
#define XDFETH_VLAN_ONLY       (1<<2)      /* Receive only VLAN frames */
#define XDFETH_FULL_DUPLEX     (1<<1)      /* Enable full duplex */
#define XDFETH_SPEED_100       (1<<0)      /* Set to 100Mb mode */

#define XDFETH_PHY_IDLE        (1<<2)      /* PHY management is idle */
#define XDFETH_MDIO_IN         (1<<1)      /* Status of mdio_in pin */
#define XDFETH_LINK_STATUS     (1<<0)      /* Status of link pin */

#define XDFETH_TX_HRESP        (1<<8)      /* Transmit hresp not OK */
#define XDFETH_LATE_COL        (1<<7)      /* Late collision */
#define XDFETH_TX_URUN         (1<<6)      /* Transmit underrun occurred */
#define XDFETH_TX_COMPLETE     (1<<5)      /* Transmit completed OK */
#define XDFETH_TX_BUF_EXH      (1<<4)      /* Transmit buffs exhausted mid frame */
#define XDFETH_TX_GO           (1<<3)      /* Status of tx_go internal variable */
#define XDFETH_TX_RETRY_EXC    (1<<2)      /* Retry limit exceeded */
#define XDFETH_TX_COL          (1<<1)      /* Collision occurred during frame tx */
#define XDFETH_TX_USED         (1<<0)      /* Used bit read in tx buffer */

#define XDFETH_RX_HRESP        (1<<3)      /* Receive hresp not OK */
#define XDFETH_RX_ORUN         (1<<2)      /* Receive overrun occurred */
#define XDFETH_RX_DONE         (1<<1)      /* Frame successfully received */
#define XDFETH_RX_BUF_USED     (1<<0)      /* Receive buffer used bit read */

#define XDFETH_IRQ_PCS_AN      (1<<16)     /* PCS autonegotiation complete */
#define XDFETH_IRQ_EXT_INT     (1<<15)     /* External interrupt pin triggered */
#define XDFETH_IRQ_PAUSE_TX    (1<<14)     /* Pause frame transmitted */
#define XDFETH_IRQ_PAUSE_0     (1<<13)     /* Pause time has reached zero */
#define XDFETH_IRQ_PAUSE_RX    (1<<12)     /* Pause frame received */
#define XDFETH_IRQ_HRESP       (1<<11)     /* hresp not ok */
#define XDFETH_IRQ_RX_ORUN     (1<<10)     /* Receive overrun occurred */
#define XDFETH_IRQ_PCS_LINK    (1<<9)      /* Status of PCS link changed */
#define XDFETH_IRQ_TX_DONE     (1<<7)      /* Frame transmitted ok */
#define XDFETH_IRQ_TX_ERROR    (1<<6)      /* Transmit err occurred or no buffers*/
#define XDFETH_IRQ_RETRY_EXC   (1<<5)      /* Retry limit exceeded */
#define XDFETH_IRQ_TX_URUN     (1<<4)      /* Transmit underrun occurred */
#define XDFETH_IRQ_TX_USED     (1<<3)      /* Tx buffer used bit read */
#define XDFETH_IRQ_RX_USED     (1<<2)      /* Rx buffer used bit read */
#define XDFETH_IRQ_RX_DONE     (1<<1)      /* Frame received ok */
#define XDFETH_IRQ_MAN_DONE    (1<<0)      /* PHY management operation complete */
#define XDFETH_IRQ_ALL         (0xFFFFFFFF)/* Everything! */

#define XDFETH_TBQE_USED       (1<<31)     /* Used bit. */
#define XDFETH_TBQE_WRAP       (1<<30)     /* Wrap bit */
#define XDFETH_TBQE_RETRY_EXC  (1<<29)     /* Retry limit exceeded. */
#define XDFETH_TBQE_URUN       (1<<28)     /* Transmit underrun occurred. */
#define XDFETH_TBQE_BUF_EXH    (1<<27)     /* Buffers exhausted mid frame. */
#define XDFETH_TBQE_LATE_COL   (1<<26)     /* Late collision. */
#define XDFETH_TBQE_NO_CRC     (1<<16)     /* No CRC */
#define XDFETH_TBQE_LAST_BUF   (1<<15)     /* Last buffer */
#define XDFETH_TBQE_LEN_MASK   (0x3FFF)    /* Mask for length field */
#define XDFETH_TX_MAX_LEN      (0x3FFF)    /* Maximum transmit length value */
#define XDFETH_TBQE_DUMMY      (0x8000BFFF)/* Dummy value to check for free buffer*/

#define XDFETH_RBQE_BROADCAST  (1<<31)     /* Broadcast frame */
#define XDFETH_RBQE_MULTICAST  (1<<30)     /* Multicast hashed frame */
#define XDFETH_RBQE_UNICAST    (1<<29)     /* Unicast hashed frame */
#define XDFETH_RBQE_EXT_ADDR   (1<<28)     /* External address match */
#define XDFETH_RBQE_SPEC_MATCH (1<<27)     /* Specific address matched */
#define XDFETH_RBQE_SPEC_BASE  (25)        /* Pos for base of specific match */
#define XDFETH_RBQE_SPEC_MAT1  (1<<(RBQE_SPEC_BASE + 1))
#define XDFETH_RBQE_SPEC_MAT0  (1<<RBQE_SPEC_BASE)
#define XDFETH_RBQE_TYPE_MATCH (1<<24)     /* Type ID matched */
#define XDFETH_RBQE_TYPE_BASE  (22)        /* Position for base of type id match */
#define XDFETH_RBQE_TYPE_MAT1  (1<<(RBQE_TYPE_BASE + 1))
#define XDFETH_RBQE_TYPE_MAT0  (1<<RBQE_TYPE_BASE)
#define XDFETH_RBQE_VLAN       (1<<21)     /* VLAN tagged */
#define XDFETH_RBQE_PRIORITY   (1<<20)     /* Priority tagged */
#define XDFETH_RBQE_VLAN_BASE  (17)        /* Position for base of VLAN priority */
#define XDFETH_RBQE_VLAN_P2    (1<<(RBQE_VLAN_BASE+2))
#define XDFETH_RBQE_VLAN_P1    (1<<(RBQE_VLAN_BASE+1))
#define XDFETH_RBQE_VLAN_P0    (1<<RBQE_VLAN_BASE)
#define XDFETH_RBQE_CFI        (1<<16)     /* CFI frame */
#define XDFETH_RBQE_EOF        (1<<15)     /* End of frame. */
#define XDFETH_RBQE_SOF        (1<<14)     /* Start of frame. */
#define XDFETH_RBQE_LEN_MASK   (0x3FFF)    /* Mask for the length field. */
#define XDFETH_RBQE_WRAP       (1<<1)      /* Wrap bit.. */
#define XDFETH_RBQE_USED       (1<<0)      /* Used bit.. */
#define XDFETH_RBQE_ADD_MASK   (0xFFFFFFFC)/* Mask for address */

#define XDFETH_REV_ID_MODEL_MASK   (0x000F0000)    /* Model ID */
#define XDFETH_REV_ID_MODEL_BASE   (16)            /* For Shifting */
#define XDFETH_REV_ID_REG_MODEL    (0x00020000)    /* GEM module ID */
#define XDFETH_REV_ID_REV_MASK     (0x0000FFFF)    /* Revision ID */

#define XDFETH_NET_CONTROL         (0x00)
#define XDFETH_NET_CONFIG          (0x04)
#define XDFETH_NET_STATUS          (0x08)
#define XDFETH_USER_IO             (0x0C)
#define XDFETH_TX_STATUS           (0x14)
#define XDFETH_RX_QPTR             (0x18)
#define XDFETH_TX_QPTR             (0x1C)
#define XDFETH_RX_STATUS           (0x20)
#define XDFETH_IRQ_STATUS          (0x24)
#define XDFETH_IRQ_ENABLE          (0x28)
#define XDFETH_IRQ_DISABLE         (0x2C)
#define XDFETH_IRQ_MASK            (0x30)
#define XDFETH_PHY_MAN             (0x34)
#define XDFETH_RX_PAUSE_TIME       (0x38)
#define XDFETH_TX_PAUSE_QUANT      (0x3C)

#define XDFETH_HASH_BOT            (0x80)
#define XDFETH_HASH_TOP            (0x84)
#define XDFETH_LADDR1_BOT          (0x88)
#define XDFETH_LADDR1_TOP          (0x8C)
#define XDFETH_LADDR2_BOT          (0x90)
#define XDFETH_LADDR2_TOP          (0x94)
#define XDFETH_LADDR3_BOT          (0x98)
#define XDFETH_LADDR3_TOP          (0x9C)
#define XDFETH_LADDR4_BOT          (0xA0)
#define XDFETH_LADDR4_TOP          (0xA4)
#define XDFETH_ID_CHECK1           (0xA8)
#define XDFETH_ID_CHECK2           (0xAC)
#define XDFETH_ID_CHECK3           (0xB0)
#define XDFETH_ID_CHECK4           (0xB4)
#define XDFETH_REV_ID              (0xFC)

#define XDFETH_OCT_TX_BOT          (0x100)
#define XDFETH_OCT_TX_TOP          (0x104)
#define XDFETH_STATS_FRAMES_TX     (0x108)
#define XDFETH_BROADCAST_TX        (0x10C)
#define XDFETH_MULTICAST_TX        (0x110)
#define XDFETH_STATS_PAUSE_TX      (0x114)
#define XDFETH_FRAME64_TX          (0x118)
#define XDFETH_FRAME65_TX          (0x11C)
#define XDFETH_FRAME128_TX         (0x120)
#define XDFETH_FRAME256_TX         (0x124)
#define XDFETH_FRAME512_TX         (0x128)
#define XDFETH_FRAME1024_TX        (0x12C)
#define XDFETH_FRAME1519_TX        (0x130)
#define XDFETH_STATS_TX_URUN       (0x134)
#define XDFETH_STATS_SINGLE_COL    (0x138)
#define XDFETH_STATS_MULTI_COL     (0x13C)
#define XDFETH_STATS_EXCESS_COL    (0x140)
#define XDFETH_STATS_LATE_COL      (0x144)
#define XDFETH_STATS_DEF_TX        (0x148)
#define XDFETH_STATS_CRS_ERRORS    (0x14C)
#define XDFETH_OCT_RX_BOT          (0x150)
#define XDFETH_OCT_RX_TOP          (0x154)
#define XDFETH_STATS_FRAMES_RX     (0x158)
#define XDFETH_BROADCAST_RX        (0x15C)
#define XDFETH_MULTICAST_RX        (0x160)
#define XDFETH_STATS_PAUSE_RX      (0x164)
#define XDFETH_FRAME64_RX          (0x168)
#define XDFETH_FRAME65_RX          (0x16C)
#define XDFETH_FRAME128_RX         (0x170)
#define XDFETH_FRAME256_RX         (0x174)
#define XDFETH_FRAME512_RX         (0x178)
#define XDFETH_FRAME1024_RX        (0x17C)
#define XDFETH_FRAME1519_RX        (0x180)
#define XDFETH_STATS_USIZE_FRAMES  (0x184)
#define XDFETH_STATS_EXCESS_LEN    (0x188)
#define XDFETH_STATS_JABBERS       (0x18C)
#define XDFETH_STATS_FCS_ERRORS    (0x190)
#define XDFETH_STATS_LENGTH_ERRORS (0x194)
#define XDFETH_STATS_RX_SYM_ERR    (0x198)
#define XDFETH_STATS_ALIGN_ERRORS  (0x19C)
#define XDFETH_STATS_RX_RES_ERR    (0x1a0)
#define XDFETH_STATS_RX_ORUN       (0x1a4)

#define XDFETH_REG_TOP             (0x23C)

#define gem_get_tbq_end(mac)    ((mac)->tbq_end)
#define gem_get_tbq_current(mac)    ((mac)->tbq_current)

#define gem_get_rbq_end(mac)    ((mac)->rbq_end)
#define gem_get_rbq_current(mac)    ((mac)->rbq_current)

#define gem_check_free_tbq_buf(mac) \

#define gem_check_a_free_tbq_buf(mac, index) \

#define gem_get_free_tbq_num(mac, free_tbq_num) \

