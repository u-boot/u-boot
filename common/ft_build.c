/*
 * OF flat tree builder
 * Written by: Pantelis Antoniou <pantelis.antoniou@gmail.com>
 * Updated by: Matthew McClintock <msm@freescale.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <environment.h>

#ifdef CONFIG_OF_FLAT_TREE

#include <asm/errno.h>
#include <stddef.h>

#include <ft_build.h>

#undef DEBUG

/* align addr on a size boundary - adjust address up if needed -- Cort */
#define _ALIGN(addr,size)       (((addr)+(size)-1)&(~((size)-1)))
#ifndef CONFIG_OF_BOOT_CPU
#define CONFIG_OF_BOOT_CPU 0
#endif
#define SIZE_OF_RSVMAP_ENTRY (2*sizeof(u64))

static void ft_put_word(struct ft_cxt *cxt, u32 v)
{
	memmove(cxt->p + sizeof(u32), cxt->p, cxt->p_end - cxt->p);

	*(u32 *) cxt->p = cpu_to_be32(v);
	cxt->p += sizeof(u32);
	cxt->p_end += sizeof(u32);
}

static inline void ft_put_bin(struct ft_cxt *cxt, const void *data, int sz)
{
	int aligned_size = ((u8 *)_ALIGN((unsigned long)cxt->p + sz,
					sizeof(u32))) - cxt->p;

	memmove(cxt->p + aligned_size, cxt->p, cxt->p_end - cxt->p);

	/* make sure the last bytes are zeroed */
	memset(cxt->p + aligned_size - (aligned_size % sizeof(u32)), 0,
			(aligned_size % sizeof(u32)));

	memcpy(cxt->p, data, sz);

	cxt->p += aligned_size;
	cxt->p_end += aligned_size;
}

void ft_begin_node(struct ft_cxt *cxt, const char *name)
{
	ft_put_word(cxt, OF_DT_BEGIN_NODE);
	ft_put_bin(cxt, name, strlen(name) + 1);
}

void ft_end_node(struct ft_cxt *cxt)
{
	ft_put_word(cxt, OF_DT_END_NODE);
}

void ft_nop(struct ft_cxt *cxt)
{
	ft_put_word(cxt, OF_DT_NOP);
}

static int lookup_string(struct ft_cxt *cxt, const char *name)
{
	u8 *p;

	p = cxt->p;
	while (p < cxt->p_end) {
		if (strcmp((char *)p, name) == 0)
			return p - cxt->p;
		p += strlen((char *)p) + 1;
	}

	return -1;
}

void ft_prop(struct ft_cxt *cxt, const char *name, const void *data, int sz)
{
	int off = 0;

	off = lookup_string(cxt, name);
	if (off == -1) {
		memcpy(cxt->p_end, name, strlen(name) + 1);
		off = cxt->p_end - cxt->p;
		cxt->p_end += strlen(name) + 1;
	}

	/* now put offset from beginning of *STRUCTURE* */
	/* will be fixed up at the end */
	ft_put_word(cxt, OF_DT_PROP);
	ft_put_word(cxt, sz);
	ft_put_word(cxt, off);
	ft_put_bin(cxt, data, sz);
}

void ft_prop_str(struct ft_cxt *cxt, const char *name, const char *str)
{
	ft_prop(cxt, name, str, strlen(str) + 1);
}

void ft_prop_int(struct ft_cxt *cxt, const char *name, int val)
{
	u32 v = cpu_to_be32((u32) val);

	ft_prop(cxt, name, &v, sizeof(u32));
}

/* pick up and start working on a tree in place */
void ft_init_cxt(struct ft_cxt *cxt, void *blob)
{
	struct boot_param_header *bph = blob;

	memset(cxt, 0, sizeof(*cxt));

	cxt->bph = bph;
	bph->boot_cpuid_phys = CONFIG_OF_BOOT_CPU;

	/* find beginning and end of reserve map table (zeros in last entry) */
	cxt->p_rsvmap = (u8 *)bph + bph->off_mem_rsvmap;
	while ( ((uint64_t *)cxt->p_rsvmap)[0] != 0 &&
		     ((uint64_t *)cxt->p_rsvmap)[1] != 0 ) {
	cxt->p_rsvmap += SIZE_OF_RSVMAP_ENTRY;
	}

	cxt->p_start = (u8 *)bph + bph->off_dt_struct;
	cxt->p_end = (u8 *)bph + bph->totalsize;
	cxt->p = (u8 *)bph + bph->off_dt_strings;
}

