/*
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <spi_flash.h>
#include <env.h>
#include <linux/ctype.h>
#include <nand.h>

#include "sys_eeprom.h"

/* File scope function prototypes */
static bool is_checksum_valid(u8 *eeprom);
static int read_eeprom(u8 *eeprom);
static void show_eeprom(u8 *eeprom);
static void decode_tlv(tlvinfo_tlv_t * tlv);
static void update_crc(u8 *eeprom);
static int prog_eeprom(u8 * eeprom);
static bool tlvinfo_find_tlv(u8 * eeprom, u8 tcode, int * eeprom_index);
static bool tlvinfo_delete_tlv(u8 * eeprom, u8 code);
static bool tlvinfo_add_tlv(u8 * eeprom, int tcode, char * strval);
static int set_mac(char *buf, const char *string);
static int set_date(char *buf, const char *string);
static int set_bytes(char *buf, const char *string, int * converted_accum);

/* Set to 1 if we've read EEPROM into memory */
static int has_been_read = 0;
/* Set to 1 if the EEPROM contents were valid when read from hardware */
static int hw_eeprom_valid = 1;
/* The EERPOM contents after being read into memory */
static u8 eeprom[TLV_INFO_MAX_LEN];

/**
 *  is_valid_tlv
 *
 *  Perform basic sanity checks on a TLV field. The TLV is pointed to
 *  by the parameter provided.
 *      1. The type code is not reserved (0x00 or 0xFF)
 */
static inline bool is_valid_tlv(tlvinfo_tlv_t *tlv)
{
	return( (tlv->type != 0x00) &&
		(tlv->type != 0xFF) );
}

/**
 *  is_hex
 *
 *  Tests if character is an ASCII hex digit
 */
static inline u8 is_hex(char p)
{
	return (((p >= '0') && (p <= '9')) ||
		((p >= 'A') && (p <= 'F')) ||
		((p >= 'a') && (p <= 'f')));
}

/**
 *  is_checksum_valid
 *
 *  Validate the checksum in the provided TlvInfo EEPROM data. First,
 *  verify that the TlvInfo header is valid, then make sure the last
 *  TLV is a CRC-32 TLV. Then calculate the CRC over the EEPROM data
 *  and compare it to the value stored in the EEPROM CRC-32 TLV.
 */
static bool is_checksum_valid(u8 *eeprom)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t    * eeprom_crc;
	unsigned int       calc_crc;
	unsigned int       stored_crc;

	// Is the eeprom header valid?
	if (!is_valid_tlvinfo_header(eeprom_hdr)) {
		return(FALSE);
	}

	// Is the last TLV a CRC?
	eeprom_crc = (tlvinfo_tlv_t *) &eeprom[sizeof(tlvinfo_header_t) +
					       be16_to_cpu(eeprom_hdr->totallen) - (sizeof(tlvinfo_tlv_t) + 4)];
	if ((eeprom_crc->type != TLV_CODE_CRC_32) || (eeprom_crc->length != 4)) {
		return(FALSE);
	}

	// Calculate the checksum
	calc_crc = crc32(0, (void *)eeprom,
			 sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen) - 4);
	stored_crc = (eeprom_crc->value[0] << 24) |
		(eeprom_crc->value[1] << 16) |
		(eeprom_crc->value[2] <<  8) |
		eeprom_crc->value[3];
	return( calc_crc == stored_crc);
}

/**
 *  read_eeprom
 *
 *  Read the EEPROM into memory, if it hasn't already been read.
 */
static int read_eeprom(u8 *eeprom)
{
	int ret;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
#ifndef CONFIG_SYS_EEPROM_USE_COMMON_NAND_IO
	tlvinfo_tlv_t    * eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[sizeof(tlvinfo_header_t)];
#endif

	if (has_been_read)
		return 0;

	/* Read the header */
	ret = read_sys_eeprom((void *)eeprom_hdr, 0, sizeof(tlvinfo_header_t));
	/* If the header was successfully read, read the TLVs */
	if ((ret == 0) && is_valid_tlvinfo_header(eeprom_hdr)) {
#ifdef CONFIG_SYS_EEPROM_USE_COMMON_NAND_IO
		/* NAND flash requires page-aligned read */
		ret = read_sys_eeprom((void *)eeprom_hdr, 0,
				be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_header_t));
#else
		ret = read_sys_eeprom((void *)eeprom_tlv, sizeof(tlvinfo_header_t),
				  be16_to_cpu(eeprom_hdr->totallen));
#endif
	}

	// If the contents are invalid, start over with default contents
	if ( !is_valid_tlvinfo_header(eeprom_hdr) || !is_checksum_valid(eeprom) ){
		strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
		eeprom_hdr->version = TLV_INFO_VERSION;
		eeprom_hdr->totallen = cpu_to_be16(0);
		update_crc(eeprom);
		/* Note that the contents of the hardware is not valid */
		hw_eeprom_valid = 0;
	}

	has_been_read = 1;

#ifdef DEBUG
	show_eeprom(eeprom);
#endif

	return ret;
}

/**
 *  show_eeprom
 *
 *  Display the contents of the EEPROM
 */
