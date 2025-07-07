/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __EXPO_H
#define __EXPO_H

#include <abuf.h>
#include <alist.h>
#include <dm/ofnode_decl.h>
#include <linux/bitops.h>
#include <linux/list.h>

struct udevice;

#include <cli.h>

/**
 * enum expo_id_t - standard expo IDs
 *
 * These are assumed to be in use at all times. Expos should use IDs starting
 * from EXPOID_BASE_ID,
 *
 * @EXPOID_NONE: Not used, invalid ID 0
 * @EXPOID_SAVE: User has requested that the expo data be saved
 * @EXPOID_DISCARD: User has requested that the expo data be discarded
 * @EXPOID_BASE_ID: First ID which can be used for expo objects
 */
enum expo_id_t {
	EXPOID_NONE,

	EXPOID_SAVE,
	EXPOID_DISCARD,

	EXPOID_BASE_ID = 5,
};

/**
 * enum expoact_type - types of actions reported by the expo
 *
 * @EXPOACT_NONE: no action
 * @EXPOACT_POINT_OBJ: object was highlighted (@id indicates which)
 * @EXPOACT_POINT_ITEM: menu item was highlighted (@id indicates which)
 * @EXPOACT_SELECT: menu item was selected (@id indicates which)
 * @EXPOACT_OPEN: menu was opened, so an item can be selected (@id indicates
 * which menu object)
 * @EXPOACT_CLOSE: menu was closed (@id indicates which menu object)
 * @EXPOACT_QUIT: request to exit the menu
 */
enum expoact_type {
	EXPOACT_NONE,
	EXPOACT_POINT_OBJ,
	EXPOACT_POINT_ITEM,
	EXPOACT_SELECT,
	EXPOACT_OPEN,
	EXPOACT_CLOSE,
	EXPOACT_QUIT,
};

/**
 * struct expo_action - an action report by the expo
 *
 * @type: Action type (EXPOACT_NONE if there is no action)
 * @select: Used for EXPOACT_POINT_ITEM and EXPOACT_SELECT
 * @select.id: ID number of the object affected.
 */
struct expo_action {
	enum expoact_type type;
	union {
		struct {
			int id;
		} select;
	};
};

/**
 * struct expo_theme - theme for the expo
 *
 * @font_size: Default font size for all text
 * @menu_inset: Inset width (on each side and top/bottom) for menu items
 * @menuitem_gap_y: Gap between menu items in pixels
 * @menu_title_margin_x: Gap between right side of menu title and left size of
 *	menu label
 */
struct expo_theme {
	u32 font_size;
	u32 menu_inset;
	u32 menuitem_gap_y;
	u32 menu_title_margin_x;
};

/**
 * struct expo - information about an expo
 *
 * A group of scenes which can be presented to the user, typically to obtain
 * input or to make a selection.
 *
 * @name: Name of the expo (allocated)
 * @display: Display to use (`UCLASS_VIDEO`), or NULL to use text mode
 * @cons: Console to use (`UCLASS_VIDEO_CONSOLE`), or NULL to use text mode
 * @scene_id: Current scene ID (0 if none)
 * @next_id: Next ID number to use, for automatic allocation
 * @action: Action selected by user. At present only one is supported, with the
 * type set to EXPOACT_NONE if there is no action
 * @text_mode: true to use text mode for the menu (no vidconsole)
 * @popup: true to use popup menus, instead of showing all items
 * @show_highlight: show a highlight bar on the selected menu item
 * @priv: Private data for the controller
 * @done: Indicates that a cedit session is complete and the user has quit
 * @save: Indicates that cedit data should be saved, rather than discarded
 * @theme: Information about fonts styles, etc.
 * @scene_head: List of scenes
 * @str_head: list of strings
 * @cch: Keyboard context for input
 */
