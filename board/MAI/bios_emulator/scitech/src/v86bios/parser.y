%{
#include <malloc.h>
#include <string.h>
#include "v86bios.h"
#include "pci.h"
    
#define YYSTYPE unsigned long
    
#define MAX_VAR 0x20

    CARD32 var[MAX_VAR];
    CARD32 var_mem;


i86biosRegs regs = { 00 };

enum mem_type { BYTE, WORD, LONG, STRING };
union mem_val {
   CARD32 integer;
   char *ptr;
} rec;

struct mem {
    enum mem_type type;
        union mem_val val;
    struct mem *next;
}; 

 
struct device Device = {FALSE,NONE,{0}};

extern void yyerror(char *s);
extern int yylex( void  );
 
static void boot(void);
static void dump_mem(CARD32 addr, int len);
static void exec_int(int num);
static void *add_to_list(enum mem_type type, union mem_val *rec, void *next);
static void do_list(struct mem *list, memType addr);
static char * normalize_string(char *ptr);
%}

%token TOK_NUM
%token TOK_REG_AX
%token TOK_REG_BX
%token TOK_REG_CX
%token TOK_REG_DX
%token TOK_REG_DI
%token TOK_REG_SI
%token TOK_SEG_DS
%token TOK_SEG_ES
%token TOK_SEP
%token TOK_VAR
%token TOK_VAR_MEM
%token TOK_COMMAND_BOOT
%token TOK_COMMAND_EXEC
%token TOK_SELECT
%token TOK_STRING
%token TOK_MODIFIER_BYTE
%token TOK_MODIFIER_WORD
%token TOK_MODIFIER_LONG
%token TOK_MODIFIER_MEMSET
%token TOK_COMMAND_MEMSET
%token TOK_COMMAND_MEMDUMP
%token TOK_COMMAND_QUIT
%token TOK_ERROR
%token TOK_END
%token TOK_ISA
%token TOK_PCI
%token TOK_BYTE
%token TOK_WORD
%token TOK_LONG
%token TOK_PRINT_PORT
%token TOK_IOSTAT
%token TOK_PRINT_IRQ
%token TOK_PPCI
%token TOK_PIP
%token TOK_TRACE
%token TOK_ON
%token TOK_OFF
%token TOK_VERBOSE
%token TOK_LOG
%token TOK_LOGOFF
%token TOK_CLSTAT
%token TOK_STDOUT
%token TOK_HLT
%token TOK_DEL
%token TOK_IOPERM
%token TOK_DUMP_PCI
%token TOK_BOOT_BIOS
%%
input:        | input line  
line:          end |  com_reg | com_var | com_select
              | com_boot | com_memset | com_memdump  | com_quit 
              | com_exec | hlp | config | verbose | logging | print | clstat 
              | com_hlt | ioperm | list_pci | boot_bios 
              | error end  { printf("unknown command\n"); }
;
end:           TOK_END
;
com_reg:        reg_off val end { *(CARD16*)$1 = $2 & 0xffff; }
              | reg_seg TOK_SEP reg_off val end {
                               *(CARD16*)$1 = ($4 & 0xf0000) >> 4;
                                       *(CARD16*)$3 = ($4 & 0x0ffff);
                                        }
              |  reg_off '?' end { printf("0x%x\n",*(CARD16*)$1);}
              |  reg_seg TOK_SEP reg_off '?' end
                                     { printf("0x%x:0x%x\n",*(CARD16*)$1,
                          *(CARD16*)$3); }
;
register_read:  reg_seg TOK_SEP reg_off { $$ = (((*(CARD16*)$1) << 4) 
                                                | ((*(CARD16*)$3) & 0xffff));
                                        }
              | reg_off          { $$ = ((*(CARD16*)$1) & 0xffff); }
;
reg_off:        TOK_REG_AX  { $$ = (unsigned long)&(regs.ax); }
              | TOK_REG_BX  { $$ = (unsigned long)&(regs.bx); }
              | TOK_REG_CX  { $$ = (unsigned long)&(regs.cx); }
              | TOK_REG_DX  { $$ = (unsigned long)&(regs.dx); }
              | TOK_REG_DI  { $$ = (unsigned long)&(regs.di); }
              | TOK_REG_SI  { $$ = (unsigned long)&(regs.si); }
;
reg_seg:        TOK_SEG_DS  { $$ = (unsigned long)&(regs.ds); }
              | TOK_SEG_ES  { $$ = (unsigned long)&(regs.es); }
