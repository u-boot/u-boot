/* PCI.h - PCI functions header file */

/* Copyright - Galileo technology. */

#ifndef __INCpcih
#define __INCpcih

/* includes */

#include "core.h"
#include "memory.h"

/* According to PCI REV 2.1 MAX agents allowed on the bus are -21- */
#define PCI_MAX_DEVICES 22


/* Macros */
#define     SELF                    32

/* Defines for the access regions. */
#define     PREFETCH_ENABLE                 BIT12
#define     PREFETCH_DISABLE                NO_BIT
#define     DELAYED_READ_ENABLE             BIT13
/* #define     CACHING_ENABLE                  BIT14 */
/* aggressive prefetch: PCI slave prefetch two burst in advance*/
#define     AGGRESSIVE_PREFETCH              BIT16
/* read line aggresive prefetch: PCI slave prefetch two burst in advance*/
#define     READ_LINE_AGGRESSIVE_PREFETCH   BIT17
/* read multiple aggresive prefetch: PCI slave prefetch two burst in advance*/
#define     READ_MULTI_AGGRESSIVE_PREFETCH  BIT18
#define     MAX_BURST_4                     NO_BIT
#define     MAX_BURST_8                     BIT20  /* Bits[21:20] = 01 */
#define     MAX_BURST_16                    BIT21  /* Bits[21:20] = 10 */
#define     PCI_BYTE_SWAP                   NO_BIT /* Bits[25:24] = 00 */
#define     PCI_NO_SWAP                     BIT24  /* Bits[25:24] = 01 */
#define     PCI_BYTE_AND_WORD_SWAP          BIT25  /* Bits[25:24] = 10 */
#define     PCI_WORD_SWAP                  (BIT24 | BIT25) /* Bits[25:24] = 11 */
#define     PCI_ACCESS_PROTECT              BIT28
#define     PCI_WRITE_PROTECT               BIT29

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

/*      Master`s memory space   */
bool pciMapSpace(PCI_HOST host, PCI_REGION region,
		unsigned int remapBase,
		unsigned int deviceBase,
		unsigned int deviceLength);
unsigned int pciGetSpaceBase(PCI_HOST host, PCI_REGION region);
unsigned int pciGetSpaceSize(PCI_HOST host, PCI_REGION region);

/*      Slave`s memory space   */
void pciMapMemoryBank(PCI_HOST host, MEMORY_BANK bank,
		      unsigned int pci0Dram0Base, unsigned int pci0Dram0Size);

/* PCI region options */

bool  pciSetRegionFeatures(PCI_HOST host, PCI_ACCESS_REGIONS region,
	unsigned int features, unsigned int baseAddress,
	unsigned int regionLength);

void  pciDisableAccessRegion(PCI_HOST host, PCI_ACCESS_REGIONS region);

/* PCI arbiter */

bool pciArbiterEnable(PCI_HOST host);
bool pciArbiterDisable(PCI_HOST host);
bool pciParkingDisable(PCI_HOST host, PCI_AGENT_PARK internalAgent,
			PCI_AGENT_PARK externalAgent0,
			PCI_AGENT_PARK externalAgent1,
			PCI_AGENT_PARK externalAgent2,
			PCI_AGENT_PARK externalAgent3,
			PCI_AGENT_PARK externalAgent4,
			PCI_AGENT_PARK externalAgent5);
bool pciSetRegionSnoopMode(PCI_HOST host, PCI_SNOOP_REGION region,
			    PCI_SNOOP_TYPE snoopType,
			    unsigned int baseAddress,
			    unsigned int regionLength);

#endif /* __INCpcih */
