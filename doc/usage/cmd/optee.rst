.. SPDX-License-Identifier: GPL-2.0

.. index::
   single: optee (command)

optee command
=============

Synopsis
--------

::

    optee hello
    optee hello <value>

Description
-----------

This is an OP-TEE sanity test which invokes the "Hello World"
Trusted Application (TA). The TA does two things:
- It prints debug and information messages to the secure console (if logging is enabled)
- It increments the integer value passed as a parameter and returns it


value
	Integer value that the TA is expected to increment and return.
	The default value is 0.

To enable the OP-TEE Hello World example please refer
https://optee.readthedocs.io/en/latest/building/gits/optee_examples/optee_examples.html

Examples
--------

::

	==> optee hello
	D/TA:  TA_CreateEntryPoint:39 has been called
	I/TA: Hello World!
	Value before: 0x0
	Calling TA
	D/TA:  inc_value:105 has been called
	I/TA: Got value: 0 from NW
	I/TA: Increase value to: 1
	Value after: 0x1
	I/TA: Goodbye!
	D/TA:  TA_DestroyEntryPoint:50 has been called

	==> optee hello 74
	D/TA:  TA_CreateEntryPoint:39 has been called
	I/TA: Hello World!
	Value before: 0x74
	Calling TA
	D/TA:  inc_value:105 has been called
	I/TA: Got value: 116 from NW
	I/TA: Increase value to: 117
	Value after: 0x75
	I/TA: Goodbye!
	D/TA:  TA_DestroyEntryPoint:50 has been called

Configuration
-------------

The optee command is enabled by CONFIG_OPTEE=y and CONFIG_CMD_OPTEE=y.

Return value
------------

The return value $? is 0 (true) if the command succeeds, 1 (false) otherwise.