;
com_var:        TOK_VAR_MEM '?' end { printf("var mem: 0x%x\n",var_mem); }
          | TOK_VAR '?' end { if ($1 < MAX_VAR)
          printf("var[%i]: 0x%x\n",(int)$1,var[$1]);
          else
          printf("var index %i out of range\n",(int)$1); }
          | TOK_VAR_MEM val end { var_mem = $2; }    
              | TOK_VAR val end    { if ($1 <= MAX_VAR)
                                 var[$1] = $2;
                              else 
                                     printf("var index %i out of range\n",(int)$1); }
              | TOK_VAR error  end { printf("$i val\n"); }
              | TOK_VAR_MEM error  end { printf("$i val\n"); }
;
com_boot:       TOK_COMMAND_BOOT  end { boot(); }
                TOK_COMMAND_BOOT error end { boot(); }
;
com_select:     TOK_SELECT TOK_ISA end { Device.booted = FALSE;
                                         Device.type = ISA;
                     CurrentPci = NULL; }   
              | TOK_SELECT TOK_PCI val TOK_SEP val TOK_SEP val end
                                    { Device.booted = FALSE;
                      Device.type = PCI; 
                                      Device.loc.pci.bus = $3;
                                      Device.loc.pci.dev = $5; 
                                      Device.loc.pci.func = $7; }
              | TOK_SELECT '?' end
                                   { switch (Device.type) {
                                     case ISA:
                                       printf("isa\n");
                                       break;
                                     case PCI:
                                      printf("pci: %x:%x:%x\n",Device.loc.pci.bus,
                                              Device.loc.pci.dev,
                          Device.loc.pci.func);
                                      break;
                                 default:
                                      printf("no device selected\n");
                                      break;
                                     }
                                    }
              | TOK_SELECT error end { printf("select ? | isa "
                                         "| pci:bus:dev:func\n"); }
;
com_quit:       TOK_COMMAND_QUIT end { return 0; }
              | TOK_COMMAND_QUIT error end { logoff(); return 0; }
;
com_exec:       TOK_COMMAND_EXEC end { exec_int(0x10); }
          | TOK_COMMAND_EXEC val end { exec_int($2); }
          | TOK_COMMAND_EXEC error end { exec_int(0x10); }
;
com_memdump:    TOK_COMMAND_MEMDUMP val val end { dump_mem($2,$3); }
              | TOK_COMMAND_MEMDUMP  error  end { printf("memdump start len\n"); }

    
;
com_memset:     TOK_COMMAND_MEMSET val list end { do_list((struct mem*)$3,$2);}
              | TOK_COMMAND_MEMSET error end { printf("setmem addr [byte val] "
                                                   "[word val] [long val] "
                                                   "[\"string\"]\n"); }
;
list:                              { $$ = 0; } 
              | TOK_BYTE val list { rec.integer = $2;
          $$ = (unsigned long)add_to_list(BYTE,&rec,(void*)$3); }
              | TOK_WORD val list { rec.integer = $2; 
          $$ = (unsigned long) add_to_list(WORD,&rec,(void*)$3); }
              | TOK_LONG val list { rec.integer = $2; 
          $$ = (unsigned long) add_to_list(LONG,&rec,(void*)$3); }
              | TOK_STRING list { rec.ptr = (void*)$1; 
          $$ = (unsigned long) add_to_list(STRING,&rec,(void*)$2); }
;
val:            TOK_VAR  {  if ($1 > MAX_VAR) {
                             printf("variable index out of range\n");
                             $$=0;
                             } else 
                            $$ = var[$1]; } 
              | TOK_NUM  {  $$ = $1; }
              | register_read
; 
bool:           TOK_ON   {  $$ = 1; }
              | TOK_OFF  {  $$ = 0; }
