/* PCI.h - PCI functions header file */

/* Copyright - Galileo technology. */

#ifndef __INCpcih
#define __INCpcih

/* includes */

#include"core.h"
#include"memory.h"

/* According to PCI REV 2.1 MAX agents allowed on the bus are -21- */
#define PCI_MAX_DEVICES 22


/* Macros */

/* The next Macros configurate the initiator board (SELF) or any any agent on
   the PCI to become: MASTER, response to MEMORY transactions , response to
   IO transactions or TWO both MEMORY_IO transactions. Those configuration
   are for both PCI0 and PCI1. */

#define PCI_MEMORY_ENABLE(host, deviceNumber) pciWriteConfigReg(host,	       \
	  PCI_STATUS_AND_COMMAND,deviceNumber,MEMORY_ENABLE |		     \
	  pciReadConfigReg(host, PCI_STATUS_AND_COMMAND,deviceNumber) )

#define PCI_IO_ENABLE(host, deviceNumber) pciWriteConfigReg(host,	       \
	  PCI_STATUS_AND_COMMAND,deviceNumber,I_O_ENABLE |		     \
	  pciReadConfigReg(host, PCI_STATUS_AND_COMMAND,deviceNumber) )

#define PCI_SLAVE_ENABLE(host, deviceNumber) pciWriteConfigReg(host,	       \
	  PCI_STATUS_AND_COMMAND,deviceNumber,MEMORY_ENABLE | I_O_ENABLE |   \
	  pciReadConfigReg(host, PCI_STATUS_AND_COMMAND,deviceNumber) )

#define PCI_DISABLE(host, deviceNumber) pciWriteConfigReg(host,		       \
	  PCI_STATUS_AND_COMMAND,deviceNumber,0xfffffff8  &		     \
	  pciReadConfigReg(host, PCI_STATUS_AND_COMMAND,deviceNumber))

#define PCI_MASTER_ENABLE(host,deviceNumber) pciWriteConfigReg(host,	       \
	  PCI_STATUS_AND_COMMAND,deviceNumber,MASTER_ENABLE |		     \
	  pciReadConfigReg(host,PCI_STATUS_AND_COMMAND,deviceNumber) )

#define PCI_MASTER_DISABLE(deviceNumber) pciWriteConfigReg(host,	      \
	  PCI_STATUS_AND_COMMAND,deviceNumber,~MASTER_ENABLE &		     \
	  pciReadConfigReg(host,PCI_STATUS_AND_COMMAND,deviceNumber) )

#define		MASTER_ENABLE			BIT2
#define		MEMORY_ENABLE			BIT1
#define		I_O_ENABLE			BIT0
#define	    SELF		    32

/* Agent on the PCI bus may have up to 6 BARS. */
#define	    BAR0		    0x10
#define	    BAR1		    0x14
#define	    BAR2		    0x18
#define	    BAR3		    0x1c
#define	    BAR4		    0x20
#define	    BAR5		    0x24
#define		BAR_SEL_MEM_IO			BIT0
#define		BAR_MEM_TYPE_32_BIT		NO_BIT
#define		BAR_MEM_TYPE_BELOW_1M		       BIT1
#define		BAR_MEM_TYPE_64_BIT			      BIT2
#define		BAR_MEM_TYPE_RESERVED		      (BIT1 | BIT2)
#define		BAR_MEM_TYPE_MASK		      (BIT1 | BIT2)
#define		BAR_PREFETCHABLE				      BIT3
#define		BAR_CONFIG_MASK			(BIT0 | BIT1 | BIT2 | BIT3)

