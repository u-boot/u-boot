%{
#include "parser.h"

#include <string.h>
#include <stdio.h>

 void getline(char *buf,int *num,int max_num);
    
#define YY_INPUT(buf,result,max_size) {\
      getline(buf,&result,max_size);\
      }

 void
 yyerror (char *s)  
 {
     printf ("%s\n", s);
 }

%}

DIGIT [0-9a-fA-F]

%%

"0x"?{DIGIT}+ { yylval = strtol(yytext,NULL,0); return TOK_NUM; }
"ax"          { return TOK_REG_AX; }
"bx"          { return TOK_REG_BX; }
"cx"          { return TOK_REG_CX; }
"dx"          { return TOK_REG_DX; }
"di"          { return TOK_REG_SI; }
"si"          { return TOK_REG_DI; }
"ds"          { return TOK_SEG_DS; }
"es"          { return TOK_SEG_ES; }
":"           { return TOK_SEP;}
"$"{DIGIT}{1,2} { yylval = strtol(yytext+1,NULL,0); return TOK_VAR; }
"$mem"        { return TOK_VAR_MEM; }
[ \t]+
"#".*[\n]   { return TOK_END; }
"boot"        { return TOK_COMMAND_BOOT; }
"do"          { return TOK_COMMAND_EXEC; }
"\"".*"\""    { yylval = (unsigned long) yytext; return TOK_STRING; }
"byte"        { return TOK_BYTE; }
"word"        { return TOK_WORD; }
"long"        { return TOK_LONG; }
"setmem"      { return TOK_COMMAND_MEMSET; }
"dumpmem"     { return TOK_COMMAND_MEMDUMP; }
"quit"        { return TOK_COMMAND_QUIT; }
"\n"          { return TOK_END; }
"select"      { return TOK_SELECT; }
"isa"         { return TOK_ISA; }
"pci"         { return TOK_PCI; }
"pport"       { return TOK_PRINT_PORT; }
"iostat"      { return TOK_IOSTAT; }
"pirq"        { return TOK_PRINT_IRQ; }
"ppci"        { return TOK_PPCI; }
"pip"         { return TOK_PIP; }
"trace"       { return TOK_TRACE; }
"on"          { return TOK_ON; }
"off"         { return TOK_OFF; }
"verbose"     { return TOK_VERBOSE; }
"log"         { return TOK_LOG; }
"print"       { return TOK_STDOUT; }
"clstat"      { return TOK_CLSTAT; }
"hlt"         { return TOK_HLT; }
"del"         { return TOK_DEL; }
"ioperm"      { return TOK_IOPERM; }
"lpci"        { return TOK_DUMP_PCI; }
"bootbios"    { return TOK_BOOT_BIOS; }
"?"           { return '?'; }
.             { return TOK_ERROR; }
    
%%







