#define PHYADDR			0x01
#define NO_PHY_REGS		0x20

#define DEFAULT_PHY_PHYID1	0x0007
#define DEFAULT_PHY_PHYID2	0xC0A3
#define PHY_MODECTL		0x00
#define PHY_MODESTAT		0x01
#define PHY_PHYID1		0x02
#define PHY_PHYID2		0x03
#define PHY_ANAR		0x04
#define PHY_ANLPAR		0x05
#define PHY_ANER		0x06

#define PHY_RESET		0x8000
#define PHY_ANEG_EN		0x1000
#define PHY_DUPLEX		0x0100
#define PHY_SPD_SET		0x2000

#define RECV_BUFSIZE		(0x614)

typedef volatile u32 reg32;
typedef volatile u16 reg16;

typedef struct ADI_DMA_CONFIG_REG {
	u16 b_DMA_EN:1;		/* 0	Enabled				*/
	u16 b_WNR:1;		/* 1	Direction			*/
	u16 b_WDSIZE:2;		/* 2:3	Transfer word size		*/
	u16 b_DMA2D:1;		/* 4	DMA mode			*/
	u16 b_RESTART:1;	/* 5	Retain FIFO			*/
	u16 b_DI_SEL:1;		/* 6	Data interrupt timing select	*/
	u16 b_DI_EN:1;		/* 7	Data interrupt enabled		*/
	u16 b_NDSIZE:4;		/* 8:11	Flex descriptor size		*/
	u16 b_FLOW:3;		/* 12:14Flow				*/
} ADI_DMA_CONFIG_REG;

typedef struct adi_ether_frame_buffer {
	u16 NoBytes;		/* the no. of following bytes	*/
	u8 Dest[6];		/* destination MAC address	*/
	u8 Srce[6];		/* source MAC address		*/
	u16 LTfield;		/* length/type field		*/
	u8 Data[0];		/* payload bytes		*/
} ADI_ETHER_FRAME_BUFFER;
/* 16 bytes/struct	*/

typedef struct dma_descriptor {
	struct dma_descriptor *NEXT_DESC_PTR;
	u32 START_ADDR;
	ADI_DMA_CONFIG_REG CONFIG;
} DMA_DESCRIPTOR;
/* 10 bytes/struct in 12 bytes */

typedef struct adi_ether_buffer {
	DMA_DESCRIPTOR Dma[2];		/* first for the frame, second for the status */
	ADI_ETHER_FRAME_BUFFER *FrmData;/* pointer to data */
	struct adi_ether_buffer *pNext;	/* next buffer */
	struct adi_ether_buffer *pPrev;	/* prev buffer */
	u16 IPHdrChksum;		/* the IP header checksum */
	u16 IPPayloadChksum;		/* the IP header and payload checksum */
	volatile u32 StatusWord;	/* the frame status word */
} ADI_ETHER_BUFFER;
/* 40 bytes/struct in 44 bytes */

void SetupMacAddr(u8 * MACaddr);

void PollMdcDone(void);
void WrPHYReg(u16 PHYAddr, u16 RegAddr, u16 Data);
u16 RdPHYReg(u16 PHYAddr, u16 RegAddr);
void SoftResetPHY(void);
void DumpPHYRegs(void);

int SetupSystemRegs(int *opmode);