static void show_eeprom(u8 *eeprom)
{
	int tlv_end;
	int curr_tlv;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t    * eeprom_tlv;

	if ( !is_valid_tlvinfo_header(eeprom_hdr) ) {
		printf("EEPROM does not contain data in a valid TlvInfo format.\n");
		return;
	}

	printf("TlvInfo Header:\n");
	printf("   Id String:    %s\n", eeprom_hdr->signature);
	printf("   Version:      %d\n", eeprom_hdr->version);
	printf("   Total Length: %d\n", be16_to_cpu(eeprom_hdr->totallen));

	printf("TLV Name             Code Len Value\n");
	printf("-------------------- ---- --- -----\n");
	curr_tlv = sizeof(tlvinfo_header_t);
	tlv_end  = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
	while (curr_tlv < tlv_end) {
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[curr_tlv];
		if (!is_valid_tlv(eeprom_tlv)) {
			printf("Invalid TLV field starting at EEPROM offset %d\n", curr_tlv);
			return;
		}
		decode_tlv(eeprom_tlv);
		curr_tlv += sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
	}

	printf("Checksum is %s.\n", is_checksum_valid(eeprom) ? "valid" : "invalid");

#ifdef DEBUG
	printf("EEPROM dump: (0x%x bytes)", TLV_INFO_MAX_LEN);
	for (i = 0; i < TLV_INFO_MAX_LEN; i++) {
		if ((i % 16) == 0)
			printf("\n%02X: ", i);
		printf("%02X ", eeprom[i]);
	}
	printf("\n");
#endif

	return;
}

/**
 *  Struct for displaying the TLV codes and names.
 */
struct tlv_code_desc {
	u8    m_code;
	char* m_name;
};

/**
 *  List of TLV codes and names.
 */
static struct tlv_code_desc tlv_code_list[] = {
	{ TLV_CODE_PRODUCT_NAME	 , "Product Name"},
	{ TLV_CODE_PART_NUMBER	 , "Part Number"},
	{ TLV_CODE_SERIAL_NUMBER , "Serial Number"},
	{ TLV_CODE_MAC_BASE	 , "Base MAC Address"},
	{ TLV_CODE_MANUF_DATE	 , "Manufacture Date"},
	{ TLV_CODE_DEVICE_VERSION, "Device Version"},
	{ TLV_CODE_LABEL_REVISION, "Label Revision"},
	{ TLV_CODE_PLATFORM_NAME , "Platform Name"},
	{ TLV_CODE_ONIE_VERSION	 , "ONIE Version"},
	{ TLV_CODE_MAC_SIZE	 , "MAC Addresses"},
	{ TLV_CODE_MANUF_NAME	 , "Manufacturer"},
	{ TLV_CODE_MANUF_COUNTRY , "Country Code"},
	{ TLV_CODE_VENDOR_NAME	 , "Vendor Name"},
	{ TLV_CODE_DIAG_VERSION	 , "Diag Version"},
	{ TLV_CODE_SERVICE_TAG   , "Service Tag"},
	{ TLV_CODE_VENDOR_EXT	 , "Vendor Extension"},
	{ TLV_CODE_CRC_32	 , "CRC-32"},
};

/**
 *  Look up a TLV name by its type.
 */
static inline const char* tlv_type2name(u8 type)
{
	char* name = "Unknown";
	int   i;

	for (i = 0; i < sizeof(tlv_code_list)/sizeof(tlv_code_list[0]); i++) {
		if (tlv_code_list[i].m_code == type) {
			name = tlv_code_list[i].m_name;
			break;
		}
	}

	return name;
}

/*
 *  decode_tlv
 *
 *  Print a string representing the contents of the TLV field. The format of
 *  the string is:
 *      1. The name of the field left justified in 20 characters
 *      2. The type code in hex right justified in 5 characters
 *      3. The length in decimal right justified in 4 characters
 *      4. The value, left justified in however many characters it takes
 *  The validity of EEPROM contents and the TLV field have been verified
 *  prior to calling this function.
 */
#define DECODE_NAME_MAX     20

/*
 * The max decode value is currently for the 'raw' type or the 'vendor
 * extension' type, both of which have the same decode format.  The
 * max decode string size is computed as follows:
 *
 *   strlen(" 0xFF") * TLV_VALUE_MAX_LEN + 1
 *
 */
#define DECODE_VALUE_MAX    ((5 * TLV_VALUE_MAX_LEN) + 1)

