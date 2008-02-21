/*
 * Boot ROM Entry Points and such
 */

/* These Blackfins all have a Boot ROM that is not reusable (at all):
 *  BF531 / BF532 / BF533
 *  BF538 / BF539
 *  BF561
 * So there is nothing for us to export ;(
 *
 * These Blackfins started to roll with the idea that the Boot ROM can
 * provide useful functions, but still only a few (and not really useful):
 *  BF534 / BF536 / BF537
 *
 * Looking forward, Boot ROM's on newer Blackfins have quite a few
 * nice entry points that are usable at runtime and beyond.  We'll
 * only define known legacy parts (listed above) and otherwise just
 * assume it's a newer part.
 *
 * These entry points are accomplished by placing a small jump table at
 * the start of the Boot ROM.  This way the addresses are fixed forever.
 */

#ifndef __BFIN_PERIPHERAL_BOOTROM__
#define __BFIN_PERIPHERAL_BOOTROM__

/* All Blackfin's have the Boot ROM entry point at the same address */
#define _BOOTROM_RESET 0xEF000000

#if defined(__ADSPBF531__) || defined(__ADSPBF532__) || defined(__ADSPBF533__) || \
    defined(__ADSPBF538__) || defined(__ADSPBF539__) || \
    defined(__ADSPBF561__)

	/* Nothing to export */

#elif defined(__ADSPBF534__) || defined(__ADSPBF536__) || defined(__ADSPBF537__)

	/* The BF537 family */

#define _BOOTROM_FINAL_INIT            0xEF000002
/*       reserved                      0xEF000004 */
#define _BOOTROM_DO_MEMORY_DMA         0xEF000006
#define _BOOTROM_BOOT_DXE_FLASH        0xEF000008
#define _BOOTROM_BOOT_DXE_SPI          0xEF00000A
#define _BOOTROM_BOOT_DXE_TWI          0xEF00000C
/*       reserved                      0xEF00000E */
#define _BOOTROM_GET_DXE_ADDRESS_FLASH 0xEF000010
#define _BOOTROM_GET_DXE_ADDRESS_SPI   0xEF000012
#define _BOOTROM_GET_DXE_ADDRESS_TWI   0xEF000014
/*       reserved                      0xEF000016 */
/*       reserved                      0xEF000018 */

	/* Glue to newer Boot ROMs */
#define _BOOTROM_MDMA                  _BOOTROM_DO_MEMORY_DMA
#define _BOOTROM_MEMBOOT               _BOOTROM_BOOT_DXE_FLASH
#define _BOOTROM_SPIBOOT               _BOOTROM_BOOT_DXE_FLASH
#define _BOOTROM_TWIBOOT               _BOOTROM_BOOT_DXE_TWI

#else

	/* All the newer Boot ROMs */

#define _BOOTROM_FINAL_INIT            0xEF000002
#define _BOOTROM_PDMA                  0xEF000004
#define _BOOTROM_MDMA                  0xEF000006
#define _BOOTROM_MEMBOOT               0xEF000008
#define _BOOTROM_SPIBOOT               0xEF00000A
#define _BOOTROM_TWIBOOT               0xEF00000C
/*       reserved                      0xEF00000E */
/*       reserved                      0xEF000010 */
/*       reserved                      0xEF000012 */
/*       reserved                      0xEF000014 */
/*       reserved                      0xEF000016 */
#define _BOOTROM_OTP_COMMAND           0xEF000018
#define _BOOTROM_OTP_READ              0xEF00001A
#define _BOOTROM_OTP_WRITE             0xEF00001C
#define _BOOTROM_ECC_TABLE             0xEF00001E
#define _BOOTROM_BOOTKERNEL            0xEF000020
#define _BOOTROM_GETPORT               0xEF000022
#define _BOOTROM_NMI                   0xEF000024
#define _BOOTROM_HWERROR               0xEF000026
#define _BOOTROM_EXCEPTION             0xEF000028
#define _BOOTROM_CRC32                 0xEF000030
#define _BOOTROM_CRC32POLY             0xEF000032
#define _BOOTROM_CRC32CALLBACK         0xEF000034
#define _BOOTROM_CRC32INITCODE         0xEF000036
#define _BOOTROM_SYSCONTROL            0xEF000038
#define _BOOTROM_REV                   0xEF000040
#define _BOOTROM_SESR                  0xEF001000

#define BOOTROM_CAPS_ADI_BOOT_STRUCTS 1

/* Not available on initial BF54x or BF52x */
#if (defined(__ADSPBF54x__) && __SILICON_REVISION__ < 1) || \
    (defined(__ADSPBF52x__) && __SILICON_REVISION__ < 2)
#define BOOTROM_CAPS_SYSCONTROL 0
#else
#define BOOTROM_CAPS_SYSCONTROL 1
#endif

#endif

#ifndef BOOTROM_CAPS_ADI_BOOT_STRUCTS
#define BOOTROM_CAPS_ADI_BOOT_STRUCTS 0
#endif
#ifndef BOOTROM_CAPS_SYSCONTROL
#define BOOTROM_CAPS_SYSCONTROL 0
#endif

