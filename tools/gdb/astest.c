#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <bfd.h>
#include "error.h"

int verbose = 0;

void
process_section(bfd *abfd, asection *sect, PTR obj)
{
    printf("Section '%s':\n", sect->name);

    printf("\tindex=%d, flags=%x\n", sect->index, sect->flags);

#if 0
    printf("\tuser_set_vma=%u, reloc_done=%u, linker_mark=%u, gc_mark=%u\n",
	(unsigned long)sect->user_set_vma, (unsigned long)sect->reloc_done,
	(unsigned long)sect->linker_mark, (unsigned long)sect->gc_mark);
#else
    printf("\tuser_set_vma=%u, reloc_done=%u\n",
	(unsigned int)sect->user_set_vma, (unsigned int)sect->reloc_done);
#endif

    printf("\tvma=%08lx, lma=%08lx\n",
	(unsigned long)sect->vma, (unsigned long)sect->lma);

    printf("\tcooked_size=%ld, raw_size=%ld, output_offset=%ld\n",
	(long)sect->_cooked_size, (long)sect->_raw_size,
	(long)sect->output_offset);

    printf("\talignment_power=%d, reloc_count=%d, filepos=%ld\n",
	sect->alignment_power, sect->reloc_count, sect->filepos);

    printf("\trel_filepos=%ld, line_filepos=%ld, lineno_count=%d\n",
	sect->rel_filepos, sect->line_filepos, sect->lineno_count);

    printf("\tmoving_line_filepos=%ld, target_index=%d\n",
	sect->moving_line_filepos, sect->target_index);
}

int
main(int ac, char **av)
{
    int c, ifd;
    char *ifn;
    bfd *bfdp;

    if ((pname = strrchr(av[0], '/')) == NULL)
	pname = av[0];
    else
	pname++;

    while ((c = getopt(ac, av, "v")) != EOF)
	switch (c) {

	case 'v':
	    verbose = 1;
	    break;

	default:
	usage:
	    fprintf(stderr, "Usage: %s [-v] imagefile\n", pname);
	    exit(1);
	}
    if (optind != ac - 1)
	goto usage;

    ifn = av[optind];

    if (verbose)
	fprintf(stderr, "Opening file...\n");

    if ((ifd = open(ifn, O_RDONLY)) < 0)
	Perror("can't open image file '%s'", ifn);

    if ((bfdp = bfd_fdopenr(ifn, "elf32-powerpc", ifd)) == NULL) {
	bfd_perror(ifn);
	close(ifd);
	Error("bfd_fdopenr of file '%s' failed", ifn);
    }
    bfdp->cacheable = true;

    if (!bfd_check_format(bfdp, bfd_object) ||
      (bfd_get_file_flags(bfdp) & EXEC_P) == 0) {
	bfd_close(bfdp);
	Error("file '%s' is not an executable object file (%s,0x%x)", ifn,
	    bfd_format_string(bfd_get_format(bfdp)), bfd_get_file_flags(bfdp));
    }

    printf("file '%s' is type '%s'...\n", ifn, bfd_get_target(bfdp));

    bfd_map_over_sections(bfdp, process_section, NULL);;

    bfd_close(bfdp);

    if (verbose)
	fprintf(stderr, "Done.\n");

    return (0);
}
