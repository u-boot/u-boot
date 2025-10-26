.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: askenv (command)

askenv command
==============

Synopsis
--------

::

    askenv name [message] [size]

Description
-----------

Display message and get environment variable name of max size characters
from stdin.

See also *env ask* in :doc:`env`.

name
    name of the environment variable

message
    message is displayed while the command waits for the  value to be
    entered from stdin.if no message is specified,a default message
    "Please enter name:" will be displayed.

size
    maximum number of characters that will be stored in environment
    variable name.this is in decimal number format (unlike in
    other commands where size values are in hexa-decimal). Default
    value of size is 1023 (CONFIG_SYS_CBSIZE - 1).

Example
-------

Value of a environment variable env1 without message and size parameters:

::

    => askenv env1
    Please enter 'env1': val1
    => printenv env1
    env1=val1

Value of a environment variable env2 with message and size parameters:

::

    => askenv env2 Please type-in a value for env2: 10
    Please type-in a value for env2: 1234567890123
    => printenv env2
    env2=1234567890

Value of a environment variable env3 with size parameter only:

::

    => askenv env3 10
    Please enter 'env3': val3
    => printenv env3
    env3=val3

Configuration
-------------

The askenv command is only available if CMD_ASKENV=y
