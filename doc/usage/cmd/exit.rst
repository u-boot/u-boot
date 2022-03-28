exit command
============

Synopsis
--------

::

    exit

Description
-----------

The exit command terminates a script started via the run or source command.
If scripts are nested, only the innermost script is left.

::

    => setenv inner 'echo entry inner; exit; echo inner done'
    => setenv outer 'echo entry outer; run inner; echo outer done'
    => run outer
    entry outer
    entry inner
    outer done
    =>

When executed outside a script a warning is written. Following commands are not
executed.

::

    => echo first; exit; echo last
    first
    exit not allowed from main input shell.
    =>

Return value
------------

$? is always set to 0 (true).
