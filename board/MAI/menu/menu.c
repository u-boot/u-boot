#include "menu.h"

#define SINGLE_BOX 0
#define DOUBLE_BOX 1

void video_draw_box(int style, int attr, char *title, int separate, int x, int y, int w, int h);
void video_draw_text(int x, int y, int attr, char *text);
void video_save_rect(int x, int y, int w, int h, void *save_area, int clearchar, int clearattr);
void video_restore_rect(int x, int y, int w, int h, void *save_area);
int  video_rows(void);
int  video_cols(void);

#define MAX_MENU_OPTIONS 200

typedef struct
{
    int used;                  /* flag if this entry is used */
    int entry_x;               /* Character column of the menu entry */
    int entry_y;               /* Character line of the entry */
    int option_x;              /* Character colum of the option (entry is same) */
} option_data_t;

option_data_t odata[MAX_MENU_OPTIONS];

int normal_attr = 0x0F;
int select_attr = 0x2F;
int disabled_attr = 0x07;

menu_t *root_menu;

int menu_init (menu_t *root)
{
    char *s;
    int i;

    s = getenv("menu_normal");
    if (s) normal_attr = atoi(s);

    s = getenv("menu_select");
    if (s) select_attr = atoi(s);

    s = getenv("menu_disabled");
    if (s) disabled_attr = atoi(s);

    for (i=0; i<MAX_MENU_OPTIONS; i++) odata[i].used = 0;

    root_menu = root;
}

option_data_t *menu_alloc_odata(void)
{
    int i;
    for (int i=0; i<MAX_MENU_OPTIONS; i++)
    {
	if (odata[i].used == 0) return &odata[i];
    }
    return NULL;
}

void menu_free_odata(option_data_t *odata)
{
    odata->used = 0;
}

void menu_layout (menu_t *menu)
{
