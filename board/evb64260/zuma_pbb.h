#ifndef ZUMA_PBB_H
#define ZUMA_PBB_H

#define MAX_NUM_BUFFER_PER_RING	32

#ifdef __BIG_ENDIAN
#define cpu_bits _be_s_bits	/* use with le32_to_cpu only */
#define pci_bits _be_bits	/* may contain swapped bytes,
				   but dont need le32_to_cpu */
#endif

#ifdef __LITTLE_ENDIAN
#define cpu_bits _le_bits
#define pci_bits _le_bits
#endif

#define VENDOR_ID_ZUMA		0x1172
#define DEVICE_ID_ZUMA_PBB	0x0004

#define RXDBP(chan)		(&sip->rx_desc[chan].base)	/* ch*8      */
#define RXDP(chan)		(&sip->rx_desc[chan].current)	/* ch*8 +  4 */
#define TXDBP(chan)		(&sip->tx_desc[chan].base)	/* ch*8 + 64 */
#define TXDP(chan)		(&sip->tx_desc[chan].current)	/* ch*8 + 68 */

#define PBB_DMA_OWN_BIT		0x80000000
#define PBB_DMA_LAST_BIT	0x40000000

#define EOF_RX_FLAG		1	/* bit 0 */
#define EOB_RX_FLAG		2	/* bit 1 */
#define EOF_TX_FLAG		4	/* bit 2 */
#define EOB_TX_FLAG		8	/* bit 3 */

#define TX_MODE(m)		(((m)&7) << 16)

#define RX_DESC(i)		(cs->rx_desc[i])
#define TX_DESC(i)		(cs->tx_desc[i])

#define RX_CONTROL(i)		(RX_DESC(i).control.word)
#define RX_CONTROL_SIZE(i)	(RX_DESC(i).control.rx.size)
#define TX_CONTROL(i)		(TX_DESC(i).control.word)

#define RX_DATA_P(i)		(&RX_DESC(i).ptr)
#define TX_DATA_P(i)		(&TX_DESC(i).ptr)

typedef volatile unsigned char V8;
typedef volatile unsigned short V16;
typedef volatile unsigned int V32;

/* RAM descriptor layout */
typedef struct _tag_dma_descriptor {
    V32 ptr;
    union {
	struct {
	    V32 owner:1;
	    V32 last:1;
	    V32 reserved0: 10;
	    V32 tx_mode: 4;

	    V32 reserved1: 5;
	    V32 size: 11;
	} tx;
	struct {
	    V32 owner:1;
	    V32 last:1;
	    V32 reserved0: 14;

	    V32 reserved1: 5;
	    V32 size: 11;
	} rx;
	V32 word;
    } control;
} DMA_DESCRIPTOR;

/*
 * NOTE: DO NOT USE structure to write non-word values... all registers
 * MUST be written 4 bytes at a time in SI version 0.
 * Non-word writes will result in "unaccessed" bytes written as zero.
 *
 * Byte reads are allowed.
 *
 * V32 pads are because the registers are spaced every 8 bytes (64 bits)
 *
 */

/* NOTE!!! 4 dwords */
typedef struct _tag_dma_descriptor_ring {
    DMA_DESCRIPTOR *base;
    V32 pad1;	/* skip high dword */
    volatile DMA_DESCRIPTOR *current;
    V32 pad3;	/* skip high dword */
} DMA_DESCRIPTOR_RING;