/* Defines for the access regions. */
#define	    PREFETCH_ENABLE		    BIT12
#define	    PREFETCH_DISABLE		    NO_BIT
#define	    DELAYED_READ_ENABLE		    BIT13
/* #define     CACHING_ENABLE		       BIT14 */
/* aggressive prefetch: PCI slave prefetch two burst in advance*/
#define	    AGGRESSIVE_PREFETCH		     BIT16
/* read line aggresive prefetch: PCI slave prefetch two burst in advance*/
#define	    READ_LINE_AGGRESSIVE_PREFETCH   BIT17
/* read multiple aggresive prefetch: PCI slave prefetch two burst in advance*/
#define	    READ_MULTI_AGGRESSIVE_PREFETCH  BIT18
#define	    MAX_BURST_4			    NO_BIT
#define	    MAX_BURST_8			    BIT20  /* Bits[21:20] = 01 */
#define	    MAX_BURST_16		    BIT21  /* Bits[21:20] = 10 */
#define	    PCI_BYTE_SWAP		    NO_BIT /* Bits[25:24] = 00 */
#define	    PCI_NO_SWAP			    BIT24  /* Bits[25:24] = 01 */
#define	    PCI_BYTE_AND_WORD_SWAP	    BIT25  /* Bits[25:24] = 10 */
#define	    PCI_WORD_SWAP		   (BIT24 | BIT25) /* Bits[25:24] = 11 */
#define	    PCI_ACCESS_PROTECT		    BIT28
#define	    PCI_WRITE_PROTECT		    BIT29

/* typedefs */

typedef enum __pciAccessRegions{REGION0,REGION1,REGION2,REGION3,REGION4,REGION5,
				REGION6,REGION7} PCI_ACCESS_REGIONS;

typedef enum __pciAgentPrio{LOW_AGENT_PRIO,HI_AGENT_PRIO} PCI_AGENT_PRIO;
typedef enum __pciAgentPark{PARK_ON_AGENT,DONT_PARK_ON_AGENT} PCI_AGENT_PARK;

typedef enum __pciSnoopType{PCI_NO_SNOOP,PCI_SNOOP_WT,PCI_SNOOP_WB}
			    PCI_SNOOP_TYPE;
typedef enum __pciSnoopRegion{PCI_SNOOP_REGION0,PCI_SNOOP_REGION1,
			      PCI_SNOOP_REGION2,PCI_SNOOP_REGION3}
			      PCI_SNOOP_REGION;

typedef enum __memPciHost{PCI_HOST0,PCI_HOST1} PCI_HOST;
typedef enum __memPciRegion{PCI_REGION0,PCI_REGION1,
			 PCI_REGION2,PCI_REGION3,
			 PCI_IO}
			 PCI_REGION;

/*ronen 7/Dec/03 */
typedef enum __pci_bar_windows{PCI_CS0_BAR, PCI_CS1_BAR, PCI_CS2_BAR,
			       PCI_CS3_BAR, PCI_DEV_CS0_BAR, PCI_DEV_CS1_BAR,
			       PCI_DEV_CS2_BAR, PCI_DEV_CS3_BAR, PCI_BOOT_CS_BAR,
			       PCI_MEM_INT_REG_BAR, PCI_IO_INT_REG_BAR,
			       PCI_P2P_MEM0_BAR, PCI_P2P_MEM1_BAR,
			       PCI_P2P_IO_BAR, PCI_CPU_BAR, PCI_INT_SRAM_BAR,
			       PCI_LAST_BAR} PCI_INTERNAL_BAR;

typedef struct pciBar {
    unsigned int detectBase;
    unsigned int base;
    unsigned int size;
    unsigned int type;
} PCI_BAR;

typedef struct pciDevice {
    PCI_HOST	     host;
    char	    type[40];
    unsigned int    deviceNum;
    unsigned int    venID;
    unsigned int    deviceID;
    PCI_BAR bar[6];
} PCI_DEVICE;