struct expo {
	char *name;
	struct udevice *display;
	struct udevice *cons;
	uint scene_id;
	uint next_id;
	struct expo_action action;
	bool text_mode;
	bool popup;
	bool show_highlight;
	void *priv;
	bool done;
	bool save;
	struct expo_theme theme;
	struct list_head scene_head;
	struct list_head str_head;
	struct cli_ch_state cch;
};

/**
 * struct expo_string - a string that can be used in an expo
 *
 * @id: ID number of the string
 * @buf: String (contains nul terminator)
 * @sibling: Node to link this object to its siblings
 */
struct expo_string {
	uint id;
	struct abuf buf;
	struct list_head sibling;
};

/**
 * struct scene - information about a scene in an expo
 *
 * A collection of text/image/menu items in an expo
 *
 * @expo: Expo this scene is part of
 * @name: Name of the scene (allocated)
 * @id: ID number of the scene
 * @title_id: String ID of title of the scene (allocated)
 * @highlight_id: ID of highlighted object, if any
 * @cls: cread state to use for input
 * @buf: Buffer for input
 * @entry_save: Buffer to hold vidconsole text-entry information
 * @sibling: Node to link this scene to its siblings
 * @obj_head: List of objects in the scene
 */
struct scene {
	struct expo *expo;
	char *name;
	uint id;
	uint title_id;
	uint highlight_id;
	struct cli_line_state cls;
	struct abuf buf;
	struct abuf entry_save;
	struct list_head sibling;
	struct list_head obj_head;
};

/**
 * enum scene_obj_t - type of a scene object
 *
 * @SCENEOBJT_NONE: Used to indicate that the type does not matter
 * @SCENEOBJT_IMAGE: Image data to render
 * @SCENEOBJT_BOX: Rectangular box
 * @SCENEOBJT_TEXT: Text line to render
 * @SCENEOBJT_MENU: Menu containing items the user can select
 * @SCENEOBJT_TEXTLINE: Line of text the user can edit
 * @SCENEOBJT_TEXTEDIT: Simple text editor
 */
enum scene_obj_t {
	SCENEOBJT_NONE		= 0,
	SCENEOBJT_IMAGE,
	SCENEOBJT_TEXT,
	SCENEOBJT_BOX,
	SCENEOBJT_TEXTEDIT,

	/* types from here on can be highlighted */
	SCENEOBJT_MENU,
	SCENEOBJT_TEXTLINE,
};

/**
 * struct scene_obj_bbox - Dimensions of an object
 *
 * @x0: x position, in pixels from left side
 * @y0: y position, in pixels from top
 * @x1: x position of right size
 * @y1: y position of bottom
 */
struct scene_obj_bbox {
	int x0;
	int y0;
	int x1;
	int y1;
};

/**
 * struct scene_obj_offset - Offsets for drawing the object
 *
 * Stores the offset from x0, x1 at which objects are drawn
 *
 * @xofs: x offset
 * @yofs: y offset
 */
struct scene_obj_offset {
	int xofs;
	int yofs;
};

/**
 * struct scene_obj_dims - Dimensions of the object being drawn
 *
 * Image and text objects have a dimension which can change depending on what
 * they contain. For images this stores the size. For text it stores the size as
 * rendered on the display
 *
 * @x: x dimension
 * @y: y dimension
 */
struct scene_obj_dims {
	int x;
	int y;
};

/* special values for dimensions */
enum {
	/* width/height of the display */
	SCENEOB_DISPLAY_MAX	= 0x7f000000,
};

/**
 * enum scene_obj_halign - Horizontal alignment of objects
 *
 * Objects are normally drawn on the left size of their bounding box. This
 * properly allows aligning on the right or having the object centred.
 *
 * @SCENEOA_LEFT: Left of object is aligned with its x coordinate
 * @SCENEOA_RIGHT: Right of object is aligned with x + w
 * @SCENEOA_CENTRE: Centre of object is aligned with centre of bounding box
 * @SCENEOA_TOP: Left of object is aligned with its x coordinate
 * @SCENEOA_BOTTOM: Right of object is aligned with x + w
 *
 * Note: It would be nice to make this a char type but Sphinx riddles:
 * ./include/expo.h:258: error: Cannot parse enum!
 * enum scene_obj_align : char {
 */
