/*
 * MTD device concatenation layer definitions
 *
 * (C) 2002 Robert Kaiser <rkaiser@sysgo.de>
 *
 * This code is GPL
 */

#ifndef MTD_CONCAT_H
#define MTD_CONCAT_H

struct mtd_info *mtd_concat_create(
    struct mtd_info *subdev[],  /* subdevices to concatenate */
    int num_devs,               /* number of subdevices      */
#ifndef __UBOOT__
    const char *name);          /* name for the new device   */
#else
    char *name);          /* name for the new device   */
#endif

void mtd_concat_destroy(struct mtd_info *mtd);

#endif