;
config:         TOK_PRINT_PORT bool end { Config.PrintPort = $2; } 
              | TOK_PRINT_PORT '?' end  { printf("print port %s\n",
                                      Config.PrintPort?"on":"off"); }
          | TOK_PRINT_PORT error end { printf("pport on | off | ?\n") } 
              | TOK_PRINT_IRQ bool end  { Config.PrintIrq = $2; } 
              | TOK_PRINT_IRQ '?' end   { printf("print irq %s\n",  
                                          Config.PrintIrq?"on":"off"); } 
          | TOK_PRINT_IRQ error end { printf("pirq on | off | ?\n") } 
              | TOK_PPCI bool end       { Config.PrintPci = $2; } 
              | TOK_PPCI '?' end        { printf("print PCI %s\n",
                                          Config.PrintPci?"on":"off"); } 
          | TOK_PPCI error end      { printf("ppci on | off | ?\n") } 
              | TOK_PIP bool end        { Config.PrintIp = $2; } 
              | TOK_PIP '?' end         { printf("printip %s\n",
                                      Config.PrintIp?"on":"off"); } 
          | TOK_PIP error end       { printf("pip on | off | ?\n") } 
              | TOK_IOSTAT bool end     { Config.IoStatistics = $2; } 
              | TOK_IOSTAT '?' end      { printf("io statistics %s\n",
                                      Config.IoStatistics?"on":"off"); } 
          | TOK_IOSTAT error end    { printf("iostat on | off | ?\n") } 
              | TOK_TRACE bool end      { Config.Trace = $2; } 
              | TOK_TRACE '?' end       { printf("trace %s\n",
                                      Config.Trace ?"on":"off"); } 
          | TOK_TRACE error end { printf("trace on | off | ?\n") } 
;
verbose:        TOK_VERBOSE val end     { Config.Verbose = $2; }
          | TOK_VERBOSE '?' end     { printf("verbose: %i\n",
                            Config.Verbose); }
          | TOK_VERBOSE error end   { printf("verbose val | ?\n"); }
;
logging:        TOK_LOG TOK_STRING end  { logon(normalize_string((char*)$2)); }
          | TOK_LOG '?' end         { if (logging) printf("logfile: %s\n",
                            logfile);
                      else printf("no logging\n?"); }
          | TOK_LOG TOK_OFF end          { logoff(); }
          | TOK_LOG error end       { printf("log \"<filename>\" | ? |"
                         " off\n"); }
;
clstat:         TOK_CLSTAT end          { clear_stat(); }
          | TOK_CLSTAT error end    { printf("clstat\n"); } 
;
print:          TOK_STDOUT bool end     { nostdout = !$2; }
                  | TOK_STDOUT '?' end      { printf("print %s\n",nostdout ?
                        "no":"yes"); }
          | TOK_STDOUT error end    { printf("print on | off\n"); }
;
com_hlt:            TOK_HLT val end         { add_hlt($2); }    
          | TOK_HLT TOK_DEL val end { del_hlt($3); }
          | TOK_HLT TOK_DEL end     { del_hlt(21); }
              | TOK_HLT '?' end         { list_hlt(); }
          | TOK_HLT error end       { printf(
                                               "hlt val | del [val] | ?\n"); }
;
ioperm:         TOK_IOPERM val val val end { int i,max;
                                             if ($2 >= 0) {
                                 max = $2 + $3 - 1;
                             if (max > IOPERM_BITS) 
                             max = IOPERM_BITS;
                                 for (i = $2;i <= max; i++)
                                ioperm_list[i] 
                                                                = $4>0 ? 1 : 0;
                          }
                                           }
          | TOK_IOPERM '?' end { int i,start;
                     for (i=0; i <= IOPERM_BITS; i++) {
                    if (ioperm_list[i]) {
                       start = i;
                       for (; i <= IOPERM_BITS; i++) 
                        if (!ioperm_list[i]) {
                         printf("ioperm on in "
                         "0x%x+0x%x\n", start,i-start);
                         break;
                        }
                         }
                     }
                                  }
          | TOK_IOPERM error end { printf("ioperm start len val\n"); }
;
list_pci:      TOK_DUMP_PCI end       { list_pci(); }
             | TOK_DUMP_PCI error end { list_pci(); }
;
boot_bios:     TOK_BOOT_BIOS '?' end { if (!BootBios) printf("No Boot BIOS\n");
                                   else printf("BootBIOS from: %i:%i:%i\n",
                           BootBios->bus, BootBios->dev,
                           BootBios->func); }
             | TOK_BOOT_BIOS error end { printf ("bootbios bus:dev:num\n"); }
;
hlp:         '?'  { printf("Command list:\n");
                    printf(" select isa | pci bus:dev:func\n");
            printf(" boot\n");
            printf(" seg:reg val | reg val \n");
            printf(" $x val | $mem val\n");
            printf(" setmem addr list; addr := val\n");
            printf(" dumpmem addr len; addr,len := val\n");
            printf(" do [val]\n");
            printf(" quit\n");
            printf(" ?\n");
            printf(" seg := ds | es;"
               " reg := ax | bx | cx | dx | si \n");
            printf(" val := var | <hex-number> | seg:reg | seg\n");
            printf(" var := $x | $mem; x := 0..20\n");
            printf(" list := byte val | word val | long val "
               "| \"string\"\n");
                printf(" pport on | off | ?\n");
                printf(" ppci on | off | ?\n");
                printf(" pirq on | off | ?\n");
                printf(" pip on | off | ?\n");
                printf(" trace on | off | ?\n");
                printf(" iostat on | off | ?\n");
            printf(" verbose val\n");
            printf(" log \"<filename>\" | off | ?\n");
            printf(" print on | off\n");
            printf(" hlt val | del [val] | ?\n");
            printf(" clstat\n");
            printf(" lpci\n");
            printf ("bootbios ?\n");
}
;

