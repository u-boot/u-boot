/*
 * Lockbox/Security Masks
 */

#ifndef __BFIN_PERIPHERAL_LOCKBOX__
#define __BFIN_PERIPHERAL_LOCKBOX__

#ifndef __ASSEMBLY__

#include "bootrom.h"

/* SESR argument structure. Expected to reside at 0xFF900018. */
typedef struct SESR_args {
  unsigned short  usFlags;            /* security firmware flags            */
  unsigned short  usIRQMask;          /* interrupt mask                     */
  unsigned long   ulMessageSize;      /* message length in bytes            */
  unsigned long   ulSFEntryPoint;     /* entry point of secure function     */
  unsigned long   ulMessagePtr;       /* pointer to the buffer containing
                                         the digital signature and message  */
  unsigned long   ulReserved1;        /* reserved                           */
  unsigned long   ulReserved2;        /* reserved                           */
} tSESR_args;

/* Secure Entry Service Routine */
void (* const sesr)(void) = (void *)_BOOTROM_SESR;

#endif

/* SESR flags argument bitfields                                            */
#define SESR_FLAGS_STAY_AT_NMI              0x0000
#define SESR_FLAGS_DROP_BELOW_NMI           0x0001
#define SESR_FLAGS_NO_SF_DMA                0x0000
#define SESR_FLAGS_DMA_SF_TO_RUN_DEST       0x0002
#define SESR_FLAGS_USE_ADI_PUB_KEY          0x0000
#define SESR_FLAGS_USE_CUST_PUB_KEY         0x0100

/* Bit masks for SECURE_SYSSWT */
#define EMUDABL                0x00000001    /* Emulation Disable */
#define RSTDABL                0x00000002    /* Reset Disable */
#define L1IDABL                0x0000001c    /* L1 Instruction Memory Disable */
#define L1DADABL               0x000000e0    /* L1 Data Bank A Memory Disable */
#define L1DBDABL               0x00000700    /* L1 Data Bank B Memory Disable */
#define DMA0OVR                0x00000800    /* DMA0 Memory Access Override */
#define DMA1OVR                0x00001000    /* DMA1 Memory Access Override */
#define EMUOVR                 0x00004000    /* Emulation Override */
#define OTPSEN                 0x00008000    /* OTP Secrets Enable */
#define L2DABL                 0x00070000    /* L2 Memory Disable */

/* Bit masks for SECURE_CONTROL */
#define SECURE0                0x0001        /* SECURE 0 */
#define SECURE1                0x0002        /* SECURE 1 */
#define SECURE2                0x0004        /* SECURE 2 */
#define SECURE3                0x0008        /* SECURE 3 */

/* Bit masks for SECURE_STATUS */
#define SECMODE                0x0003        /* Secured Mode Control State */
#define NMI                    0x0004        /* Non Maskable Interrupt */
#define AFVALID                0x0008        /* Authentication Firmware Valid */
#define AFEXIT                 0x0010        /* Authentication Firmware Exit */
#define SECSTAT                0x00e0        /* Secure Status */

#endif