/* 1 dword */
typedef union _tag_dma_generic {
    struct {	/* byte 3 2 1 0 */
	V32 chan7:4;	/* bits 31-28 */
	V32 chan6:4;	/* bits 27-24 */
	V32 chan5:4;	/* bits 23-20 */
	V32 chan4:4;	/* bits 19-16 */
	V32 chan3:4;	/* bits 15-12 */
	V32 chan2:4;	/* bits 11-8 */
	V32 chan1:4;	/* bits 7-4 */
	V32 chan0:4;	/* bits 3-0 */
    } _be_s_bits;
    struct {	/* byte 0 1 2 3 */
	V32 chan1:4;	/* bits 7-4 */
	V32 chan0:4;	/* bits 3-0 */
	V32 chan3:4;	/* bits 15-12 */
	V32 chan2:4;	/* bits 11-8 */
	V32 chan5:4;	/* bits 23-20 */
	V32 chan4:4;	/* bits 19-16 */
	V32 chan7:4;	/* bits 31-28 */
	V32 chan6:4;	/* bits 27-24 */
    } _be_bits;
    struct {	/* byte 0 1 2 3 */
	V32 chan0:4;	/* bits 0-3 */
	V32 chan1:4;	/* bits 4-7 */
	V32 chan2:4;	/* bits 8-11 */
	V32 chan3:4;	/* bits 12-15 */
	V32 chan4:4;	/* bits 16-19 */
	V32 chan5:4;	/* bits 20-23 */
	V32 chan6:4;	/* bits 24-27 */
	V32 chan7:4;	/* bits 28-31 */
    } _le_bits;
    V8 byte[4];
    V32 word;
} DMA_RXTX_ENABLE, DMA_RX_DELETE,
  DMA_INT_STATUS, DMA_INT_MASK,
  DMA_RX_LEVEL_STATUS, DMA_RX_LEVEL_INT_MASK;

/* 1 dword */
typedef union _tag_dma_rx_timer{
    struct {
	V32 res0:8;	/* bits 32-24 */
	V32 res1:7;	/* bits 23-17 */
	V32 enable:1;	/* bit 16 */
	V32 value:16;	/* bits 15-0 */
    } _be_s_bits;
    struct {
	/* crosses byte boundary. must use swap. */
	V32 s_value:16;	/* bits 7-0,15-8 */
	V32 enable:1;	/* bit 16 */
	V32 res1:7;	/* bits 23-17 */
	V32 res0:8;	/* bits 32-24 */
    } _be_bits;
    struct {
	V32 value:16;	/* bits 0-15 */
	V32 enable:1;	/* bit 16 */
	V32 res1:7;	/* bits 17-23 */
	V32 res0:8;	/* bits 24-32 */
    } _le_bits;
    V8 byte[4];
    V32 word;
} DMA_RX_TIMER;

/* NOTE!!!: 2 dwords */
typedef struct _tag_dma_desc_level{
    union {
	struct {
	    V32 res1:8;	/* bits 31-24 */
	    V32 res0:7;	/* bits 23-17 */
	    V32 write:1;	/* bit 16 */
	    V32 thresh:8;	/* bits 15-8 */
	    V32 level:8;	/* bits 7-0 */
	} _be_s_bits;
	struct {
	    V32 level:8;	/* bits 7-0 */
	    V32 thresh:8;	/* bits 15-8 */
	    V32 res0:7;	/* bits 30-17 */
	    V32 write:1;	/* bit 16 */
	    V32 res1:8;	/* bits 31-24 */
	} _be_bits;
	struct {
	    V32 level:8;	/* bits 0-7 */
	    V32 thresh:8;	/* bits 8-15 */
	    V32 write:1;	/* bit 16 */
	    V32 res0:7;	/* bit 17-30 */
	    V32 res1:8;	/* bits 24-31 */
	} _le_bits;
	V8 byte[4];
	V32 word;
    } desc;
    V32 pad1;
} DMA_DESC_LEVEL;

