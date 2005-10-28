/*
 * OF flat tree builder
 */

#include <common.h>
#include <malloc.h>
#include <environment.h>

#ifdef CONFIG_OF_FLAT_TREE

#include <asm/errno.h>
#include <stddef.h>

#include <ft_build.h>

/* align addr on a size boundary - adjust address up if needed -- Cort */
#define _ALIGN(addr,size)       (((addr)+(size)-1)&(~((size)-1)))

static void ft_put_word(struct ft_cxt *cxt, u32 v)
{
	if (cxt->overflow)	/* do nothing */
		return;

	/* check for overflow */
	if (cxt->p + 4 > cxt->pstr) {
		cxt->overflow = 1;
		return;
	}

	*(u32 *) cxt->p = cpu_to_be32(v);
	cxt->p += 4;
}

static inline void ft_put_bin(struct ft_cxt *cxt, const void *data, int sz)
{
	u8 *p;

	if (cxt->overflow)	/* do nothing */
		return;

	/* next pointer pos */
	p = (u8 *) _ALIGN((unsigned long)cxt->p + sz, 4);

	/* check for overflow */
	if (p > cxt->pstr) {
		cxt->overflow = 1;
		return;
	}

	memcpy(cxt->p, data, sz);
	if ((sz & 3) != 0)
		memset(cxt->p + sz, 0, 4 - (sz & 3));
	cxt->p = p;
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

	p = cxt->pstr;
	while (p < cxt->pstr_begin) {
		if (strcmp(p, name) == 0)
			return p - cxt->p_begin;
		p += strlen(p) + 1;
	}

	return -1;
}

