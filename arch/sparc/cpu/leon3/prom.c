/* prom.c - emulates a sparc v0 PROM for the linux kernel.
 *
 * Copyright (C) 2003 Konrad Eisele <eiselekd@web.de>
 * Copyright (C) 2004 Stefan Holst <mail@s-holst.de>
 * Copyright (C) 2007 Daniel Hellstrom <daniel@gaisler.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 *
 */

#include <common.h>
#include <asm/prom.h>
#include <asm/machines.h>
#include <asm/srmmu.h>
#include <asm/processor.h>
#include <asm/irq.h>
#include <asm/leon.h>
#include <ambapp.h>

#include <config.h>
/*
#define PRINT_ROM_VEC
*/
extern struct linux_romvec *kernel_arg_promvec;
extern ambapp_dev_apbuart *leon3_apbuart;

#define PROM_PGT __attribute__ ((__section__ (".prom.pgt")))
#define PROM_TEXT __attribute__ ((__section__ (".prom.text")))
#define PROM_DATA __attribute__ ((__section__ (".prom.data")))

ambapp_dev_gptimer *gptimer;

/* for __va */
extern int __prom_start;
#define PAGE_OFFSET 0xf0000000
#define phys_base CONFIG_SYS_SDRAM_BASE
#define PROM_OFFS 8192
#define PROM_SIZE_MASK (PROM_OFFS-1)
#define __va(x) ( \
	(void *)( ((unsigned long)(x))-PROM_OFFS+ \
	(CONFIG_SYS_PROM_OFFSET-phys_base)+PAGE_OFFSET-CONFIG_SYS_TEXT_BASE ) \
	)
#define __phy(x) ((void *)(((unsigned long)(x))-PROM_OFFS+CONFIG_SYS_PROM_OFFSET-CONFIG_SYS_TEXT_BASE))

struct property {
	char *name;
	char *value;
	int length;
};

struct node {
	int level;
	struct property *properties;
};

static void leon_reboot(char *bcommand);
static void leon_halt(void);
static int leon_nbputchar(int c);
static int leon_nbgetchar(void);

static int no_nextnode(int node);
static int no_child(int node);
static int no_proplen(int node, char *name);
static int no_getprop(int node, char *name, char *value);
static int no_setprop(int node, char *name, char *value, int len);
static char *no_nextprop(int node, char *name);

static struct property PROM_TEXT *find_property(int node, char *name);
static int PROM_TEXT leon_strcmp(const char *s1, const char *s2);
static void *PROM_TEXT leon_memcpy(void *dest, const void *src, size_t n);
static void PROM_TEXT leon_reboot_physical(char *bcommand);

void __inline__ leon_flush_cache_all(void)
{
	__asm__ __volatile__(" flush ");
      __asm__ __volatile__("sta %%g0, [%%g0] %0\n\t"::"i"(ASI_DFLUSH):"memory");
}

void __inline__ leon_flush_tlb_all(void)
{
	leon_flush_cache_all();
	__asm__ __volatile__("sta %%g0, [%0] %1\n\t"::"r"(0x400),
			     "i"(ASI_MMUFLUSH):"memory");
}

typedef struct {
	unsigned int ctx_table[256];
	unsigned int pgd_table[256];
} sparc_srmmu_setup;

