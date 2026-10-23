.. SPDX-License-Identifier: GPL-2.0-or-later

Splash screen
=============

Enabling the splash screen
--------------------------

CONFIG_SPLASH_SCREEN shows a BMP image on the display during boot. The image is
taken from the hexadecimal address given by the environment variable
*splashimage*. If *splashimage* is not set, no splash screen is displayed.

Note that enabling this option also suppresses the compiled-in U-Boot logo (see
CONFIG_VIDEO_LOGO), whether or not *splashimage* is set, so a board that enables
CONFIG_SPLASH_SCREEN without setting *splashimage* shows nothing at all.

Normally the U-Boot version string is shown on the display once the splash
screen is enabled, since video starts up after U-Boot has displayed the initial
banner and the banner is otherwise not visible. CONFIG_HIDE_LOGO_VERSION hides
this version information.

Note that this version banner is only printed on the video console when the
splash image is left at its default position (0,0, see *splashpos* below). If
the image is moved away from 0,0, the banner is skipped regardless of
CONFIG_HIDE_LOGO_VERSION.

Preparing the splash image
--------------------------

The splash_screen_prepare() function is a weak function defined in
``common/splash.c``. It is called as part of the splash screen display sequence.
It gives the board an opportunity to prepare the splash image data before it is
processed and sent to the frame buffer by U-Boot. Define your own version to use
this feature.

If CONFIG_SPLASH_SOURCE is not enabled but CONFIG_VIDEO_LOGO is, the default
splash_screen_prepare() falls back to copying the compiled-in U-Boot logo to the
address given by *splashimage*, so it is shown as the splash image instead of a
board-supplied one. With neither enabled, no splash image is loaded.

Selecting the splash image source
---------------------------------

CONFIG_SPLASH_SOURCE enables the splash_source.c library. This library provides
facilities to declare board specific splash image locations, routines for
loading a splash image from supported locations, and a way of controlling the
selected splash location using the *splashsource* environment variable.

*splashsource* works as follows:

* If *splashsource* is set to a supported location name as defined by board
  code, use that splash location.
* If *splashsource* is undefined, use the first splash location as default.
* If *splashsource* is set to an unsupported value, do not load a splash screen.

A splash source location can describe either storage with raw data, a storage
formatted with a file system or a FIT image. In case of a filesystem, the splash
screen data is loaded as a file. The name of the splash screen file can be
controlled with the environment variable *splashfile*.

To enable loading the splash image from a FIT image, CONFIG_FIT must be enabled.
The FIT image has to start at the 'offset' field address in the selected splash
location. The name of the splash image within the FIT shall be specified by the
environment variable *splashfile*.

In case the environment variable *splashfile* is not defined the default name
``splash.bmp`` will be used.

For storage backed locations (MMC, SATA, USB, etc.) the device and partition to
read from can be overridden with the environment variable *splashdevpart*. Its
value follows the same ``<dev>[:<part>]`` or ``<dev>#<partition name>``
conventions used by the load commands, e.g. ``0:1`` or ``0#splash``. When
*splashdevpart* is not set, the ``devpart`` field of the board's splash location
entry is used instead.

Where the splash data starts in that storage -- the BMP itself for a raw
location, or the FIT for a FIT location -- can be overridden with the
environment variable *splashoffset*, given in hexadecimal. When *splashoffset*
is not set, the ``offset`` field of the board's splash location entry is used
instead. Only the SPI flash and NAND backends read from the offset at all; MMC
always starts at the beginning of the partition selected by *splashdevpart*.

The default weak splash_screen_prepare() passes the ``default_splash_locations``
array, defined in ``common/splash.c``, to splash_source_load(). It provides the
following *splashsource* names out of the box:

======== ============ ================== ===============
Name     Storage      Type               devpart default
======== ============ ================== ===============
sf       SPI flash    raw, offset 0x0    n/a
mmc_fs   MMC          file system        ``0:1``
mmc_raw  MMC          raw                ``0:1``
usb_fs   USB          file system        ``0:1``
sata_fs  SATA         file system        ``0:1``
scsi_fs  SCSI         file system        ``0:1``
======== ============ ================== ===============

A board that needs different locations, other storage backends (e.g. NAND,
UBI/UBIFS) or a different devpart default should provide its own array of
``struct splash_location`` entries (see ``include/splash.h``) and call
splash_source_load() from its own splash_screen_prepare() implementation
instead of relying on the default one.

Positioning the splash image
----------------------------

If CONFIG_SPLASH_SCREEN_ALIGN is enabled, the splash image can be freely
positioned on the display using the environment variable *splashpos*, given as
``x,y``:

* A positive number is the number of pixels from the left/top.
* A negative number is the number of pixels from the right/bottom.
* ``m`` centers the image on that axis.

Examples::

  setenv splashpos m,m
  	=> image at center of screen

  setenv splashpos 30,20
  	=> image at x = 30 and y = 20

  setenv splashpos -10,m
  	=> vertically centered image
  	   at x = dspWidth - bmpWidth - 9

Splash screen in SPL
--------------------

The splash screen feature is also available in SPL, controlled by the SPL
counterparts of the above options: CONFIG_SPL_SPLASH_SCREEN,
CONFIG_SPL_SPLASH_SCREEN_ALIGN, CONFIG_SPL_SPLASH_SOURCE and
CONFIG_SPL_HIDE_LOGO_VERSION. They behave the same as their non-SPL
equivalents, but apply only at the SPL stage, and are selected and
configured independently of the U-Boot proper options.
