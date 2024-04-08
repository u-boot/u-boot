// SPDX-License-Identifier: GPL-2.0+
/*
 * Command for accessing SPI flash.
 *
 * Copyright (C) 2008 Atmel Corporation
 */
#include <common.h>
#include <string.h>
#include <fdt_support.h>
#include <linux/libfdt.h>

#define FDT_ALIGNMENT           0x1000
#define FDT_EXTRA_SIZE          0x10
#define FDT_CPUS_PATH           "/cpus"
#define FDT_CPU_NODE_PREFIX     "cpu@"
#define FDT_CPU_VECTOR_NODE     "cpu-vector"
#define FDT_CPU_VECTOR_VALUE    "1.0"

typedef struct {
    unsigned int 	version;
    unsigned int 	has_sv57;
    unsigned int 	has_sv48;
    unsigned int 	has_sv39;
    unsigned int 	has_c;
    unsigned int 	has_d;
    unsigned int 	has_f;
    unsigned int 	has_svpbmt;
    unsigned int 	has_zihpm;
    unsigned int 	has_zicsr;
    char 	*ext_tag;
    char 	*ext_payload;
    char 	*rvv_vlen;
} asn_data_t;

static const char *const fdt_node_name[] = {
    "ud_version",
    "mmu-type",
    "riscv,isa-extensions",
    "riscv,isa-extension-config",
    "ext",
    "tag",
    "payload",
    "rvv",
    "vlen",
};

static const char *const mmu_type[] = {
    "riscv,sv57",
    "riscv,sv48",
    "riscv,sv39",
    NULL,
};

static const char *const riscv_extensions[] = {
    "c",
    "d",
    "f",
    "svpbmt",
    "zihpm",
    "zicsr",
    NULL,
};

static asn_data_t asn_data = {
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    "v0.7Vector",
    "\x10\x20\x30\x40\x60",
    "0001",
};

static int asn_fdt_resize(struct fdt_header *blob, struct fdt_header **new_blob) {
    int original_totalsize;
    int new_totalsize;
    int ret = 0;

    ret = fdt_check_header(blob);
    if (ret) {
        printf("FDT blob not valid, returning %s", fdt_strerror(ret));
        return CMD_RET_FAILURE;
    }

    original_totalsize = fdt_totalsize(blob);
    new_totalsize = (original_totalsize + FDT_EXTRA_SIZE + FDT_ALIGNMENT - 1) & ~(FDT_ALIGNMENT - 1);
    *new_blob = malloc(new_totalsize);
    if (!*new_blob) {
        printf("Failed to allocate memory for the new FDT\n");
        return CMD_RET_FAILURE;
    }

    /* reopen fdt to bigger space */
    ret = fdt_open_into(blob, *new_blob, new_totalsize);
    if (ret) {
        free(*new_blob);
        printf("FDT resize failed: %s\n", fdt_strerror(ret));
        return CMD_RET_FAILURE;
    }
    return 0;
}