sparc_srmmu_setup srmmu_tables PROM_PGT = {
	{0},
	{0x1e,
	 0x10001e,
	 0x20001e,
	 0x30001e,
	 0x40001e,
	 0x50001e,
	 0x60001e,
	 0x70001e,
	 0x80001e,
	 0x90001e,
	 0xa0001e,
	 0xb0001e,
	 0xc0001e,
	 0xd0001e,
	 0xe0001e,
	 0xf0001e,
	 0x100001e,
	 0x110001e,
	 0x120001e,
	 0x130001e,
	 0x140001e,
	 0x150001e,
	 0x160001e,
	 0x170001e,
	 0x180001e,
	 0x190001e,
	 0x1a0001e,
	 0x1b0001e,
	 0x1c0001e,
	 0x1d0001e,
	 0x1e0001e,
	 0x1f0001e,
	 0x200001e,
	 0x210001e,
	 0x220001e,
	 0x230001e,
	 0x240001e,
	 0x250001e,
	 0x260001e,
	 0x270001e,
	 0x280001e,
	 0x290001e,
	 0x2a0001e,
	 0x2b0001e,
	 0x2c0001e,
	 0x2d0001e,
	 0x2e0001e,
	 0x2f0001e,
	 0x300001e,
	 0x310001e,
	 0x320001e,
	 0x330001e,
	 0x340001e,
	 0x350001e,
	 0x360001e,
	 0x370001e,
	 0x380001e,
	 0x390001e,
	 0x3a0001e,
	 0x3b0001e,
	 0x3c0001e,
	 0x3d0001e,
	 0x3e0001e,
	 0x3f0001e,
	 0x400001e,
	 0x410001e,
	 0x420001e,
	 0x430001e,
	 0x440001e,
	 0x450001e,
	 0x460001e,
	 0x470001e,
	 0x480001e,
	 0x490001e,
	 0x4a0001e,
	 0x4b0001e,
	 0x4c0001e,
	 0x4d0001e,
	 0x4e0001e,
	 0x4f0001e,
	 0x500001e,
	 0x510001e,
	 0x520001e,
	 0x530001e,
	 0x540001e,
	 0x550001e,
	 0x560001e,
	 0x570001e,
	 0x580001e,
	 0x590001e,
	 0x5a0001e,
	 0x5b0001e,
	 0x5c0001e,
	 0x5d0001e,
	 0x5e0001e,
	 0x5f0001e,
	 0x600001e,
	 0x610001e,
	 0x620001e,
	 0x630001e,
	 0x640001e,
	 0x650001e,
	 0x660001e,
	 0x670001e,
	 0x680001e,
	 0x690001e,
	 0x6a0001e,
	 0x6b0001e,
	 0x6c0001e,
	 0x6d0001e,
	 0x6e0001e,
	 0x6f0001e,
	 0x700001e,
	 0x710001e,
	 0x720001e,
	 0x730001e,
	 0x740001e,
	 0x750001e,
	 0x760001e,
	 0x770001e,
	 0x780001e,
	 0x790001e,
	 0x7a0001e,
	 0x7b0001e,
	 0x7c0001e,
	 0x7d0001e,
	 0x7e0001e,
	 0x7f0001e,
	 0x800001e,
	 0x810001e,
	 0x820001e,
	 0x830001e,
	 0x840001e,
	 0x850001e,
	 0x860001e,
	 0x870001e,
	 0x880001e,
	 0x890001e,
	 0x8a0001e,
	 0x8b0001e,
	 0x8c0001e,
	 0x8d0001e,
	 0x8e0001e,
	 0x8f0001e,
	 0x900001e,
	 0x910001e,
	 0x920001e,
	 0x930001e,
	 0x940001e,
	 0x950001e,
	 0x960001e,
	 0x970001e,
	 0x980001e,
	 0x990001e,
	 0x9a0001e,
	 0x9b0001e,
	 0x9c0001e,
	 0x9d0001e,
	 0x9e0001e,
	 0x9f0001e,
	 0xa00001e,
	 0xa10001e,
	 0xa20001e,
	 0xa30001e,
	 0xa40001e,
	 0xa50001e,
	 0xa60001e,
	 0xa70001e,
	 0xa80001e,
	 0xa90001e,
	 0xaa0001e,
	 0xab0001e,
	 0xac0001e,
	 0xad0001e,
	 0xae0001e,
	 0xaf0001e,
	 0xb00001e,
	 0xb10001e,
	 0xb20001e,
	 0xb30001e,
	 0xb40001e,
	 0xb50001e,
	 0xb60001e,
	 0xb70001e,
	 0xb80001e,
	 0xb90001e,
	 0xba0001e,
	 0xbb0001e,
	 0xbc0001e,
	 0xbd0001e,
	 0xbe0001e,
	 0xbf0001e,
	 0xc00001e,
	 0xc10001e,
	 0xc20001e,
	 0xc30001e,
	 0xc40001e,
	 0xc50001e,
	 0xc60001e,
	 0xc70001e,
	 0xc80001e,
	 0xc90001e,
	 0xca0001e,
	 0xcb0001e,
	 0xcc0001e,
	 0xcd0001e,
	 0xce0001e,
	 0xcf0001e,
	 0xd00001e,
	 0xd10001e,
	 0xd20001e,
	 0xd30001e,
	 0xd40001e,
	 0xd50001e,
	 0xd60001e,
	 0xd70001e,
	 0xd80001e,
	 0xd90001e,
	 0xda0001e,
	 0xdb0001e,
	 0xdc0001e,
	 0xdd0001e,
	 0xde0001e,
	 0xdf0001e,
	 0xe00001e,
	 0xe10001e,
	 0xe20001e,
	 0xe30001e,
	 0xe40001e,
	 0xe50001e,
	 0xe60001e,
	 0xe70001e,
	 0xe80001e,
	 0xe90001e,
	 0xea0001e,
	 0xeb0001e,
	 0xec0001e,
	 0xed0001e,
	 0xee0001e,
	 0xef0001e,
	 0x400001e		/* default */
	 }
};

