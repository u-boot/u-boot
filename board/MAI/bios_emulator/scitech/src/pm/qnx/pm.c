/****************************************************************************
*
*                   SciTech OS Portability Manager Library
*
*  ========================================================================
*
*    The contents of this file are subject to the SciTech MGL Public
*    License Version 1.0 (the "License"); you may not use this file
*    except in compliance with the License. You may obtain a copy of
*    the License at http://www.scitechsoft.com/mgl-license.txt
*
*    Software distributed under the License is distributed on an
*    "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
*    implied. See the License for the specific language governing
*    rights and limitations under the License.
*
*    The Original Code is Copyright (C) 1991-1998 SciTech Software, Inc.
*
*    The Initial Developer of the Original Code is SciTech Software, Inc.
*    All Rights Reserved.
*
*  ========================================================================
*
* Language:     ANSI C
* Environment:  QNX
*
* Description:  Implementation for the OS Portability Manager Library, which
*               contains functions to implement OS specific services in a
*               generic, cross platform API. Porting the OS Portability
*               Manager library is the first step to porting any SciTech
*               products to a new platform.
*
****************************************************************************/

#include "pmapi.h"
#include "drvlib/os/os.h"
#include "mtrr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/mman.h>
#include "qnx/vbios.h"
#ifndef __QNXNTO__
#include <sys/seginfo.h>
#include <sys/console.h>
#include <conio.h>
#include <i86.h>
#else
#include <sys/neutrino.h>
#include <sys/dcmd_chr.h>
#endif

/*--------------------------- Global variables ----------------------------*/

static uint VESABuf_len = 1024;     /* Length of the VESABuf buffer     */
static void *VESABuf_ptr = NULL;    /* Near pointer to VESABuf          */
static uint VESABuf_rseg;           /* Real mode segment of VESABuf     */
static uint VESABuf_roff;           /* Real mode offset of VESABuf      */
static VBIOSregs_t  *VRegs = NULL;  /* Pointer to VBIOS registers       */
static int raw_count = 0;
static struct _console_ctrl *cc = NULL;
static int console_count = 0;
static int rmbuf_inuse = 0;

static void (PMAPIP fatalErrorCleanup)(void) = NULL;

/*----------------------------- Implementation ----------------------------*/

void PMAPI PM_init(void)
{
    char *force;

    if (VRegs == NULL) {
#ifdef  __QNXNTO__
	ThreadCtl(_NTO_TCTL_IO, 0); /* Get IO privilidge */
#endif
	force = getenv("VBIOS_METHOD");
	VRegs = VBIOSinit(force ? atoi(force) : 0);
	}
#ifndef  __QNXNTO__
    MTRR_init();
#endif
}

ibool PMAPI PM_haveBIOSAccess(void)
{ return VRegs != NULL; }

long PMAPI PM_getOSType(void)
{ return _OS_QNX; }

int PMAPI PM_getModeType(void)
{ return PM_386; }

void PMAPI PM_backslash(char *s)
{
    uint pos = strlen(s);
    if (s[pos-1] != '/') {
	s[pos] = '/';
	s[pos+1] = '\0';
	}
}

void PMAPI PM_setFatalErrorCleanup(
    void (PMAPIP cleanup)(void))
{
    fatalErrorCleanup = cleanup;
}

void PMAPI PM_fatalError(const char *msg)
{
    if (fatalErrorCleanup)
	fatalErrorCleanup();
    fprintf(stderr,"%s\n", msg);
    exit(1);
}

static void ExitVBEBuf(void)
{
    if (VESABuf_ptr)
	PM_freeRealSeg(VESABuf_ptr);
    VESABuf_ptr = 0;
}

void * PMAPI PM_getVESABuf(uint *len,uint *rseg,uint *roff)
{
    if (!VESABuf_ptr) {
	/* Allocate a global buffer for communicating with the VESA VBE */
	if ((VESABuf_ptr = PM_allocRealSeg(VESABuf_len, &VESABuf_rseg, &VESABuf_roff)) == NULL)
	    return NULL;
	atexit(ExitVBEBuf);
	}
    *len = VESABuf_len;
    *rseg = VESABuf_rseg;
    *roff = VESABuf_roff;
    return VESABuf_ptr;
}

