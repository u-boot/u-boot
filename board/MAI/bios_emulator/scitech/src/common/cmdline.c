/****************************************************************************
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
* Environment:  any
*
* Description:  This module contains code to parse the command line,
*               extracting options and parameters in standard System V
*               style.
*
****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cmdline.h"

/*------------------------- Global variables ------------------------------*/

int     nextargv    =   1;          /* Index into argv array            */
char    *nextchar   =   NULL;       /* Pointer to next character        */

/*-------------------------- Implementation -------------------------------*/

#define IS_SWITCH_CHAR(c)       ((c) == '-')
#define IS_NOT_SWITCH_CHAR(c)   ((c) != '-')

/****************************************************************************
DESCRIPTION:
Parse the command line for specific options

HEADER:
cmdline.h

PARAMETERS:
argc        - Value passed to program through argc variable
argv        - Pointer to the argv array passed to the program
format      - A string representing the expected format of the command line
argument    - Pointer to optional argument on command line

RETURNS:
Character code representing the next option parsed from the command line by
getcmdopt. Returns ALLDONE (-1) when there are no more parameters to be parsed
on the command line, PARAMETER (-2) when the argument being parsed is a
parameter and not an option switch and lastly INVALID (-3) if an error
occured while parsing the command line.

REMARKS:
Function to parse the command line option switches in UNIX System V style.
When getcmdopt is called, it returns the character code of the next valid
option that is parsed from the command line as specified by the Format
string. The format string should be in the following form:

    "abcd:e:f:"

where a,b and c represent single switch style options and the character
code returned by getcmdopt is the only value returned. Also d, e and f
represent options that expect arguments immediately after them on the
command line. The argument that follows the option on the command line is
returned via a reference in the pointer argument. Thus a valid command line
for this format string might be:

    myprogram -adlines -b -f format infile outfile

where a and b will be returned as single character options with no argument,
while d is returned with the argument lines and f is returned with the
argument format.

When getcmdopt returns with PARAMETER (we attempted to parse a paramter, not
an option), the global variable NextArgv will hold an index in the argv
array to the argument on the command line AFTER the options, ie in the
above example the string 'infile'. If the parameter is successfully used,
NextArgv should be incremented and getcmdopt can be called again to parse any
more options. Thus you can also have options interspersed throught the
command line. eg:

    myprogram -adlines infile -b outfile -f format

can be made to be a valid form of the above command line.
****************************************************************************/
int getcmdopt(
    int argc,
    char **argv,
    char *format,
    char **argument)
{
    char    ch;
    char    *formatchar;

    if (argc > nextargv) {
	if (nextchar == NULL) {
	    nextchar = argv[nextargv];      /* Index next argument      */
	    if (nextchar == NULL) {
		nextargv++;
		return ALLDONE;             /* No more options          */
		}
	    if (IS_NOT_SWITCH_CHAR(*nextchar)) {
		nextchar = NULL;
		return PARAMETER;           /* We have a parameter      */
		}
	    nextchar++;                     /* Move past switch operator */
	    if (IS_SWITCH_CHAR(*nextchar)) {
		nextchar = NULL;
		return INVALID;             /* Ignore rest of line      */
		}
	    }
	if ((ch = *(nextchar++)) == 0) {
	    nextchar = NULL;
	    return INVALID;                 /* No options on line       */
	    }

	if (ch == ':' ||  (formatchar = strchr(format, ch)) == NULL)
	    return INVALID;

	if (*(++formatchar) == ':') {   /* Expect an argument after option */
	    nextargv++;
	    if (*nextchar == 0) {
		if (argc <= nextargv)
		    return INVALID;
		nextchar = argv[nextargv++];
		}
	    *argument = nextchar;
	    nextchar = NULL;
	    }
	else {                      /* We have a switch style option    */
	    if (*nextchar == 0) {
		nextargv++;
		nextchar = NULL;
		}
	    *argument = NULL;
	    }
	return ch;                  /* return the option specifier      */
	}
    nextchar = NULL;
    nextargv++;
    return ALLDONE;                 /* no arguments on command line     */
}

