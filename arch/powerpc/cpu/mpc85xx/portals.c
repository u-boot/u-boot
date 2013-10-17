/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <libfdt.h>
#include <fdt_support.h>

#include <asm/processor.h>
#include <asm/io.h>

#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>

void setup_portals(void)
{
	ccsr_qman_t *qman = (void *)CONFIG_SYS_FSL_QMAN_ADDR;
#ifdef CONFIG_FSL_CORENET
	int i;

	for (i = 0; i < CONFIG_SYS_QMAN_NUM_PORTALS; i++) {
		u8 sdest = qp_info[i].sdest;
		u16 fliodn = qp_info[i].fliodn;
		u16 dliodn = qp_info[i].dliodn;
		u16 liodn_off = qp_info[i].liodn_offset;

		out_be32(&qman->qcsp[i].qcsp_lio_cfg, (liodn_off << 16) |
					dliodn);
		/* set frame liodn */
		out_be32(&qman->qcsp[i].qcsp_io_cfg, (sdest << 16) | fliodn);
	}
#endif

	/* Set the Qman initiator BAR to match the LAW (for DQRR stashing) */
#ifdef CONFIG_PHYS_64BIT
	out_be32(&qman->qcsp_bare, (u32)(CONFIG_SYS_QMAN_MEM_PHYS >> 32));
#endif
	out_be32(&qman->qcsp_bar, (u32)CONFIG_SYS_QMAN_MEM_PHYS);
}

/* Update portal containter to match LAW setup of portal in phy map */
void fdt_portal(void *blob, const char *compat, const char *container,
			u64 addr, u32 size)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, compat);
	if (off < 0)
		return ;

	off = fdt_parent_offset(blob, off);
	/* if non-zero assume we have a container */
	if (off > 0) {
		char buf[60];
		const char *p, *name;
		u32 *range;
		int len;

		/* fixup ranges */
		range = fdt_getprop_w(blob, off, "ranges", &len);
		if (range == NULL) {
			printf("ERROR: container for %s has no ranges", compat);
			return ;
		}

		range[0] = 0;
		if (len == 16) {
			range[1] = addr >> 32;
			range[2] = addr & 0xffffffff;
			range[3] = size;
		} else {
			range[1] = addr & 0xffffffff;
			range[2] = size;
		}
		fdt_setprop_inplace(blob, off, "ranges", range, len);

		/* fixup the name */
		name = fdt_get_name(blob, off, &len);
		p = memchr(name, '@', len);

		if (p)
			len = p - name;

		/* if we are given a container name check it
		 * against what we found, if it doesnt match exit out */
		if (container && (memcmp(container, name, len))) {
			printf("WARNING: container names didn't match %s %s\n",
				container, name);
			return ;
		}

		memcpy(&buf, name, len);
		len += sprintf(&buf[len], "@%llx", addr);
		fdt_set_name(blob, off, buf);
		return ;
	}

	printf("ERROR: %s isn't in a container.  Not supported\n", compat);
}

static int fdt_qportal(void *blob, int off, int id, char *name,
		       enum fsl_dpaa_dev dev, int create)
{
	int childoff, dev_off, ret = 0;
	uint32_t dev_handle;
#ifdef CONFIG_FSL_CORENET
	int num;
	u32 liodns[2];
#endif

	childoff = fdt_subnode_offset(blob, off, name);
	if (create) {
		char handle[64], *p;

		strncpy(handle, name, sizeof(handle));
		p = strchr(handle, '@');
		if (!strncmp(name, "fman", 4)) {
			*p = *(p + 1);
			p++;
		}
		*p = '\0';

		dev_off = fdt_path_offset(blob, handle);
		/* skip this node if alias is not found */
		if (dev_off == -FDT_ERR_BADPATH)
			return 0;
		if (dev_off < 0)
			return dev_off;

		if (childoff <= 0)
			childoff = fdt_add_subnode(blob, off, name);

		/* need to update the dev_off after adding a subnode */
		dev_off = fdt_path_offset(blob, handle);
		if (dev_off < 0)
			return dev_off;

		if (childoff > 0) {
			dev_handle = fdt_get_phandle(blob, dev_off);
			if (dev_handle <= 0) {
				dev_handle = fdt_alloc_phandle(blob);
				ret = fdt_set_phandle(blob, dev_off,
							 dev_handle);
				if (ret < 0)
					return ret;
			}

			ret = fdt_setprop(blob, childoff, "dev-handle",
					  &dev_handle, sizeof(dev_handle));
			if (ret < 0)
				return ret;

#ifdef CONFIG_FSL_CORENET
			num = get_dpaa_liodn(dev, &liodns[0], id);
			ret = fdt_setprop(blob, childoff, "fsl,liodn",
					  &liodns[0], sizeof(u32) * num);
			if (!strncmp(name, "pme", 3)) {
				u32 pme_rev1, pme_rev2;
				ccsr_pme_t *pme_regs =
					(void *)CONFIG_SYS_FSL_CORENET_PME_ADDR;

				pme_rev1 = in_be32(&pme_regs->pm_ip_rev_1);
				pme_rev2 = in_be32(&pme_regs->pm_ip_rev_2);
				ret = fdt_setprop(blob, childoff,
					"fsl,pme-rev1", &pme_rev1, sizeof(u32));
				if (ret < 0)
					return ret;
				ret = fdt_setprop(blob, childoff,
					"fsl,pme-rev2", &pme_rev2, sizeof(u32));
			}
#endif
		} else {
			return childoff;
		}
	} else {
		if (childoff > 0)
			ret = fdt_del_node(blob, childoff);
	}