enum scene_obj_align {
	SCENEOA_LEFT,
	SCENEOA_RIGHT,
	SCENEOA_CENTRE,
	SCENEOA_TOP = SCENEOA_LEFT,
	SCENEOA_BOTTOM = SCENEOA_RIGHT,
};

/**
 * enum scene_obj_flags_t - flags for objects
 *
 * @SCENEOF_HIDE: object should be hidden
 * @SCENEOF_POINT: object should be highlighted
 * @SCENEOF_OPEN: object should be opened (e.g. menu is opened so that an option
 * can be selected)
 * @SCENEOF_SIZE_VALID: object's size (width/height) is valid, so any adjustment
 * to x0/y0 should maintain the width/height of the object
 */
enum scene_obj_flags_t {
	SCENEOF_HIDE	= 1 << 0,
	SCENEOF_POINT	= 1 << 1,
	SCENEOF_OPEN	= 1 << 2,
	SCENEOF_SIZE_VALID	= BIT(3),
};

enum {
	/* Maximum number of characters allowed in an line editor */
	EXPO_MAX_CHARS		= 250,
};

/**
 * struct scene_obj - information about an object in a scene
 *
 * @scene: Scene that this object relates to
 * @name: Name of the object (allocated)
 * @id: ID number of the object
 * @type: Type of this object
 * @bbox: Bounding box for this object
 * @ofs: Offset from x0, y0 where the object is drawn
 * @dims: Dimensions of the text/image (may be smaller than bbox)
 * @horiz: Horizonal alignment
 * @vert: Vertical alignment
 * @flags: Flags for this object
 * @bit_length: Number of bits used for this object in CMOS RAM
 * @start_bit: Start bit to use for this object in CMOS RAM
 * @sibling: Node to link this object to its siblings
 */
struct scene_obj {
	struct scene *scene;
	char *name;
	uint id;
	enum scene_obj_t type;
	struct scene_obj_bbox bbox;
	struct scene_obj_offset ofs;
	struct scene_obj_dims dims;
	enum scene_obj_align horiz;
	enum scene_obj_align vert;
	u8 flags;
	u8 bit_length;
	u16 start_bit;
	struct list_head sibling;
};

/* object can be highlighted when moving around expo */
static inline bool scene_obj_can_highlight(const struct scene_obj *obj)
{
	return obj->type >= SCENEOBJT_MENU;
}

/**
 * struct scene_obj_img - information about an image object in a scene
 *
 * This is a rectangular image which is blitted onto the display
 *
 * @obj: Basic object information
 * @data: Image data in BMP format
 */
struct scene_obj_img {
	struct scene_obj obj;
	char *data;
};

/**
 * struct scene_txt_generic - Generic information common to text objects
 *
 * @str_id: ID of the text string to display
 * @font_name: Name of font (allocated by caller)
 * @font_size: Nominal size of font in pixels
 * @lines: alist of struct vidconsole_mline with a separate record for each
 *	line of text
 */
struct scene_txt_generic {
	uint str_id;
	const char *font_name;
	uint font_size;
	struct alist lines;
};

/**
 * struct scene_obj_txt - information about a text object in a scene
 *
 * This is a single-line text object
 *
 * @obj: Basic object information
 * @gen: Generic information common to all objects which show text
 */
struct scene_obj_txt {
	struct scene_obj obj;
	struct scene_txt_generic gen;
};

