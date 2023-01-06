.. SPDX-License-Identifier: GPL-2.0+

Expo menu
=========

U-Boot provides a menu implementation for use with selecting bootflows and
changing U-Boot settings. This is in early stages of development.

Motivation
----------

U-Boot already has a text-based menu system accessed via the
:doc:`../usage/cmd/bootmenu`. This works using environment variables, or via
some EFI-specific hacks.

The command makes use of a lower-level `menu` implementation, which is quite
flexible and can be used to make menu hierarchies.

However this system is not flexible enough for use with standard boot. It does
not support a graphical user interface and cannot currently support anything
more than a very simple list of items. While it does support multiple menus in
hierarchies, these are implemented by the caller. See for example `eficonfig.c`.

Another challenge with the current menu implementation is that it controls
the event loop, such that bootmenu_loop() does not return until a key is
pressed. This makes it difficult to implement dynamic displays or to do other
things while the menu is running, such as searching for more bootflows.

For these reasons an attempt has been made to develop a more flexible system
which can handle menus as well as other elements. This is called 'expo', short
for exposition, in an attempt to avoid common words like display, screen, menu
and the like. The primary goal is to support Verified Boot for Embedded (VBE),
although it is available to any boot method, using the 'bootflow menu' command.

Efforts have been made to use common code with the existing menu, including
key processing in particular.

Previous work looked at integrating Nuklear into U-Boot. This works fine and
could provide a way to provide a more flexible UI, perhaps with expo dealing
with the interface to Nuklear. But this is quite a big step and it may be years
before this becomes desirable, if at all. For now, U-Boot only needs a fairly
simple set of menus and options, so rendering them directly is fairly
straightforward.

Concepts
--------

The creator of the expo is here called a `controller` and it controls most
aspects of the expo. This is the code that you must write to use expo.

An `expo` is a set of scenes which can be presented to the user one at a time,
to show information and obtain input from the user.

A `scene` is a collection of objects which are displayed together on the screen.
Only one scene is visible at a time and scenes do not share objects.

A `scene object` is something that appears in the scene, such as some text, an
image or a menu. Objects can be positioned and hidden.

A `menu object` contains a title, a set of `menu items` and a pointer to the
current item. Menu items consist of a keypress (indicating what to press to
select the item), label and description. All three are shown in a single line
within the menu. Items can also have a preview image, which is shown when the
item is highlighted.

All components have a name. This is purely for debugging, so it is easy to see
what object is referred to. Of course the ID numbers can help as well, but they
are less easy to distinguish.

While the expo implementation provides support for handling keypresses and
rendering on the display or serial port, it does not actually deal with reading
input from the user, nor what should be done when a particular menu item is
selected. This is deliberate since having the event loop outside the expo is
more flexible, particularly in a single-threaded environment like U-Boot.

Everything within an expo has a unique ID number. This is done so that it is
easy to refer to things after the expo has been created. The expectation is that
the controller declares an enum containing all of the elements in the expo,
passing the ID of each object as it is created. When a menu item is selected,
its ID is returned. When a object's font or position needs to change, the ID is
passed to expo functions to indicate which object it is. It is possible for expo
to auto-allocate IDs, but this is not recommended. The use of IDs is a
convenience, removing the need for the controller to store pointers to objects,
or even the IDs of objects. Programmatic creation of many items in a loop can be
handled by allocating space in the enum for a maximum number of items, then
adding the loop count to the enum values to obtain unique IDs.

All text strings are stored in a structure attached to the expo, referenced by
a text ID. This makes it easier at some point to implement multiple languages or
to support Unicode strings.

Menu objects do not have their own text and image objects. Instead they simply
refer to objects which have been created. So a menu item is just a collection
of IDs of text and image objects. When adding a menu item you must create these
objects first, then create the menu item, passing in the relevant IDs.

Creating an expo
----------------

To create an expo, use `expo_new()` followed by `scene_new()` to create a scene.
Then add objects to the scene, using functions like `scene_txt_str()` and
`scene_menu()`. For every menu item, add text and image objects, then create
the menu item with `scene_menuitem()`, referring to those objects.

Layout
------

Individual objects can be positioned using `scene_obj_set_pos()`. Menu items
cannot be positioned manually: this is done by `scene_arrange()` which is called
automatically when something changes. The menu itself determines the position of
its items.

Rendering
---------

Rendering is performed by calling `expo_render()`. This uses either the
vidconsole, if present, or the serial console in `text mode`. Expo handles
presentation automatically in either case, without any change in how the expo is
created.

For the vidconsole, Truetype fonts can be used if enabled, to enhance the
quality of the display. For text mode, each menu item is shown in a single line,
allowing easy selection using arrow keys.

Input
-----

The controller is responsible for collecting keyboard input. A good way to do
this is to use `cli_ch_process()`, since it handles conversion of escape
sequences into keys. However, expo has some special menu-key codes for
navigating the interface. These are defined in `enum bootmenu_key` and include
`BKEY_UP` for moving up and `BKEY_SELECT` for selecting an item. You can use
`bootmenu_conv_key()` to convert an ASCII key into one of these.

Once a keypress is decoded, call `expo_send_key()` to send it to the expo. This
may cause an update to the expo state and may produce an action.

Actions
-------

Call `expo_action_get()` in the event loop to check for any actions that the
expo wants to report. These can include selecting a particular menu item, or
quitting the menu. Processing of these is the responsibility of your controller.

Event loop
----------

Expo is intended to be used in an event loop. For an example loop, see
`bootflow_menu_run()`. It is possible to perform other work in your event loop,
such as scanning devices for more bootflows.

Themes
------

Expo does not itself support themes. The bootflow_menu implement supposed a
basic theme, applying font sizes to the various text objects in the expo.

API documentation
-----------------

.. kernel-doc:: include/expo.h

Future ideas
------------

Some ideas for future work:

- Default menu item and a timeout
- Higher-level / automatic / more flexible layout of objects
- Image formats other than BMP
- Use of ANSI sequences to control a serial terminal
- Colour selection
- Better support for handling lots of settings, e.g. with multiple menus and
  radio/option widgets
- Mouse support
- Integrate Nuklear, NxWidgets or some other library for a richer UI
- Optimise rendering by only updating the display with changes since last render
- Use expo to replace the existing menu implementation
- Add a Kconfig option to drop the names to save code / data space
- Add a Kconfig option to disable vidconsole support to save code / data space
- Support both graphical and text menus at the same time on different devices
- Implement proper measurement of object bounding boxes, to permit more exact
  layout. This would tidy up the layout when Truetype is not used
- Support unicode
- Support curses for proper serial-terminal menus

.. Simon Glass <sjg@chromium.org>
.. 7-Oct-22