static int convertFDTNode(struct fdt_header *blob, asn_data_t *data,
                          int nodeoffset)
{
    unsigned int *p;
    int i = 0, j;
    int new_nodeoffset;
    char tmpbuf[64] = {0};
    int tmpval;
    int ret = 0;

    ret = fdt_check_header(blob);
    if (ret) {
        printf("fdt blob not valid, returning %s", fdt_strerror(nodeoffset));
    }

    if (!data) {
        printf("null pointer error\n");
        return CMD_RET_FAILURE;
    }

    p = (unsigned int *)data;
    if (nodeoffset < 0) {
        /*
        * Not found or something else bad happened.
        */
        printf("libfdt fdt_path_offset() returned %s\n",
                fdt_strerror(nodeoffset));
        return 1;
    }

    /* set /cpus/cpu@0/version*/
    tmpval = cpu_to_fdt32(*p);
    ret = fdt_setprop(blob, nodeoffset, fdt_node_name[i], &tmpval, sizeof(unsigned int));
    if (ret != 0) {
        printf("set %s property failed, returning %s\n", fdt_node_name[i], fdt_strerror(ret));
        return ret;
    }
    p++;
    i++;

    /* set mmu-type properties */
    j = 0;
    while (mmu_type[j] != NULL) {
        if (*p != 0) {
            ret = fdt_setprop(blob, nodeoffset, fdt_node_name[i], mmu_type[j], strlen(mmu_type[j]) + 1);
            if (ret != 0) {
                printf("set %s property failed, returning %s\n", fdt_node_name[i], fdt_strerror(ret));
                return ret;
             }
             break;
        }
        p++;
        j++;
    }
    i++;

    /* set riscv,isa-extensions */
    j = 0;
    tmpval = 0;/* tmpbuf length */
    while (riscv_extensions[j] != NULL) {
        if (*p != 0) {
            strcpy(tmpbuf + tmpval, riscv_extensions[j]);
            tmpval += strlen(riscv_extensions[j]) + 1;
        }
        p++;
        j++;
    }

    /* add "v" and "v0.7Vector" if exists " */
    if (data->rvv_vlen != NULL) {
        strcpy(tmpbuf + tmpval, "v");
        tmpval += strlen("v") + 1;
        /* set cpu-vector to 1.0 */
        ret = fdt_setprop(blob, nodeoffset, FDT_CPU_VECTOR_NODE, FDT_CPU_VECTOR_VALUE, strlen(FDT_CPU_VECTOR_VALUE) + 1);
        if (ret != 0) {
            printf("set %s property failed, returning %s\n", FDT_CPU_VECTOR_NODE, fdt_strerror(ret));
            return ret;
            }
    }

    if (data->ext_tag != NULL) {
        strcpy(tmpbuf + tmpval, data->ext_tag);
        tmpval += strlen(data->ext_tag) + 1;
    }

    ret = fdt_setprop(blob, nodeoffset, fdt_node_name[i], tmpbuf, tmpval);
    if (ret != 0) {
        printf("set %s property failed, returning %s\n", fdt_node_name[i], fdt_strerror(ret));
        return ret;
    }
    i++;

    /* add /cpus/cpu@x/riscv,isa-extensions-configs node if not exist */
    new_nodeoffset = fdt_add_subnode(blob, nodeoffset, fdt_node_name[i]);
    if (new_nodeoffset < 0 && new_nodeoffset != -FDT_ERR_EXISTS) {
        printf("fdt mknode %s failed, returning %s\n", fdt_node_name[i], fdt_strerror(new_nodeoffset));
        return new_nodeoffset;
    }

    nodeoffset = new_nodeoffset;/* now nodeoffset points to riscv,isa-extensions-configs */
    i++;

    /* add ext subnode if not exist */
    new_nodeoffset = fdt_add_subnode(blob, nodeoffset, fdt_node_name[i]);
    if (new_nodeoffset < 0 && new_nodeoffset != -FDT_ERR_EXISTS) {
        printf("fdt mknode %s failed, returning %s\n", fdt_node_name[i], fdt_strerror(new_nodeoffset));
        return new_nodeoffset;
    }
    i++;

    /* set /cpus/cpu@0/ext/payload and /cpus/cpu@0/ext/tag */
    if (data->ext_tag != NULL) {
        ret = fdt_setprop(blob, new_nodeoffset, fdt_node_name[i], data->ext_tag, strlen(data->ext_tag) + 1);
        if (ret != 0) {
            printf("set %s property failed, returning %s\n", fdt_node_name[i], fdt_strerror(ret));
            return ret;
        }
    }
    i++;

    if (data->ext_payload != NULL) {
        ret = fdt_setprop(blob, new_nodeoffset, fdt_node_name[i], data->ext_payload, strlen(data->ext_payload));
        if (ret != 0) {
            printf("set %s property failed, returning %s\n", fdt_node_name[i], fdt_strerror(ret));
            return ret;
        }
    }
    i++;

    /* add rvv subnode if not exist */
    new_nodeoffset = fdt_add_subnode(blob, nodeoffset, fdt_node_name[i]);
    if (new_nodeoffset < 0 && new_nodeoffset != -FDT_ERR_EXISTS) {
        printf("fdt mknode %s failed, returning %s\n", fdt_node_name[i], fdt_strerror(new_nodeoffset));
        return new_nodeoffset;
    }
    i++;

    /* set /cpus/cpu@0/rvv/vlen */
    if (data->rvv_vlen != NULL) {
        ret = fdt_setprop(blob, new_nodeoffset, fdt_node_name[i], data->rvv_vlen, strlen(data->rvv_vlen) + 1);
        if (ret != 0) {
            printf("set %s property failed, returning %s\n", fdt_node_name[i], fdt_strerror(ret));
            return ret;
        }
    }

    return 0;
}