void ft_prop(struct ft_cxt *cxt, const char *name, const void *data, int sz)
{
	int len, off;

	if (cxt->overflow)
		return;

	len = strlen(name) + 1;

	off = lookup_string(cxt, name);
	if (off == -1) {
		/* check if we have space */
		if (cxt->p + 12 + sz + len > cxt->pstr) {
			cxt->overflow = 1;
			return;
		}

		cxt->pstr -= len;
		memcpy(cxt->pstr, name, len);
		off = cxt->pstr - cxt->p_begin;
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

	ft_prop(cxt, name, &v, 4);
}

/* start construction of the flat OF tree */
void ft_begin(struct ft_cxt *cxt, void *blob, int max_size)
{
	struct boot_param_header *bph = blob;
	u32 off;

	/* clear the cxt */
	memset(cxt, 0, sizeof(*cxt));

	cxt->bph = bph;
	cxt->max_size = max_size;

	/* zero everything in the header area */
	memset(bph, 0, sizeof(*bph));

	bph->magic = cpu_to_be32(OF_DT_HEADER);
	bph->version = cpu_to_be32(0x10);
	bph->last_comp_version = cpu_to_be32(0x10);

	/* start pointers */
	cxt->pres_begin = (u8 *) _ALIGN((unsigned long)(bph + 1), 8);
	cxt->pres = cxt->pres_begin;

	off = (unsigned long)cxt->pres_begin - (unsigned long)bph;
	bph->off_mem_rsvmap = cpu_to_be32(off);

	((u64 *) cxt->pres)[0] = 0;	/* phys = 0, size = 0, terminate */
	((u64 *) cxt->pres)[1] = 0;

	cxt->p_anchor = cxt->pres + 16;	/* over the terminator */
}

/* add a reserver physical area to the rsvmap */
void ft_add_rsvmap(struct ft_cxt *cxt, u64 physaddr, u64 size)
{
	((u64 *) cxt->pres)[0] = cpu_to_be64(physaddr);	/* phys = 0, size = 0, terminate */
	((u64 *) cxt->pres)[1] = cpu_to_be64(size);

	cxt->pres += 18;	/* advance */

	((u64 *) cxt->pres)[0] = 0;	/* phys = 0, size = 0, terminate */
	((u64 *) cxt->pres)[1] = 0;

	/* keep track of size */
	cxt->res_size = cxt->pres + 16 - cxt->pres_begin;

	cxt->p_anchor = cxt->pres + 16;	/* over the terminator */
}

void ft_begin_tree(struct ft_cxt *cxt)
{
	cxt->p_begin = cxt->p_anchor;
	cxt->pstr_begin = (char *)cxt->bph + cxt->max_size;	/* point at the end */

	cxt->p = cxt->p_begin;
	cxt->pstr = cxt->pstr_begin;
}

int ft_end_tree(struct ft_cxt *cxt)
{
	struct boot_param_header *bph = cxt->bph;
	int off, sz, sz1;
	u32 tag, v;
	u8 *p;

	ft_put_word(cxt, OF_DT_END);

	if (cxt->overflow)
		return -ENOMEM;

	/* size of the areas */
	cxt->struct_size = cxt->p - cxt->p_begin;
	cxt->strings_size = cxt->pstr_begin - cxt->pstr;

	/* the offset we must move */
	off = (cxt->pstr_begin - cxt->p_begin) - cxt->strings_size;

	/* the new strings start */
	cxt->pstr_begin = cxt->p_begin + cxt->struct_size;

	/* move the whole string area */
	memmove(cxt->pstr_begin, cxt->pstr, cxt->strings_size);

	/* now perform the fixup of the strings */
	p = cxt->p_begin;
	while ((tag = be32_to_cpu(*(u32 *) p)) != OF_DT_END) {
		p += 4;

		if (tag == OF_DT_BEGIN_NODE) {
			p = (u8 *) _ALIGN((unsigned long)p + strlen(p) + 1, 4);
			continue;
		}

		if (tag == OF_DT_END_NODE || tag == OF_DT_NOP)
			continue;

		if (tag != OF_DT_PROP)
			return -EINVAL;

		sz = be32_to_cpu(*(u32 *) p);
		p += 4;

		v = be32_to_cpu(*(u32 *) p);
		v -= off;
		*(u32 *) p = cpu_to_be32(v);	/* move down */
		p += 4;

		p = (u8 *) _ALIGN((unsigned long)p + sz, 4);
	}

	/* fix sizes */
	p = (char *)cxt->bph;
	sz = (cxt->pstr_begin + cxt->strings_size) - p;
	sz1 = _ALIGN(sz, 16);	/* align at 16 bytes */
	if (sz != sz1)
		memset(p + sz, 0, sz1 - sz);
	bph->totalsize = cpu_to_be32(sz1);
	bph->off_dt_struct = cpu_to_be32(cxt->p_begin - p);
	bph->off_dt_strings = cpu_to_be32(cxt->pstr_begin - p);

	/* the new strings start */
	cxt->pstr_begin = cxt->p_begin + cxt->struct_size;
	cxt->pstr = cxt->pstr_begin + cxt->strings_size;

	return 0;
}

/**********************************************************************/

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
		printf(" = \"%s\"", (char *)data);
		return;
	}

	switch (len) {
	case 1:		/* byte */
		printf(" = <0x%02x>", (*(u8 *) data) & 0xff);
		break;
	case 2:		/* half-word */
		printf(" = <0x%04x>", be16_to_cpu(*(u16 *) data) & 0xffff);
		break;
	case 4:		/* word */
		printf(" = <0x%08x>", be32_to_cpu(*(u32 *) data) & 0xffffffffU);
		break;
	case 8:		/* double-word */
		printf(" = <0x%16llx>", be64_to_cpu(*(uint64_t *) data));
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

		printf("/memreserve/ 0x%llx 0x%llx;\n", addr, size);
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
			fprintf(stderr, "%*s ** Unknown tag 0x%08x\n",
				depth * shift, "", tag);
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
	if (be32_to_cpu(*(u32 *) (cxt->p - 4)) != OF_DT_END_NODE)
		return;		/* XXX only for node */

	cxt->p -= 4;
}