/**
 * struct scene_obj_menu - information about a menu object in a scene
 *
 * A menu has a number of items which can be selected by the user
 *
 * It also has:
 *
 * - a text/image object (@pointer_id) which points to the current item
 *   (@cur_item_id)
 *
 * - a preview object which shows an image related to the current item
 *
 * @obj: Basic object information
 * @title_id: ID of the title text, or 0 if none
 * @cur_item_id: ID of the current menu item, or 0 if none
 * @pointer_id: ID of the object pointing to the current selection
 * @item_head: List of items in the menu
 */
struct scene_obj_menu {
	struct scene_obj obj;
	uint title_id;
	uint cur_item_id;
	uint pointer_id;
	struct list_head item_head;
};

/**
 * enum scene_menuitem_flags_t - flags for menu items
 *
 * @SCENEMIF_GAP_BEFORE: Add a gap before this item
 */
enum scene_menuitem_flags_t {
	SCENEMIF_GAP_BEFORE	= 1 << 0,
};

/**
 * struct scene_menitem - a menu item in a menu
 *
 * A menu item has:
 *
 * - text object holding the name (short) and description (can be longer)
 * - a text object holding the keypress
 *
 * @name: Name of the item (this is allocated by this call)
 * @id: ID number of the object
 * @key_id: ID of text object to use as the keypress to show
 * @label_id: ID of text object to use as the label text
 * @desc_id: ID of text object to use as the description text
 * @preview_id: ID of the preview object, or 0 if none
 * @flags: Flags for this item
 * @value: Value for this item, or INT_MAX to use sequence
 * @sibling: Node to link this item to its siblings
 */
struct scene_menitem {
	char *name;
	uint id;
	uint key_id;
	uint label_id;
	uint desc_id;
	uint preview_id;
	uint flags;
	int value;
	struct list_head sibling;
};

/**
 * struct scene_obj_textline - information about a textline in a scene
 *
 * A textline has a prompt and a line of editable text
 *
 * @obj: Basic object information
 * @label_id: ID of the label text, or 0 if none
 * @edit_id: ID of the editable text
 * @max_chars: Maximum number of characters allowed
 * @buf: Text buffer containing current text
 * @pos: Cursor position
 */
struct scene_obj_textline {
	struct scene_obj obj;
	uint label_id;
	uint edit_id;
	uint max_chars;
	struct abuf buf;
	uint pos;
};

/**
 * struct scene_obj_box - information about a box in a scene
 *
 * A box surrounds a part of the screen with a border
 *
 * @obj: Basic object information
 * @width: Line-width in pixels
 */
struct scene_obj_box {
	struct scene_obj obj;
	uint width;
};

/**
 * struct scene_obj_txtedit - information about a box in a scene
 *
 * A text editor which allows users to edit a small text file
 *
 * @obj: Basic object information
 * @gen: Generic information common to all objects which show text
 * @buf: Text buffer containing current text
 */
struct scene_obj_txtedit {
	struct scene_obj obj;
	struct scene_txt_generic gen;
	struct abuf buf;
};

/**
 * struct expo_arrange_info - Information used when arranging a scene
 *
 * @label_width: Maximum width of labels in scene
 */
struct expo_arrange_info {
	int label_width;
};

/**
 * expo_new() - create a new expo
 *
 * Allocates a new expo
 *
 * @name: Name of expo (this is allocated by this call)
 * @priv: Private data for the controller
 * @expp: Returns a pointer to the new expo on success
 * Returns: 0 if OK, -ENOMEM if out of memory
 */
int expo_new(const char *name, void *priv, struct expo **expp);

/**
 * expo_destroy() - Destroy an expo and free all its memory
 *
 * @exp: Expo to destroy
 */
void expo_destroy(struct expo *exp);

/**
 * expo_set_dynamic_start() - Set the start of the 'dynamic' IDs
 *
 * It is common for a set of 'static' IDs to be used to refer to objects in the
 * expo. These typically use an enum so that they are defined in sequential
 * order.
 *
 * Dynamic IDs (for objects not in the enum) are intended to be used for
 * objects to which the code does not need to refer. These are ideally located
 * above the static IDs.
 *
 * Use this function to set the start of the dynamic range, making sure that the
 * value is higher than all the statically allocated IDs.
 *
 * @exp: Expo to update
 * @dyn_start: Start ID that expo should use for dynamic allocation
 */