/* add a reserver physical area to the rsvmap */
void ft_add_rsvmap(struct ft_cxt *cxt, u64 physstart, u64 physend)
{
	memmove(cxt->p_rsvmap + SIZE_OF_RSVMAP_ENTRY, cxt->p_rsvmap,
				 cxt->p_end - cxt->p_rsvmap);

	((u64 *)cxt->p_rsvmap)[0] = cpu_to_be64(physstart);
	((u64 *)cxt->p_rsvmap)[1] = cpu_to_be64(physend);
	((u64 *)cxt->p_rsvmap)[2] = 0;
	((u64 *)cxt->p_rsvmap)[3] = 0;

	cxt->p_rsvmap += SIZE_OF_RSVMAP_ENTRY;
	cxt->p_start += SIZE_OF_RSVMAP_ENTRY;
	cxt->p += SIZE_OF_RSVMAP_ENTRY;
	cxt->p_end += SIZE_OF_RSVMAP_ENTRY;
}

void ft_end_tree(struct ft_cxt *cxt)
{
	ft_put_word(cxt, OF_DT_END);
}

/* update the boot param header with correct values */
void ft_finalize_tree(struct ft_cxt *cxt) {
	struct boot_param_header *bph = cxt->bph;

	bph->totalsize = cxt->p_end - (u8 *)bph;
	bph->off_dt_struct = cxt->p_start - (u8 *)bph;
	bph->off_dt_strings = cxt->p - (u8 *)bph;
	bph->dt_strings_size = cxt->p_end - cxt->p;
}

static inline int isprint(int c)
{
	return c >= 0x20 && c <= 0x7e;
}

static int is_printable_string(const void *data, int len)
{
	const char *s = data;
	const char *ss;

	/* zero length is not */
	if (len == 0)
		return 0;

	/* must terminate with zero */
	if (s[len - 1] != '\0')
		return 0;

	ss = s;
	while (*s && isprint(*s))
		s++;

	/* not zero, or not done yet */
	if (*s != '\0' || (s + 1 - ss) < len)
		return 0;

	return 1;
}

static void print_data(const void *data, int len)
{
	int i;
	const u8 *s;

	/* no data, don't print */
	if (len == 0)
		return;

	if (is_printable_string(data, len)) {
		puts(" = \"");
		puts(data);
		puts("\"");
		return;
	}

	switch (len) {
	case 1:		/* byte */
		printf(" = <%02x>", (*(u8 *) data) & 0xff);
		break;
	case 2:		/* half-word */
		printf(" = <%04x>", be16_to_cpu(*(u16 *) data) & 0xffff);
		break;
	case 4:		/* word */
		printf(" = <%x>", be32_to_cpu(*(u32 *) data) & 0xffffffffU);
		break;
	case 8:		/* double-word */
		printf(" = <%qx>", be64_to_cpu(*(uint64_t *) data));
		break;
	default:		/* anything else... hexdump */
		printf(" = [");
		for (i = 0, s = data; i < len; i++)
			printf("%02x%s", s[i], i < len - 1 ? " " : "");
		printf("]");

		break;
	}
}

void ft_dump_blob(const void *bphp)
{
	const struct boot_param_header *bph = bphp;
	const uint64_t *p_rsvmap = (const uint64_t *)
		((const char *)bph + be32_to_cpu(bph->off_mem_rsvmap));
	const u32 *p_struct = (const u32 *)
		((const char *)bph + be32_to_cpu(bph->off_dt_struct));
	const u32 *p_strings = (const u32 *)
		((const char *)bph + be32_to_cpu(bph->off_dt_strings));
	u32 tag;
	const u32 *p;
	const char *s, *t;
	int depth, sz, shift;
	int i;
	uint64_t addr, size;

	if (be32_to_cpu(bph->magic) != OF_DT_HEADER) {
		/* not valid tree */
		return;
	}

	depth = 0;
	shift = 4;

	for (i = 0;; i++) {
		addr = be64_to_cpu(p_rsvmap[i * 2]);
		size = be64_to_cpu(p_rsvmap[i * 2 + 1]);
		if (addr == 0 && size == 0)
			break;

		printf("/memreserve/ %qx %qx;\n", addr, size);
	}

	p = p_struct;
	while ((tag = be32_to_cpu(*p++)) != OF_DT_END) {

		/* printf("tag: 0x%08x (%d)\n", tag, p - p_struct); */

		if (tag == OF_DT_BEGIN_NODE) {
			s = (const char *)p;
			p = (u32 *) _ALIGN((unsigned long)p + strlen(s) + 1, 4);

			printf("%*s%s {\n", depth * shift, "", s);

			depth++;
			continue;
		}

		if (tag == OF_DT_END_NODE) {
			depth--;

			printf("%*s};\n", depth * shift, "");
			continue;
		}

		if (tag == OF_DT_NOP) {
			printf("%*s[NOP]\n", depth * shift, "");
			continue;
		}

		if (tag != OF_DT_PROP) {
			fprintf(stderr, "%*s ** Unknown tag 0x%08x at 0x%x\n",
				depth * shift, "", tag, --p);
			break;
		}
		sz = be32_to_cpu(*p++);
		s = (const char *)p_strings + be32_to_cpu(*p++);
		t = (const char *)p;
		p = (const u32 *)_ALIGN((unsigned long)p + sz, 4);
		printf("%*s%s", depth * shift, "", s);
		print_data(t, sz);
		printf(";\n");
	}
}

