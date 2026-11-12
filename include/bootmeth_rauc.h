/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Bootmethod for distro boot with RAUC
 */

#ifndef __BOOTMETH_RAUC_H
#define __BOOTMETH_RAUC_H

/**
 * struct distro_rauc_plat - Platform data of the RAUC bootmeth
 *
 * This is set from Kconfig when the bootmeth is bound. Tests can change it
 * afterwards to check other configurations.
 *
 * @partitions: Boot and root partitions of each slot, given by number or
 *	by name, see CONFIG_BOOTMETH_RAUC_PARTITIONS
 * @bootcmd: Command to boot the selected slot, see
 *	CONFIG_BOOTMETH_RAUC_BOOTCMD. Empty to run the boot script instead
 */
struct distro_rauc_plat {
	const char *partitions;
	const char *bootcmd;
};

#endif