static void decode_tlv(tlvinfo_tlv_t * tlv)
{
	char name[DECODE_NAME_MAX];
	char value[DECODE_VALUE_MAX];
	int i;

	strncpy(name, tlv_type2name(tlv->type), DECODE_NAME_MAX);

	switch (tlv->type) {
	case TLV_CODE_PRODUCT_NAME:
	case TLV_CODE_PART_NUMBER:
	case TLV_CODE_SERIAL_NUMBER:
	case TLV_CODE_MANUF_DATE:
	case TLV_CODE_LABEL_REVISION:
	case TLV_CODE_PLATFORM_NAME:
	case TLV_CODE_ONIE_VERSION:
	case TLV_CODE_MANUF_NAME:
	case TLV_CODE_MANUF_COUNTRY:
	case TLV_CODE_VENDOR_NAME:
	case TLV_CODE_DIAG_VERSION:
	case TLV_CODE_SERVICE_TAG:
		memcpy(value, tlv->value, tlv->length);
		value[tlv->length] = 0;
		break;
	case TLV_CODE_MAC_BASE:
		sprintf(value, "%02X:%02X:%02X:%02X:%02X:%02X",
			tlv->value[0], tlv->value[1], tlv->value[2],
			tlv->value[3], tlv->value[4], tlv->value[5]);
		break;
	case TLV_CODE_DEVICE_VERSION:
		sprintf(value, "%u", tlv->value[0]);
		break;
	case TLV_CODE_MAC_SIZE:
		sprintf(value, "%u", (tlv->value[0] << 8) | tlv->value[1]);
		break;
	case TLV_CODE_VENDOR_EXT:
		value[0] = 0;
		for (i = 0; (i < (DECODE_VALUE_MAX/5)) && (i < tlv->length); i++) {
			sprintf(value, "%s 0x%02X", value, tlv->value[i]);
		}
		break;
	case TLV_CODE_CRC_32:
		sprintf(value, "0x%02X%02X%02X%02X",
			tlv->value[0], tlv->value[1], tlv->value[2], tlv->value[3]);
		break;
	default:
		value[0] = 0;
		for (i = 0; (i < (DECODE_VALUE_MAX/5)) && (i < tlv->length); i++) {
			sprintf(value, "%s 0x%02X", value, tlv->value[i]);
		}
		break;
	}

	name[DECODE_NAME_MAX-1] = 0;
	printf("%-20s 0x%02X %3d %s\n", name, tlv->type, tlv->length, value);
	return;
}

/**
 *  update_crc
 *
 *  This function updates the CRC-32 TLV. If there is no CRC-32 TLV, then
 *  one is added. This function should be called after each update to the
 *  EEPROM structure, to make sure the CRC is always correct.
 */
static void update_crc(u8 *eeprom)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t    * eeprom_crc;
	unsigned int       calc_crc;
	int                eeprom_index;

	// Discover the CRC TLV
	if (!tlvinfo_find_tlv(eeprom, TLV_CODE_CRC_32, &eeprom_index)) {
		if ((be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + 4) > TLV_TOTAL_LEN_MAX) {
			return;
		}
		eeprom_index = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
		eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) +
						   sizeof(tlvinfo_tlv_t) + 4);
	}
	eeprom_crc = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
	eeprom_crc->type = TLV_CODE_CRC_32;
	eeprom_crc->length = 4;

	// Calculate the checksum
	calc_crc = crc32(0, (void *)eeprom,
			 sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen) - 4);
	eeprom_crc->value[0] = (calc_crc >> 24) & 0xFF;
	eeprom_crc->value[1] = (calc_crc >> 16) & 0xFF;
	eeprom_crc->value[2] = (calc_crc >>  8) & 0xFF;
	eeprom_crc->value[3] = (calc_crc >>  0) & 0xFF;

	return;
}

/**
 *  prog_eeprom
 *
 *  Write the EEPROM data from CPU memory to the hardware.
 */
static int prog_eeprom(u8 * eeprom)
{
	int ret = 0;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	int eeprom_len;

	update_crc(eeprom);

	eeprom_len = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
	ret = write_sys_eeprom(eeprom, eeprom_len);
	if (ret) {
		printf("Programming failed.\n");
		return -1;
	}

	/* After writing the HW contents are valid */
	hw_eeprom_valid = 1;

	printf("Programming passed.\n");
	return 0;
}

/**
 * is_sys_eeprom_valid - Is the EEPROM binary data in hardware valid
 */
int is_sys_eeprom_valid()
{
	return hw_eeprom_valid;
}

/**
 *  show_tlv_code_list - Display the list of TLV codes and names
 */
void show_tlv_code_list(void)
{
	int i;

	printf("TLV Code    TLV Name\n");
	printf("========    =================\n");
	for (i = 0; i < sizeof(tlv_code_list)/sizeof(tlv_code_list[0]); i++) {
		printf("0x%02X        %s\n",
		       tlv_code_list[i].m_code,
		       tlv_code_list[i].m_name);
	}
}

/**
 *  do_sys_eeprom
 *
 *  This function implements the sys_eeprom command.
 */
int do_sys_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char cmd;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;

	// If no arguments, read the EERPOM and display its contents
	if (argc == 1) {
		read_eeprom(eeprom);
		show_eeprom(eeprom);
		return 0;
	}

	// We only look at the first character to the command, so "read" and "reset"
	// will both be treated as "read".
	cmd = argv[1][0];

	// Read the EEPROM contents
	if (cmd == 'r') {
		has_been_read = 0;
		if (!read_eeprom(eeprom))
			printf("EEPROM data loaded from device to memory.\n");
		return 0;
	}

	// Subsequent commands require that the EEPROM has already been read.
	if (!has_been_read) {
		printf("Please read the EEPROM data first, using the 'sys_eeprom read' command.\n");
		return 0;
	}

	// Handle the commands that don't take parameters
	if (argc == 2) {
		switch (cmd) {
		case 'w':   /* write */
			prog_eeprom(eeprom);
			break;
		case 'e':   /* erase */
			strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
			eeprom_hdr->version = TLV_INFO_VERSION;
			eeprom_hdr->totallen = cpu_to_be16(0);
			update_crc(eeprom);
			printf("EEPROM data in memory reset.\n");
			break;
		case 'l':   /* list */
			show_tlv_code_list();
			break;
		default:
			cmd_usage(cmdtp);
			break;
		}
		return 0;
	}

	// The set command takes one or two args.
	if (argc > 4) {
		cmd_usage(cmdtp);
		return 0;
	}

	// Set command. If the TLV exists in the EEPROM, delete it. Then if data was
	// supplied for this TLV add the TLV with the new contents at the end.
	if (cmd == 's') {
		int tcode;
		tcode = simple_strtoul(argv[2], NULL, 0);
		tlvinfo_delete_tlv(eeprom, tcode);
		if (argc == 4) {
			tlvinfo_add_tlv(eeprom, tcode, argv[3]);
		}
	} else {
		cmd_usage(cmdtp);
	}

	return 0;
}

