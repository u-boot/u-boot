/*
 * (C) Copyright 2007
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FDT_SUPPORT_H
#define __FDT_SUPPORT_H

#ifdef CONFIG_OF_LIBFDT

#include <libfdt.h>

u32 fdt_getprop_u32_default(const void *fdt, const char *path,
				const char *prop, const u32 dflt);
int fdt_chosen(void *fdt, int force);
int fdt_initrd(void *fdt, ulong initrd_start, ulong initrd_end, int force);
void do_fixup_by_path(void *fdt, const char *path, const char *prop,
		      const void *val, int len, int create);
void do_fixup_by_path_u32(void *fdt, const char *path, const char *prop,
			  u32 val, int create);

static inline void do_fixup_by_path_string(void *fdt, const char *path,
					   const char *prop, const char *status)
{
	do_fixup_by_path(fdt, path, prop, status, strlen(status) + 1, 1);
}

void do_fixup_by_prop(void *fdt,
		      const char *pname, const void *pval, int plen,
		      const char *prop, const void *val, int len,
		      int create);
void do_fixup_by_prop_u32(void *fdt,
			  const char *pname, const void *pval, int plen,
			  const char *prop, u32 val, int create);
void do_fixup_by_compat(void *fdt, const char *compat,
			const char *prop, const void *val, int len, int create);
void do_fixup_by_compat_u32(void *fdt, const char *compat,
			    const char *prop, u32 val, int create);
int fdt_fixup_memory(void *blob, u64 start, u64 size);
int fdt_fixup_memory_banks(void *blob, u64 start[], u64 size[], int banks);
void fdt_fixup_ethernet(void *fdt);
int fdt_find_and_setprop(void *fdt, const char *node, const char *prop,
			 const void *val, int len, int create);
void fdt_fixup_qe_firmware(void *fdt);

#if defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_MPH_USB)
void fdt_fixup_dr_usb(void *blob, bd_t *bd);
#else
static inline void fdt_fixup_dr_usb(void *blob, bd_t *bd) {}
#endif /* defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_MPH_USB) */

#if defined(CONFIG_SYS_FSL_SEC_COMPAT)
void fdt_fixup_crypto_node(void *blob, int sec_rev);
#else
static inline void fdt_fixup_crypto_node(void *blob, int sec_rev) {}
#endif

#ifdef CONFIG_PCI
#include <pci.h>
int fdt_pci_dma_ranges(void *blob, int phb_off, struct pci_controller *hose);
#endif

void ft_board_setup(void *blob, bd_t *bd);
void ft_cpu_setup(void *blob, bd_t *bd);
void ft_pci_setup(void *blob, bd_t *bd);

void set_working_fdt_addr(void *addr);
int fdt_resize(void *blob);
int fdt_increase_size(void *fdt, int add_len);

int fdt_fixup_nor_flash_size(void *blob);

void fdt_fixup_mtdparts(void *fdt, void *node_info, int node_info_size);
void fdt_del_node_and_alias(void *blob, const char *alias);
u64 fdt_translate_address(void *blob, int node_offset, const __be32 *in_addr);
int fdt_node_offset_by_compat_reg(void *blob, const char *compat,
					phys_addr_t compat_off);
int fdt_alloc_phandle(void *blob);
int fdt_set_phandle(void *fdt, int nodeoffset, uint32_t phandle);
unsigned int fdt_create_phandle(void *fdt, int nodeoffset);
int fdt_add_edid(void *blob, const char *compat, unsigned char *buf);

int fdt_verify_alias_address(void *fdt, int anode, const char *alias,
			      u64 addr);
u64 fdt_get_base_address(void *fdt, int node);

enum fdt_status {
	FDT_STATUS_OKAY,
	FDT_STATUS_DISABLED,
	FDT_STATUS_FAIL,
	FDT_STATUS_FAIL_ERROR_CODE,
};
int fdt_set_node_status(void *fdt, int nodeoffset,
			enum fdt_status status, unsigned int error_code);
static inline int fdt_status_okay(void *fdt, int nodeoffset)
{
	return fdt_set_node_status(fdt, nodeoffset, FDT_STATUS_OKAY, 0);
}
static inline int fdt_status_disabled(void *fdt, int nodeoffset)
{
	return fdt_set_node_status(fdt, nodeoffset, FDT_STATUS_DISABLED, 0);
}

int fdt_set_status_by_alias(void *fdt, const char* alias,
			    enum fdt_status status, unsigned int error_code);
static inline int fdt_status_okay_by_alias(void *fdt, const char* alias)
{
	return fdt_set_status_by_alias(fdt, alias, FDT_STATUS_OKAY, 0);
}
static inline int fdt_status_disabled_by_alias(void *fdt, const char* alias)
{
	return fdt_set_status_by_alias(fdt, alias, FDT_STATUS_DISABLED, 0);
}

#endif /* ifdef CONFIG_OF_LIBFDT */
#endif /* ifndef __FDT_SUPPORT_H */