/****************************************************************************
PARAMETERS:
optarr      - Description for the option we are parsing
argument    - String to parse

RETURNS:
INVALID on error, ALLDONE on success.

REMARKS:
Parses the argument string depending on the type of argument that is
expected, filling in the argument for that option. Note that to parse a
string, we simply return a pointer to argument.
****************************************************************************/
static int parse_option(
    Option *optarr,
    char *argument)
{
    int     num_read;

    switch ((int)(optarr->type)) {
	case OPT_INTEGER:
	    num_read = sscanf(argument,"%d",(int*)optarr->arg);
	    break;
	case OPT_HEX:
	    num_read = sscanf(argument,"%x",(int*)optarr->arg);
	    break;
	case OPT_OCTAL:
	    num_read = sscanf(argument,"%o",(int*)optarr->arg);
	    break;
	case OPT_UNSIGNED:
	    num_read = sscanf(argument,"%u",(uint*)optarr->arg);
	    break;
	case OPT_LINTEGER:
	    num_read = sscanf(argument,"%ld",(long*)optarr->arg);
	    break;
	case OPT_LHEX:
	    num_read = sscanf(argument,"%lx",(long*)optarr->arg);
	    break;
	case OPT_LOCTAL:
	    num_read = sscanf(argument,"%lo",(long*)optarr->arg);
	    break;
	case OPT_LUNSIGNED:
	    num_read = sscanf(argument,"%lu",(ulong*)optarr->arg);
	    break;
	case OPT_FLOAT:
	    num_read = sscanf(argument,"%f",(float*)optarr->arg);
	    break;
	case OPT_DOUBLE:
	    num_read = sscanf(argument,"%lf",(double*)optarr->arg);
	    break;
	case OPT_LDOUBLE:
	    num_read = sscanf(argument,"%Lf",(long double*)optarr->arg);
	    break;
	case OPT_STRING:
	    num_read = 1;           /* This always works    */
	    *((char**)optarr->arg) = argument;
	    break;
	default:
	    return INVALID;
	}

    if (num_read == 0)
	return INVALID;
    else
	return ALLDONE;
}

/****************************************************************************
HEADER:
cmdline.h

PARAMETERS:
argc        - Number of arguments on command line
argv        - Array of command line arguments
num_opt     - Number of options in option array
optarr      - Array to specify how to parse the command line
do_param    - Routine to handle a command line parameter

RETURNS:
ALLDONE, INVALID or HELP

REMARKS:
Function to parse the command line according to a table of options. This
routine calls getcmdopt above to parse each individual option and attempts
to parse each option into a variable of the specified type. The routine
can parse integers and long integers in either decimal, octal, hexadecimal
notation, unsigned integers and unsigned longs, strings and option switches.
Option switches are simply boolean variables that get turned on if the
switch was parsed.

Parameters are extracted from the command line by calling a user supplied
routine do_param() to handle each parameter as it is encountered. The
routine do_param() should accept a pointer to the parameter on the command
line and an integer representing how many parameters have been encountered
(ie: 1 if this is the first parameter, 10 if it is the 10th etc), and return
ALLDONE upon successfully parsing it or INVALID if the parameter was invalid.

We return either ALLDONE if all the options were successfully parsed,
INVALID if an invalid option was encountered or HELP if any of -h, -H or
-? were present on the command line.
****************************************************************************/
int getargs(
    int argc,
    char *argv[],
    int num_opt,
    Option optarr[],
    int (*do_param)(
	char *param,
	int num))
{
    int     i,opt;
    char    *argument;
    int     param_num = 1;
    char    cmdstr[MAXARG*2 + 4];

    /* Build the command string from the array of options   */

    strcpy(cmdstr,"hH?");
    for (i = 0,opt = 3; i < num_opt; i++,opt++) {
	cmdstr[opt] = optarr[i].opt;
	if (optarr[i].type != OPT_SWITCH) {
	    cmdstr[++opt] = ':';
	    }
	}
    cmdstr[opt] = '\0';

    for (;;) {
	opt = getcmdopt(argc,argv,cmdstr,&argument);
	switch (opt) {
	    case 'H':
	    case 'h':
	    case '?':
		return HELP;
	    case ALLDONE:
		return ALLDONE;
	    case INVALID:
		return INVALID;
	    case PARAMETER:
		if (do_param == NULL)
		    return INVALID;
		if (do_param(argv[nextargv],param_num) == INVALID)
		    return INVALID;
		nextargv++;
		param_num++;
		break;
	    default:

		/* Search for the option in the option array. We are
		 * guaranteed to find it.
		 */

		for (i = 0; i < num_opt; i++) {
		    if (optarr[i].opt == opt)
			break;
		    }
		if (optarr[i].type == OPT_SWITCH)
		    *((ibool*)optarr[i].arg) = true;
		else {
		    if (parse_option(&optarr[i],argument) == INVALID)
			return INVALID;
		    }
		break;
	    }
	}
}