/**
 *  This macro defines the sys_eeprom command line command.
 */
U_BOOT_CMD(
	sys_eeprom, 4, 1,  do_sys_eeprom,
	"Display and program the system EEPROM data block.",
	"[read|write|set <type_code> <string_value>|erase|list]\n"
	"sys_eeprom\n"
	"    - With no arguments display the current contents.\n"
	"sys_eeprom read\n"
	"    - Load EEPROM data from device to memory.\n"
	"sys_eeprom write\n"
	"    - Write the EEPROM data to persistent storage.\n"
	"sys_eeprom set <type_code> <string_value>\n"
	"    - Set a field to a value.\n"
	"    - If no string_value, field is deleted.\n"
	"    - Use 'sys_eeprom write' to make changes permanent.\n"
	"sys_eeprom erase\n"
	"    - Reset the in memory EEPROM data.\n"
	"    - Use 'sys_eeprom read' to refresh the in memory EEPROM data.\n"
	"    - Use 'sys_eeprom write' to make changes permanent.\n"
	"sys_eeprom list\n"
	"    - List the understood TLV codes and names.\n"
	);

/**
 *  tlvinfo_find_tlv
 *
 *  This function finds the TLV with the supplied code in the EERPOM.
 *  An offset from the beginning of the EEPROM is returned in the
 *  eeprom_index parameter if the TLV is found.
 */
static bool tlvinfo_find_tlv(u8 * eeprom, u8 tcode, int * eeprom_index)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t    * eeprom_tlv;
	int eeprom_end;

	// Search through the TLVs, looking for the first one which matches the
	// supplied type code.
	*eeprom_index = sizeof(tlvinfo_header_t);
	eeprom_end    = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
	while (*eeprom_index < eeprom_end) {
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[*eeprom_index];
		if (!is_valid_tlv(eeprom_tlv)) {
			return(FALSE);
		}
		if (eeprom_tlv->type == tcode) {
			return(TRUE);
		}
		*eeprom_index += sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
	}
	return(FALSE);
}

/**
 *  tlvinfo_delete_tlv
 *
 *  This function deletes the TLV with the specified type code from the
 *  EEPROM.
 */
static bool tlvinfo_delete_tlv(u8 * eeprom, u8 code)
{
	int eeprom_index;
	int tlength;
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;

	// Find the TLV and then move all following TLVs "forward"
	if (tlvinfo_find_tlv(eeprom, code, &eeprom_index)) {
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
		tlength = sizeof(tlvinfo_tlv_t) + eeprom_tlv->length;
		memcpy(&eeprom[eeprom_index], &eeprom[eeprom_index+tlength],
		       sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen) - eeprom_index - tlength);
		eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) - tlength);
		update_crc(eeprom);
		return(TRUE);
	}
	return(FALSE);
}

/**
 *  tlvinfo_add_tlv
 *
 *  This function adds a TLV to the EEPROM, converting the value (a string) to
 *  the format in which it will be stored in the EEPROM.
 */