/*
 * Add asn_data to fdt
 */
static int asn2fdtdemo(asn_data_t data)
{
    char *dtb_file = env_get("fdt_file");
    void *dtb_addr = (void *)(uintptr_t)env_get_hex("dtb_addr", 0);
    const char *path = "/cpus";
    const char *cpu_node_name = NULL;
    int cpus_offset, offset;
    struct fdt_header *blob, *new_blob = NULL;
    const char *loadfdt =
        "ext4load mmc ${mmcdev}:${mmcbootpart} $dtb_addr ${fdt_file};";
    char cmd[64] = {0};
    int len = 0;
    int retry = 4;
    char *bootcmd_load = NULL;
    char *pos = NULL;


    /* 1. Load fdt file */
    if (run_command(loadfdt, 0)) {
        printf("load dtb file failed\n");
        return CMD_RET_FAILURE;
    }

    blob = (struct fdt_header *)dtb_addr;
    if (fdt_check_header(blob)) {
        printf("Invalid FDT file\n");
        return CMD_RET_FAILURE;
    }

    /* 2. set properties for every cpu@n node */
    cpus_offset = fdt_path_offset(blob, FDT_CPUS_PATH);
    if (cpus_offset < 0) {
        printf ("Cannot find /cpus node");
        return CMD_RET_FAILURE;
    }

    for (offset = fdt_first_subnode(blob, cpus_offset);
         offset > 0;
         offset = fdt_next_subnode(blob, offset)) {

        cpu_node_name = fdt_get_name(blob, offset, NULL);
        if (cpu_node_name == NULL) {
            continue;
        }

        if (strncmp(cpu_node_name, FDT_CPU_NODE_PREFIX, 4) == 0) {
            while (convertFDTNode(blob, &data, offset) == -FDT_ERR_NOSPACE) {
                if (retry-- <= 0) {
                    printf("FDT resizing failed\n");
                    return CMD_RET_FAILURE;
                }
                printf("try to resize fdt...\n");
                if (new_blob) {
                    free(new_blob);
                    new_blob = NULL;
                }
                if (asn_fdt_resize(blob, &new_blob)) {
                    return CMD_RET_FAILURE;
                }
                blob = new_blob;
            }
        }
    }

    if (blob != dtb_addr) {
        memcpy(dtb_addr, blob, fdt_totalsize(blob));
        free(blob);
    }

    /* 3. change bootcmd_load */
    bootcmd_load = env_get("bootcmd_load");
    if (!bootcmd_load) {
        printf("bootcmd_load not found\n");
        return CMD_RET_USAGE;
    }

    len = strlen(loadfdt);
    pos = strstr(bootcmd_load, loadfdt);

    if (pos != NULL) {
        memmove(pos, pos + len, strlen(pos + len) + 1);
    }

    env_set("bootcmd_load", bootcmd_load);
    return 0;
}

static int do_asn2fdt(cmd_tbl_t *cmdtp, int flag, int argc,
                      char *const argv[])
{
    if (asn2fdtdemo(asn_data)) {
        return CMD_RET_USAGE;
    }

    return 0;
}

U_BOOT_CMD(
    asn2fdt, 2,	1, do_asn2fdt,
    "Convert ASN1 data to FDT node",
    ""
);

