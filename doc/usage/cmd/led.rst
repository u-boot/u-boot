.. SPDX-License-Identifier: GPL-2.0-or-later

.. index::
   single: led (command)

led command
===========

Synopsis
--------

::

    led <led_label> <on|off|toggle>
    led <led_label> blink <blink-freq in ms>
    led <led_label>
    led list

led <led_label>
---------------

Get state of the LED whose label is the one passed as ``<led_label>`` argument.

Provided ``<led_label>`` is *module_led*, the possible outputs for this command
are::

    LED 'module_led': off
    LED 'module_led': on
    LED 'module_led': blink

led <led_label> on|off|toggle
-----------------------------

Turn on, off or toggle state of the LED whose label is the one passed as
``<led_label>`` argument.

led <led_label> blink <blink-freq in ms>
----------------------------------------

Make the LED whose label is the one passed as ``<led_label>`` argument blink at
a frequency specified by the argument ``<blink-freq in ms>``.

The frequency is parsed as a decimal number and its unit is milliseconds. The
duty cycle is 50%. Example::

    led blue blink 1000

will make the *blue*-labeled LED blink with a state (on or off) kept for 500ms
before switching to the other state (respectively off or on) for 500ms,
looping endlessly.

led list
--------

List all available LEDs by their label and provide their known state, which can
be either *off*, *on* or *blink*.

If a LED has not been probed yet, its state will be shown as *<inactive>* in the
list.

Examples
--------

::

    => led list
    module_led      on
    sd_card_led     <inactive>
    => led module_led
    LED 'module_led': on
    => led module_led off
    => led module_led
    LED 'module_led': off
    => led module_led toggle
    => led module_led
    LED 'module_led': on
    => led module_led toggle
    => led module_led
    LED 'module_led': off
    => led module_led blink 1000
    => led module_led
    LED 'module_led': blink
    => led sd_card_led
    LED 'sd_card_led': off
    => led list
    module_led      blink
    sd_card_led     off

Configuration
-------------

The *led* commands are only available if ``CONFIG_CMD_LED=y``.

The *led <led_label> blink* command is only available if ``CONFIG_CMD_LED=y``
and either ``CONFIG_LED_BLINK=y`` or ``CONFIG_LED_SW_BLINK=y``.