typedef struct pciSelfBars {
    unsigned int    SCS0Base;
    unsigned int    SCS0Size;
    unsigned int    SCS1Base;
    unsigned int    SCS1Size;
    unsigned int    SCS2Base;
    unsigned int    SCS2Size;
    unsigned int    SCS3Base;
    unsigned int    SCS3Size;
    unsigned int    internalMemBase;
    unsigned int    internalIOBase;
    unsigned int    CS0Base;
    unsigned int    CS0Size;
    unsigned int    CS1Base;
    unsigned int    CS1Size;
    unsigned int    CS2Base;
    unsigned int    CS2Size;
    unsigned int    CS3Base;
    unsigned int    CS3Size;
    unsigned int    CSBootBase;
    unsigned int    CSBootSize;
    unsigned int    P2PMem0Base;
    unsigned int    P2PMem0Size;
    unsigned int    P2PMem1Base;
    unsigned int    P2PMem1Size;
    unsigned int    P2PIOBase;
    unsigned int    P2PIOSize;
    unsigned int    CPUBase;
    unsigned int    CPUSize;
} PCI_SELF_BARS;

/* read/write configuration registers on local PCI bus. */
void pciWriteConfigReg(PCI_HOST host, unsigned int regOffset,
		       unsigned int pciDevNum, unsigned int data);
unsigned int pciReadConfigReg (PCI_HOST host, unsigned int regOffset,
			       unsigned int pciDevNum);

/* read/write configuration registers on another PCI bus. */
void pciOverBridgeWriteConfigReg(PCI_HOST host,
				 unsigned int regOffset,
				 unsigned int pciDevNum,
				 unsigned int busNum,unsigned int data);
unsigned int pciOverBridgeReadConfigReg(PCI_HOST host,
					unsigned int regOffset,
					unsigned int pciDevNum,
					unsigned int busNum);

/* Performs full scane on both PCI and returns all detail possible on the
   agents which exist on the bus. */
void pciScanDevices(PCI_HOST host, PCI_DEVICE *pci0Detect,
		    unsigned int numberOfElment);

/*	Master`s memory space	*/
bool pciMapSpace(PCI_HOST host, PCI_REGION region,
		unsigned int remapBase,
		unsigned int deviceBase,
		unsigned int deviceLength);
unsigned int pciGetSpaceBase(PCI_HOST host, PCI_REGION region);
unsigned int pciGetSpaceSize(PCI_HOST host, PCI_REGION region);

/*	Slave`s memory space   */
void pciMapMemoryBank(PCI_HOST host, MEMORY_BANK bank,
		      unsigned int pci0Dram0Base, unsigned int pci0Dram0Size);

#if 0 /* GARBAGE routines - dont use till they get cleaned up */
void pci0ScanSelfBars(PCI_SELF_BARS *pci0SelfBars);
void pci1ScanSelfBars(PCI_SELF_BARS *pci1SelfBars);
void pci0MapInternalRegSpace(unsigned int pci0InternalBase);
void pci1MapInternalRegSpace(unsigned int pci1InternalBase);
void pci0MapInternalRegIOSpace(unsigned int pci0InternalBase);
void pci1MapInternalRegIOSpace(unsigned int pci1InternalBase);
void pci0MapDevice0MemorySpace(unsigned int pci0Dev0Base,
			       unsigned int pci0Dev0Length);
void pci1MapDevice0MemorySpace(unsigned int pci1Dev0Base,
			       unsigned int pci1Dev0Length);
void pci0MapDevice1MemorySpace(unsigned int pci0Dev1Base,
			       unsigned int pci0Dev1Length);
void pci1MapDevice1MemorySpace(unsigned int pci1Dev1Base,
			       unsigned int pci1Dev1Length);
void pci0MapDevice2MemorySpace(unsigned int pci0Dev2Base,
			       unsigned int pci0Dev2Length);
void pci1MapDevice2MemorySpace(unsigned int pci1Dev2Base,
			       unsigned int pci1Dev2Length);
void pci0MapDevice3MemorySpace(unsigned int pci0Dev3Base,
			       unsigned int pci0Dev3Length);
void pci1MapDevice3MemorySpace(unsigned int pci1Dev3Base,
			       unsigned int pci1Dev3Length);
