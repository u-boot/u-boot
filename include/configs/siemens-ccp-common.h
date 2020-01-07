/* SPDX-License-Identifier: GPL-2.0+ */
/* Be very careful updating CONFIG_IDENT_STRING
 * This string will control the update flow whether an U-Boot should be
 * updated or not. If the version of installed U-Boot (in flash) is smaller
 * than the version to be installed (from update file), an update will
 * be performed.
 *
 * General rules:
 * 1. First 4 characters ' ##v' or  IDENT_MAGIC represent kind of a magic number
 *    to identify the following strings after easily. Don't change them!
 *
 * 2. First 2 digits after 'v' or CCP_MAJOR are updated with U-Boot version
 *    change, e.g. from 2015.04 to 2018.03
 *
 * 3. Second 2 digits after '.' or CCP_MINOR are updated if we want to upgrade
 *    U-Boot within an U-Boot version.
 */
#define CCP_IDENT_MAGIC			" ##v"
#define GENERATE_CCP_VERSION(MAJOR, MINOR)	CCP_IDENT_MAGIC MAJOR "." MINOR
