/* Memory.h - Memory mappings and remapping functions declarations */

/* Copyright - Galileo technology. */

#ifndef __INCmemoryh
#define __INCmemoryh

/* includes */

#include "core.h"

/* defines */

#define DONT_MODIFY	0xffffffff
#define PARITY_SUPPORT	0x40000000
#define MINIMUM_MEM_BANK_SIZE		0x10000
#define MINIMUM_DEVICE_WINDOW_SIZE	0x10000
#define MINIMUM_PCI_WINDOW_SIZE		0x10000
#define MINIMUM_ACCESS_WIN_SIZE		0x10000

#define _8BIT		0x00000000
#define _16BIT		0x00100000
#define _32BIT		0x00200000
#define _64BIT		0x00300000

/* typedefs */

 typedef struct deviceParam
{						/* boundary values  */
    unsigned int    turnOff;			/* 0x0 - 0xf	    */
    unsigned int    acc2First;			/* 0x0 - 0x1f	    */
    unsigned int    acc2Next;		/* 0x0 - 0x1f	    */
    unsigned int    ale2Wr;			/* 0x0 - 0xf	    */
    unsigned int    wrLow;			/* 0x0 - 0xf	    */
    unsigned int    wrHigh;			/* 0x0 - 0xf	    */
    unsigned int    badrSkew;		/* 0x0 - 0x2	   */
    unsigned int    DPEn;			/* 0x0 - 0x1	   */
    unsigned int    deviceWidth;	/* in Bytes  */
} DEVICE_PARAM;


typedef enum __memBank{BANK0,BANK1,BANK2,BANK3} MEMORY_BANK;
typedef enum __memDevice{DEVICE0,DEVICE1,DEVICE2,DEVICE3,BOOT_DEVICE} DEVICE;

/*typedef enum __memoryProtectRegion{MEM_REGION0,MEM_REGION1,MEM_REGION2,	\
				   MEM_REGION3,MEM_REGION4,MEM_REGION5,	      \
				   MEM_REGION6,MEM_REGION7}		      \
				   MEMORY_PROTECT_REGION;*/
/* There are four possible windows that can be defined as protected */
typedef enum _memoryProtectWindow{MEM_WINDOW0,MEM_WINDOW1,MEM_WINDOW2,
				  MEM_WINDOW3
				 } MEMORY_PROTECT_WINDOW;
/* When defining a protected window , this paramter indicates whether it
   is accessible or not */
typedef enum __memoryAccess{MEM_ACCESS_ALLOWED,MEM_ACCESS_FORBIDEN}	      \
			    MEMORY_ACCESS;
typedef enum __memoryWrite{MEM_WRITE_ALLOWED,MEM_WRITE_FORBIDEN}	      \
			   MEMORY_ACCESS_WRITE;
typedef enum __memoryCacheProtect{MEM_CACHE_ALLOWED,MEM_CACHE_FORBIDEN}	      \
				  MEMORY_CACHE_PROTECT;
typedef enum __memorySnoopType{MEM_NO_SNOOP,MEM_SNOOP_WT,MEM_SNOOP_WB}	      \
			       MEMORY_SNOOP_TYPE;
typedef enum __memorySnoopRegion{MEM_SNOOP_REGION0,MEM_SNOOP_REGION1,	      \
				 MEM_SNOOP_REGION2,MEM_SNOOP_REGION3}	      \
				 MEMORY_SNOOP_REGION;

/* There are 21 memory windows dedicated for the varios interfaces (PCI,
   devCS (devices), CS(DDR), interenal registers and SRAM) used by the CPU's
   address decoding mechanism. */
typedef enum _memoryWindow {CS_0_WINDOW = BIT0, CS_1_WINDOW = BIT1,
			    CS_2_WINDOW = BIT2, CS_3_WINDOW = BIT3,
			    DEVCS_0_WINDOW = BIT4, DEVCS_1_WINDOW = BIT5,
			    DEVCS_2_WINDOW = BIT6, DEVCS_3_WINDOW = BIT7,
			    BOOT_CS_WINDOW = BIT8, PCI_0_IO_WINDOW = BIT9,
			    PCI_0_MEM0_WINDOW = BIT10,
			    PCI_0_MEM1_WINDOW = BIT11,
			    PCI_0_MEM2_WINDOW = BIT12,
			    PCI_0_MEM3_WINDOW = BIT13, PCI_1_IO_WINDOW = BIT14,
			    PCI_1_MEM0_WINDOW = BIT15, PCI_1_MEM1_WINDOW =BIT16,
			    PCI_1_MEM2_WINDOW = BIT17, PCI_1_MEM3_WINDOW =BIT18,
			    INTEGRATED_SRAM_WINDOW = BIT19,
			    INTERNAL_SPACE_WINDOW = BIT20,
			    ALL_WINDOWS = 0X1FFFFF
			   } MEMORY_WINDOW;