/* note that the root node of the blob is "peeled" off */
void ft_merge_blob(struct ft_cxt *cxt, void *blob)
{
	struct boot_param_header *bph = (struct boot_param_header *)blob;
	u32 *p_struct = (u32 *) ((char *)bph + be32_to_cpu(bph->off_dt_struct));
	u32 *p_strings =
	    (u32 *) ((char *)bph + be32_to_cpu(bph->off_dt_strings));
	u32 tag, *p;
	char *s, *t;
	int depth, sz;

	if (be32_to_cpu(*(u32 *) (cxt->p - 4)) != OF_DT_END_NODE)
		return;		/* XXX only for node */

	cxt->p -= 4;

	depth = 0;
	p = p_struct;
	while ((tag = be32_to_cpu(*p++)) != OF_DT_END) {

		/* printf("tag: 0x%08x (%d) - %d\n", tag, p - p_struct, depth); */

		if (tag == OF_DT_BEGIN_NODE) {
			s = (char *)p;
			p = (u32 *) _ALIGN((unsigned long)p + strlen(s) + 1, 4);

			if (depth++ > 0)
				ft_begin_node(cxt, s);

			continue;
		}

		if (tag == OF_DT_END_NODE) {
			ft_end_node(cxt);
			if (--depth == 0)
				break;
			continue;
		}

		if (tag == OF_DT_NOP)
			continue;

		if (tag != OF_DT_PROP)
			break;

		sz = be32_to_cpu(*p++);
		s = (char *)p_strings + be32_to_cpu(*p++);
		t = (char *)p;
		p = (u32 *) _ALIGN((unsigned long)p + sz, 4);

		ft_prop(cxt, s, t, sz);
	}
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

extern unsigned char oftree_dtb[];
extern unsigned int oftree_dtb_len;

/* Function that returns a character from the environment */
extern uchar(*env_get_char) (int);

#define BDM(x)	{	.name = #x, .offset = offsetof(bd_t, bi_ ##x ) }

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

void ft_setup(void *blob, int size, bd_t * bd)
{
	DECLARE_GLOBAL_DATA_PTR;
	u8 *end;
	u32 *p;
	int len;
	struct ft_cxt cxt;
	int i, k, nxt;
	static char tmpenv[256];
	char *s, *lval, *rval;
	ulong clock;
	uint32_t v;

	/* disable OF tree; booting old kernel */
	if (getenv("disable_of") != NULL) {
		memcpy(blob, bd, sizeof(*bd));
		return;
	}

	ft_begin(&cxt, blob, size);

	/* fs_add_rsvmap not used */

	ft_begin_tree(&cxt);

	ft_begin_node(&cxt, "");

	ft_end_node(&cxt);

	/* copy RO tree */
	ft_merge_blob(&cxt, oftree_dtb);

	/* back into root */
	ft_backtrack_node(&cxt);

	ft_begin_node(&cxt, "u-boot-env");

	for (i = 0; env_get_char(i) != '\0'; i = nxt + 1) {
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

	ft_begin_node(&cxt, "chosen");

	ft_prop_str(&cxt, "name", "chosen");
	ft_prop_str(&cxt, "bootargs", getenv("bootargs"));
	ft_prop_int(&cxt, "linux,platform", 0x600);	/* what is this? */

	ft_end_node(&cxt);

	ft_end_node(&cxt);	/* end root */

	ft_end_tree(&cxt);

	/*
	   printf("merged OF-tree\n");
	   ft_dump_blob(blob);
	 */

	/* paste the bd_t at the end of the flat tree */
	end = (char *)blob +
	    be32_to_cpu(((struct boot_param_header *)blob)->totalsize);
	memcpy(end, bd, sizeof(*bd));

#ifdef CONFIG_PPC

	for (i = 0; i < sizeof(bd_map)/sizeof(bd_map[0]); i++) {
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

	clock = bd->bi_intfreq;
	p = ft_get_prop(blob, "/cpus/" OF_CPU "/clock-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

#ifdef OF_TBCLK
	clock = OF_TBCLK;
	p = ft_get_prop(blob, "/cpus/" OF_CPU "/timebase-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(OF_TBCLK);
#endif

#endif				/* __powerpc__ */

	/*
	   printf("final OF-tree\n");
	   ft_dump_blob(blob);
	 */

}

#endif
