/*
 * OF Flat tree builder
 *
 */

#ifndef FT_BUILD_H
#define FT_BUILD_H

#include <linux/types.h>
#include <asm/u-boot.h>

/* Definitions used by the flattened device tree */
#define OF_DT_HEADER		0xd00dfeed	/* marker */
#define OF_DT_BEGIN_NODE	0x1	/* Start of node, full name */
#define OF_DT_END_NODE		0x2	/* End node */
#define OF_DT_PROP		0x3	/* Property: name off, size,
					 * content */
#define OF_DT_NOP		0x4	/* nop */
#define OF_DT_END		0x9

#define OF_DT_VERSION		0x10

struct boot_param_header {
	u32 magic;		/* magic word OF_DT_HEADER */
	u32 totalsize;		/* total size of DT block */
	u32 off_dt_struct;	/* offset to structure */
	u32 off_dt_strings;	/* offset to strings */
	u32 off_mem_rsvmap;	/* offset to memory reserve map */
	u32 version;		/* format version */
	u32 last_comp_version;	/* last compatible version */
	/* version 2 fields below */
	u32 boot_cpuid_phys;	/* Physical CPU id we're booting on */
	/* version 3 fields below */
	u32 dt_strings_size;	/* size of the DT strings block */
};

struct ft_cxt {
	struct boot_param_header *bph;
	int max_size;		/* maximum size of tree */
	int overflow;		/* set when this happens */
	u8 *p, *pstr, *pres;	/* running pointers */
	u8 *p_begin, *pstr_begin, *pres_begin;	/* starting pointers */
	u8 *p_anchor;		/* start of constructed area */
	int struct_size, strings_size, res_size;
};

void ft_begin_node(struct ft_cxt *cxt, const char *name);
void ft_end_node(struct ft_cxt *cxt);

void ft_begin_tree(struct ft_cxt *cxt);
int ft_end_tree(struct ft_cxt *cxt);

void ft_nop(struct ft_cxt *cxt);
void ft_prop(struct ft_cxt *cxt, const char *name, const void *data, int sz);
void ft_prop_str(struct ft_cxt *cxt, const char *name, const char *str);
void ft_prop_int(struct ft_cxt *cxt, const char *name, int val);
void ft_begin(struct ft_cxt *cxt, void *blob, int max_size);
void ft_add_rsvmap(struct ft_cxt *cxt, u64 physaddr, u64 size);

void ft_setup(void *blob, int size, bd_t * bd, ulong initrd_start, ulong initrd_end);

void ft_dump_blob(const void *bphp);
void ft_merge_blob(struct ft_cxt *cxt, void *blob);
void *ft_get_prop(void *bphp, const char *propname, int *szp);

void ft_board_setup(void *blob, bd_t *bd);

#endif
