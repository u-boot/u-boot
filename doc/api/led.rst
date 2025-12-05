.. SPDX-License-Identifier: GPL-2.0+

LED
===

.. kernel-doc:: include/led.h
   :doc: Overview

.. kernel-doc:: include/led.h
   :internal:

Legacy LED
==========

Please use the new LED API as defined above. This section is only for reference
for currently supported devices and to aid for migration to the new API.

Status LED
----------

This README describes the status LED API.

The API is defined by the include file include/status_led.h

The first step is to enable CONFIG_LED_STATUS in menuconfig::

   > Device Drivers > LED Support.

If the LED support is only for specific board, enable
CONFIG_LED_STATUS_BOARD_SPECIFIC in the menuconfig.

Status LEDS 0 to 5 are enabled by the following configurations at menuconfig:
CONFIG_STATUS_LED0, CONFIG_STATUS_LED1, ... CONFIG_STATUS_LED5

The following should be configured for each of the enabled LEDs:

- CONFIG_STATUS_LED_BIT<n>
- CONFIG_STATUS_LED_STATE<n>
- CONFIG_STATUS_LED_FREQ<n>

Where <n> is an integer 1 through 5 (empty for 0).

CONFIG_STATUS_LED_BIT is passed into the __led_* functions to identify which LED
is being acted on. As such, the value choose must be unique with respect to
the other CONFIG_STATUS_LED_BIT's. Mapping the value to a physical LED is the
reponsiblity of the __led_* function.

CONFIG_STATUS_LED_STATE is the initial state of the LED. It should be set to one
of these values: CONFIG_LED_STATUS_OFF or CONFIG_LED_STATUS_ON.

CONFIG_STATUS_LED_FREQ determines the LED blink frequency.
Values range from 2 to 10.

Some other LED macros
~~~~~~~~~~~~~~~~~~~~~

CONFIG_STATUS_LED_BOOT is the LED to light when the board is booting.
This must be a valid LED number (0-5).

General LED functions
~~~~~~~~~~~~~~~~~~~~~
The following functions should be defined:

__led_init is called once to initialize the LED to CONFIG_STATUS_LED_STATE.
One time start up code should be placed here.

__led_set is called to change the state of the LED.

__led_toggle is called to toggle the current state of the LED.

TBD : Describe older board dependent macros similar to what is done for

TBD : Describe general support via asm/status_led.h
