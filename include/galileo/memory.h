/* Memory.h - Memory mappings and remapping functions declarations */

/* Copyright - Galileo technology. */

#ifndef __INCmemoryh
#define __INCmemoryh

/* includes */

#include "core.h"

/* defines */

#define DONT_MODIFY     0xffffffff
#define PARITY_SUPPORT  0x40000000

#define _8BIT           0x00000000
#define _16BIT          0x00100000
#define _32BIT          0x00200000
#define _64BIT          0x00300000

/* typedefs */

 typedef struct deviceParam
{                               /* boundary values  */
    unsigned int    turnOff;    /* 0x0 - 0xf        */
    unsigned int    acc2First;  /* 0x0 - 0x1f       */
    unsigned int    acc2Next;   /* 0x0 - 0x1f       */
    unsigned int    ale2Wr;     /* 0x0 - 0xf        */
    unsigned int    wrLow;      /* 0x0 - 0xf        */
    unsigned int    wrHigh;     /* 0x0 - 0xf        */
    unsigned int    deviceWidth;    /* in Bytes  */
} DEVICE_PARAM;

typedef enum __memBank{BANK0,BANK1,BANK2,BANK3} MEMORY_BANK;
typedef enum __memDevice{DEVICE0,DEVICE1,DEVICE2,DEVICE3,BOOT_DEVICE} DEVICE;

typedef enum __memoryProtectRegion{MEM_REGION0,MEM_REGION1,MEM_REGION2,       \
				   MEM_REGION3,MEM_REGION4,MEM_REGION5,       \
				   MEM_REGION6,MEM_REGION7}                   \
				   MEMORY_PROTECT_REGION;
typedef enum __memoryAccess{MEM_ACCESS_ALLOWED,MEM_ACCESS_FORBIDEN}           \
			    MEMORY_ACCESS;
typedef enum __memoryWrite{MEM_WRITE_ALLOWED,MEM_WRITE_FORBIDEN}              \
			   MEMORY_ACCESS_WRITE;
typedef enum __memoryCacheProtect{MEM_CACHE_ALLOWED,MEM_CACHE_FORBIDEN}       \
				  MEMORY_CACHE_PROTECT;
typedef enum __memorySnoopType{MEM_NO_SNOOP,MEM_SNOOP_WT,MEM_SNOOP_WB}        \
			       MEMORY_SNOOP_TYPE;
typedef enum __memorySnoopRegion{MEM_SNOOP_REGION0,MEM_SNOOP_REGION1,         \
				 MEM_SNOOP_REGION2,MEM_SNOOP_REGION3}         \
				 MEMORY_SNOOP_REGION;

/* functions */
unsigned int memoryGetBankBaseAddress(MEMORY_BANK bank);
unsigned int memoryGetDeviceBaseAddress(DEVICE device);
unsigned int memoryGetBankSize(MEMORY_BANK bank);
unsigned int memoryGetDeviceSize(DEVICE device);
unsigned int memoryGetDeviceWidth(DEVICE device);

/* when given base Address and size Set new WINDOW for SCS_X. (X = 0,1,2 or 3*/
bool memoryMapBank(MEMORY_BANK bank, unsigned int bankBase,unsigned int bankLength);
bool memoryMapDeviceSpace(DEVICE device, unsigned int deviceBase,unsigned int deviceLength);

/* Change the Internal Register Base Address to a new given Address. */
bool memoryMapInternalRegistersSpace(unsigned int internalRegBase);
/* returns internal Register Space Base Address. */
unsigned int memoryGetInternalRegistersSpace(void);
/* Configurate the protection feature to a given space. */
bool memorySetProtectRegion(MEMORY_PROTECT_REGION region,
			    MEMORY_ACCESS memoryAccess,
			    MEMORY_ACCESS_WRITE memoryWrite,
			    MEMORY_CACHE_PROTECT cacheProtection,
			    unsigned int baseAddress,
			    unsigned int regionLength);
/* Configurate the snoop feature to a given space. */
bool memorySetRegionSnoopMode(MEMORY_SNOOP_REGION region,
			      MEMORY_SNOOP_TYPE snoopType,
			      unsigned int baseAddress,
			      unsigned int regionLength);

bool memoryRemapAddress(unsigned int remapReg, unsigned int remapValue);
bool memoryGetDeviceParam(DEVICE_PARAM *deviceParam, DEVICE deviceNum);
bool memorySetDeviceParam(DEVICE_PARAM *deviceParam, DEVICE deviceNum);
#endif  /* __INCmemoryh */
