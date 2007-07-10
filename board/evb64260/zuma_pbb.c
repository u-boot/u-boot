#include <common.h>
#include <malloc.h>

#if defined(CONFIG_CMD_BSP)
#include <command.h>
#endif

#include <pci.h>
#include <galileo/pci.h>
#include "zuma_pbb.h"

#undef DEBUG

#define PAT_LO 0x00010203
#define PAT_HI 0x04050607

static PBB_DMA_REG_MAP *zuma_pbb_reg = NULL;
static char test_buf1[2048];
static char test_buf2[2048];
void zuma_init_pbb(void);
int zuma_mbox_init(void);
int zuma_test_dma(int cmd, int size);

int zuma_test_dma (int cmd, int size)
{
	static const char *const test_legend[] = {
		"write", "verify",
		"copy", "compare",
		"write inc", "verify inc"
	};
	register int i, j;
	unsigned int p1 = ((unsigned int) test_buf1 + 0xff) & (~0xff);
	unsigned int p2 = ((unsigned int) test_buf2 + 0xff) & (~0xff);
	volatile unsigned int *ps = (unsigned int *) p1;
	volatile unsigned int *pd = (unsigned int *) p2;
	unsigned int funct, pat_lo = PAT_LO, pat_hi = PAT_HI;
	DMA_INT_STATUS stat;
	int ret = 0;

	if (!zuma_pbb_reg) {
		printf ("not initted\n");
		return -1;
	}

	if (cmd < 0 || cmd > 5) {
		printf ("inv cmd %d\n", cmd);
		return -1;
	}

	if (cmd == 2 || cmd == 3) {
		/* not implemented */
		return 0;
	}

	if (size <= 0 || size > 1024)
		size = 1024;

	size &= (~7);				/* throw away bottom 3 bits */

	p1 = ((unsigned int) test_buf1 + 0xff) & (~0xff);
	p2 = ((unsigned int) test_buf2 + 0xff) & (~0xff);

	memset ((void *) p1, 0, size);
	memset ((void *) p2, 0, size);

	for (i = 0; i < size / 4; i += 2) {
		ps[i] = pat_lo;
		ps[i + 1] = pat_hi;
		if (cmd == 4 || cmd == 5) {
			unsigned char *pl = (unsigned char *) &pat_lo;
			unsigned char *ph = (unsigned char *) &pat_hi;

			for (j = 0; j < 4; j++) {
				pl[j] += 8;
				ph[j] += 8;
			}
		}
	}

	funct = (1 << 31) | (cmd << 24) | (size);

	zuma_pbb_reg->int_mask.pci_bits.chan0 =
			EOF_RX_FLAG | EOF_TX_FLAG | EOB_TX_FLAG;

	zuma_pbb_reg->debug_57 = PAT_LO;	/* patl */
	zuma_pbb_reg->debug_58 = PAT_HI;	/* path */

	zuma_pbb_reg->debug_54 = cpu_to_le32 (p1);	/* src 0x01b0 */
	zuma_pbb_reg->debug_55 = cpu_to_le32 (p2);	/* dst 0x01b8 */
	zuma_pbb_reg->debug_56 = cpu_to_le32 (funct);	/* func, 0x01c0 */

	/* give DMA time to chew on things.. dont use DRAM or PCI */
	/* if you can avoid it. */
	do {
		for (i = 0; i < 1000 * 10; i++);
	} while (le32_to_cpu (zuma_pbb_reg->debug_56) & (1 << 31));

	stat.word = zuma_pbb_reg->status.word;
	zuma_pbb_reg->int_mask.word = 0;

	printf ("stat: %08x (%x)\n", stat.word, stat.pci_bits.chan0);

	printf ("func: %08x\n", le32_to_cpu (zuma_pbb_reg->debug_56));
	printf ("src @%08x: %08x %08x %08x %08x\n", p1, ps[0], ps[1], ps[2],
			ps[3]);
	printf ("dst @%08x: %08x %08x %08x %08x\n", p2, pd[0], pd[1], pd[2],
			pd[3]);
	printf ("func: %08x\n", le32_to_cpu (zuma_pbb_reg->debug_56));


	if (cmd == 0 || cmd == 4) {
		/* this is a write */
		if (!(stat.pci_bits.chan0 & EOF_RX_FLAG) ||	/* not done */
			(memcmp ((void *) ps, (void *) pd, size) != 0)) {	/* cmp error */
			for (i = 0; i < size / 4; i += 2) {
				if ((ps[i] != pd[i]) || (ps[i + 1] != pd[i + 1])) {
					printf ("s @%p:%08x %08x\n", &ps[i], ps[i], ps[i + 1]);
					printf ("d @%p:%08x %08x\n", &pd[i], pd[i], pd[i + 1]);
				}
			}
			ret = -1;
		}
	} else {
		/* this is a verify */
		if (!(stat.pci_bits.chan0 & EOF_TX_FLAG) ||	/* not done */
			(stat.pci_bits.chan0 & EOB_TX_FLAG)) {	/* cmp error */
			printf ("%08x: %08x %08x\n",
					le32_to_cpu (zuma_pbb_reg->debug_63),
					zuma_pbb_reg->debug_61, zuma_pbb_reg->debug_62);
			ret = -1;
		}
	}

	printf ("%s cmd %d, %d bytes: %s!\n", test_legend[cmd], cmd, size,
			(ret == 0) ? "PASSED" : "FAILED");
	return 0;
}