#define MAX_TLV_VALUE_LEN   256
static bool tlvinfo_add_tlv(u8 * eeprom, int tcode, char * strval)
{
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;
	tlvinfo_tlv_t * eeprom_tlv;
	int new_tlv_len = 0;
	u32 value;
	char data[MAX_TLV_VALUE_LEN];
	int eeprom_index;
	int max_size = min(TLV_TOTAL_LEN_MAX,
			   CONFIG_SYS_EEPROM_MAX_SIZE - sizeof(tlvinfo_header_t));

	// Encode each TLV type into the format to be stored in the EERPOM
	switch (tcode) {
	case TLV_CODE_PRODUCT_NAME:
	case TLV_CODE_PART_NUMBER:
	case TLV_CODE_SERIAL_NUMBER:
	case TLV_CODE_LABEL_REVISION:
	case TLV_CODE_PLATFORM_NAME:
	case TLV_CODE_ONIE_VERSION:
	case TLV_CODE_MANUF_NAME:
	case TLV_CODE_MANUF_COUNTRY:
	case TLV_CODE_VENDOR_NAME:
	case TLV_CODE_DIAG_VERSION:
	case TLV_CODE_SERVICE_TAG:
		strncpy(data, strval, MAX_TLV_VALUE_LEN);
		new_tlv_len = min(MAX_TLV_VALUE_LEN, strlen(strval));
		break;
	case TLV_CODE_DEVICE_VERSION:
		value = simple_strtoul(strval, NULL, 0);
		if (value >= 256) {
			printf("ERROR: Device version must be 255 or less. Value supplied: %u", value);
			return(FALSE);
		}
		data[0] = value & 0xFF;
		new_tlv_len = 1;
		break;
	case TLV_CODE_MAC_SIZE:
		value = simple_strtoul(strval, NULL, 0);
		if (value >= 65536) {
			printf("ERROR: MAC Size must be 65535 or less. Value supplied: %u", value);
			return(FALSE);
		}
		data[0] = (value >> 8) & 0xFF;
		data[1] = value & 0xFF;
		new_tlv_len = 2;
		break;
	case TLV_CODE_MANUF_DATE:
		if (set_date(data, strval) != 0) {
			return(FALSE);
		}
		new_tlv_len = 19;
		break;
	case TLV_CODE_MAC_BASE:
		if (set_mac(data, strval) != 0) {
			return(FALSE);
		}
		new_tlv_len = 6;
		break;
	case TLV_CODE_CRC_32:
		printf("WARNING: The CRC TLV is set automatically and cannot be set manually.\n");
		return(FALSE);
	case TLV_CODE_VENDOR_EXT:
	default:
		if (set_bytes(data, strval, &new_tlv_len) != 0 ) {
			return(FALSE);
		}
		break;
	}

	// Is there room for this TLV?
	if ((be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + new_tlv_len) > max_size) {
		printf("ERROR: There is not enough room in the EERPOM to save data.\n");
		return(FALSE);
	}

	// Add TLV at the end, overwriting CRC TLV if it exists
	if (tlvinfo_find_tlv(eeprom, TLV_CODE_CRC_32, &eeprom_index)) {
		eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) - sizeof(tlvinfo_tlv_t) - 4);
	}
	else {
		eeprom_index = sizeof(tlvinfo_header_t) + be16_to_cpu(eeprom_hdr->totallen);
	}
	eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
	eeprom_tlv->type = tcode;
	eeprom_tlv->length = new_tlv_len;
	memcpy(eeprom_tlv->value, data, new_tlv_len);

	// Update the total length and calculate (add) a new CRC-32 TLV
	eeprom_hdr->totallen = cpu_to_be16(be16_to_cpu(eeprom_hdr->totallen) + sizeof(tlvinfo_tlv_t) + new_tlv_len);
	update_crc(eeprom);

	return(TRUE);
}

/**
 *  set_mac
 *
 *  Converts a string MAC address into a binary buffer.
 *
 *  This function takes a pointer to a MAC address string
 *  (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number).
 *  The string format is verified and then converted to binary and
 *  stored in a buffer.
 */
static int set_mac(char *buf, const char *string)
{
	char *p = (char *) string;
	int   i;
	int   err = 0;
	char *end;

	if (!p) {
		printf("ERROR: NULL mac addr string passed in.\n");
		return -1;
	}

	if (strlen(p) != 17) {
		printf("ERROR: MAC address strlen() != 17 -- %d\n", strlen(p));
		printf("ERROR: Bad MAC address format: %s\n", string);
		return -1;
	}

	for (i = 0; i < 17; i++) {
		if ((i % 3) == 2) {
			if (p[i] != ':') {
				err++;
				printf("ERROR: mac: p[%i] != :, found: `%c'\n",
				       i, p[i]);
				break;
			}
			continue;
		} else if (!is_hex(p[i])) {
			err++;
			printf("ERROR: mac: p[%i] != hex digit, found: `%c'\n",
			       i, p[i]);
			break;
		}
	}

	if (err != 0) {
		printf("ERROR: Bad MAC address format: %s\n", string);
		return -1;
	}

	/* Convert string to binary */
	for (i = 0, p = (char *)string; i < 6; i++) {
		buf[i] = p ? simple_strtoul(p, &end, 16) : 0;
		if (p) {
			p = (*end) ? end + 1 : end;
		}
	}

	if (!is_valid_ethaddr((u8 *)buf)) {
		printf("ERROR: MAC address must not be 00:00:00:00:00:00, "
		       "a multicast address or FF:FF:FF:FF:FF:FF.\n");
		printf("ERROR: Bad MAC address format: %s\n", string);
		return -1;
	}

	return 0;
}

/**
 *  set_date
 *
 *  Validates the format of the data string
 *
 *  This function takes a pointer to a date string (i.e. MM/DD/YYYY hh:mm:ss)
 *  and validates that the format is correct. If so the string is copied
 *  to the supplied buffer.
 */
static int set_date(char *buf, const char *string)
{
	int i;

	if (!string) {
		printf("ERROR: NULL date string passed in.\n");
		return -1;
	}

	if (strlen(string) != 19) {
		printf("ERROR: Date strlen() != 19 -- %d\n", strlen(string));
		printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
		return -1;
	}

	for (i = 0; string[i] != 0; i++) {
		switch (i) {
		case 2:
		case 5:
			if (string[i] != '/') {
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
				return -1;
			}
			break;
		case 10:
			if (string[i] != ' ') {
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
				return -1;
			}
			break;
		case 13:
		case 16:
			if (string[i] != ':') {
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
				return -1;
			}
			break;
		default:
			if (!is_digit(string[i])) {
				printf("ERROR: Bad date format (MM/DD/YYYY hh:mm:ss): %s\n", string);
				return -1;
			}
			break;
		}
	}

	strcpy(buf, string);
	return 0;
}

/**
 *  set_bytes
 *
 *  Converts a space-separated string of decimal numbers into a
 *  buffer of bytes.
 *
 *  This function takes a pointer to a space-separated string of decimal
 *  numbers (i.e. "128 0x55 0321") with "C" standard radix specifiers
 *  and converts them to an array of bytes.
 */