void ft_backtrack_node(struct ft_cxt *cxt)
{
	int i = 4;

	while (be32_to_cpu(*(u32 *) (cxt->p - i)) != OF_DT_END_NODE)
		i += 4;

	memmove (cxt->p - i, cxt->p, cxt->p_end - cxt->p);

	cxt->p_end -= i;
	cxt->p -= i;
}

void *ft_get_prop(void *bphp, const char *propname, int *szp)
{
	struct boot_param_header *bph = bphp;
	uint32_t *p_struct =
	    (uint32_t *) ((char *)bph + be32_to_cpu(bph->off_dt_struct));
	uint32_t *p_strings =
	    (uint32_t *) ((char *)bph + be32_to_cpu(bph->off_dt_strings));
	uint32_t version = be32_to_cpu(bph->version);
	uint32_t tag;
	uint32_t *p;
	char *s, *t;
	char *ss;
	int sz;
	static char path[256], prop[256];

	path[0] = '\0';

	p = p_struct;
	while ((tag = be32_to_cpu(*p++)) != OF_DT_END) {

		if (tag == OF_DT_BEGIN_NODE) {
			s = (char *)p;
			p = (uint32_t *) _ALIGN((unsigned long)p + strlen(s) +
						1, 4);
			strcat(path, s);
			strcat(path, "/");
			continue;
		}

		if (tag == OF_DT_END_NODE) {
			path[strlen(path) - 1] = '\0';
			ss = strrchr(path, '/');
			if (ss != NULL)
				ss[1] = '\0';
			continue;
		}

		if (tag == OF_DT_NOP)
			continue;

		if (tag != OF_DT_PROP)
			break;

		sz = be32_to_cpu(*p++);
		s = (char *)p_strings + be32_to_cpu(*p++);
		if (version < 0x10 && sz >= 8)
			p = (uint32_t *) _ALIGN((unsigned long)p, 8);
		t = (char *)p;
		p = (uint32_t *) _ALIGN((unsigned long)p + sz, 4);

		strcpy(prop, path);
		strcat(prop, s);

		if (strcmp(prop, propname) == 0) {
			*szp = sz;
			return t;
		}
	}

	return NULL;
}

/********************************************************************/

/* Function that returns a character from the environment */
extern uchar(*env_get_char) (int);

#define BDM(x)	{	.name = #x, .offset = offsetof(bd_t, bi_ ##x ) }

#ifdef CONFIG_OF_HAS_BD_T
static const struct {
	const char *name;
	int offset;
} bd_map[] = {
	BDM(memstart),
	BDM(memsize),
	BDM(flashstart),
	BDM(flashsize),
	BDM(flashoffset),
	BDM(sramstart),
	BDM(sramsize),
#if defined(CONFIG_5xx) || defined(CONFIG_8xx) || defined(CONFIG_8260) \
	|| defined(CONFIG_E500)
	BDM(immr_base),
#endif
#if defined(CONFIG_MPC5xxx)
	BDM(mbar_base),
#endif
#if defined(CONFIG_MPC83XX)
	BDM(immrbar),
#endif
#if defined(CONFIG_MPC8220)
	BDM(mbar_base),
	BDM(inpfreq),
	BDM(pcifreq),
	BDM(pevfreq),
	BDM(flbfreq),
	BDM(vcofreq),
#endif
	BDM(bootflags),
	BDM(ip_addr),
	BDM(intfreq),
	BDM(busfreq),
#ifdef CONFIG_CPM2
	BDM(cpmfreq),
	BDM(brgfreq),
	BDM(sccfreq),
	BDM(vco),
#endif
#if defined(CONFIG_MPC5xxx)
	BDM(ipbfreq),
	BDM(pcifreq),
#endif
	BDM(baudrate),
};
#endif

