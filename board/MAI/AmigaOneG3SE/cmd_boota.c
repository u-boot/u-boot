#include <common.h>
#include <command.h>
#include "../disk/part_amiga.h"
#include <asm/cache.h>


#undef BOOTA_DEBUG

#ifdef BOOTA_DEBUG
#define PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

struct block_header {
	u32 id;
	u32 summed_longs;
	s32 chk_sum;
};

extern block_dev_desc_t *ide_get_dev (int dev);
extern struct bootcode_block *get_bootcode (block_dev_desc_t * dev_desc);
extern int sum_block (struct block_header *header);

struct bootcode_block bblk;

int do_boota (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	unsigned char *load_address = (unsigned char *) CFG_LOAD_ADDR;
	unsigned char *base_address;
	unsigned long offset;

	unsigned long part_number = 0;
	block_dev_desc_t *boot_disk;
	char *s;
	struct bootcode_block *boot_code;

	/* Get parameters */

	switch (argc) {
	case 2:
		load_address = (unsigned char *) simple_strtol (argv[1], NULL, 16);
		part_number = 0;
		break;
	case 3:
		load_address = (unsigned char *) simple_strtol (argv[1], NULL, 16);
		part_number = simple_strtol (argv[2], NULL, 16);
		break;
	}

	base_address = load_address;

	PRINTF ("Loading boot code from disk %d to %p\n", part_number,
			load_address);

	/* Find the appropriate disk device */
	boot_disk = ide_get_dev (part_number);
	if (!boot_disk) {
		PRINTF ("Unknown disk %d\n", part_number);
		return 1;
	}

	/* Find the bootcode block */
	boot_code = get_bootcode (boot_disk);
	if (!boot_code) {
		PRINTF ("Not a bootable disk %d\n", part_number);
		return 1;
	}

	/* Only use the offset from the first block */
	offset = boot_code->load_data[0];
	memcpy (load_address, &boot_code->load_data[1], 122 * 4);
	load_address += 122 * 4;

	/* Setup for the loop */
	bblk.next = boot_code->next;
	boot_code = &bblk;

	/* Scan the chain, and copy the loader succesively into the destination area */
	while (0xffffffff != boot_code->next) {
		PRINTF ("Loading block %d\n", boot_code->next);

		/* Load block */
		if (1 !=
			boot_disk->block_read (boot_disk->dev, boot_code->next, 1,
								   (ulong *) & bblk)) {
			PRINTF ("Read error\n");
			return 1;
		}

		/* check sum */
		if (sum_block ((struct block_header *) (ulong *) & bblk) != 0) {
			PRINTF ("Checksum error\n");
			return 1;
		}

		/* Ok, concatenate it to the already loaded code */
		memcpy (load_address, boot_code->load_data, 123 * 4);
		load_address += 123 * 4;
	}

	printf ("Bootcode loaded to %p (size %d)\n", base_address,
			load_address - base_address);
	printf ("Entry point at %p\n", base_address + offset);

	flush_cache (base_address, load_address - base_address);


	s = getenv ("autostart");
	if (s && strcmp (s, "yes") == 0) {
		DECLARE_GLOBAL_DATA_PTR;

		void (*boot) (bd_t *, char *, block_dev_desc_t *);
		char *args;

		boot = (void (*)(bd_t *, char *, block_dev_desc_t *)) (base_address + offset);
		boot (gd->bd, getenv ("amiga_bootargs"), boot_disk);
	}


	return 0;
}
#if defined(CONFIG_AMIGAONEG3SE) && (CONFIG_COMMANDS & CFG_CMD_BSP)
U_BOOT_CMD(
	boota,   3,      1,      do_boota,
	"boota   - boot an Amiga kernel\n",
	"address disk"
);
#endif /* _CMD_BOOTA_H */