/* a self contained prom info structure */
struct leon_reloc_func {
	struct property *(*find_property) (int node, char *name);
	int (*strcmp) (char *s1, char *s2);
	void *(*memcpy) (void *dest, const void *src, size_t n);
	void (*reboot_physical) (char *cmd);
	ambapp_dev_apbuart *leon3_apbuart;
};

struct leon_prom_info {
	int freq_khz;
	int leon_nctx;
	int mids[32];
	int baudrates[2];
	struct leon_reloc_func reloc_funcs;
	struct property root_properties[4];
	struct property cpu_properties[7];
#undef  CPUENTRY
#define CPUENTRY(idx) struct property cpu_properties##idx[4]
	 CPUENTRY(1);
	 CPUENTRY(2);
	 CPUENTRY(3);
	 CPUENTRY(4);
	 CPUENTRY(5);
	 CPUENTRY(6);
	 CPUENTRY(7);
	 CPUENTRY(8);
	 CPUENTRY(9);
	 CPUENTRY(10);
	 CPUENTRY(11);
	 CPUENTRY(12);
	 CPUENTRY(13);
	 CPUENTRY(14);
	 CPUENTRY(15);
	 CPUENTRY(16);
	 CPUENTRY(17);
	 CPUENTRY(18);
	 CPUENTRY(19);
	 CPUENTRY(20);
	 CPUENTRY(21);
	 CPUENTRY(22);
	 CPUENTRY(23);
	 CPUENTRY(24);
	 CPUENTRY(25);
	 CPUENTRY(26);
	 CPUENTRY(27);
	 CPUENTRY(28);
	 CPUENTRY(29);
	 CPUENTRY(30);
	 CPUENTRY(31);
	struct idprom idprom;
	struct linux_nodeops nodeops;
	struct linux_mlist_v0 *totphys_p;
	struct linux_mlist_v0 totphys;
	struct linux_mlist_v0 *avail_p;
	struct linux_mlist_v0 avail;
	struct linux_mlist_v0 *prommap_p;
	void (*synchook) (void);
	struct linux_arguments_v0 *bootargs_p;
	struct linux_arguments_v0 bootargs;
	struct linux_romvec romvec;
	struct node nodes[35];
	char s_device_type[12];
	char s_cpu[4];
	char s_mid[4];
	char s_idprom[7];
	char s_compatability[14];
	char s_leon2[6];
	char s_mmu_nctx[9];
	char s_frequency[16];
	char s_uart1_baud[11];
	char s_uart2_baud[11];
	char arg[256];
};

