#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <getopt.h>
#if defined(__alpha__) || defined (__ia64__)
#include <sys/io.h>
#elif defined(HAVE_SYS_PERM)
#include <sys/perm.h>
#endif
#include "debug.h"
#include "v86bios.h"
#include "pci.h"
#include "AsmMacros.h"

#define SIZE 0x100000
#define VRAM_START 0xA0000
#define VRAM_SIZE 0x1FFFF
#define V_BIOS_SIZE 0x1FFFF
#define BIOS_START 0x7C00            /* default BIOS entry */
#define BIOS_MEM 0x600

CARD8 code[] = { 0xcd, 0x10, 0xf4 };
struct config Config;

static int map(void);
static void unmap(void);
static void runBIOS(int argc, char **argv);
static int map_vram(void);
static void unmap_vram(void);
static int copy_vbios(memType base);
static int copy_sys_bios(void);
static CARD32 setup_int_vect(void);
static void update_bios_vars(void);
static int chksum(CARD8 *start);
static void setup_bios_regs(i86biosRegsPtr regs, int argc, char **argv);
static void print_regs(i86biosRegsPtr regs);
void dprint(unsigned long start, unsigned long size);

void loadCodeToMem(unsigned char *ptr, CARD8 *code);

static int vram_mapped = 0;
static char* bios_var;


int
main(int argc,char **argv)
{
    CARD32 vbios_base;

    Config.PrintPort = PRINT_PORT;
    Config.IoStatistics = IO_STATISTICS;
    Config.PrintIrq = PRINT_IRQ;
    Config.PrintPci = PRINT_PCI;
    Config.ShowAllDev = SHOW_ALL_DEV;
    Config.PrintIp = PRINT_IP;
    Config.SaveBios = SAVE_BIOS;
    Config.Trace = TRACE;
    Config.ConfigActiveOnly = CONFIG_ACTIVE_ONLY;
    Config.ConfigActiveDevice = CONFIG_ACTIVE_DEVICE;
    Config.MapSysBios = MAP_SYS_BIOS;
    Config.Resort = RESORT;
    Config.FixRom = FIX_ROM;
    Config.NoConsole = NO_CONSOLE;
    Config.Verbose = VERBOSE;

    if (!map())
    exit(1);
    if (!copy_sys_bios())
    exit(1);
    if (!(vbios_base = setup_int_vect()))
    exit(1);
    if (!map_vram())
    exit(1);
    if (!copy_vbios(vbios_base))
    exit(1);

    iopl(3);
    setup_io();
    runBIOS(argc,argv);
    update_bios_vars();
    unmap_vram();
    iopl(0);
    unmap();
    printf("done !\n");
    exit (1);
}

int
map(void)
{
    void* mem;

    mem = mmap(0, (size_t)SIZE,
	       PROT_EXEC | PROT_READ | PROT_WRITE,
	       MAP_FIXED | MAP_PRIVATE | MAP_ANON,
	       -1, 0 );
    if (mem != 0) {
	perror("anonymous map");
	return (0);
    }
    memset(mem,0,SIZE);

    loadCodeToMem((unsigned char *) BIOS_START, code);
    return (1);
}

static int
copy_sys_bios(void)
{
#define SYS_BIOS 0xF0000
    int mem_fd;

    if ((mem_fd = open(MEM_FILE,O_RDONLY))<0) {
	perror("opening memory");
	return (0);
    }

    if (lseek(mem_fd,(off_t) SYS_BIOS,SEEK_SET) != (off_t) SYS_BIOS)
	goto Error;
    if (read(mem_fd, (char *)SYS_BIOS, (size_t) 0xFFFF) != (size_t) 0xFFFF)
	goto Error;

    close(mem_fd);
    return (1);

Error:
    perror("sys_bios");
    close(mem_fd);
    return (0);
}

static int
map_vram(void)
{
    int mem_fd;

#ifdef __ia64__
    if ((mem_fd = open(MEM_FILE,O_RDWR | O_SYNC))<0)
#else
    if ((mem_fd = open(MEM_FILE,O_RDWR))<0)
#endif
      {
	perror("opening memory");
	return 0;
    }

#ifndef __alpha__
    if (mmap((void *) VRAM_START, (size_t) VRAM_SIZE,
		     PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
		     mem_fd, VRAM_START) == (void *) -1)
#else
	 if (!_bus_base()) sparse_shift = 7; /* Uh, oh, JENSEN... */
	 if (!_bus_base_sparse()) sparse_shift = 0;
	 if ((vram_map = mmap(0,(size_t) (VRAM_SIZE << sparse_shift),
						 PROT_READ | PROT_WRITE,
						 MAP_SHARED,
						 mem_fd, (VRAM_START << sparse_shift)
						 | _bus_base_sparse())) == (void *) -1)
#endif
      {
	perror("mmap error in map_hardware_ram");
	    close(mem_fd);
	    return (0);
	}
    vram_mapped = 1;
    close(mem_fd);
    return (1);
}