static int set_bytes(char *buf, const char *string, int * converted_accum)
{
	char *p = (char *) string;
	int   i;
	uint  byte;

	if (!p) {
		printf("ERROR: NULL string passed in.\n");
		return -1;
	}

	/* Convert string to bytes */
	for (i = 0, p = (char *)string; (i < TLV_VALUE_MAX_LEN) && (*p != 0); i++) {
		while ((*p == ' ') || (*p == '\t') || (*p == ',') || (*p == ';')) {
			p++;
		}
		if (*p != 0) {
			if (!is_digit(*p)) {
				printf("ERROR: Non-digit found in byte string: (%s)\n", string);
				return -1;
			}
			byte = simple_strtoul(p, &p, 0);
			if (byte >= 256) {
				printf("ERROR: The value specified is greater than 255: (%u) in string: %s\n",
				       byte, string);
				return -1;
			}
			buf[i] = byte & 0xFF;
		}
	}

	if ((i == TLV_VALUE_MAX_LEN) && (*p != 0)) {
		printf("ERROR: Trying to assign too many bytes "
		       "(max: %d) in string: %s\n", TLV_VALUE_MAX_LEN, string);
		return -1;
	}

	*converted_accum = i;
	return 0;
}

#ifdef CONFIG_SYS_EEPROM_USE_COMMON_FLASH_IO

/**
 * Provide sys_eeprom read/write functions for platforms that store
 * the information in a flash sector.
 */

#ifndef CONFIG_SYS_FLASH_HWINFO_ADDR
# error CONFIG_SYS_FLASH_HWINFO_ADDR must be defined when using common flash i/o.
#endif

#ifndef CONFIG_SYS_FLASH_HWINFO_SECT_SIZE
# error CONFIG_SYS_FLASH_HWINFO_SECT_SIZE must be defined when using common flash i/o.
#endif

/**
 * read_sys_eeprom - read the hwinfo from flash
 */
int read_sys_eeprom(void *eeprom, int offset, int len)
{
	memcpy(eeprom, (void *)(CONFIG_SYS_FLASH_HWINFO_ADDR + offset), len);
	return 0;
}

/**
 * write_sys_eeprom - write the hwinfo to flash
 */
int write_sys_eeprom(void *eeprom, int len)
{
	int ret;
	int addr_start, addr_last;

	addr_start = CONFIG_SYS_FLASH_HWINFO_ADDR;
	addr_last  = CONFIG_SYS_FLASH_HWINFO_ADDR +
		CONFIG_SYS_FLASH_HWINFO_SECT_SIZE - 1;

	/* unprotect CONFIG_SYS_FLASH_HWINFO_ADDR */
	ret = flash_sect_protect(0 /* protect off */,
				 addr_start, addr_last);
	if (ret != 0) {
		printf("Unprotecting flash range 0x%08x - 0x%08x failed.\n",
		       addr_start, addr_last);
		flash_perror(ret);
		return -1;
	}

	/* erase CONFIG_SYS_FLASH_HWINFO_ADDR */
	ret = flash_sect_erase(addr_start, addr_last);
	if (ret != 0) {
		printf("Erasing flash range 0x%08x - 0x%08x failed.\n",
		       addr_start, addr_last);
		flash_perror(ret);
		return -1;
	}

	/* flash write eeprom data to CONFIG_SYS_FLASH_HWINFO_ADDR */
	ret = flash_write(eeprom, addr_start, len);
	if (ret != 0) {
		printf("Writing %d bytes to flash addr 0x%08x failed.\n",
		       len, addr_start);
		flash_perror(ret);
		return -1;
	}

	/* protect CONFIG_SYS_FLASH_HWINFO_ADDR */
	ret = flash_sect_protect(1 /* protect on */,
				 addr_start, addr_last);
	if (ret != 0) {
		printf("Protecting flash range 0x%08x - 0x%08x failed.\n",
		       addr_start, addr_last);
		flash_perror(ret);
		return -1;
	}

	return 0;
}

#endif /* CONFIG_SYS_EEPROM_USE_COMMON_FLASH_IO */

#ifdef CONFIG_SYS_EEPROM_USE_COMMON_SPI_IO

/**
 *  Use common spi read/write functions
 *
 */

#ifndef CONFIG_SYS_SPI_FLASH_HWINFO_ADDR
# error CONFIG_SYS_SPI_FLASH_HWINFO_ADDR must be defined when using common spi i/o.
#endif
#ifndef CONFIG_SYS_SPI_FLASH_HWINFO_SECT_SIZE
# error CONFIG_SYS_SPI_FLASH_HWINFO_SECT_SIZE must be defined when using common spi i/o.
#endif

int read_sys_eeprom(void *eeprom, int offset, int len)
{
	int ret;
	struct spi_flash *env_flash = NULL;

	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
				CONFIG_ENV_SPI_CS,
				CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
		if (!env_flash) {
			env_set_default("!spi_flash_probe() failed", 0);
			return -1;
		}
	}

	ret = spi_flash_read(env_flash, CONFIG_SYS_SPI_FLASH_HWINFO_ADDR+offset, len, eeprom);
	if (ret) {
		printf("SPI Read Fail!\n");
		return -1;
	}

	return 0;
}