static int term_raw(void)
{
    struct termios  termios_p;

    if (raw_count++ > 0)
	return 0;

    /* Go into "raw" input mode */
    if (tcgetattr(STDIN_FILENO, &termios_p))
	return -1;

    termios_p.c_cc[VMIN] =  1;
    termios_p.c_cc[VTIME] =  0;
    termios_p.c_lflag &= ~( ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &termios_p);
    return 0;
}

static void term_restore(void)
{
    struct termios  termios_p;

    if (raw_count-- != 1)
	return;

    tcgetattr(STDIN_FILENO, &termios_p);
    termios_p.c_lflag |= (ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL);
    termios_p.c_oflag |= (OPOST);
    tcsetattr(STDIN_FILENO, TCSADRAIN, &termios_p);
}

int PMAPI PM_kbhit(void)
{
    int blocking, c;

    if (term_raw() == -1)
	return 0;

    /* Go into non blocking mode */
    blocking = fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK;
    fcntl(STDIN_FILENO, F_SETFL, blocking);
    c = getc(stdin);

    /* restore blocking mode */
    fcntl(STDIN_FILENO, F_SETFL, blocking & ~O_NONBLOCK);
    term_restore();
    if (c != EOF) {
	ungetc(c, stdin);
	return c;
	}
    clearerr(stdin);
    return 0;
}

int PMAPI PM_getch(void)
{
    int c;

    if (term_raw() == -1)
	return (0);
    c = getc(stdin);
#if defined(__QNX__) && !defined(__QNXNTO__)
    if (c == 0xA)
	c = 0x0D;
    else if (c == 0x7F)
	c = 0x08;
#endif
    term_restore();
    return c;
}

PM_HWND PMAPI PM_openConsole(
    PM_HWND hwndUser,
    int device,
    int xRes,
    int yRes,
    int bpp,
    ibool fullScreen)
{
#ifndef __QNXNTO__
    int fd;

    if (console_count++)
	return 0;
    if ((fd = open("/dev/con1", O_RDWR)) == -1)
	return -1;
    cc = console_open(fd, O_RDWR);
    close(fd);
    if (cc == NULL)
	return -1;
#endif
    return 1;
}

int PMAPI PM_getConsoleStateSize(void)
{
    return PM_getVGAStateSize() + sizeof(int) * 3;
}

void PMAPI PM_saveConsoleState(void *stateBuf,int console_id)
{
#ifdef __QNXNTO__
    int     fd;
    int     flags;

    if ((fd = open("/dev/con1", O_RDWR)) == -1)
	return;
    flags = _CONCTL_INVISIBLE_CHG | _CONCTL_INVISIBLE;
    devctl(fd, DCMD_CHR_SERCTL, &flags, sizeof flags, 0);
    close(fd);
#else
    uchar   *buf = &((uchar*)stateBuf)[PM_getVGAStateSize()];

    /* Save QNX 4 console state */
    console_read(cc, -1, 0, NULL, 0,
	(int *)buf+1, (int *)buf+2, NULL);
    *(int *)buf = console_ctrl(cc, -1,
	CONSOLE_NORESIZE | CONSOLE_NOSWITCH | CONSOLE_INVISIBLE,
	CONSOLE_NORESIZE | CONSOLE_NOSWITCH | CONSOLE_INVISIBLE);

    /* Save state of VGA registers */
    PM_saveVGAState(stateBuf);
#endif
}

void PMAPI PM_setSuspendAppCallback(int (_ASMAPIP saveState)(int flags))
{
    /* TODO: Implement support for console switching if possible */
}

void PMAPI PM_restoreConsoleState(const void *stateBuf,PM_HWND hwndConsole)
{
#ifdef __QNXNTO__
    int     fd;
    int     flags;

    if ((fd = open("/dev/con1", O_RDWR)) == -1)
	return;
    flags = _CONCTL_INVISIBLE_CHG;
    devctl(fd, DCMD_CHR_SERCTL, &flags, sizeof flags, 0);
    close(fd);
#else
    uchar   *buf = &((uchar*)stateBuf)[PM_getVGAStateSize()];

    /* Restore the state of the VGA compatible registers */
    PM_restoreVGAState(stateBuf);

    /* Restore QNX 4 console state */
    console_ctrl(cc, -1, *(int *)buf,
	CONSOLE_NORESIZE | CONSOLE_NOSWITCH | CONSOLE_INVISIBLE);
    console_write(cc, -1, 0, NULL, 0,
	(int *)buf+1, (int *)buf+2, NULL);
#endif
}