%%

static void
dump_mem(CARD32 addr, int len)
{
    dprint(addr,len);
}

static void
exec_int(int num)
{
    if (num == 0x10) {  /* video interrupt */
    if (Device.type == NONE) {
        CurrentPci = PciList;
        while (CurrentPci) {
        if (CurrentPci->active)
            break;
        CurrentPci = CurrentPci->next;
        }
        if (!CurrentPci)
        Device.type = ISA;
        else {
        Device.type = PCI;
        Device.loc.pci.dev = CurrentPci->dev;
        Device.loc.pci.bus = CurrentPci->bus;
        Device.loc.pci.func = CurrentPci->func;
        }
    }
        if (Device.type != ISA) {
       if (!Device.booted) {
              if (!CurrentPci || (Device.type == PCI
               && (!CurrentPci->active
                   && (Device.loc.pci.dev != CurrentPci->dev
                 || Device.loc.pci.bus != CurrentPci->bus
                 || Device.loc.pci.func != CurrentPci->func)))) {
                printf("boot the device fist\n");
            return;
          }
           }
    } else
       CurrentPci = NULL;
    } else {
    Device.booted = FALSE; /* we need this for sanity! */
    }
    
    runINT(num,&regs);
}

static void
boot(void)
{
    if (Device.type == NONE) {
    printf("select a device fist\n");
    return;
    }
    
    call_boot(&Device);
}

static void *
add_to_list(enum mem_type type, union mem_val *rec, void *next)
{
    struct mem *mem_rec = (struct mem *) malloc(sizeof(mem_rec));

    mem_rec->type = type;
    mem_rec->next = next;
    
    switch (type) {
    case BYTE:
    case WORD:
    case LONG:
    mem_rec->val.integer = rec->integer;
       break;
    case STRING:
    mem_rec->val.ptr = normalize_string(rec->ptr);
    break;
    }
    return mem_rec;
}

static int
validRange(int addr,int len)
{
    int end = addr + len;

    if (addr < 0x1000 || end > 0xc0000)
    return 0;
    return 1;
}

static void
do_list(struct mem *list, memType addr)
{
   struct mem *prev;
   int len;
   
   while (list) {
    switch (list->type) {
         case BYTE:
         if (!validRange(addr,1)) goto error;
         *(CARD8*)addr = list->val.integer;
         addr =+ 1;
             break;     
         case WORD:
         if (!validRange(addr,2)) goto error;
         *(CARD16*)addr = list->val.integer;
         addr =+ 2;
             break;     
         case LONG:
         if (!validRange(addr,4)) goto error;
         *(CARD32*)addr = list->val.integer;
         addr =+ 4;
             break;
         case STRING:
         len = strlen((char*)list->val.ptr);
         if (!validRange(addr,len)) goto error;
         memcpy((CARD8*)addr,(void*)list->val.ptr,len);
         addr =+ len;
             free(list->val.ptr);
             break;
       }     
       prev = list;
       list = list->next;
       free(prev);
       continue;
   error:
       printf("address out of range\n");
       while (list) {
       prev = list;
       list = list->next;
       free(prev);
       }
       break;
   }
}

static char *
normalize_string(char *ptr)
{
    int i = 0, j = 0, c = 0, esc= 0;
    int size;
    char *mem_ptr;
    
    size = strlen(ptr);
    mem_ptr = malloc(size);
    while (1) {
        switch (*(ptr + i)) {
        case '\\':
            if (esc) {
                *(mem_ptr + j++) = *(ptr + i);
                esc = 0;
            } else 
                esc = 1;
            break;
        case '\"':
            if (esc) {
                *(mem_ptr + j++) = *(ptr + i);
                esc = 0;
            } else 
                c++;
            break;
        default:
            *(mem_ptr + j++) = *(ptr + i);
            break;
        }
        if (c > 1) {
            *(mem_ptr + j) = '\0';     
            break;
        }
        i++;
    }
    return mem_ptr;
}