void expo_set_dynamic_start(struct expo *exp, uint dyn_start);

/**
 * expo_str() - add a new string to an expo
 *
 * @exp: Expo to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new object (0 to allocate one)
 * @str: Pointer to text to display (allocated by caller)
 * Returns: ID number for the object (typically @id), or -ve on error
 */
int expo_str(struct expo *exp, const char *name, uint id, const char *str);

/**
 * expo_get_str() - Get a string by ID
 *
 * @exp: Expo to use
 * @id: String ID to look up
 * @returns string, or NULL if not found
 */
const char *expo_get_str(struct expo *exp, uint id);

/**
 * expo_edit_str() - Make a string writeable
 *
 * This allows a string to be updated under the control of the caller. The
 * buffer must remain valid while the expo is active.
 *
 * @exp: Expo to use
 * @id: String ID to look up
 * @orig: If non-NULL, returns the original buffer, which can be used by the
 *	caller. It is no-longer used by expo so must be uninited by the caller.
 *	It contains a snapshot of the string contents
 * @copyp: Returns a pointer to the new, writeable buffer
 * Return: 0 if OK, -ENOENT if the id was not found, -ENOMEM if out of memory
 */
int expo_edit_str(struct expo *exp, uint id, struct abuf *orig,
		  struct abuf **copyp);

/**
 * expo_set_display() - set the display to use for a expo
 *
 * @exp: Expo to update
 * @dev: Display to use (`UCLASS_VIDEO`), NULL to use text mode
 * Returns: 0 (always)
 */
int expo_set_display(struct expo *exp, struct udevice *dev);

/**
 * expo_calc_dims() - Calculate the dimensions of the objects
 *
 * Updates the width and height of all objects based on their contents
 *
 * @exp: Expo to update
 * Returns 0 if OK, -ENOTSUPP if there is no graphical console
 */
int expo_calc_dims(struct expo *exp);

/**
 * expo_set_scene_id() - Set the current scene ID
 *
 * @exp: Expo to update
 * @scene_id: New scene ID to use (0 to select no scene)
 * Returns: 0 if OK, -ENOENT if there is no scene with that ID
 */
int expo_set_scene_id(struct expo *exp, uint scene_id);

/**
 * expo_first_scene_id() - Get the ID of the first scene
 *
 * @exp: Expo to check
 * Returns: Scene ID of first scene, or -ENOENT if there are no scenes
 */
int expo_first_scene_id(struct expo *exp);

/**
 * expo_render() - render the expo on the display / console
 *
 * @exp: Expo to render
 *
 * Returns: 0 if OK, -ECHILD if there is no current scene, -ENOENT if the
 * current scene is not found, other error if something else goes wrong
 */
int expo_render(struct expo *exp);

/**
 * expo_set_text_mode() - Controls whether the expo renders in text mode
 *
 * @exp: Expo to update
 * @text_mode: true to use text mode, false to use the console
 */
void expo_set_text_mode(struct expo *exp, bool text_mode);

/**
 * scene_new() - create a new scene in a expo
 *
 * The scene is given the ID @id which must be unique across all scenes, objects
 * and items. The expo's @next_id is updated to at least @id + 1
 *
 * @exp: Expo to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new scene (0 to allocate one)
 * @scnp: Returns a pointer to the new scene on success
 * Returns: ID number for the scene (typically @id), or -ve on error
 */
int scene_new(struct expo *exp, const char *name, uint id, struct scene **scnp);

/**
 * expo_lookup_scene_id() - Look up a scene by ID
 *
 * @exp: Expo to check
 * @scene_id: Scene ID to look up
 * @returns pointer to scene if found, else NULL
 */
