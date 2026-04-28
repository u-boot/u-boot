/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Common sysinfo helpers for TQ-Systems SOMs
 *
 * Copyright (c) 2025-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>
 * D-82229 Seefeld, Germany.
 * Author: Nora Schiffer
 */

#ifndef __TQ_SYSINFO_H
#define __TQ_SYSINFO_H

/**
 * tq_common_sysinfo_set_eeprom_macaddrs_by_index() - Write MAC address(es) from EEPROM to
 * environment
 * @sysinfo:		Pointer to structure to receive the EERPOM device
 * @start_index:	start number (ethXaddr) of first MAC address in environment
 * @macaddr_num:	number of mac addresses write to environment
 *
 * Return:
 * * 0 on success
 * * A negative error if there was a problem looking up the device
 */
int tq_common_sysinfo_set_eeprom_macaddrs_by_index(struct udevice *sysinfo, u8 start_index,
						   u8 macaddr_num);
int tq_common_sysinfo_setup(void);

#endif /* __TQ_SYSINFO_H */