/* static prom info */
static struct leon_prom_info PROM_DATA spi = {
	CONFIG_SYS_CLK_FREQ / 1000,
	256,
	{
#undef	CPUENTRY
#define	CPUENTRY(idx) idx
	 CPUENTRY(0),
	 CPUENTRY(1),
	 CPUENTRY(2),
	 CPUENTRY(3),
	 CPUENTRY(4),
	 CPUENTRY(5),
	 CPUENTRY(6),
	 CPUENTRY(7),
	 CPUENTRY(8),
	 CPUENTRY(9),
	 CPUENTRY(10),
	 CPUENTRY(11),
	 CPUENTRY(12),
	 CPUENTRY(13),
	 CPUENTRY(14),
	 CPUENTRY(15),
	 CPUENTRY(16),
	 CPUENTRY(17),
	 CPUENTRY(18),
	 CPUENTRY(19),
	 CPUENTRY(20),
	 CPUENTRY(21),
	 CPUENTRY(22),
	 CPUENTRY(23),
	 CPUENTRY(24),
	 CPUENTRY(25),
	 CPUENTRY(26),
	 CPUENTRY(27),
	 CPUENTRY(28),
	 CPUENTRY(29),
	 CPUENTRY(30),
	 31},
	{38400, 38400},
	{
	 __va(find_property),
	 __va(leon_strcmp),
	 __va(leon_memcpy),
	 __phy(leon_reboot_physical),
	 },
	{
	 {__va(spi.s_device_type), __va(spi.s_idprom), 4},
	 {__va(spi.s_idprom), (char *)__va(&spi.idprom), sizeof(struct idprom)},
	 {__va(spi.s_compatability), __va(spi.s_leon2), 5},
	 {NULL, NULL, -1}
	 },
	{
	 {__va(spi.s_device_type), __va(spi.s_cpu), 4},
	 {__va(spi.s_mid), __va(&spi.mids[0]), 4},
	 {__va(spi.s_mmu_nctx), (char *)__va(&spi.leon_nctx), 4},
	 {__va(spi.s_frequency), (char *)__va(&spi.freq_khz), 4},
	 {__va(spi.s_uart1_baud), (char *)__va(&spi.baudrates[0]), 4},
	 {__va(spi.s_uart2_baud), (char *)__va(&spi.baudrates[1]), 4},
	 {NULL, NULL, -1}
	 },
#undef  CPUENTRY
#define CPUENTRY(idx) \
	{ /* cpu_properties */						\
		{__va(spi.s_device_type), __va(spi.s_cpu), 4},		\
		{__va(spi.s_mid), __va(&spi.mids[idx]), 4},			\
		{__va(spi.s_frequency), (char *)__va(&spi.freq_khz), 4},	\
		{NULL, NULL, -1}						\
	}
	CPUENTRY(1),
	CPUENTRY(2),
	CPUENTRY(3),
	CPUENTRY(4),
	CPUENTRY(5),
	CPUENTRY(6),
	CPUENTRY(7),
	CPUENTRY(8),
	CPUENTRY(9),
	CPUENTRY(10),
	CPUENTRY(11),
	CPUENTRY(12),
	CPUENTRY(13),
	CPUENTRY(14),
	CPUENTRY(15),
	CPUENTRY(16),
	CPUENTRY(17),
	CPUENTRY(18),
	CPUENTRY(19),
	CPUENTRY(20),
	CPUENTRY(21),
	CPUENTRY(22),
	CPUENTRY(23),
	CPUENTRY(24),
	CPUENTRY(25),
	CPUENTRY(26),
	CPUENTRY(27),
	CPUENTRY(28),
	CPUENTRY(29),
	CPUENTRY(30),
	CPUENTRY(31),
	{
	 0x01,			/* format */
	 M_LEON2 | M_LEON2_SOC,	/* machine type */
	 {0, 0, 0, 0, 0, 0},	/* eth */
	 0,			/* date */
	 0,			/* sernum */
	 0,			/* checksum */
	 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}	/* reserved */
	 },
	{
	 __va(no_nextnode),
	 __va(no_child),
	 __va(no_proplen),
	 __va(no_getprop),
	 __va(no_setprop),
	 __va(no_nextprop)
	 },
	__va(&spi.totphys),
	{
	 NULL,
	 (char *)CONFIG_SYS_SDRAM_BASE,
	 0,
	 },
	__va(&spi.avail),
	{
	 NULL,
	 (char *)CONFIG_SYS_SDRAM_BASE,
	 0,
	 },
	NULL,			/* prommap_p */
	NULL,
	__va(&spi.bootargs),
	{
	 {NULL, __va(spi.arg), NULL /*... */ },
	 /*... */
	 },
	{
	 0,
	 0,			/* sun4c v0 prom */
	 0, 0,
	 {__va(&spi.totphys_p), __va(&spi.prommap_p), __va(&spi.avail_p)},
	 __va(&spi.nodeops),
	 NULL, {NULL /* ... */ },
	 NULL, NULL,
	 NULL, NULL,		/* pv_getchar, pv_putchar */
	 __va(leon_nbgetchar), __va(leon_nbputchar),
	 NULL,
	 __va(leon_reboot),
	 NULL,
	 NULL,
	 NULL,
	 __va(leon_halt),
	 __va(&spi.synchook),
	 {NULL},
	 __va(&spi.bootargs_p)
	 /*... */
	 },
	{
	 {0, __va(spi.root_properties + 3) /* NULL, NULL, -1 */ },
	 {0, __va(spi.root_properties)},
	 /* cpu 0, must be spi.nodes[2] see leon_prom_init() */
	 {1, __va(spi.cpu_properties)},

#undef  CPUENTRY
#define CPUENTRY(idx) \
	  {1, __va(spi.cpu_properties##idx) }	/* cpu <idx> */
	 CPUENTRY(1),
	 CPUENTRY(2),
	 CPUENTRY(3),
	 CPUENTRY(4),
	 CPUENTRY(5),
	 CPUENTRY(6),
	 CPUENTRY(7),
	 CPUENTRY(8),
	 CPUENTRY(9),
	 CPUENTRY(10),
	 CPUENTRY(11),
	 CPUENTRY(12),
	 CPUENTRY(13),
	 CPUENTRY(14),
	 CPUENTRY(15),
	 CPUENTRY(16),
	 CPUENTRY(17),
	 CPUENTRY(18),
	 CPUENTRY(19),
	 CPUENTRY(20),
	 CPUENTRY(21),
	 CPUENTRY(22),
	 CPUENTRY(23),
	 CPUENTRY(24),
	 CPUENTRY(25),
	 CPUENTRY(26),
	 CPUENTRY(27),
	 CPUENTRY(28),
	 CPUENTRY(29),
	 CPUENTRY(30),
	 CPUENTRY(31),
	 {-1, __va(spi.root_properties + 3) /* NULL, NULL, -1 */ }
	 },
	"device_type",
	"cpu",
	"mid",
	"idprom",
	"compatability",
	"leon2",
	"mmu-nctx",
	"clock-frequency",
	"uart1_baud",
	"uart2_baud",
	CONFIG_DEFAULT_KERNEL_COMMAND_LINE
};

/* from arch/sparc/kernel/setup.c */
#define RAMDISK_LOAD_FLAG 0x4000
extern unsigned short root_flags;
extern unsigned short root_dev;
extern unsigned short ram_flags;
extern unsigned int sparc_ramdisk_image;
extern unsigned int sparc_ramdisk_size;
extern int root_mountflags;

extern char initrd_end, initrd_start;

/* Reboot the CPU = jump to beginning of flash again.
 *
 * Make sure that all function are inlined here.
 */
static void PROM_TEXT leon_reboot(char *bcommand)
{
	register char *arg = bcommand;
	void __attribute__ ((noreturn)) (*reboot_physical) (char *cmd);

	/* get physical address */
	struct leon_prom_info *pspi =
	    (void *)(CONFIG_SYS_PROM_OFFSET + sizeof(srmmu_tables));

	unsigned int *srmmu_ctx_table;

	/* Turn of Interrupts */
	set_pil(0xf);

	/* Set kernel's context, context zero */
	srmmu_set_context(0);

	/* Get physical address of the MMU shutdown routine */
	reboot_physical = (void *)
	    SPARC_BYPASS_READ(&pspi->reloc_funcs.reboot_physical);

	/* Now that we know the physical address of the function
	 * we can make the MMU allow jumping to it.
	 */
	srmmu_ctx_table = (unsigned int *)srmmu_get_ctable_ptr();

	srmmu_ctx_table = (unsigned int *)SPARC_BYPASS_READ(srmmu_ctx_table);

	/* get physical address of kernel's context table (assume ptd) */
	srmmu_ctx_table = (unsigned int *)
	    (((unsigned int)srmmu_ctx_table & 0xfffffffc) << 4);

	/* enable access to physical address of MMU shutdown function */
	SPARC_BYPASS_WRITE(&srmmu_ctx_table
			   [((unsigned int)reboot_physical) >> 24],
			   (((unsigned int)reboot_physical & 0xff000000) >> 4) |
			   0x1e);

	/* flush TLB cache */
	leon_flush_tlb_all();

	/* flash instruction & data cache */
	sparc_icache_flush_all();
	sparc_dcache_flush_all();

	/* jump to physical address function
	 * so that when the MMU is disabled
	 * we can continue to execute
	 */
	reboot_physical(arg);
}

static void PROM_TEXT leon_reboot_physical(char *bcommand)
{
	void __attribute__ ((noreturn)) (*reset) (void);

	/* Turn off MMU */
	srmmu_set_mmureg(0);

	/* Hardcoded start address */
	reset = CONFIG_SYS_MONITOR_BASE;

	/* flush data cache */
	sparc_dcache_flush_all();

	/* flush instruction cache */
	sparc_icache_flush_all();

	/* Jump to start in Flash */
	reset();
}

static void PROM_TEXT leon_halt(void)
{
	while (1) ;
}

/* get single char, don't care for blocking*/
static int PROM_TEXT leon_nbgetchar(void)
{
	return -1;
}

/* put single char, don't care for blocking*/
static int PROM_TEXT leon_nbputchar(int c)
{
	ambapp_dev_apbuart *uart;

	/* get physical address */
	struct leon_prom_info *pspi =
	    (void *)(CONFIG_SYS_PROM_OFFSET + sizeof(srmmu_tables));

	uart = (ambapp_dev_apbuart *)
	    SPARC_BYPASS_READ(&pspi->reloc_funcs.leon3_apbuart);

	/* no UART? */
	if (!uart)
		return 0;

	/***** put char in buffer... ***********
	 * Make sure all functions are inline! *
	 ***************************************/

	/* Wait for last character to go. */
	while (!(SPARC_BYPASS_READ(&uart->status)
		 & LEON_REG_UART_STATUS_THE)) ;

	/* Send data */
	SPARC_BYPASS_WRITE(&uart->data, c);

	/* Wait for data to be sent */
	while (!(SPARC_BYPASS_READ(&uart->status)
		 & LEON_REG_UART_STATUS_TSE)) ;

	return 0;
}

/* node ops */

/*#define nodes ((struct node *)__va(&pspi->nodes))*/
#define nodes ((struct node *)(pspi->nodes))

static int PROM_TEXT no_nextnode(int node)
{
	/* get physical address */
	struct leon_prom_info *pspi =
	    (void *)(CONFIG_SYS_PROM_OFFSET + sizeof(srmmu_tables));

	/* convert into virtual address */
	pspi = (struct leon_prom_info *)
	    (((unsigned int)pspi & 0x0fffffff) | PAGE_OFFSET);

	if (nodes[node].level == nodes[node + 1].level)
		return node + 1;
	return -1;
}

static int PROM_TEXT no_child(int node)
{
	/* get physical address */
	struct leon_prom_info *pspi = (struct leon_prom_info *)
	    (CONFIG_SYS_PROM_OFFSET + sizeof(srmmu_tables));

	/* convert into virtual address */
	pspi = (struct leon_prom_info *)
	    (((unsigned int)pspi & 0x0fffffff) | PAGE_OFFSET);

	if (nodes[node].level == nodes[node + 1].level - 1)
		return node + 1;
	return -1;
}

static struct property PROM_TEXT *find_property(int node, char *name)
{
	/* get physical address */
	struct leon_prom_info *pspi = (struct leon_prom_info *)
	    (CONFIG_SYS_PROM_OFFSET + sizeof(srmmu_tables));

	/* convert into virtual address */
	pspi = (struct leon_prom_info *)
	    (((unsigned int)pspi & 0x0fffffff) | PAGE_OFFSET);

	struct property *prop = &nodes[node].properties[0];
	while (prop && prop->name) {
		if (pspi->reloc_funcs.strcmp(prop->name, name) == 0)
			return prop;
		prop++;
	}
	return NULL;
}

static int PROM_TEXT no_proplen(int node, char *name)
{
	/* get physical address */
	struct leon_prom_info *pspi = (struct leon_prom_info *)
	    (CONFIG_SYS_PROM_OFFSET + sizeof(srmmu_tables));

	/* convert into virtual address */
	pspi = (struct leon_prom_info *)
	    (((unsigned int)pspi & 0x0fffffff) | PAGE_OFFSET);

	struct property *prop = pspi->reloc_funcs.find_property(node, name);
	if (prop)
		return prop->length;
	return -1;
}

static int PROM_TEXT no_getprop(int node, char *name, char *value)
{
	/* get physical address */
	struct leon_prom_info *pspi = (struct leon_prom_info *)
	    (CONFIG_SYS_PROM_OFFSET + sizeof(srmmu_tables));

	/* convert into virtual address */
	pspi = (struct leon_prom_info *)
	    (((unsigned int)pspi & 0x0fffffff) | PAGE_OFFSET);

	struct property *prop = pspi->reloc_funcs.find_property(node, name);
	if (prop) {
		pspi->reloc_funcs.memcpy(value, prop->value, prop->length);
		return 1;
	}
	return -1;
}

static int PROM_TEXT no_setprop(int node, char *name, char *value, int len)
{
	return -1;
}

static char PROM_TEXT *no_nextprop(int node, char *name)
{
	/* get physical address */
	struct leon_prom_info *pspi = (struct leon_prom_info *)
	    (CONFIG_SYS_PROM_OFFSET + sizeof(srmmu_tables));
	struct property *prop;

	/* convert into virtual address */
	pspi = (struct leon_prom_info *)
	    (((unsigned int)pspi & 0x0fffffff) | PAGE_OFFSET);

	if (!name || !name[0])
		return nodes[node].properties[0].name;

	prop = pspi->reloc_funcs.find_property(node, name);
	if (prop)
		return prop[1].name;
	return NULL;
}

static int PROM_TEXT leon_strcmp(const char *s1, const char *s2)
{
	register char result;

	while (1) {
		result = *s1 - *s2;
		if (result || !*s1)
			break;
		s2++;
		s1++;
	}

	return result;
}

static void *PROM_TEXT leon_memcpy(void *dest, const void *src, size_t n)
{
	char *dst = (char *)dest, *source = (char *)src;

	while (n--) {
		*dst = *source;
		dst++;
		source++;
	}
	return dest;
}

#define GETREGSP(sp) __asm__ __volatile__("mov %%sp, %0" : "=r" (sp))

void leon_prom_init(struct leon_prom_info *pspi)
{
	unsigned long i;
	unsigned char cksum, *ptr;
	char *addr_str, *end;
	unsigned long sp;
	GETREGSP(sp);

	pspi->freq_khz = CONFIG_SYS_CLK_FREQ / 1000;

	/* Set Available main memory size */
	pspi->totphys.num_bytes = CONFIG_SYS_PROM_OFFSET - CONFIG_SYS_SDRAM_BASE;
	pspi->avail.num_bytes = pspi->totphys.num_bytes;

	/* Set the pointer to the Console UART in romvec */
	pspi->reloc_funcs.leon3_apbuart = leon3_apbuart;

	{
		int j = 1;
#ifdef CONFIG_SMP
		ambapp_dev_irqmp *b;
		b = (ambapp_dev_irqmp *) leon3_getapbbase(VENDOR_GAISLER,
							  GAISLER_IRQMP);
		if (b) {
			j = 1 + ((LEON3_BYPASS_LOAD_PA(&(b->mpstatus))
				  >> LEON3_IRQMPSTATUS_CPUNR) & 0xf);
		}
#endif
#undef nodes
		pspi->nodes[2 + j].level = -1;
		pspi->nodes[2 + j].properties = __va(spi.root_properties + 3);
	}

	/* Set Ethernet MAC address from environment */
	if ((addr_str = getenv("ethaddr")) != NULL) {
		for (i = 0; i < 6; i++) {
			pspi->idprom.id_ethaddr[i] = addr_str ?
			    simple_strtoul(addr_str, &end, 16) : 0;
			if (addr_str) {
				addr_str = (*end) ? end + 1 : end;
			}
		}
	} else {
		/* HW Address not found in environment,
		 * Set default HW address
		 */
		pspi->idprom.id_ethaddr[0] = 0;
		pspi->idprom.id_ethaddr[1] = 0;
		pspi->idprom.id_ethaddr[2] = 0;
		pspi->idprom.id_ethaddr[3] = 0;
		pspi->idprom.id_ethaddr[4] = 0;
		pspi->idprom.id_ethaddr[5] = 0;
	}

	ptr = (unsigned char *)&pspi->idprom;
	for (i = cksum = 0; i <= 0x0E; i++)
		cksum ^= *ptr++;
	pspi->idprom.id_cksum = cksum;
}

static inline void set_cache(unsigned long regval)
{
	asm volatile ("sta %0, [%%g0] %1\n\t":: "r" (regval), "i"(2):"memory");
}

extern unsigned short bss_start, bss_end;

/* mark as section .img.main.text, to be referenced in linker script */
int prom_init(void)
{
	struct leon_prom_info *pspi = (void *)
	    ((((unsigned int)&spi) & PROM_SIZE_MASK) + CONFIG_SYS_PROM_OFFSET);

	/* disable mmu */
	srmmu_set_mmureg(0x00000000);
	__asm__ __volatile__("flush\n\t");

	/* init prom info struct */
	leon_prom_init(pspi);

	kernel_arg_promvec = &pspi->romvec;
#ifdef PRINT_ROM_VEC
	printf("Kernel rom vec: 0x%lx\n", (unsigned int)(&pspi->romvec));
#endif
	return 0;
}

/* Copy current kernel boot argument to ROMvec */
void prepare_bootargs(char *bootargs)
{
	struct leon_prom_info *pspi;
	char *src, *dst;
	int left;

	/* if no bootargs set, skip copying ==> default bootline */
	if (bootargs && (*bootargs != '\0')) {
		pspi = (void *)((((unsigned int)&spi) & PROM_SIZE_MASK) +
				CONFIG_SYS_PROM_OFFSET);
		src = bootargs;
		dst = &pspi->arg[0];
		left = 255;	/* max len */
		while (*src && left > 0) {
			*dst++ = *src++;
			left--;
		}
		/* terminate kernel command line string */
		*dst = 0;
	}
}

void srmmu_init_cpu(unsigned int entry)
{
	sparc_srmmu_setup *psrmmu_tables = (void *)
	    ((((unsigned int)&srmmu_tables) & PROM_SIZE_MASK) +
	     CONFIG_SYS_PROM_OFFSET);

	/* Make context 0 (kernel's context) point
	 * to our prepared memory mapping
	 */
#define PTD 1
	psrmmu_tables->ctx_table[0] =
	    ((unsigned int)&psrmmu_tables->pgd_table[0x00]) >> 4 | PTD;

	/* Set virtual kernel address 0xf0000000
	 * to SRAM/SDRAM address.
	 * Make it READ/WRITE/EXEC to SuperUser
	 */
#define PTE 2
#define ACC_SU_ALL 0x1c
	psrmmu_tables->pgd_table[0xf0] =
	    (CONFIG_SYS_SDRAM_BASE >> 4) | ACC_SU_ALL | PTE;
	psrmmu_tables->pgd_table[0xf1] =
	    ((CONFIG_SYS_SDRAM_BASE + 0x1000000) >> 4) | ACC_SU_ALL | PTE;
	psrmmu_tables->pgd_table[0xf2] =
	    ((CONFIG_SYS_SDRAM_BASE + 0x2000000) >> 4) | ACC_SU_ALL | PTE;
	psrmmu_tables->pgd_table[0xf3] =
	    ((CONFIG_SYS_SDRAM_BASE + 0x3000000) >> 4) | ACC_SU_ALL | PTE;
	psrmmu_tables->pgd_table[0xf4] =
	    ((CONFIG_SYS_SDRAM_BASE + 0x4000000) >> 4) | ACC_SU_ALL | PTE;
	psrmmu_tables->pgd_table[0xf5] =
	    ((CONFIG_SYS_SDRAM_BASE + 0x5000000) >> 4) | ACC_SU_ALL | PTE;
	psrmmu_tables->pgd_table[0xf6] =
	    ((CONFIG_SYS_SDRAM_BASE + 0x6000000) >> 4) | ACC_SU_ALL | PTE;
	psrmmu_tables->pgd_table[0xf7] =
	    ((CONFIG_SYS_SDRAM_BASE + 0x7000000) >> 4) | ACC_SU_ALL | PTE;

	/* convert rom vec pointer to virtual address */
	kernel_arg_promvec = (struct linux_romvec *)
	    (((unsigned int)kernel_arg_promvec & 0x0fffffff) | 0xf0000000);

	/* Set Context pointer to point to context table
	 * 256 contexts supported.
	 */
	srmmu_set_ctable_ptr((unsigned int)&psrmmu_tables->ctx_table[0]);

	/* Set kernel's context, context zero */
	srmmu_set_context(0);

	/* Invalidate all Cache */
	__asm__ __volatile__("flush\n\t");

	srmmu_set_mmureg(0x00000001);
	leon_flush_tlb_all();
	leon_flush_cache_all();
}
