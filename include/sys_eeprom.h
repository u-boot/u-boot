/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SYS_EEPROM_H_
#define __SYS_EEPROM_H_

/*
 *  Without getting too philosophical, define truth, falsehood, and the
 *  boolean type, if they are not already defined.
 */
#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

#ifndef bool
typedef unsigned char bool;
#endif

#ifndef is_digit
#define is_digit(c)             ((c) >= '0' && (c) <= '9')
#endif

/*
 *  The Definition of the TlvInfo EEPROM format can be found at onie.org or
 *  github.com/onie
 */

/*
 * TlvInfo header: Layout of the header for the TlvInfo format
 *
 * See the end of this file for details of this eeprom format
 */
struct __attribute__ ((__packed__)) tlvinfo_header_s {
    char    signature[8];       /* 0x00 - 0x07 EEPROM Tag "TlvInfo" */
    u8      version;            /* 0x08        Structure version    */
    u16     totallen;           /* 0x09 - 0x0A Length of all data which follows */
};
typedef struct tlvinfo_header_s tlvinfo_header_t;

// Header Field Constants
#define TLV_INFO_ID_STRING      "TlvInfo"
#define TLV_INFO_VERSION        0x01
#define TLV_INFO_MAX_LEN        2048
#define TLV_TOTAL_LEN_MAX       (TLV_INFO_MAX_LEN - sizeof(tlvinfo_header_t))

/*
 * TlvInfo TLV: Layout of a TLV field
 */
struct __attribute__ ((__packed__)) tlvinfo_tlv_s {
    u8  type;
    u8  length;
    u8  value[0];
};
typedef struct tlvinfo_tlv_s tlvinfo_tlv_t;

/* Maximum length of a TLV value in bytes */
#define TLV_VALUE_MAX_LEN        255

/**
 *  The TLV Types.
 *
 *  Keep these in sync with tlv_code_list in cmd_sys_eeprom.c
 */
#define TLV_CODE_PRODUCT_NAME   0x21
#define TLV_CODE_PART_NUMBER    0x22
#define TLV_CODE_SERIAL_NUMBER  0x23
#define TLV_CODE_MAC_BASE       0x24
#define TLV_CODE_MANUF_DATE     0x25
#define TLV_CODE_DEVICE_VERSION 0x26
#define TLV_CODE_LABEL_REVISION 0x27
#define TLV_CODE_PLATFORM_NAME  0x28
#define TLV_CODE_ONIE_VERSION   0x29
#define TLV_CODE_MAC_SIZE       0x2A
#define TLV_CODE_MANUF_NAME     0x2B
#define TLV_CODE_MANUF_COUNTRY  0x2C
#define TLV_CODE_VENDOR_NAME    0x2D
#define TLV_CODE_DIAG_VERSION   0x2E
#define TLV_CODE_SERVICE_TAG    0x2F
#define TLV_CODE_VENDOR_EXT     0xFD
#define TLV_CODE_CRC_32         0xFE

/**
 * read_sys_eeprom - Read the EEPROM binary data from the hardware
 * @eeprom: Pointer to buffer to hold the binary data
 * @offset: Offset within EEPROM block to read data from
 * @len   : Maximum size of buffer
 *
 * This callback is implemented by board specific code.
 *
 */

extern int read_sys_eeprom(void *eeprom, int offset, int len);

/**
 * write_sys_eeprom - Write the entire EEPROM binary data to the hardware
 * @hwinfo: Pointer to buffer to hold the binary data
 * @len   : Maximum size of buffer
 *
 * This callback is implemented by board specific code.
 *
 */
extern int write_sys_eeprom(void *eeprom, int len);

/**
 * is_sys_eeprom_valid - Is the EEPROM binary data in hardware valid
 *
 * An external caller can use this to determine if the hardware
 * contains valid TLV EEPROM data.
 *
 * Returns non-zero when the binary data in hardware is valid,
 * otherwise returns zero.
 *
 */
extern int is_sys_eeprom_valid(void);

/**
 *  is_valid_tlvinfo_header
 *
 *  Perform sanity checks on the first 11 bytes of the TlvInfo EEPROM
 *  data pointed to by the parameter:
 *      1. First 8 bytes contain null-terminated ASCII string "TlvInfo"
 *      2. Version byte is 1
 *      3. Total length bytes contain value which is less than or equal
 *         to the allowed maximum (2048-11)
 *
 */
static inline bool is_valid_tlvinfo_header(tlvinfo_header_t *hdr)
{

	int max_size = min(TLV_TOTAL_LEN_MAX,
			   CONFIG_SYS_EEPROM_MAX_SIZE - sizeof(tlvinfo_header_t));

	return( (strcmp(hdr->signature, TLV_INFO_ID_STRING) == 0) &&
		(hdr->version == TLV_INFO_VERSION) &&
		(be16_to_cpu(hdr->totallen) <= max_size) );
}

#endif /* __SYS_EEPROM_H_ */
