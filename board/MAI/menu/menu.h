#ifndef MENU_H
#define MENU_H

/* A single menu */
typedef void (*menu_finish_callback)(struct menu_s *menu);

typedef struct menu_s
{
    char *name;                     /* Menu name */
    int  num_options;               /* Number of options in this menu */
    int  flags;                     /* Various flags - see below */
    int  option_align;              /* Aligns options to a field width of this much characters if != 0 */

    struct menu_option_s **options; /* Pointer to this menu's options */
    menu_finish_callback callback;  /* Called when the menu closes */
} menu_t;

/*
 * type: Type of the option (see below)
 * name: Name to display for this option
 * help: Optional help string
 * id  : optional id number
 * sys : pointer for system-specific data, init to NULL and don't touch
 */

#define OPTION_PREAMBLE				\
    int type;                     		\
    char *name;   				\
    char *help;   				\
    int id;                                     \
    void *sys;                                  \


/*
 * Menu option types.
 * There are a number of different layouts for menu options depending
 * on their types. Currently there are the following possibilities:
 *
 * Submenu:
 *   This entry links to a new menu.
 *
 * Boolean:
 *   A simple on/off toggle entry. Booleans can be either yes/no, 0/1 or on/off.
 *   Optionally, this entry can enable/disable a set of other options. An example would
 *   be to enable/disable on-board USB, and if enabled give access to further options like
 *   irq settings, base address etc.
 *
 * Text:
 *   A single line/limited number of characters text entry box. Text can be restricted
 *   to a certain charset (digits/hex digits/all/custom). Result is also available as an
 *   int if numeric.
 *
 * Selection:
 *   One-of-many type of selection entry. User may choose on of a set of strings, which
 *   maps to a specific value for the variable.
 *
 * Routine:
 *   Selecting this calls an entry-specific routine. This can be used for saving contents etc.
 *
 * Custom:
 *   Display and behaviour of this entry is defined by a set of callbacks.
 */

#define MENU_SUBMENU_TYPE 0
typedef struct menu_submenu_s
{
    OPTION_PREAMBLE

    menu_t *   submenu;            /* Pointer to the submenu */
} menu_submenu_t;

#define MENU_BOOLEAN_TYPE 1
typedef struct menu_boolean_s
{
    OPTION_PREAMBLE

    char *variable;                /* Name of the variable to getenv()/setenv() */
    int subtype;                   /* Subtype (on/off, 0/1, yes/no, enable/disable), see below */
    int mutex;                     /* Bit mask of options to enable/disable. Bit 0 is the option
				      immediately following this one, bit 1 is the next one etc.
				      bit 7 = 0 means to disable when this option is off,
				      bit 7 = 1 means to disable when this option is on.
				      An option is disabled when the type field's upper bit is set */
} menu_boolean_t;

/* BOOLEAN Menu flags */
#define MENU_BOOLEAN_ONOFF         0x01
#define MENU_BOOLEAN_01            0x02
#define MENU_BOOLEAN_YESNO         0x03
#define MENU_BOOLEAN_ENDIS         0x04
#define MENU_BOOLEAN_TYPE_MASK     0x07


#define MENU_TEXT_TYPE 2
typedef struct menu_text_s
{
    OPTION_PREAMBLE

    char *variable;                /* Name of the variable to getenv()/setenv() */
    int maxchars;                  /* Max number of characters */
    char *charset;                 /* Optional charset to use */
    int flags;                     /* Flags - see below */
} menu_text_t;

/* TEXT entry menu flags */
#define MENU_TEXT_NUMERIC         0x01
#define MENU_TEXT_HEXADECIMAL     0x02
#define MENU_TEXT_FREE            0x03
#define MENU_TEXT_TYPE_MASK       0x07


#define MENU_SELECTION_TYPE 3
typedef struct menu_select_option_s
{
    char *map_from;               /* Map this variable contents ... */
    char *map_to;                 /* ... to this menu text and vice versa */
} menu_select_option_t;

typedef struct menu_select_s
{
    OPTION_PREAMBLE

    int num_options;             /* Number of mappings */
    menu_select_option_t **options;
				 /* Option list array */
} menu_select_t;


#define MENU_ROUTINE_TYPE 4
typedef void (*menu_routine_callback)(struct menu_routine_s *);

typedef struct menu_routine_s
{
    OPTION_PREAMBLE
    menu_routine_callback callback;
				 /* routine to be called */
    void *user_data;             /* User data, don't care for system */
} menu_routine_t;


#define MENU_CUSTOM_TYPE 5
typedef void (*menu_custom_draw)(struct menu_custom_s *);
typedef void (*menu_custom_key)(struct menu_custom_s *, int);

typedef struct menu_custom_s
{
    OPTION_PREAMBLE
    menu_custom_draw drawfunc;
    menu_custom_key  keyfunc;
    void *user_data;
} menu_custom_t;

/*
 * The menu option superstructure
 */
typedef struct menu_option_s
{
    union
    {
	menu_submenu_t m_sub_menu;
	menu_boolean_t m_boolean;
	menu_text_t    m_text;
	menu_select_t  m_select;
	menu_routine_t m_routine;
    };
} menu_option_t;

/* Init the menu system. Returns <0 on error */
int menu_init(menu_t *root);

/* Execute a single menu. Returns <0 on error */
int menu_do(menu_t *menu);

#endif