typedef struct _tag_pbb_dma_reg_map {
    /* 0-15 (0x000-0x078) */
    DMA_DESCRIPTOR_RING rx_desc[8];	/* 4 dwords each, 128 bytes tot. */

    /* 16-31 (0x080-0x0f8) */
    DMA_DESCRIPTOR_RING tx_desc[8];	/* 4 dwords each, 128 bytes tot. */

    /* 32/33 (0x100/0x108) */
    V32 reserved_32;
    V32 pad_32;
    V32 reserved_33;
    V32 pad_33;

    /* 34 (0x110) */
    DMA_RXTX_ENABLE rxtx_enable;
    V32 pad_34;

    /* 35 (0x118) */
    DMA_RX_DELETE rx_delete;
    V32 pad_35;

    /* 36-38 (0x120-0x130) */
    DMA_INT_STATUS status;
    V32 pad_36;
    DMA_INT_STATUS last_status;
    V32 pad_37;
    DMA_INT_MASK int_mask;
    V32 pad_38;

    /* 39/40 (0x138/0x140) */
    union {
	/* NOTE!! 4 dwords */
	struct {
	    V32 channel_3:8;
	    V32 channel_2:8;
	    V32 channel_1:8;
	    V32 channel_0:8;
	    V32 pad1;
	    V32 channel_7:8;
	    V32 channel_6:8;
	    V32 channel_5:8;
	    V32 channel_4:8;
	    V32 pad3;
	} _be_s_bits;
	struct {
	    V32 channel_0:8;
	    V32 channel_1:8;
	    V32 channel_2:8;
	    V32 channel_3:8;
	    V32 pad1;
	    V32 channel_4:8;
	    V32 channel_5:8;
	    V32 channel_6:8;
	    V32 channel_7:8;
	    V32 pad3;
	} _be_bits, _le_bits;
	V8 byte[16];
	V32 word[4];
    } rx_size;

    /* 41/42 (0x148/0x150) */
    V32 reserved_41;
    V32 pad_41;
    V32 reserved_42;
    V32 pad_42;

    /* 43/44 (0x158/0x160) */
    DMA_RX_LEVEL_STATUS rx_level_status;
    V32 pad_43;
    DMA_RX_LEVEL_INT_MASK rx_level_int_mask;
    V32 pad_44;

    /* 45 (0x168) */
    DMA_RX_TIMER rx_timer;
    V32 pad_45;

    /* 46 (0x170) */
    V32 reserved_46;
    V32 pad_46;

    /* 47 (0x178) */
    V32 mbox_status;
    V32 pad_47;

    /* 48/49 (0x180/0x188) */
    V32 mbox_out;
    V32 pad_48;
    V32 mbox_in;
    V32 pad_49;

    /* 50 (0x190) */
    V32 config;
    V32 pad_50;

    /* 51/52 (0x198/0x1a0) */
    V32 c2a_ctr;
    V32 pad_51;
    V32 a2c_ctr;
    V32 pad_52;

    /* 53 (0x1a8) */
    union {
	struct {
	    V32 rev_major:8;	/* bits 31-24 */
	    V32 rev_minor:8;	/* bits 23-16 */
	    V32 reserved:16;	/* bits 15-0 */
	} _be_s_bits;
	struct {
	    V32 s_reserved:16;	/* bits 7-0, 15-8 */
	    V32 rev_minor:8;	/* bits 23-16 */
	    V32 rev_major:8;	/* bits 31-24 */
	} _be_bits;
	struct {
	    V32 reserved:16;	/* bits 0-15 */
	    V32 rev_minor:8;	/* bits 16-23 */
	    V32 rev_major:8;	/* bits 24-31 */
	} _le_bits;
	V8 byte[4];
	V32 word;
    } version;
    V32 pad_53;

    /* 54-59 (0x1b0-0x1d8) */
    V32 debug_54;
    V32 pad_54;
    V32 debug_55;
    V32 pad_55;
    V32 debug_56;
    V32 pad_56;
    V32 debug_57;
    V32 pad_57;
    V32 debug_58;
    V32 pad_58;
    V32 debug_59;
    V32 pad_59;

    /* 60 (0x1e0) */
    V32 timestamp;
    V32 pad_60;

    /* 61-63 (0x1e8-0x1f8) */
    V32 debug_61;
    V32 pad_61;
    V32 debug_62;
    V32 pad_62;
    V32 debug_63;
    V32 pad_63;

    /* 64-71 (0x200 - 0x238) */
    DMA_DESC_LEVEL rx_desc_level[8];	/* 2 dwords each, 32 bytes tot. */

    /* 72-98 (0x240 - 0x2f8) */
    /* reserved */

    /* 96-127 (0x300 - 0x3f8) */
    /* mirrors (0x100 - 0x1f8) */

} PBB_DMA_REG_MAP;


#endif /* ZUMA_PBB_H */