void ft_setup(void *blob, bd_t * bd, ulong initrd_start, ulong initrd_end)
{
	u32 *p;
	int len;
	struct ft_cxt cxt;
	ulong clock;
#if defined(CONFIG_OF_HAS_UBOOT_ENV)
	int k, nxt;
#endif
#if defined(CONFIG_OF_HAS_BD_T)
	u8 *end;
#endif
#if defined(CONFIG_OF_HAS_UBOOT_ENV) || defined(CONFIG_OF_HAS_BD_T)
	int i;
	static char tmpenv[256];
#endif

	/* disable OF tree; booting old kernel */
	if (getenv("disable_of") != NULL) {
		memcpy(blob, bd, sizeof(*bd));
		return;
	}

#ifdef DEBUG
	printf ("recieved oftree\n");
	ft_dump_blob(blob);
#endif

	ft_init_cxt(&cxt, blob);

	if (initrd_start && initrd_end)
		ft_add_rsvmap(&cxt, initrd_start, initrd_end - initrd_start + 1);

	/* back into root */
	ft_backtrack_node(&cxt);

#ifdef CONFIG_OF_HAS_UBOOT_ENV
	ft_begin_node(&cxt, "u-boot-env");

	for (i = 0; env_get_char(i) != '\0'; i = nxt + 1) {
		char *s, *lval, *rval;

		for (nxt = i; env_get_char(nxt) != '\0'; ++nxt) ;
		s = tmpenv;
		for (k = i; k < nxt && s < &tmpenv[sizeof(tmpenv) - 1]; ++k)
			*s++ = env_get_char(k);
		*s++ = '\0';
		lval = tmpenv;
		s = strchr(tmpenv, '=');
		if (s != NULL) {
			*s++ = '\0';
			rval = s;
		} else
			continue;
		ft_prop_str(&cxt, lval, rval);
	}

	ft_end_node(&cxt);
#endif

	ft_begin_node(&cxt, "chosen");
	ft_prop_str(&cxt, "name", "chosen");

	ft_prop_str(&cxt, "bootargs", getenv("bootargs"));
	ft_prop_int(&cxt, "linux,platform", 0x600);	/* what is this? */
	if (initrd_start && initrd_end) {
		ft_prop_int(&cxt, "linux,initrd-start", initrd_start);
		ft_prop_int(&cxt, "linux,initrd-end", initrd_end);
	}
#ifdef OF_STDOUT_PATH
	ft_prop_str(&cxt, "linux,stdout-path", OF_STDOUT_PATH);
#endif

	ft_end_node(&cxt);

	ft_end_node(&cxt);	/* end root */

	ft_end_tree(&cxt);
	ft_finalize_tree(&cxt);

#ifdef CONFIG_OF_HAS_BD_T
	/* paste the bd_t at the end of the flat tree */
	end = (char *)blob +
	    be32_to_cpu(((struct boot_param_header *)blob)->totalsize);
	memcpy(end, bd, sizeof(*bd));
#endif

#ifdef CONFIG_PPC

#ifdef CONFIG_OF_HAS_BD_T
	for (i = 0; i < sizeof(bd_map)/sizeof(bd_map[0]); i++) {
		uint32_t v;

		sprintf(tmpenv, "/bd_t/%s", bd_map[i].name);
		v = *(uint32_t *)((char *)bd + bd_map[i].offset);

		p = ft_get_prop(blob, tmpenv, &len);
		if (p != NULL)
			*p = cpu_to_be32(v);
	}

	p = ft_get_prop(blob, "/bd_t/enetaddr", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enetaddr, 6);

	p = ft_get_prop(blob, "/bd_t/ethspeed", &len);
	if (p != NULL)
		*p = cpu_to_be32((uint32_t) bd->bi_ethspeed);
#endif

	clock = bd->bi_intfreq;
	p = ft_get_prop(blob, "/cpus/" OF_CPU "/clock-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

#ifdef OF_TBCLK
	clock = OF_TBCLK;
	p = ft_get_prop(blob, "/cpus/" OF_CPU "/timebase-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);
#endif
#endif				/* __powerpc__ */

#ifdef CONFIG_OF_BOARD_SETUP
	ft_board_setup(blob, bd);
#endif

	/* in case the size changed in the platform code */
	ft_finalize_tree(&cxt);

#ifdef DEBUG
	printf("final OF-tree\n");
	ft_dump_blob(blob);
#endif
}
#endif