void zuma_init_pbb (void)
{
	unsigned int iobase;
	pci_dev_t dev =
			pci_find_device (VENDOR_ID_ZUMA, DEVICE_ID_ZUMA_PBB, 0);

	if (dev == -1) {
		printf ("no zuma pbb\n");
		return;
	}

	pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &iobase);

	zuma_pbb_reg =
			(PBB_DMA_REG_MAP *) (iobase & PCI_BASE_ADDRESS_MEM_MASK);

	if (!zuma_pbb_reg) {
		printf ("zuma pbb bar none! (hah hah, get it?)\n");
		return;
	}

	zuma_pbb_reg->int_mask.word = 0;

	printf ("pbb @ %p v%d.%d, timestamp %08x\n", zuma_pbb_reg,
			zuma_pbb_reg->version.pci_bits.rev_major,
			zuma_pbb_reg->version.pci_bits.rev_minor,
			zuma_pbb_reg->timestamp);

}

#if defined(CONFIG_CMD_BSP)

static int last_cmd = 4;		/* write increment */
static int last_size = 64;

int
do_zuma_init_pbb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	zuma_init_pbb ();
	return 0;
}

int
do_zuma_test_dma (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 1) {
		last_cmd = simple_strtoul (argv[1], NULL, 10);
	}
	if (argc > 2) {
		last_size = simple_strtoul (argv[2], NULL, 10);
	}
	zuma_test_dma (last_cmd, last_size);
	return 0;
}

int
do_zuma_init_mbox (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	zuma_mbox_init ();
	return 0;
}

U_BOOT_CMD(
	zinit,	 1,	 0,	 do_zuma_init_pbb,
	"zinit   - init zuma pbb\n",
	"\n"
	"    - init zuma pbb\n"
);
U_BOOT_CMD(
	zdtest,	  3,	  1,	  do_zuma_test_dma,
	"zdtest  - run dma test\n",
	"[cmd [count]]\n"
	"    - run dma cmd (w=0,v=1,cp=2,cmp=3,wi=4,vi=5), count bytes\n"
);
U_BOOT_CMD(
	zminit,	  1,	  0,	  do_zuma_init_mbox,
	"zminit  - init zuma mbox\n",
	"\n"
	"    - init zuma mbox\n"
);

#endif
