.. SPDX-License-Identifier: GPL-2.0-or-later

.. index::
   single: pwm (command)

pwm command
===========

Synopsis
--------

::

    pwm invert <pwm_dev_num> <channel> <polarity>
    pwm config <pwm_dev_num> <channel> <period_ns> <duty_ns>
    pwm enable <pwm_dev_num> <channel>
    pwm disable <pwm_dev_num> <channel>


Description
-----------

The ``pwm`` command is used to access and configure PWM (Pulse Width Modulation)
signals.

pwm invert
----------

* If the value of ``polarity`` is 0, the default polarity is used.
* If the value of ``polarity`` is 1, the polarity is inverted.

pwm config
----------

Configure the period and duty period in nanoseconds.

pwm enable
----------

Enable output on the configured device and channel.

pwm disable
-----------

Disable output on the configured device and channel.

pwm_dev_num
    Device number of the pulse width modulation device

channel
    Output channel of the PWM device

polarity
    * 0 - Use normal polarity
    * 1 - Use inverted polarity

duty_ns
    Duty period in ns

period_ns
    Period time in ns

Examples
--------

Configure device 0, channel 0 to 20 µs period and 14 µs (that is, 70%) duty period::

    => pwm config 0 0 20000 14000

Enable output on the configured device and channel::

    => pwm enable 0 0

Disable output on the configured device and channel::

    => pwm disable 0 0

Invert the signal on the configured device and channel::

    => pwm invert 0 0 1

Configuration
-------------

The ``pwm`` command is only available if CONFIG_CMD_PWM=y.

Return value
------------

If the command succeeds, the return value ``$?`` is set to 0. If an error occurs, the
return value ``$?`` is set to 1.