struct scene *expo_lookup_scene_id(struct expo *exp, uint scene_id);

/**
 * scene_highlight_first() - Highlight the first item in a scene
 *
 * This highlights the first item, so that the user can see that it is pointed
 * to
 *
 * @scn: Scene to update
 */
void scene_highlight_first(struct scene *scn);

/**
 * scene_set_highlight_id() - Set the object which is highlighted
 *
 * Sets a new object to highlight in the scene
 *
 * @scn: Scene to update
 * @id: ID of object to highlight
 */
void scene_set_highlight_id(struct scene *scn, uint id);

/**
 * scene_set_open() - Set whether an item is open or not
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @open: true to open the object, false to close it
 * Returns: 0 if OK, -ENOENT if @id is invalid
 */
int scene_set_open(struct scene *scn, uint id, bool open);

/**
 * scene_obj_count() - Count the number of objects in a scene
 *
 * @scn: Scene to check
 * Returns: number of objects in the scene, 0 if none
 */
int scene_obj_count(struct scene *scn);

/**
 * scene_img() - add a new image to a scene
 *
 * @scn: Scene to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new object (0 to allocate one)
 * @data: Pointer to image data
 * @imgp: If non-NULL, returns the new object
 * Returns: ID number for the object (typically @id), or -ve on error
 */
int scene_img(struct scene *scn, const char *name, uint id, char *data,
	      struct scene_obj_img **imgp);

/**
 * scene_txt() - add a new text object to a scene
 *
 * @scn: Scene to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new object (0 to allocate one)
 * @str_id: ID of the string to use
 * @txtp: If non-NULL, returns the new object
 * Returns: ID number for the object (typically @id), or -ve on error
 */
int scene_txt(struct scene *scn, const char *name, uint id, uint str_id,
	      struct scene_obj_txt **txtp);

/**
 * scene_txt_str() - add a new string to expo and text object to a scene
 *
 * @scn: Scene to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new object (0 to allocate one)
 * @str_id: ID of the string to use
 * @str: Pointer to text to display (allocated by caller)
 * @txtp: If non-NULL, returns the new object
 * Returns: ID number for the object (typically @id), or -ve on error
 */
int scene_txt_str(struct scene *scn, const char *name, uint id, uint str_id,
		  const char *str, struct scene_obj_txt **txtp);

/**
 *  scene_menu() - create a menu
 *
 * @scn: Scene to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new object (0 to allocate one)
 * @menup: If non-NULL, returns the new object
 * Returns: ID number for the object (typically @id), or -ve on error
 */
int scene_menu(struct scene *scn, const char *name, uint id,
	       struct scene_obj_menu **menup);

/**
 *  scene_textline() - create a textline
 *
 * @scn: Scene to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new object (0 to allocate one)
 * @max_chars: Maximum length of the textline in characters
 * @tlinep: If non-NULL, returns the new object
 * Returns: ID number for the object (typically @id), or -ve on error
 */
int scene_textline(struct scene *scn, const char *name, uint id, uint max_chars,
		   struct scene_obj_textline **tlinep);

/**
 *  scene_box() - create a box
 *
 * @scn: Scene to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new object (0 to allocate one)
 * @width: Line-width in pixels
 * @boxp: If non-NULL, returns the new object
 * Returns: ID number for the object (typically @id), or -ve on error
 */
int scene_box(struct scene *scn, const char *name, uint id, uint width,
	      struct scene_obj_box **boxp);

/**
 *  scene_texted() - create a text editor
 *
 * @scn: Scene to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new object (0 to allocate one)
 * @strid: ID of the string to edit
 * @teditp: If non-NULL, returns the new object
 * Returns: ID number for the object (typically @id), or -ve on error
 */
int scene_texted(struct scene *scn, const char *name, uint id, uint strid,
		 struct scene_obj_txtedit **teditp);

