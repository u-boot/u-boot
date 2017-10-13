/*
 *  EFI device path interface
 *
 *  Copyright (c) 2017 Heinrich Schuchardt
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>

#define MAC_OUTPUT_LEN 22
#define UNKNOWN_OUTPUT_LEN 23

const efi_guid_t efi_guid_device_path_to_text_protocol =
		EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;

static char *dp_unknown(char *s, struct efi_device_path *dp)
{
	s += sprintf(s, "/UNKNOWN(%04x,%04x)", dp->type, dp->sub_type);
	return s;
}

static char *dp_hardware(char *s, struct efi_device_path *dp)
{
	switch (dp->sub_type) {
	case DEVICE_PATH_SUB_TYPE_MEMORY: {
		struct efi_device_path_memory *mdp =
			(struct efi_device_path_memory *)dp;
		s += sprintf(s, "/MemoryMapped(0x%x,0x%llx,0x%llx)",
			     mdp->memory_type,
			     mdp->start_address,
			     mdp->end_address);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_VENDOR: {
		struct efi_device_path_vendor *vdp =
			(struct efi_device_path_vendor *)dp;
		s += sprintf(s, "/VenHw(%pUl)", &vdp->guid);
		break;
	}
	default:
		s = dp_unknown(s, dp);
		break;
	}
	return s;
}

static char *dp_acpi(char *s, struct efi_device_path *dp)
{
	switch (dp->sub_type) {
	case DEVICE_PATH_SUB_TYPE_ACPI_DEVICE: {
		struct efi_device_path_acpi_path *adp =
			(struct efi_device_path_acpi_path *)dp;
		s += sprintf(s, "/Acpi(PNP%04x", EISA_PNP_NUM(adp->hid));
		if (adp->uid)
			s += sprintf(s, ",%d", adp->uid);
		s += sprintf(s, ")");
		break;
	}
	default:
		s = dp_unknown(s, dp);
		break;
	}
	return s;
}

static char *dp_msging(char *s, struct efi_device_path *dp)
{
	switch (dp->sub_type) {
	case DEVICE_PATH_SUB_TYPE_MSG_USB: {
		struct efi_device_path_usb *udp =
			(struct efi_device_path_usb *)dp;
		s += sprintf(s, "/Usb(0x%x,0x%x)", udp->parent_port_number,
			     udp->usb_interface);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_MAC_ADDR: {
		struct efi_device_path_mac_addr *mdp =
			(struct efi_device_path_mac_addr *)dp;

		if (mdp->if_type != 0 && mdp->if_type != 1)
			break;

		s += sprintf(s, "/MAC(%02x%02x%02x%02x%02x%02x,0x%1x)",
			mdp->mac.addr[0], mdp->mac.addr[1],
			mdp->mac.addr[2], mdp->mac.addr[3],
			mdp->mac.addr[4], mdp->mac.addr[5],
			mdp->if_type);

		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_USB_CLASS: {
		struct efi_device_path_usb_class *ucdp =
			(struct efi_device_path_usb_class *)dp;

		s += sprintf(s, "/USBClass(%x,%x,%x,%x,%x)",
			ucdp->vendor_id, ucdp->product_id,
			ucdp->device_class, ucdp->device_subclass,
			ucdp->device_protocol);

		break;
	}
	case DEVICE_PATH_SUB_TYPE_MSG_SD:
	case DEVICE_PATH_SUB_TYPE_MSG_MMC: {
		const char *typename =
			(dp->sub_type == DEVICE_PATH_SUB_TYPE_MSG_SD) ?
					"SDCard" : "MMC";
		struct efi_device_path_sd_mmc_path *sddp =
			(struct efi_device_path_sd_mmc_path *)dp;
		s += sprintf(s, "/%s(Slot%u)", typename, sddp->slot_number);
		break;
	}
	default:
		s = dp_unknown(s, dp);
		break;
	}
	return s;
}

static char *dp_media(char *s, struct efi_device_path *dp)
{
	switch (dp->sub_type) {
	case DEVICE_PATH_SUB_TYPE_HARD_DRIVE_PATH: {
		struct efi_device_path_hard_drive_path *hddp =
			(struct efi_device_path_hard_drive_path *)dp;
		void *sig = hddp->partition_signature;

		switch (hddp->signature_type) {
		case SIG_TYPE_MBR:
			s += sprintf(s, "/HD(Part%d,Sig%08x)",
				     hddp->partition_number,
				     *(uint32_t *)sig);
			break;
		case SIG_TYPE_GUID:
			s += sprintf(s, "/HD(Part%d,Sig%pUl)",
				     hddp->partition_number, sig);
		default:
			s += sprintf(s, "/HD(Part%d,MBRType=%02x,SigType=%02x)",
				     hddp->partition_number, hddp->partmap_type,
				     hddp->signature_type);
		}

		break;
	}
	case DEVICE_PATH_SUB_TYPE_CDROM_PATH: {
		struct efi_device_path_cdrom_path *cddp =
			(struct efi_device_path_cdrom_path *)dp;
		s += sprintf(s, "/CDROM(0x%x)", cddp->boot_entry);
		break;
	}
	case DEVICE_PATH_SUB_TYPE_FILE_PATH: {
		struct efi_device_path_file_path *fp =
			(struct efi_device_path_file_path *)dp;
		int slen = (dp->length - sizeof(*dp)) / 2;
		s += sprintf(s, "/%-*ls", slen, fp->str);
		break;
	}
	default:
		s = dp_unknown(s, dp);
		break;
	}
	return s;
}

static uint16_t *efi_convert_device_node_to_text(
		struct efi_device_path *dp,
		bool display_only,
		bool allow_shortcuts)
{
	unsigned long len;
	efi_status_t r;
	char buf[512];  /* this ought be be big enough for worst case */
	char *str = buf;
	uint16_t *out;

	while (dp) {
		switch (dp->type) {
		case DEVICE_PATH_TYPE_HARDWARE_DEVICE:
			str = dp_hardware(str, dp);
			break;
		case DEVICE_PATH_TYPE_ACPI_DEVICE:
			str = dp_acpi(str, dp);
			break;
		case DEVICE_PATH_TYPE_MESSAGING_DEVICE:
			str = dp_msging(str, dp);
			break;
		case DEVICE_PATH_TYPE_MEDIA_DEVICE:
			str = dp_media(str, dp);
			break;
		default:
			str = dp_unknown(str, dp);
		}

		dp = efi_dp_next(dp);
	}

	*str++ = '\0';

	len = str - buf;
	r = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES, 2 * len, (void **)&out);
	if (r != EFI_SUCCESS)
		return NULL;

	ascii2unicode(out, buf);
	out[len - 1] = 0;

	return out;
}

