#ifndef jffs2_private_h
#define jffs2_private_h

#include <jffs2/jffs2.h>

struct b_node {
	u32 offset;
	struct b_node *next;
};

struct b_lists {
	char *partOffset;
	struct b_node *dirListTail;
	struct b_node *dirListHead;
	u32 dirListCount;
	u32 dirListMemBase;
	struct b_node *fragListTail;
	struct b_node *fragListHead;
	u32 fragListCount;
	u32 fragListMemBase;

};
struct b_compr_info {
	u32 num_frags;
	u32 compr_sum;
	u32 decompr_sum;
};

struct b_jffs2_info {
	struct b_compr_info compr_info[JFFS2_NUM_COMPR];
};

static inline int
hdr_crc(struct jffs2_unknown_node *node)
{
        u32 crc = crc32_no_comp(0, (unsigned char *)node, sizeof(struct jffs2_unknown_node) - 4);
        u32 crc_blah = crc32_no_comp(~0, (unsigned char *)node, sizeof(struct jffs2_unknown_node) - 4);

        crc_blah ^= ~0;

        if (node->hdr_crc != crc) {
                return 0;
        } else {
                return 1;
        }
}

static inline int
dirent_crc(struct jffs2_raw_dirent *node)
{
        if (node->node_crc != crc32_no_comp(0, (unsigned char *)node, sizeof(struct jffs2_raw_dirent) - 8)) {
                return 0;
        } else {
                return 1;
        }
}

static inline int
dirent_name_crc(struct jffs2_raw_dirent *node)
{
        if (node->name_crc != crc32_no_comp(0, (unsigned char *)&(node->name), node->nsize)) {
                return 0;
        } else {
                return 1;
        }
}

static inline int
inode_crc(struct jffs2_raw_inode *node)
{
        if (node->node_crc != crc32_no_comp(0, (unsigned char *)node, sizeof(struct jffs2_raw_inode) - 8)) {
                return 0;
        } else {
                return 1;
        }
}

#endif /* jffs2_private.h */