/**
 * scene_txt_set_font() - Set the font for an object
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @font_name: Font name to use (allocated by caller)
 * @font_size: Font size to use (nominal height in pixels)
 */
int scene_txt_set_font(struct scene *scn, uint id, const char *font_name,
		       uint font_size);

/**
 * scene_txted_set_font() - Set the font for an object
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @font_name: Font name to use (allocated by caller)
 * @font_size: Font size to use (nominal height in pixels)
 */
int scene_txted_set_font(struct scene *scn, uint id, const char *font_name,
			 uint font_size);

/**
 * scene_obj_set_pos() - Set the postion of an object
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @x: x position, in pixels from left side
 * @y: y position, in pixels from top
 * Returns: 0 if OK, -ENOENT if @id is invalid
 */
int scene_obj_set_pos(struct scene *scn, uint id, int x, int y);

/**
 * scene_obj_set_size() - Set the size of an object
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @w: width in pixels
 * @h: height in pixels
 * Returns: 0 if OK, -ENOENT if @id is invalid
 */
int scene_obj_set_size(struct scene *scn, uint id, int w, int h);

/**
 * scene_obj_set_width() - Set the width of an object
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @w: width in pixels
 * Returns: 0 if OK, -ENOENT if @id is invalid
 */
int scene_obj_set_width(struct scene *scn, uint id, int w);

/**
 * scene_obj_set_bbox() - Set the bounding box of an object
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @x0: x position, in pixels from left side
 * @y0: y position, in pixels from top
 * @x1: ending x position (right side)
 * @y1: ending y position (botton side)
 * Returns: 0 if OK, -ENOENT if @id is invalid
 */
int scene_obj_set_bbox(struct scene *scn, uint id, int x0, int y0, int x1,
		       int y1);

/**
 * scene_obj_set_halign() - Set the horizontal alignment of an object
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @aln: Horizontal alignment to use
 * Returns: 0 if OK, -ENOENT if @id is invalid
 */
int scene_obj_set_halign(struct scene *scn, uint id, enum scene_obj_align aln);

/**
 * scene_obj_set_valign() - Set the vertical alignment of an object
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @aln: Vertical alignment to use
 * Returns: 0 if OK, -ENOENT if @id is invalid
 */
int scene_obj_set_valign(struct scene *scn, uint id, enum scene_obj_align aln);

/**
 * scene_obj_set_hide() - Set whether an object is hidden
 *
 * The update happens when the expo is next rendered.
 *
 * @scn: Scene to update
 * @id: ID of object to update
 * @hide: true to hide the object, false to show it
 * Returns: 0 if OK, -ENOENT if @id is invalid
 */
int scene_obj_set_hide(struct scene *scn, uint id, bool hide);

/**
 * scene_menu_set_title() - Set the title of a menu
 *
 * @scn: Scene to update
 * @id: ID of menu object to update
 * @title_id: ID of text object to use as the title
 * Returns: 0 if OK, -ENOENT if @id is invalid, -EINVAL if @title_id is invalid
 */
int scene_menu_set_title(struct scene *scn, uint id, uint title_id);

/**
 * scene_menu_set_pointer() - Set the item pointer for a menu
 *
 * This is a visual indicator of the current item, typically a ">" character
 * which sits next to the current item and moves when the user presses the
 * up/down arrow keys
 *
 * @scn: Scene to update
 * @id: ID of menu object to update
 * @cur_item_id: ID of text or image object to use as a pointer to the current
 * item
 * Returns: 0 if OK, -ENOENT if @id is invalid, -EINVAL if @cur_item_id is invalid
 */
int scene_menu_set_pointer(struct scene *scn, uint id, uint cur_item_id);

/**
 * scene_menu_select_item() - move the pointer/highlight to an item
 *
 * @scn: Scene to update
 * @id: ID of menu object to update
 * @sel_id: ID of the menuitem to select
 * Return 0 on success, -ENOENT if there was no such item
 */