typedef enum _memoryWindowStatus {MEM_WINDOW_ENABLED,MEM_WINDOW_DISABLED
				 } MEMORY_WINDOW_STATUS;


typedef enum _pciMemWindow{PCI_0_IO,PCI_0_MEM0,PCI_0_MEM1,PCI_0_MEM2,PCI_0_MEM3
#ifdef INCLUDE_PCI_1
			  ,PCI_1_IO,PCI_1_MEM0,PCI_1_MEM1,PCI_1_MEM2,PCI_1_MEM3
#endif /* INCLUDE_PCI_1 */
			  } PCI_MEM_WINDOW;


/* -------------------------------------------------------------------------------------------------*/

/* functions */
unsigned int memoryGetBankBaseAddress(MEMORY_BANK bank);
unsigned int memoryGetDeviceBaseAddress(DEVICE device);
/* New at MV6436x */
unsigned int MemoryGetPciBaseAddr(PCI_MEM_WINDOW pciWindow);
unsigned int memoryGetBankSize(MEMORY_BANK bank);
unsigned int memoryGetDeviceSize(DEVICE device);
unsigned int memoryGetDeviceWidth(DEVICE device);
/* New at MV6436x */
unsigned int gtMemoryGetPciWindowSize(PCI_MEM_WINDOW pciWindow);

/* when given base Address and size Set new WINDOW for SCS_X. (X = 0,1,2 or 3*/
bool memoryMapBank(MEMORY_BANK bank, unsigned int bankBase,unsigned int bankLength);
/* Set a new base and size for one of the memory banks (CS0 - CS3) */
bool gtMemorySetMemoryBank(MEMORY_BANK bank, unsigned int bankBase,
			   unsigned int bankSize);
bool memoryMapDeviceSpace(DEVICE device, unsigned int deviceBase,unsigned int deviceLength);

/* Change the Internal Register Base Address to a new given Address. */
bool memoryMapInternalRegistersSpace(unsigned int internalRegBase);
/* returns internal Register Space Base Address. */
unsigned int memoryGetInternalRegistersSpace(void);

/* Returns the integrated SRAM Base Address. */
unsigned int memoryGetInternalSramBaseAddr(void);
/* -------------------------------------------------------------------------------------------------*/

/* Set new base address for the integrated SRAM. */
void memorySetInternalSramBaseAddr(unsigned int sramBaseAddress);
/* -------------------------------------------------------------------------------------------------*/

/* Delete a protection feature to a given space. */
void memoryDisableProtectRegion(MEMORY_PROTECT_WINDOW window);
/* -------------------------------------------------------------------------------------------------*/

/* Writes a new remap value to the remap register */
unsigned int memorySetPciRemapValue(PCI_MEM_WINDOW memoryWindow,
				      unsigned int remapValueHigh,
				      unsigned int remapValueLow);
/* -------------------------------------------------------------------------------------------------*/

/* Configurate the protection feature to a given space. */
bool memorySetProtectRegion(MEMORY_PROTECT_WINDOW window,
			      MEMORY_ACCESS gtMemoryAccess,
			      MEMORY_ACCESS_WRITE gtMemoryWrite,
			      MEMORY_CACHE_PROTECT cacheProtection,
			      unsigned int baseAddress,
			      unsigned int size);

/* Configurate the protection feature to a given space. */
/*bool memorySetProtectRegion(MEMORY_PROTECT_REGION region,
			    MEMORY_ACCESS memoryAccess,
			    MEMORY_ACCESS_WRITE memoryWrite,
			    MEMORY_CACHE_PROTECT cacheProtection,
			    unsigned int baseAddress,
			    unsigned int regionLength); */
/* Configurate the snoop feature to a given space. */
bool memorySetRegionSnoopMode(MEMORY_SNOOP_REGION region,
			      MEMORY_SNOOP_TYPE snoopType,
			      unsigned int baseAddress,
			      unsigned int regionLength);

bool memoryRemapAddress(unsigned int remapReg, unsigned int remapValue);
bool memoryGetDeviceParam(DEVICE_PARAM *deviceParam, DEVICE deviceNum);
bool memorySetDeviceParam(DEVICE_PARAM *deviceParam, DEVICE deviceNum);
/* Set a new base and size for one of the PCI windows. */
bool memorySetPciWindow(PCI_MEM_WINDOW pciWindow, unsigned int pciWindowBase,
			  unsigned int pciWindowSize);

/* Disable or enable one of the 21 windows dedicated for the CPU's
   address decoding mechanism */
void MemoryDisableWindow(MEMORY_WINDOW window);
void MemoryEnableWindow (MEMORY_WINDOW window);
MEMORY_WINDOW_STATUS MemoryGetMemWindowStatus(MEMORY_WINDOW window);
#endif	/* __INCmemoryh */