int write_sys_eeprom(void *eeprom, int len)
{
	int ret;
	struct spi_flash *env_flash = NULL;

	if (len > CONFIG_SYS_SPI_FLASH_HWINFO_SECT_SIZE) {
		printf("write_sys_eeprom() failed, len > HWINFO SIZE\n");
		return -1;
	}

	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
				CONFIG_ENV_SPI_CS,
				CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE);
		if (!env_flash) {
			env_set_default("!spi_flash_probe() failed", 0);
			return -1;
		}
	}

	ret = spi_flash_erase(env_flash, CONFIG_SYS_SPI_FLASH_HWINFO_ADDR,
			CONFIG_SYS_SPI_FLASH_HWINFO_SECT_SIZE);
	if (ret) {
		printf("SPI Flash erase failed\n");
		return 1;
	}

	ret = spi_flash_write(env_flash, CONFIG_SYS_SPI_FLASH_HWINFO_ADDR, len, eeprom);
	if (ret) {
		printf("SPI Flash write failed\n");
		return 1;
	}

	return 0;
}

#endif /* CONFIG_SYS_EEPROM_USE_COMMON_SPI_IO */

#ifdef CONFIG_SYS_EEPROM_USE_COMMON_I2C_IO

/**
 *  Use common i2c read/write functions
 *
 */

#ifndef CONFIG_SYS_I2C_EEPROM_ADDR
# error CONFIG_SYS_I2C_EEPROM_ADDR must be defined when using common i2c i/o.
#endif
#ifndef CONFIG_SYS_I2C_EEPROM_ADDR_LEN
# error CONFIG_SYS_I2C_EEPROM_ADDR_LEN must be defined when using common i2c i/o.
#endif
#ifndef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
# error CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS must be defined when using common i2c i/o.
#endif
#ifndef CONFIG_SYS_EEPROM_PAGE_WRITE_BITS
# error CONFIG_SYS_EEPROM_PAGE_WRITE_BITS must be defined when using common i2c i/o.
#endif

#ifdef CONFIG_SYS_EEPROM_OFFSET
#  define EEPROM_OFFSET CONFIG_SYS_EEPROM_OFFSET
#else
#  define EEPROM_OFFSET 0x0
#endif

/**
 * read_sys_eeprom - read the hwinfo from i2c EEPROM
 */
int read_sys_eeprom(void *eeprom, int offset, int len)
{
	int ret;
#ifndef CONFIG_DM_I2C
#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	unsigned int bus;
#endif
#ifdef CONFIG_SYS_I2C_EEPROM_MUX_ADDR
	uchar data = (1 << CONFIG_SYS_I2C_EEPROM_MUX_CHAN);
#endif

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_EEPROM_BUS_NUM);
#endif
#ifdef CONFIG_SYS_I2C_EEPROM_MUX_ADDR
	i2c_write(CONFIG_SYS_I2C_EEPROM_MUX_ADDR,
			CONFIG_SYS_I2C_EEPROM_MUX_CTRL, 1, &data, 1);
#endif
#endif
	ret = eeprom_read(CONFIG_SYS_I2C_EEPROM_ADDR,
			  EEPROM_OFFSET + offset,
			  eeprom, len);

#ifndef CONFIG_DM_I2C
#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	i2c_set_bus_num(bus);
#endif
#endif

	return ret;
}

/**
 * write_sys_eeprom - write the hwinfo to i2c EEPROM
 */
int write_sys_eeprom(void *eeprom, int len)
{
	int ret = 0;
	int i;
	void *p;
#ifndef CONFIG_DM_I2C
#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	unsigned int bus;
#endif
#ifdef CONFIG_SYS_I2C_EEPROM_MUX_ADDR
	uchar data = (1 << CONFIG_SYS_I2C_EEPROM_MUX_CHAN);
#endif

#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_EEPROM_BUS_NUM);
#endif
#ifdef CONFIG_SYS_I2C_EEPROM_MUX_ADDR
	i2c_write(CONFIG_SYS_I2C_EEPROM_MUX_ADDR,
			CONFIG_SYS_I2C_EEPROM_MUX_CTRL, 1, &data, 1);
#endif
#endif
	ret = eeprom_write(CONFIG_SYS_I2C_EEPROM_ADDR,
			   EEPROM_OFFSET,
			   eeprom, len);

#ifndef CONFIG_DM_I2C
#ifdef CONFIG_SYS_EEPROM_BUS_NUM
	i2c_set_bus_num(bus);
#endif
#endif

	return ret;
}

#endif /* CONFIG_SYS_EEPROM_USE_COMMON_I2C_IO */

#ifdef CONFIG_SYS_EEPROM_USE_COMMON_NAND_IO

/**
 * Provide sys_eeprom read/write functions for platforms that store
 * the information in a flash sector.
 */

#ifndef CONFIG_SYS_NAND_HWINFO_OFFSET
# error CONFIG_SYS_NAND_HWINFO_OFFSET must be defined when using common nand i/o.
#endif

#ifndef CONFIG_SYS_NAND_HWINFO_SECT_SIZE
# error CONFIG_SYS_NAND_HWINFO_SECT_SIZE must be defined when using common nand i/o.
#endif

/**
 * read_sys_eeprom - read the hwinfo from nand flash
 */
