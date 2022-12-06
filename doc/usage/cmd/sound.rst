.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2022, Heinrich Schuchardt <xypron.glpk@gmx.de>

sound command
=============

Synopsis
--------

::

    sound init
    sound play [len [freq]]

Description
-----------

The *sound* command is used to play a beep sound.

sound init
    initializes the sound driver.

sound play
    plays a square wave sound. It does not depend on previously calling
    *sound init*.

len
    duration of the sound in ms, defaults to 1000 ms

freq
    frequency of the sound in Hz, defaults to 400 Hz

Configuration
-------------

The sound command is enabled by CONFIG_CMD_SOUND=y.

Return value
------------

The return value $? is 0 (true) if the command succeeds, 1 (false) otherwise.