void PMAPI PM_closeConsole(PM_HWND hwndConsole)
{
#ifndef __QNXNTO__
    if (--console_count == 0) {
	console_close(cc);
	cc = NULL;
	}
#endif
}

void PM_setOSCursorLocation(int x,int y)
{
    if (!cc)
	return;
#ifndef __QNXNTO__
    console_write(cc, -1, 0, NULL, 0, &y, &x, NULL);
#endif
}

void PM_setOSScreenWidth(int width,int height)
{
}

ibool PMAPI PM_setRealTimeClockHandler(PM_intHandler ih, int frequency)
{
    /* TODO: Implement this for QNX */
    return false;
}

void PMAPI PM_setRealTimeClockFrequency(int frequency)
{
    /* TODO: Implement this for QNX */
}

void PMAPI PM_restoreRealTimeClockHandler(void)
{
    /* TODO: Implement this for QNX */
}

char * PMAPI PM_getCurrentPath(
    char *path,
    int maxLen)
{
    return getcwd(path,maxLen);
}

char PMAPI PM_getBootDrive(void)
{ return '/'; }

const char * PMAPI PM_getVBEAFPath(void)
{ return PM_getNucleusConfigPath(); }

const char * PMAPI PM_getNucleusPath(void)
{
    char *env = getenv("NUCLEUS_PATH");
#ifdef __QNXNTO__
#ifdef __X86__
    return env ? env : "/nto/scitech/x86/bin";
#elif defined (__PPC__)
    return env ? env : "/nto/scitech/ppcbe/bin";
#elif defined (__MIPS__)
#ifdef __BIGENDIAN__
    return env ? env : "/nto/scitech/mipsbe/bin";
#else
    return env ? env : "/nto/scitech/mipsle/bin";
#endif
#elif defined (__SH__)
#ifdef __BIGENDIAN__
    return env ? env : "/nto/scitech/shbe/bin";
#else
    return env ? env : "/nto/scitech/shle/bin";
#endif
#elif defined (__ARM__)
    return env ? env : "/nto/scitech/armle/bin";
#endif
#else   /* QNX 4 */
    return env ? env : "/qnx4/scitech/bin";
#endif
}

const char * PMAPI PM_getNucleusConfigPath(void)
{
    static char path[512];
    char        *env;
#ifdef __QNXNTO__
    char temp[64];
    gethostname(temp, sizeof (temp));
    temp[sizeof (temp) - 1] = '\0';     /* Paranoid */
    sprintf(path,"/etc/config/scitech/%s/config", temp);
#else
    sprintf(path,"/etc/config/scitech/%d/config", getnid());
#endif
    if ((env = getenv("NUCLEUS_PATH")) != NULL) {
	strcpy(path,env);
	PM_backslash(path);
	strcat(path,"config");
	}
    return path;
}

const char * PMAPI PM_getUniqueID(void)
{
    static char buf[128];
#ifdef __QNXNTO__
    gethostname(buf, sizeof (buf));
#else
    sprintf(buf,"node%d", getnid());
#endif
    return buf;
}

const char * PMAPI PM_getMachineName(void)
{
    static char buf[128];
#ifdef __QNXNTO__
    gethostname(buf, sizeof (buf));
#else
    sprintf(buf,"node%d", getnid());
#endif
    return buf;
}

void * PMAPI PM_getBIOSPointer(void)
{
    return PM_mapRealPointer(0, 0x400);
}