static int
copy_vbios(memType v_base)
{
    int mem_fd;
    unsigned char *tmp;
    int size;

    if ((mem_fd = open(MEM_FILE,O_RDONLY))<0) {
	perror("opening memory");
	return (0);
    }

    if (lseek(mem_fd,(off_t) v_base, SEEK_SET) != (off_t) v_base) {
	  fprintf(stderr,"Cannot lseek\n");
	  goto Error;
      }
    tmp = (unsigned char *)malloc(3);
    if (read(mem_fd, (char *)tmp, (size_t) 3) != (size_t) 3) {
	    fprintf(stderr,"Cannot read\n");
	goto Error;
    }
    if (lseek(mem_fd,(off_t) v_base,SEEK_SET) != (off_t) v_base)
	goto Error;

    if (*tmp != 0x55 || *(tmp+1) != 0xAA ) {
	fprintf(stderr,"No bios found at: 0x%lx\n",v_base);
	goto Error;
    }
#ifdef DEBUG
	dprint((unsigned long)tmp,0x100);
#endif
    size = *(tmp+2) * 512;

    if (read(mem_fd, (char *)v_base, (size_t) size) != (size_t) size) {
	    fprintf(stderr,"Cannot read\n");
	goto Error;
    }
    free(tmp);
    close(mem_fd);
    if (!chksum((CARD8*)v_base))
	return (0);

    return (1);

Error:
    perror("v_bios");
    close(mem_fd);
    return (0);
}

static void
unmap(void)
{
    munmap(0,SIZE);
}

static void
unmap_vram(void)
{
    if (!vram_mapped) return;

    munmap((void*)VRAM_START,VRAM_SIZE);
    vram_mapped = 0;
}

static void
runBIOS(int argc, char ** argv)
{
    i86biosRegs bRegs;
#ifdef V86BIOS_DEBUG
    printf("starting BIOS\n");
#endif
    setup_bios_regs(&bRegs, argc, argv);
    do_x86(BIOS_START,&bRegs);
    print_regs(&bRegs);
#ifdef V86BIOS_DEBUG
    printf("done\n");
#endif
}

static CARD32
setup_int_vect(void)
{
    int mem_fd;
    CARD32 vbase;
    void *map;

    if ((mem_fd = open(MEM_FILE,O_RDONLY))<0) {
    perror("opening memory");
    return (0);
    }

    if ((map = mmap((void *) 0, (size_t) 0x2000,
	 PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED,
	 mem_fd, 0)) == (void *)-1)   {
    perror("mmap error in map_hardware_ram");
    close(mem_fd);
    return (0);
    }

    close(mem_fd);
    memcpy(0,map,BIOS_MEM);
    munmap(map,0x2000);
    /*
     * create a backup copy of the bios variables to write back the
     * modified values
     */
    bios_var = (char *)malloc(BIOS_MEM);
    memcpy(bios_var,0,BIOS_MEM);

    vbase = (*((CARD16*)(0x10 << 2) + 1)) << 4;
    fprintf(stderr,"vbase: 0x%x\n",vbase);
    return vbase;
}

static void
update_bios_vars(void)
{
    int mem_fd;
    void *map;
    memType i;

#ifdef __ia64__
    if ((mem_fd = open(MEM_FILE,O_RDWR | O_SYNC))<0)
#else
    if ((mem_fd = open(MEM_FILE,O_RDWR))<0)
#endif
      {
	perror("opening memory");
	return;
      }

    if ((map = mmap((void *) 0, (size_t) 0x2000,
	 PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED,
	 mem_fd, 0)) == (void *)-1)   {
    perror("mmap error in map_hardware_ram");
    close(mem_fd);
    return;
    }

    for (i = 0; i < BIOS_MEM; i++) {
    if (bios_var[i] != *(CARD8*)i)
	*((CARD8*)map + i) = *(CARD8*)i;
    }

    munmap(map,0x2000);
    close(mem_fd);
}


static void
setup_bios_regs(i86biosRegsPtr regs, int argc, char **argv)
{
    int c;

    regs->ax = 0;
    regs->bx = 0;
    regs->cx = 0;
    regs->dx = 0;
    regs->es = 0;
    regs->di = 0;
    opterr = 0;
    while ((c = getopt(argc,argv,"a:b:c:d:e:i:")) != EOF) {
    switch (c) {
    case 'a':
	regs->ax = strtol(optarg,NULL,0);
	break;
    case 'b':
	regs->bx = strtol(optarg,NULL,0);
	break;
    case 'c':
	regs->cx = strtol(optarg,NULL,0);
	break;
    case 'd':
	regs->dx = strtol(optarg,NULL,0);
	break;
    case 'e':
	regs->es = strtol(optarg,NULL,0);
	break;
    case 'i':
	regs->di = strtol(optarg,NULL,0);
	break;
    }
    }
}


static int
chksum(CARD8 *start)
{
  CARD16 size;
  CARD8 val = 0;
  int i;

  size = *(start+2) * 512;
  for (i = 0; i<size; i++)
    val += *(start + i);

  if (!val)
    return 1;

    fprintf(stderr,"BIOS cksum wrong!\n");
  return 0;
}

static void
print_regs(i86biosRegsPtr regs)
{
    printf("ax=%x bx=%x cx=%x dx=%x es=%x di=%x\n",(CARD16)regs->ax,
       (CARD16)regs->bx,(CARD16)regs->cx,(CARD16)regs->dx,
       (CARD16)regs->es,(CARD16)regs->di);
}

void
loadCodeToMem(unsigned char *ptr, CARD8 code[])
{
    int i;
    CARD8 val;

    for ( i=0;;i++) {
	val = code[i];
	*ptr++ = val;
	if (val == 0xf4) break;
    }
    return;
}

void
dprint(unsigned long start, unsigned long size)
{
    int i,j;
    char *c = (char *)start;

    for (j = 0; j < (size >> 4); j++) {
	printf ("\n0x%lx:  ",(unsigned long)c);
	for (i = 0; i<16; i++)
	    printf("%x ",(unsigned char) (*(c++)));
    }
    printf("\n");
}