#ifndef __ASSEMBLY__

/* Structures for the syscontrol() function */
typedef struct ADI_SYSCTRL_VALUES {
	uint16_t uwVrCtl;
	uint16_t uwPllCtl;
	uint16_t uwPllDiv;
	uint16_t uwPllLockCnt;
	uint16_t uwPllStat;
} ADI_SYSCTRL_VALUES;

#ifndef _BOOTROM_SYSCONTROL
#define _BOOTROM_SYSCONTROL 0
#endif
static uint32_t (* const syscontrol)(uint32_t action_flags, ADI_SYSCTRL_VALUES *power_settings, void *reserved) = (void *)_BOOTROM_SYSCONTROL;

#endif /* __ASSEMBLY__ */

/* Possible syscontrol action flags */
#define SYSCTRL_READ        0x00000000    /* read registers */
#define SYSCTRL_WRITE       0x00000001    /* write registers */
#define SYSCTRL_SYSRESET    0x00000002    /* perform system reset */
#define SYSCTRL_SOFTRESET   0x00000004    /* perform core and system reset */
#define SYSCTRL_VRCTL       0x00000010    /* read/write VR_CTL register */
#define SYSCTRL_EXTVOLTAGE  0x00000020    /* VDDINT supplied externally */
#define SYSCTRL_INTVOLTAGE  0x00000000    /* VDDINT generated by on-chip regulator */
#define SYSCTRL_OTPVOLTAGE  0x00000040    /* For Factory Purposes Only */
#define SYSCTRL_PLLCTL      0x00000100    /* read/write PLL_CTL register */
#define SYSCTRL_PLLDIV      0x00000200    /* read/write PLL_DIV register */
#define SYSCTRL_LOCKCNT     0x00000400    /* read/write PLL_LOCKCNT register */
#define SYSCTRL_PLLSTAT     0x00000800    /* read/write PLL_STAT register */

#ifndef __ASSEMBLY__

/* Structures for working with LDRs and boot rom callbacks */
typedef struct ADI_BOOT_HEADER {
	int32_t dBlockCode;
	void    *pTargetAddress;
	int32_t dByteCount;
	int32_t dArgument;
} ADI_BOOT_HEADER;

typedef struct ADI_BOOT_BUFFER {
	void    *pSource;
	int32_t dByteCount;
} ADI_BOOT_BUFFER;

typedef struct ADI_BOOT_DATA {
	void    *pSource;
	void    *pDestination;
	int16_t *pControlRegister;
	int16_t *pDmaControlRegister;
	int32_t dControlValue;
	int32_t dByteCount;
	int32_t dFlags;
	int16_t uwDataWidth;
	int16_t uwSrcModifyMult;
	int16_t uwDstModifyMult;
	int16_t uwHwait;
	int16_t uwSsel;
	int16_t uwUserShort;
	int32_t dUserLong;
	int32_t dReserved2;
	void    *pErrorFunction;
	void    *pLoadFunction;
	void    *pCallBackFunction;
	ADI_BOOT_HEADER *pHeader;
	void    *pTempBuffer;
	void    *pTempCurrent;
	int32_t dTempByteCount;
	int32_t dBlockCount;
	int32_t dClock;
	void    *pLogBuffer;
	void    *pLogCurrent;
	int32_t dLogByteCount;
} ADI_BOOT_DATA;

#endif /* __ASSEMBLY__ */

/* Bit defines for ADI_BOOT_DATA->dFlags */
#define BFLAG_DMACODE_MASK 0x0000000F
#define BFLAG_SAFE         0x00000010
#define BFLAG_AUX          0x00000020
#define BFLAG_FILL         0x00000100
#define BFLAG_QUICKBOOT    0x00000200
#define BFLAG_CALLBACK     0x00000400
#define BFLAG_INIT         0x00000800
#define BFLAG_IGNORE       0x00001000
#define BFLAG_INDIRECT     0x00002000
#define BFLAG_FIRST        0x00004000
#define BFLAG_FINAL        0x00008000
#define BFLAG_HOOK         0x00400000
#define BFLAG_HDRINDIRECT  0x00800000
#define BFLAG_TYPE_MASK    0x00300000
#define BFLAG_TYPE_1       0x00000000
#define BFLAG_TYPE_2       0x00100000
#define BFLAG_TYPE_3       0x00200000
#define BFLAG_TYPE_4       0x00300000
#define BFLAG_FASTREAD     0x00400000
#define BFLAG_NOAUTO       0x01000000
#define BFLAG_PERIPHERAL   0x02000000
#define BFLAG_SLAVE        0x04000000
#define BFLAG_WAKEUP       0x08000000
#define BFLAG_NEXTDXE      0x10000000
#define BFLAG_RETURN       0x20000000
#define BFLAG_RESET        0x40000000
#define BFLAG_NONRESTORE   0x80000000

#endif