void * PMAPI PM_getA0000Pointer(void)
{
    static void *ptr = NULL;
    void *freeptr;
    unsigned offset, i, maplen;

    if (ptr != NULL)
	return ptr;

    /* Some trickery is required to get the linear address 64K aligned */
    for (i = 0; i < 5; i++) {
	ptr = PM_mapPhysicalAddr(0xA0000,0xFFFF,true);
	offset = 0x10000 - ((unsigned)ptr % 0x10000);
	if (!offset)
	    break;
	munmap(ptr, 0x10000);
	maplen = 0x10000 + offset;
	freeptr = PM_mapPhysicalAddr(0xA0000-offset, maplen-1,true);
	ptr = (void *)(offset + (unsigned)freeptr);
	if (0x10000 - ((unsigned)ptr % 0x10000))
	    break;
	munmap(freeptr, maplen);
	}
    if (i == 5) {
	printf("Could not get a 64K aligned linear address for A0000 region\n");
	exit(1);
	}
    return ptr;
}

void * PMAPI PM_mapPhysicalAddr(ulong base,ulong limit,ibool isCached)
{
    uchar_t *p;
    unsigned o;
    unsigned prot = PROT_READ|PROT_WRITE|(isCached?0:PROT_NOCACHE);
#ifdef __PAGESIZE
    int pagesize = __PAGESIZE;
#else
    int pagesize = 4096;
#endif
    int rounddown = base % pagesize;
#ifndef __QNXNTO__
    static int __VidFD = -1;
#endif

    if (rounddown) {
	if (base < rounddown)
	    return NULL;
	base -= rounddown;
	limit += rounddown;
	}

#ifndef __QNXNTO__
    if (__VidFD < 0) {
	if ((__VidFD = shm_open( "Physical", O_RDWR, 0777 )) == -1) {
	    perror( "Cannot open Physical memory" );
	    exit(1);
	    }
	}
    o = base & 0xFFF;
    limit = (limit + o + 0xFFF) & ~0xFFF;
    if ((int)(p = mmap( 0, limit, prot, MAP_SHARED,
	    __VidFD, base )) == -1 ) {
	return NULL;
	}
    p += o;
#else
    if ((p = mmap(0, limit, prot, MAP_PHYS | MAP_SHARED,
	    NOFD, base)) == MAP_FAILED) {
	return (void *)-1;
	}
#endif
    return (p + rounddown);
}

void PMAPI PM_freePhysicalAddr(void *ptr,ulong limit)
{
    munmap(ptr,limit+1);
}

ulong PMAPI PM_getPhysicalAddr(void *p)
{
    /* TODO: This function should find the physical address of a linear */
    /*       address. */
    return 0xFFFFFFFFUL;
}

ibool PMAPI PM_getPhysicalAddrRange(
    void *p,
    ulong length,
    ulong *physAddress)
{
    /* TODO: Implement this! */
    return false;
}

void PMAPI PM_sleep(ulong milliseconds)
{
    /* TODO: Put the process to sleep for milliseconds */
}

int PMAPI PM_getCOMPort(int port)
{
    /* TODO: Re-code this to determine real values using the Plug and Play */
    /*       manager for the OS. */
    switch (port) {
	case 0: return 0x3F8;
	case 1: return 0x2F8;
	}
    return 0;
}

int PMAPI PM_getLPTPort(int port)
{
    /* TODO: Re-code this to determine real values using the Plug and Play */
    /*       manager for the OS. */
    switch (port) {
	case 0: return 0x3BC;
	case 1: return 0x378;
	case 2: return 0x278;
	}
    return 0;
}

void * PMAPI PM_mallocShared(long size)
{
    return PM_malloc(size);
}

void PMAPI PM_freeShared(void *ptr)
{
    PM_free(ptr);
}

void * PMAPI PM_mapToProcess(void *base,ulong limit)
{ return (void*)base; }

void * PMAPI PM_mapRealPointer(uint r_seg,uint r_off)
{
    void *p;

    PM_init();

    if ((p = VBIOSgetmemptr(r_seg, r_off, VRegs)) == (void *)-1)
	return NULL;
    return p;
}

void * PMAPI PM_allocRealSeg(uint size,uint *r_seg,uint *r_off)
{
    if (size > 1024) {
	printf("PM_allocRealSeg: can't handle %d bytes\n", size);
	return 0;
	}
    if (rmbuf_inuse != 0) {
	printf("PM_allocRealSeg: transfer area already in use\n");
	return 0;
	}
    PM_init();
    rmbuf_inuse = 1;
    *r_seg = VBIOS_TransBufVSeg(VRegs);
    *r_off = VBIOS_TransBufVOff(VRegs);
    return (void*)VBIOS_TransBufPtr(VRegs);
}

