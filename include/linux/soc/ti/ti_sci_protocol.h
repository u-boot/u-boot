/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Texas Instruments System Control Interface Protocol
 * Based on include/linux/soc/ti/ti_sci_protocol.h from Linux.
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Nishanth Menon
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#ifndef __TISCI_PROTOCOL_H
#define __TISCI_PROTOCOL_H

/**
 * struct ti_sci_version_info - version information structure
 * @abi_major:	Major ABI version. Change here implies risk of backward
 *		compatibility break.
 * @abi_minor:	Minor ABI version. Change here implies new feature addition,
 *		or compatible change in ABI.
 * @firmware_revision:	Firmware revision (not usually used).
 * @firmware_description: Firmware description (not usually used).
 */
struct ti_sci_version_info {
	u8 abi_major;
	u8 abi_minor;
	u16 firmware_revision;
	char firmware_description[32];
};

struct ti_sci_handle;

/**
 * struct ti_sci_handle - Handle returned to TI SCI clients for usage.
 * @version:	structure containing version information
 */
struct ti_sci_handle {
	struct ti_sci_version_info version;
};

#if IS_ENABLED(CONFIG_TI_SCI_PROTOCOL)

const struct ti_sci_handle *ti_sci_get_handle_from_sysfw(struct udevice *dev);
const struct ti_sci_handle *ti_sci_get_handle(struct udevice *dev);
const struct ti_sci_handle *ti_sci_get_by_phandle(struct udevice *dev,
						  const char *property);

#else	/* CONFIG_TI_SCI_PROTOCOL */

static inline
const struct ti_sci_handle *ti_sci_get_handle_from_sysfw(struct udevice *dev)
{
	return ERR_PTR(-EINVAL);
}

static inline const struct ti_sci_handle *ti_sci_get_handle(struct udevice *dev)
{
	return ERR_PTR(-EINVAL);
}

static inline
const struct ti_sci_handle *ti_sci_get_by_phandle(struct udevice *dev,
						  const char *property)
{
	return ERR_PTR(-EINVAL);
}
#endif	/* CONFIG_TI_SCI_PROTOCOL */

#endif	/* __TISCI_PROTOCOL_H */
