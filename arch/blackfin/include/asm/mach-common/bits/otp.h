/*
 * OTP Masks
 */

#ifndef __BFIN_PERIPHERAL_OTP__
#define __BFIN_PERIPHERAL_OTP__

#ifndef __ASSEMBLY__

#include "bootrom.h"

static uint32_t (* const bfrom_OtpCommand)(uint32_t command, uint32_t value) = (void *)_BOOTROM_OTP_COMMAND;
static uint32_t (* const bfrom_OtpRead)(uint32_t page, uint32_t flags, uint64_t *page_content) = (void *)_BOOTROM_OTP_READ;
static uint32_t (* const bfrom_OtpWrite)(uint32_t page, uint32_t flags, uint64_t *page_content) = (void *)_BOOTROM_OTP_WRITE;

#endif

/* otp_command(): defines for "command" */
#define OTP_INIT                 0x00000001
#define OTP_CLOSE                0x00000002

/* otp_{read,write}(): defines for "flags" */
#define OTP_LOWER_HALF           0x00000000 /* select upper/lower 64-bit half (bit 0) */
#define OTP_UPPER_HALF           0x00000001
#define OTP_NO_ECC               0x00000010 /* do not use ECC */
#define OTP_LOCK                 0x00000020 /* sets page protection bit for page */
#define OTP_CHECK_FOR_PREV_WRITE 0x00000080

/* Return values for all functions */
#define OTP_SUCCESS          0x00000000
#define OTP_MASTER_ERROR     0x001
#define OTP_WRITE_ERROR      0x003
#define OTP_READ_ERROR       0x005
#define OTP_ACC_VIO_ERROR    0x009
#define OTP_DATA_MULT_ERROR  0x011
#define OTP_ECC_MULT_ERROR   0x021
#define OTP_PREV_WR_ERROR    0x041
#define OTP_DATA_SB_WARN     0x100
#define OTP_ECC_SB_WARN      0x200

/* Predefined otp pages: Factory Programmed Settings */
#define FPS00                0x0004
#define FPS01                0x0005
#define FPS02                0x0006
#define FPS03                0x0007
#define FPS04                0x0008
#define FPS05                0x0009
#define FPS06                0x000A
#define FPS07                0x000B
#define FPS08                0x000C
#define FPS09                0x000D
#define FPS10                0x000E
#define FPS11                0x000F

/* Predefined otp pages: Customer Programmed Settings */
#define CPS00                0x0010
#define CPS01                0x0011
#define CPS02                0x0012
#define CPS03                0x0013
#define CPS04                0x0014
#define CPS05                0x0015
#define CPS06                0x0016
#define CPS07                0x0017

/* Predefined otp pages: Pre-Boot Settings */
#define PBS00                0x0018
#define PBS01                0x0019
#define PBS02                0x001A
#define PBS03                0x001B

#endif
