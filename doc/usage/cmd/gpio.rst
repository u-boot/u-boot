.. SPDX-License-Identifier: GPL-2.0+:

gpio command
============

Synopsis
--------

::

    gpio <input|set|clear|toggle> <pin>
    gpio read <name> <pin>
    gpio status [-a] [<bank>|<pin>]

The gpio command is used to access General Purpose Inputs/Outputs.

gpio input
----------

Switch the GPIO *pin* to input mode.

gpio set
--------

Switch the GPIO *pin* to output mode and set the signal to 1.

gpio clear
----------

Switch the GPIO *pin* to output mode and set the signal to 0.

gpio toggle
-----------

Switch the GPIO *pin* to output mode and reverse the signal state.

gpio read
---------

Read the signal state of the GPIO *pin* and save it in environment variable
*name*.

gpio status
-----------

Display the status of one or multiple GPIOs. By default only claimed GPIOs
are displayed.

-a
    Display GPIOs irrespective of being claimed.

bank
    Name of a bank of GPIOs to be displayed.

pin
    Name of a single GPIO to be displayed or manipulated.

Examples
--------

Switch the status of a GPIO::

    => gpio set a5
    gpio: pin a5 (gpio 133) value is 1
    => gpio clear a5
    gpio: pin a5 (gpio 133) value is 0
    => gpio toggle a5
    gpio: pin a5 (gpio 133) value is 1
    => gpio read myvar a5
    gpio: pin a5 (gpio 133) value is 1
    => echo $myvar
    1
    => gpio toggle a5
    gpio: pin a5 (gpio 133) value is 0
    => gpio read myvar a5
    gpio: pin a5 (gpio 133) value is 0
    => echo $myvar
    0

Configuration
-------------

The *gpio* command is only available if CONFIG_CMD_GPIO=y.
The *gpio read* command is only available if CONFIG_CMD_GPIO_READ=y.

Return value
------------

If the command succeds the return value $? is set to 0. If an error occurs, the
return value $? is set to 1.
