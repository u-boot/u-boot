#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <malloc.h>

#define PROMPT ">"


void
getline(char *buf,int *num,int max_num)
{
    static int line_len = 0;
    static char *line = NULL;
    static char *line_pointer = NULL;
    static int len = 0;
    int tmp_len;
    char *buff;

    if (len <= 0) {
    buff = readline(PROMPT);
    add_history(buff);

    if ((tmp_len = strlen(buff)) > line_len) {
	free(line);
	line = malloc(tmp_len);
	line_len = tmp_len;
    }
    sprintf(line,"%s\n",buff);
    free(buff);
    line_pointer = line;
    len = strlen(line);
    }

    *num = max_num > len? len : max_num;
    strncpy(buf,line_pointer,*num);
    line_pointer = line_pointer + *num;
    len = len - *num;
}