int scene_menu_select_item(struct scene *scn, uint id, uint sel_id);

/**
 * scene_menu_get_cur_item() - get the currently pointed-to item
 *
 * @scn: Scene to update
 * @id: ID of menu object to update
 * Return ID of the current item the menu is pointing to, -ENOENT if @id is not
 * valid, 0 if no item is pointed to
 */
int scene_menu_get_cur_item(struct scene *scn, uint id);

/**
 * scene_obj_get_hw() - Get width and height of an object in a scene
 *
 * @scn: Scene to check
 * @id: ID of menu object to check
 * @widthp: If non-NULL, returns width of object in pixels
 * Returns: Height of object in pixels
 */
int scene_obj_get_hw(struct scene *scn, uint id, int *widthp);

/**
 * scene_menuitem() - Add an item to a menu
 *
 * @scn: Scene to update
 * @menu_id: ID of menu object to update
 * @name: Name to use (this is allocated by this call)
 * @id: ID to use for the new object (0 to allocate one)
 * @key_id: ID of text object to use as the keypress to show
 * @label_id: ID of text object to use as the label text
 * @desc_id: ID of text object to use as the description text
 * @preview_id: ID of object to use as the preview (text or image)
 * @flags: Flags for this item (enum scene_menuitem_flags_t)
 * @itemp: If non-NULL, returns the new object
 * Returns: ID number for the item (typically @id), or -ve on error
 */
int scene_menuitem(struct scene *scn, uint menu_id, const char *name, uint id,
		   uint key_id, uint label_id, uint desc_id, uint preview_id,
		   uint flags, struct scene_menitem **itemp);

/**
 * scene_arrange() - Arrange the scene to deal with object sizes
 *
 * Updates any menus in the scene so that their objects are in the right place.
 *
 * @scn: Scene to arrange
 * Returns: 0 if OK, -ve on error
 */
int scene_arrange(struct scene *scn);

/**
 * expo_send_key() - set a keypress to the expo
 *
 * @exp: Expo to receive the key
 * @key: Key to send (ASCII or enum bootmenu_key)
 * Returns: 0 if OK, -ECHILD if there is no current scene
 */
int expo_send_key(struct expo *exp, int key);

/**
 * expo_action_get() - read user input from the expo
 *
 * @exp: Expo to check
 * @act: Returns action
 * Returns: 0 if OK, -EAGAIN if there was no action to return
 */
int expo_action_get(struct expo *exp, struct expo_action *act);

/**
 * expo_apply_theme() - Apply a theme to an expo
 *
 * @exp: Expo to update
 * @node: Node containing the theme
 */
int expo_apply_theme(struct expo *exp, ofnode node);

/**
 * expo_build() - Build an expo from an FDT description
 *
 * Build a complete expo from a description in the provided devicetree.
 *
 * See doc/develop/expo.rst for a description of the format
 *
 * @root: Root node for expo description
 * @expp: Returns the new expo
 * Returns: 0 if OK, -ENOMEM if out of memory, -EINVAL if there is a format
 * error, -ENOENT if there is a references to a non-existent string
 */
int expo_build(ofnode root, struct expo **expp);

/**
 * cb_expo_build() - Build an expo for coreboot CMOS RAM
 *
 * @expp: Returns the expo created
 * Return: 0 if OK, -ve on error
 */
int cb_expo_build(struct expo **expp);

/**
 * expo_poll() - see if the user takes an action
 *
 * This checks for a keypress. If there is one, it is processed and the
 * resulting action returned, if any.
 *
 * Note that expo_render() should normally be called immediately before this
 * function so that the user can see the latest state.
 *
 * @exp: Expo to poll
 * @act: Returns action on success
 * Return: 0 if an action was obtained, -EAGAIN if not, other error if something
 *	went wrong
 */
int expo_poll(struct expo *exp, struct expo_action *act);

#endif /*__EXPO_H */
