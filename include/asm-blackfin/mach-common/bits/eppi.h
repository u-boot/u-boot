/*
 * Enhanced PPI (EPPI)
 */

#ifndef __BFIN_PERIPHERAL_EPPI__
#define __BFIN_PERIPHERAL_EPPI__

/* Bit masks for EPPIx_STATUS */
#define CFIFO_ERR              0x0001        /* Chroma FIFO Error */
#define YFIFO_ERR              0x0002        /* Luma FIFO Error */
#define LTERR_OVR              0x0004        /* Line Track Overflow */
#define LTERR_UNDR             0x0008        /* Line Track Underflow */
#define FTERR_OVR              0x0010        /* Frame Track Overflow */
#define FTERR_UNDR             0x0020        /* Frame Track Underflow */
#define ERR_NCOR               0x0040        /* Preamble Error Not Corrected */
#define DMA1URQ                0x0080        /* DMA1 Urgent Request */
#define DMA0URQ                0x0100        /* DMA0 Urgent Request */
#define ERR_DET                0x4000        /* Preamble Error Detected */
#define FLD                    0x8000        /* Field */

/* Bit masks for EPPIx_CONTROL */
#define EPPI_EN                0x00000001    /* Enable */
#define EPPI_DIR               0x00000002    /* Direction */
#define XFR_TYPE               0x0000000c    /* Operating Mode */
#define FS_CFG                 0x00000030    /* Frame Sync Configuration */
#define FLD_SEL                0x00000040    /* Field Select/Trigger */
#define ITU_TYPE               0x00000080    /* ITU Interlaced or Progressive */
#define BLANKGEN               0x00000100    /* ITU Output Mode with Internal Blanking Generation */
#define ICLKGEN                0x00000200    /* Internal Clock Generation */
#define IFSGEN                 0x00000400    /* Internal Frame Sync Generation */
#define POLC                   0x00001800    /* Frame Sync and Data Driving/Sampling Edges */
#define POLS                   0x00006000    /* Frame Sync Polarity */
#define DLENGTH                0x00038000    /* Data Length */
#define SKIP_EN                0x00040000    /* Skip Enable */
#define SKIP_EO                0x00080000    /* Skip Even or Odd */
#define PACKEN                 0x00100000    /* Packing/Unpacking Enable */
#define SWAPEN                 0x00200000    /* Swap Enable */
#define SIGN_EXT               0x00400000    /* Sign Extension or Zero-filled / Data Split Format */
#define SPLT_EVEN_ODD          0x00800000    /* Split Even and Odd Data Samples */
#define SUBSPLT_ODD            0x01000000    /* Sub-split Odd Samples */
#define DMACFG                 0x02000000    /* One or Two DMA Channels Mode */
#define RGB_FMT_EN             0x04000000    /* RGB Formatting Enable */
#define FIFO_RWM               0x18000000    /* FIFO Regular Watermarks */
#define FIFO_UWM               0x60000000    /* FIFO Urgent Watermarks */

#define DLEN_8                 (0 << 15)     /* 000 - 8 bits */
#define DLEN_10                (1 << 15)     /* 001 - 10 bits */
#define DLEN_12                (2 << 15)     /* 010 - 12 bits */
#define DLEN_14                (3 << 15)     /* 011 - 14 bits */
#define DLEN_16                (4 << 15)     /* 100 - 16 bits */
#define DLEN_18                (5 << 15)     /* 101 - 18 bits */
#define DLEN_24                (6 << 15)     /* 110 - 24 bits */

/* Bit masks for EPPIx_FS2W_LVB */
#define F1VB_BD                0x000000ff    /* Vertical Blanking before Field 1 Active Data */
#define F1VB_AD                0x0000ff00    /* Vertical Blanking after Field 1 Active Data */
#define F2VB_BD                0x00ff0000    /* Vertical Blanking before Field 2 Active Data */
#define F2VB_AD                0xff000000    /* Vertical Blanking after Field 2 Active Data */

/* Bit masks for EPPIx_FS2W_LAVF */
#define F1_ACT                 0x0000ffff    /* Number of Lines of Active Data in Field 1 */
#define F2_ACT                 0xffff0000    /* Number of Lines of Active Data in Field 2 */

/* Bit masks for EPPIx_CLIP */
#define LOW_ODD                0x000000ff    /* Lower Limit for Odd Bytes (Chroma) */
#define HIGH_ODD               0x0000ff00    /* Upper Limit for Odd Bytes (Chroma) */
#define LOW_EVEN               0x00ff0000    /* Lower Limit for Even Bytes (Luma) */
#define HIGH_EVEN              0xff000000    /* Upper Limit for Even Bytes (Luma) */

#endif