/* helper for debug prints.. efi_free_pool() the result. */
uint16_t *efi_dp_str(struct efi_device_path *dp)
{
	return efi_convert_device_node_to_text(dp, true, true);
}


static uint16_t EFIAPI *efi_convert_device_node_to_text_ext(
		struct efi_device_path *device_node,
		bool display_only,
		bool allow_shortcuts)
{
	uint16_t *buffer;

	EFI_ENTRY("%p, %d, %d", device_node, display_only, allow_shortcuts);

	buffer = efi_convert_device_node_to_text(device_node, display_only,
						 allow_shortcuts);

	EFI_EXIT(EFI_SUCCESS);
	return buffer;
}

static uint16_t EFIAPI *efi_convert_device_path_to_text(
		struct efi_device_path *device_path,
		bool display_only,
		bool allow_shortcuts)
{
	uint16_t *buffer;

	EFI_ENTRY("%p, %d, %d", device_path, display_only, allow_shortcuts);

	/*
	 * Our device paths are all of depth one. So its is sufficient to
	 * to convert the first node.
	 */
	buffer = efi_convert_device_node_to_text(device_path, display_only,
						 allow_shortcuts);

	EFI_EXIT(EFI_SUCCESS);
	return buffer;
}

const struct efi_device_path_to_text_protocol efi_device_path_to_text = {
	.convert_device_node_to_text = efi_convert_device_node_to_text_ext,
	.convert_device_path_to_text = efi_convert_device_path_to_text,
};