	return ret;
}

void fdt_fixup_qportals(void *blob)
{
	int off, err;
	unsigned int maj, min;
	unsigned int ip_cfg;
	ccsr_qman_t *qman = (void *)CONFIG_SYS_FSL_QMAN_ADDR;
	u32 rev_1 = in_be32(&qman->ip_rev_1);
	u32 rev_2 = in_be32(&qman->ip_rev_2);
	char compat[64];
	int compat_len;

	maj = (rev_1 >> 8) & 0xff;
	min = rev_1 & 0xff;
	ip_cfg = rev_2 & 0xff;

	compat_len = sprintf(compat, "fsl,qman-portal-%u.%u.%u",
					maj, min, ip_cfg) + 1;
	compat_len += sprintf(compat + compat_len, "fsl,qman-portal") + 1;

	off = fdt_node_offset_by_compatible(blob, -1, "fsl,qman-portal");
	while (off != -FDT_ERR_NOTFOUND) {
#ifdef CONFIG_FSL_CORENET
		u32 liodns[2];
#endif
		const int *ci = fdt_getprop(blob, off, "cell-index", NULL);
		int i = *ci;
#ifdef CONFIG_SYS_DPAA_FMAN
		int j;
#endif

		err = fdt_setprop(blob, off, "compatible", compat, compat_len);
		if (err < 0)
			goto err;

#ifdef CONFIG_FSL_CORENET
		liodns[0] = qp_info[i].dliodn;
		liodns[1] = qp_info[i].fliodn;

		err = fdt_setprop(blob, off, "fsl,liodn",
				  &liodns, sizeof(u32) * 2);
		if (err < 0)
			goto err;
#endif

		i++;

		err = fdt_qportal(blob, off, i, "crypto@0", FSL_HW_PORTAL_SEC,
				  IS_E_PROCESSOR(get_svr()));
		if (err < 0)
			goto err;

#ifdef CONFIG_FSL_CORENET
#ifdef CONFIG_SYS_DPAA_PME
		err = fdt_qportal(blob, off, i, "pme@0", FSL_HW_PORTAL_PME, 1);
		if (err < 0)
			goto err;
#else
		fdt_qportal(blob, off, i, "pme@0", FSL_HW_PORTAL_PME, 0);
#endif
#endif

#ifdef CONFIG_SYS_DPAA_FMAN
		for (j = 0; j < CONFIG_SYS_NUM_FMAN; j++) {
			char name[] = "fman@0";

			name[sizeof(name) - 2] = '0' + j;
			err = fdt_qportal(blob, off, i, name,
					  FSL_HW_PORTAL_FMAN1 + j, 1);
			if (err < 0)
				goto err;
		}
#endif
#ifdef CONFIG_SYS_DPAA_RMAN
		err = fdt_qportal(blob, off, i, "rman@0",
				  FSL_HW_PORTAL_RMAN, 1);
		if (err < 0)
			goto err;
#endif

err:
		if (err < 0) {
			printf("ERROR: unable to create props for %s: %s\n",
				fdt_get_name(blob, off, NULL), fdt_strerror(err));
			return;
		}

		off = fdt_node_offset_by_compatible(blob, off, "fsl,qman-portal");
	}
}

void fdt_fixup_bportals(void *blob)
{
	int off, err;
	unsigned int maj, min;
	unsigned int ip_cfg;
	ccsr_bman_t *bman = (void *)CONFIG_SYS_FSL_BMAN_ADDR;
	u32 rev_1 = in_be32(&bman->ip_rev_1);
	u32 rev_2 = in_be32(&bman->ip_rev_2);
	char compat[64];
	int compat_len;

	maj = (rev_1 >> 8) & 0xff;
	min = rev_1 & 0xff;

	ip_cfg = rev_2 & 0xff;

	compat_len = sprintf(compat, "fsl,bman-portal-%u.%u.%u",
				 maj, min, ip_cfg) + 1;
	compat_len += sprintf(compat + compat_len, "fsl,bman-portal") + 1;

	off = fdt_node_offset_by_compatible(blob, -1, "fsl,bman-portal");
	while (off != -FDT_ERR_NOTFOUND) {
		err = fdt_setprop(blob, off, "compatible", compat, compat_len);
		if (err < 0) {
			printf("ERROR: unable to create props for %s: %s\n",
				fdt_get_name(blob, off, NULL),
						 fdt_strerror(err));
			return;
		}

		off = fdt_node_offset_by_compatible(blob, off, "fsl,bman-portal");
	}

}