void pci0MapBootDeviceMemorySpace(unsigned int pci0DevBootBase,
				  unsigned int pci0DevBootLength);
void pci1MapBootDeviceMemorySpace(unsigned int pci1DevBootBase,
				  unsigned int pci1DevBootLength);
void pci0MapP2pMem0Space(unsigned int pci0P2pMem0Base,
			 unsigned int pci0P2pMem0Length);
void pci1MapP2pMem0Space(unsigned int pci1P2pMem0Base,
			 unsigned int pci1P2pMem0Length);
void pci0MapP2pMem1Space(unsigned int pci0P2pMem1Base,
			 unsigned int pci0P2pMem1Length);
void pci1MapP2pMem1Space(unsigned int pci1P2pMem1Base,
			 unsigned int pci1P2pMem1Length);
void pci0MapP2pIoSpace(unsigned int pci0P2pIoBase,
		       unsigned int pci0P2pIoLength);
void pci1MapP2pIoSpace(unsigned int pci1P2pIoBase,
		       unsigned int pci1P2pIoLength);

void pci0MapCPUspace(unsigned int pci0CpuBase, unsigned int pci0CpuLengs);
void pci1MapCPUspace(unsigned int pci1CpuBase, unsigned int pci1CpuLengs);
#endif

/* PCI region options */

bool  pciSetRegionFeatures(PCI_HOST host, PCI_ACCESS_REGIONS region,
	unsigned int features, unsigned int baseAddress,
	unsigned int regionLength);

void  pciDisableAccessRegion(PCI_HOST host, PCI_ACCESS_REGIONS region);

/* PCI arbiter */

bool pciArbiterEnable(PCI_HOST host);
bool pciArbiterDisable(PCI_HOST host);
bool pciSetArbiterAgentsPriority(PCI_HOST host, PCI_AGENT_PRIO internalAgent,
				  PCI_AGENT_PRIO externalAgent0,
				  PCI_AGENT_PRIO externalAgent1,
				  PCI_AGENT_PRIO externalAgent2,
				  PCI_AGENT_PRIO externalAgent3,
				  PCI_AGENT_PRIO externalAgent4,
				  PCI_AGENT_PRIO externalAgent5);
bool pciSetArbiterAgentsPriority(PCI_HOST host, PCI_AGENT_PRIO internalAgent,
				  PCI_AGENT_PRIO externalAgent0,
				  PCI_AGENT_PRIO externalAgent1,
				  PCI_AGENT_PRIO externalAgent2,
				  PCI_AGENT_PRIO externalAgent3,
				  PCI_AGENT_PRIO externalAgent4,
				  PCI_AGENT_PRIO externalAgent5);
bool pciParkingDisable(PCI_HOST host, PCI_AGENT_PARK internalAgent,
			PCI_AGENT_PARK externalAgent0,
			PCI_AGENT_PARK externalAgent1,
			PCI_AGENT_PARK externalAgent2,
			PCI_AGENT_PARK externalAgent3,
			PCI_AGENT_PARK externalAgent4,
			PCI_AGENT_PARK externalAgent5);
bool pciEnableBrokenAgentDetection(PCI_HOST host, unsigned char brokenValue);
bool pciEnableBrokenAgentDetection(PCI_HOST host, unsigned char brokenValue);

/* PCI-to-PCI (P2P) */

bool pciP2PConfig(PCI_HOST host,
		  unsigned int SecondBusLow,unsigned int SecondBusHigh,
		  unsigned int busNum,unsigned int devNum);
/* PCI Cache-coherency */

bool pciSetRegionSnoopMode(PCI_HOST host, PCI_SNOOP_REGION region,
			    PCI_SNOOP_TYPE snoopType,
			    unsigned int baseAddress,
			    unsigned int regionLength);

PCI_DEVICE * pciFindDevice(unsigned short ven, unsigned short dev);

#endif /* __INCpcih */
