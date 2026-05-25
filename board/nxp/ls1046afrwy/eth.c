// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */
#include <config.h>
#include <fdt_support.h>
#include <asm/io.h>

#ifdef CONFIG_FMAN_ENET
int fdt_update_ethernet_dt(void *blob)
{
	u32 srds_s1;
	int i, prop;
	int offset, nodeoff;
	const char *path;
	struct ccsr_gur *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);

	srds_s1 = in_be32(&gur->rcwsr[4]) &
			FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS2_RCWSR4_SRDS1_PRTCL_SHIFT;

	/* Cycle through all aliases */
	for (prop = 0; ; prop++) {
		const char *name;

		/* FDT might have been edited, recompute the offset */
		offset = fdt_first_property_offset(blob,
						   fdt_path_offset(blob,
								   "/aliases")
						   );
		/* Select property number 'prop' */
		for (i = 0; i < prop; i++)
			offset = fdt_next_property_offset(blob, offset);

		if (offset < 0)
			break;

		path = fdt_getprop_by_offset(blob, offset, &name, NULL);
		nodeoff = fdt_path_offset(blob, path);

		switch (srds_s1) {
		case 0x3040:
			if (!strcmp(name, "ethernet1"))
				fdt_status_disabled(blob, nodeoff);
			if (!strcmp(name, "ethernet2"))
				fdt_status_disabled(blob, nodeoff);
			if (!strcmp(name, "ethernet3"))
				fdt_status_disabled(blob, nodeoff);
			if (!strcmp(name, "ethernet6"))
				fdt_status_disabled(blob, nodeoff);
		break;
		default:
			printf("%s:Invalid SerDes prtcl 0x%x for LS1046AFRWY\n",
			       __func__, srds_s1);
		break;
		}
	}

	return 0;
}
#endif
