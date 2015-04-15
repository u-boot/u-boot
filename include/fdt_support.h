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

u32 fdt_getprop_u32_default_node(const void *fdt, int off, int cell,
				const char *prop, const u32 dflt);
u32 fdt_getprop_u32_default(const void *fdt, const char *path,
				const char *prop, const u32 dflt);
int fdt_chosen(void *fdt);
int fdt_initrd(void *fdt, ulong initrd_start, ulong initrd_end);
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

int fdt_find_or_add_subnode(void *fdt, int parentoffset, const char *name);

/**
 * Add board-specific data to the FDT before booting the OS.
 *
 * Use CONFIG_SYS_FDT_PAD to ensure there is sufficient space.
 * This function is called if CONFIG_OF_BOARD_SETUP is defined
 *
 * @param blob		FDT blob to update
 * @param bd_t		Pointer to board data
 * @return 0 if ok, or -FDT_ERR_... on error
 */
int ft_board_setup(void *blob, bd_t *bd);

/*
 * The keystone2 SOC requires all 32 bit aliased addresses to be converted
 * to their 36 physical format. This has to happen after all fdt nodes
 * are added or modified by the image_setup_libfdt(). The ft_board_setup_ex()
 * called at the end of the image_setup_libfdt() is to do that convertion.
 */
void ft_board_setup_ex(void *blob, bd_t *bd);
void ft_cpu_setup(void *blob, bd_t *bd);
void ft_pci_setup(void *blob, bd_t *bd);

/**
 * Add system-specific data to the FDT before booting the OS.
 *
 * Use CONFIG_SYS_FDT_PAD to ensure there is sufficient space.
 * This function is called if CONFIG_OF_SYSTEM_SETUP is defined
 *
 * @param blob		FDT blob to update
 * @param bd_t		Pointer to board data
 * @return 0 if ok, or -FDT_ERR_... on error
 */
int ft_system_setup(void *blob, bd_t *bd);

void set_working_fdt_addr(ulong addr);
int fdt_shrink_to_minimum(void *blob);
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
int fdt_read_range(void *fdt, int node, int n, uint64_t *child_addr,
		   uint64_t *addr, uint64_t *len);

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
static inline int fdt_status_fail(void *fdt, int nodeoffset)
{
	return fdt_set_node_status(fdt, nodeoffset, FDT_STATUS_FAIL, 0);
}

int fdt_set_status_by_alias(void *fdt, const char *alias,
			    enum fdt_status status, unsigned int error_code);
static inline int fdt_status_okay_by_alias(void *fdt, const char *alias)
{
	return fdt_set_status_by_alias(fdt, alias, FDT_STATUS_OKAY, 0);
}
static inline int fdt_status_disabled_by_alias(void *fdt, const char *alias)
{
	return fdt_set_status_by_alias(fdt, alias, FDT_STATUS_DISABLED, 0);
}
static inline int fdt_status_fail_by_alias(void *fdt, const char *alias)
{
	return fdt_set_status_by_alias(fdt, alias, FDT_STATUS_FAIL, 0);
}

/* Helper to read a big number; size is in cells (not bytes) */
static inline u64 of_read_number(const fdt32_t *cell, int size)
{
	u64 r = 0;
	while (size--)
		r = (r << 32) | fdt32_to_cpu(*(cell++));
	return r;
}

void of_bus_default_count_cells(void *blob, int parentoffset,
					int *addrc, int *sizec);
int ft_verify_fdt(void *fdt);
int arch_fixup_memory_node(void *blob);

int fdt_setup_simplefb_node(void *fdt, int node, u64 base_address, u32 width,
			    u32 height, u32 stride, const char *format);

#endif /* ifdef CONFIG_OF_LIBFDT */

#ifdef USE_HOSTCC
int fdtdec_get_int(const void *blob, int node, const char *prop_name,
		int default_val);
#endif
#endif /* ifndef __FDT_SUPPORT_H */