/****************************************************************************
HEADER:
cmdline.h

PARAMETERS:
num_opt - Number of options in the table
optarr  - Table of option descriptions

REMARKS:
Prints the description of each option in a standard format to the standard
output device. The description for each option is obtained from the table
of options.
****************************************************************************/
void print_desc(
    int num_opt,
    Option optarr[])
{
    int     i;

    for (i = 0; i < num_opt; i++) {
	if (optarr[i].type == OPT_SWITCH)
	    printf("  -%c       %s\n",optarr[i].opt,optarr[i].desc);
	else
	    printf("  -%c<arg>  %s\n",optarr[i].opt,optarr[i].desc);
	}
}

/****************************************************************************
HEADER:
cmdline.h

PARAMETERS:
moduleName  - Module name for program
cmdLine     - Command line to parse
pargc       - Pointer to 'argc' parameter
pargv       - Pointer to 'argv' parameter
maxArgc     - Maximum argv array index

REMARKS:
Parses a command line from a single string into the C style 'argc' and
'argv' format. Most useful for Windows programs where the command line
is passed in verbatim.
****************************************************************************/
int parse_commandline(
    char *moduleName,
    char *cmdLine,
    int *pargc,
    char *argv[],
    int maxArgv)
{
    static char str[512];
    static char filename[260];
    char        *prevWord = NULL;
    ibool        inQuote = FALSE;
    ibool        noStrip = FALSE;
    int         argc;

    argc = 0;
    strcpy(filename,moduleName);
    argv[argc++] = filename;
    cmdLine = strncpy(str, cmdLine, sizeof(str)-1);
    while (*cmdLine) {
	switch (*cmdLine) {
	    case '"' :
		if (prevWord != NULL) {
		    if (inQuote) {
			if (!noStrip)
			    *cmdLine = '\0';
			argv [argc++] = prevWord;
			prevWord = NULL;
			}
		    else
			noStrip = TRUE;
		    }
		inQuote = !inQuote;
		break;
	    case ' ' :
	    case '\t' :
		if (!inQuote) {
		    if (prevWord != NULL) {
			*cmdLine = '\0';
			argv [argc++] = prevWord;
			prevWord = NULL;
			noStrip = FALSE;
			}
		    }
		break;
	    default :
		if (prevWord == NULL)
		    prevWord = cmdLine;
		break;
		}
	if (argc >= maxArgv - 1)
	    break;
	cmdLine++;
	}

    if ((prevWord != NULL || (inQuote && prevWord != NULL)) && argc < maxArgv - 1) {
	*cmdLine = '\0';
	argv [argc++] = prevWord;
	}
    argv[argc] = NULL;

    /* Return updated parameters */
    return (*pargc = argc);
}