int read_sys_eeprom(void *eeprom, int offset, int len)
{
	size_t readlen = len;

	return nand_read_skip_bad(&nand_info[0],
			CONFIG_SYS_NAND_HWINFO_OFFSET + offset,
			&readlen, (u_char *)eeprom);
}

/**
 * write_sys_eeprom - write the hwinfo to nand flash
 */
int write_sys_eeprom(void *eeprom, int len)
{
	size_t writelen = len;

	return nand_write_skip_bad(&nand_info[0],
			CONFIG_SYS_NAND_HWINFO_OFFSET, &writelen,
			(u_char *)eeprom, 0);
}
#endif /* CONFIG_SYS_EEPROM_USE_COMMON_NAND_IO */

#ifdef CONFIG_SYS_EEPROM_LOAD_ENV_MAC

/**
 *  mac_read_from_eeprom
 *
 *  Read the MAC addresses from EEPROM
 *
 *  This function reads the MAC addresses from EEPROM and sets the
 *  appropriate environment variables for each one read.
 *
 *  The environment variables are only set if they haven't been set already.
 *  This ensures that any user-saved variables are never overwritten.
 *
 *  This function must be called after relocation.
 */
int mac_read_from_eeprom(void)
{
	unsigned int i;
	int eeprom_index;
	tlvinfo_tlv_t * eeprom_tlv;
	int maccount;
	u8 macbase[6];
	tlvinfo_header_t * eeprom_hdr = (tlvinfo_header_t *) eeprom;

	puts("EEPROM: ");

#ifdef CONFIG_SYS_I2C_EEPROM_MUX_ADDR
	/*
	 * Required one time in the case of Helix4 / Broadcom iproc
	 * i2c driver in order to init the i2c before read/write
	 */
	i2c_probe(CONFIG_SYS_I2C_EEPROM_MUX_ADDR);
#endif
	if (read_eeprom(eeprom)) {
		printf("Read failed.\n");
		return -1;
	}

	maccount = 1;
	if (tlvinfo_find_tlv(eeprom, TLV_CODE_MAC_SIZE, &eeprom_index)) {
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
		maccount = (eeprom_tlv->value[0] << 8) | eeprom_tlv->value[1];
	}

	memcpy(macbase, "\0\0\0\0\0\0", 6);
	if (tlvinfo_find_tlv(eeprom, TLV_CODE_MAC_BASE, &eeprom_index)) {
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
		memcpy(macbase, eeprom_tlv->value, 6);
	}

	for (i = 0; i < min(maccount, CONFIG_SYS_EEPROM_MAX_NUM_ETH_PORTS); i++) {
		if (is_valid_ethaddr(macbase)) {
			char ethaddr[18];
			char enetvar[11];

			sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
				macbase[0], macbase[1], macbase[2],
				macbase[3], macbase[4], macbase[5]);
			sprintf(enetvar, i ? "eth%daddr" : "ethaddr", i);
#if 1
{
			/* aaron_lien, 2013/08/05,
			 * Initialize $ethaddr if it is blank or
			 * if the current value is different from non-volatile board information
			 * then change it.
			 */
			char *env;
			env = env_get(enetvar);
			if (!env) {
				env_set(enetvar, ethaddr);
			} else {
				if(strcmp(env, ethaddr))
					env_set(enetvar, ethaddr);
			}
}
#else
			/* Only initialize environment variables that are blank
			 * (i.e. have not yet been set)
			 */
			if (!env_get(enetvar))
				env_set(enetvar, ethaddr);
#endif

			macbase[5]++;
			if (macbase[5] == 0) {
				macbase[4]++;
				if (macbase[4] == 0) {
					macbase[3]++;
					if (macbase[3] == 0) {
						macbase[0] = 0;
						macbase[1] = 0;
						macbase[2] = 0;
					}
				}
			}
		}
	}

	printf("%s v%u len=%u\n", eeprom_hdr->signature, eeprom_hdr->version,
	       be16_to_cpu(eeprom_hdr->totallen));

	return 0;
}

#endif /* CONFIG_SYS_EEPROM_LOAD_ENV_MAC */

#ifdef CONFIG_POPULATE_SERIAL_NUMBER

/**
 *  populate_serial_number - read the serial number from EEPROM
 *
 *  This function reads the serial number from the EEPROM and sets the
 *  appropriate environment variable.
 *
 *  The environment variable is only set if it has not been set
 *  already.  This ensures that any user-saved variables are never
 *  overwritten.
 *
 *  This function must be called after relocation.
 */
int populate_serial_number(void)
{
	char serialstr[257];
	int eeprom_index;
	tlvinfo_tlv_t * eeprom_tlv;

	if (read_eeprom(eeprom)) {
		printf("Read failed.\n");
		return -1;
	}

	if (tlvinfo_find_tlv(eeprom, TLV_CODE_SERIAL_NUMBER, &eeprom_index)) {
		eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[eeprom_index];
		memcpy(serialstr, eeprom_tlv->value, eeprom_tlv->length);
		serialstr[eeprom_tlv->length] = 0;
		char *env;
		env = env_get("serial#");
		if (!env) {
			env_set("serial#", serialstr);
		} else {
			if(strcmp(env, serialstr))
				env_set("serial#", serialstr);
		}
	}

	return 0;
}

#endif /* CONFIG_POPULATE_SERIAL_NUMBER */