void PMAPI PM_freeRealSeg(void *mem)
{
    if (rmbuf_inuse == 0) {
	printf("PM_freeRealSeg: nothing was allocated\n");
	return;
	}
    rmbuf_inuse = 0;
}

void PMAPI DPMI_int86(int intno, DPMI_regs *regs)
{
    PM_init();
    if (VRegs == NULL)
	return;

    VRegs->l.eax = regs->eax;
    VRegs->l.ebx = regs->ebx;
    VRegs->l.ecx = regs->ecx;
    VRegs->l.edx = regs->edx;
    VRegs->l.esi = regs->esi;
    VRegs->l.edi = regs->edi;

    VBIOSint(intno, VRegs, 1024);

    regs->eax = VRegs->l.eax;
    regs->ebx = VRegs->l.ebx;
    regs->ecx = VRegs->l.ecx;
    regs->edx = VRegs->l.edx;
    regs->esi = VRegs->l.esi;
    regs->edi = VRegs->l.edi;
    regs->flags = VRegs->w.flags & 0x1;
}

int PMAPI PM_int86(int intno, RMREGS *in, RMREGS *out)
{
    PM_init();
    if (VRegs == NULL)
	return 0;

    VRegs->l.eax = in->e.eax;
    VRegs->l.ebx = in->e.ebx;
    VRegs->l.ecx = in->e.ecx;
    VRegs->l.edx = in->e.edx;
    VRegs->l.esi = in->e.esi;
    VRegs->l.edi = in->e.edi;

    VBIOSint(intno, VRegs, 1024);

    out->e.eax = VRegs->l.eax;
    out->e.ebx = VRegs->l.ebx;
    out->e.ecx = VRegs->l.ecx;
    out->e.edx = VRegs->l.edx;
    out->e.esi = VRegs->l.esi;
    out->e.edi = VRegs->l.edi;
    out->x.cflag = VRegs->w.flags & 0x1;

    return out->x.ax;
}

int PMAPI PM_int86x(int intno, RMREGS *in, RMREGS *out,
    RMSREGS *sregs)
{
    PM_init();
    if (VRegs == NULL)
	return 0;

    if (intno == 0x21) {
	time_t today = time(NULL);
	struct tm *t;
	t = localtime(&today);
	out->x.cx = t->tm_year + 1900;
	out->h.dh = t->tm_mon + 1;
	out->h.dl = t->tm_mday;
	return 0;
	}
    else {
	VRegs->l.eax = in->e.eax;
	VRegs->l.ebx = in->e.ebx;
	VRegs->l.ecx = in->e.ecx;
	VRegs->l.edx = in->e.edx;
	VRegs->l.esi = in->e.esi;
	VRegs->l.edi = in->e.edi;
	VRegs->w.es = sregs->es;
	VRegs->w.ds = sregs->ds;

	VBIOSint(intno, VRegs, 1024);

	out->e.eax = VRegs->l.eax;
	out->e.ebx = VRegs->l.ebx;
	out->e.ecx = VRegs->l.ecx;
	out->e.edx = VRegs->l.edx;
	out->e.esi = VRegs->l.esi;
	out->e.edi = VRegs->l.edi;
	out->x.cflag = VRegs->w.flags & 0x1;
	sregs->es = VRegs->w.es;
	sregs->ds = VRegs->w.ds;

	return out->x.ax;
	}
}

void PMAPI PM_callRealMode(uint seg,uint off, RMREGS *in,
    RMSREGS *sregs)
{
    PM_init();
    if (VRegs == NULL)
	return;

    VRegs->l.eax = in->e.eax;
    VRegs->l.ebx = in->e.ebx;
    VRegs->l.ecx = in->e.ecx;
    VRegs->l.edx = in->e.edx;
    VRegs->l.esi = in->e.esi;
    VRegs->l.edi = in->e.edi;
    VRegs->w.es = sregs->es;
    VRegs->w.ds = sregs->ds;

    VBIOScall(seg, off, VRegs, 1024);

    in->e.eax = VRegs->l.eax;
    in->e.ebx = VRegs->l.ebx;
    in->e.ecx = VRegs->l.ecx;
    in->e.edx = VRegs->l.edx;
    in->e.esi = VRegs->l.esi;
    in->e.edi = VRegs->l.edi;
    in->x.cflag = VRegs->w.flags & 0x1;
    sregs->es = VRegs->w.es;
    sregs->ds = VRegs->w.ds;
}

void PMAPI PM_availableMemory(ulong *physical,ulong *total)
{
#ifndef __QNXNTO__
    *physical = *total = _memavl();
#endif
}

void * PMAPI PM_allocLockedMem(
    uint size,
    ulong *physAddr,
    ibool contiguous,
    ibool below16M)
{
    /* TODO: Implement this on QNX */
    return NULL;
}

void PMAPI PM_freeLockedMem(
    void *p,
    uint size,
    ibool contiguous)
{
    /* TODO: Implement this on QNX */
}

void * PMAPI PM_allocPage(
    ibool locked)
{
    /* TODO: Implement this on QNX */
    return NULL;
}

void PMAPI PM_freePage(
    void *p)
{
    /* TODO: Implement this on QNX */
}

void PMAPI PM_setBankA(int bank)
{
    PM_init();
    if (VRegs == NULL)
	return;

    VRegs->l.eax = 0x4F05;
    VRegs->l.ebx = 0x0000;
    VRegs->l.edx = bank;
    VBIOSint(0x10, VRegs, 1024);
}

void PMAPI PM_setBankAB(int bank)
{
    PM_init();
    if (VRegs == NULL)
	return;

    VRegs->l.eax = 0x4F05;
    VRegs->l.ebx = 0x0000;
    VRegs->l.edx = bank;
    VBIOSint(0x10, VRegs, 1024);

    VRegs->l.eax = 0x4F05;
    VRegs->l.ebx = 0x0001;
    VRegs->l.edx = bank;
    VBIOSint(0x10, VRegs, 1024);
}

void PMAPI PM_setCRTStart(int x,int y,int waitVRT)
{
    PM_init();
    if (VRegs == NULL)
	return;

    VRegs->l.eax = 0x4F07;
    VRegs->l.ebx = waitVRT;
    VRegs->l.ecx = x;
    VRegs->l.edx = y;
    VBIOSint(0x10, VRegs, 1024);
}

ibool PMAPI PM_doBIOSPOST(
    ushort axVal,
    ulong BIOSPhysAddr,
    void *copyOfBIOS,
    ulong BIOSLen)
{
    (void)axVal;
    (void)BIOSPhysAddr;
    (void)copyOfBIOS;
    (void)BIOSLen;
    return false;
}

int PMAPI PM_lockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    p = p;  len = len;
    return 1;
}

int PMAPI PM_unlockDataPages(void *p,uint len,PM_lockHandle *lh)
{
    p = p;  len = len;
    return 1;
}

int PMAPI PM_lockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    p = p;  len = len;
    return 1;
}

int PMAPI PM_unlockCodePages(void (*p)(),uint len,PM_lockHandle *lh)
{
    p = p;  len = len;
    return 1;
}

PM_MODULE PMAPI PM_loadLibrary(
    const char *szDLLName)
{
    /* TODO: Implement this to load shared libraries! */
    (void)szDLLName;
    return NULL;
}

void * PMAPI PM_getProcAddress(
    PM_MODULE hModule,
    const char *szProcName)
{
    /* TODO: Implement this! */
    (void)hModule;
    (void)szProcName;
    return NULL;
}

void PMAPI PM_freeLibrary(
    PM_MODULE hModule)
{
    /* TODO: Implement this! */
    (void)hModule;
}

int PMAPI PM_setIOPL(
    int level)
{
    /* QNX handles IOPL selection at the program link level. */
    return level;
}

/****************************************************************************
PARAMETERS:
base    - The starting physical base address of the region
size    - The size in bytes of the region
type    - Type to place into the MTRR register

RETURNS:
Error code describing the result.

REMARKS:
Function to enable write combining for the specified region of memory.
****************************************************************************/
int PMAPI PM_enableWriteCombine(
    ulong base,
    ulong size,
    uint type)
{
#ifndef  __QNXNTO__
    return MTRR_enableWriteCombine(base,size,type);
#else
    return PM_MTRR_NOT_SUPPORTED;
#endif
}
