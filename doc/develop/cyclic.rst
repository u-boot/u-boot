.. SPDX-License-Identifier: GPL-2.0+

Cyclic functions
================

The cyclic function execution infrastruture provides a way to periodically
execute code, e.g. every 100ms. Examples for such functions might be LED
blinking etc. The functions that are hooked into this cyclic list should
be small timewise as otherwise the execution of the other code that relies
on a high frequent polling (e.g. UART rx char ready check) might be
delayed too much. To detect cyclic functions with an excessive execution
time, the Kconfig option `CONFIG_CYCLIC_MAX_CPU_TIME_US` was introduced.
It defines the maximum allowable execution time for such a cyclic function. The
first time the execution of a cyclic function exceeds this interval, a warning
will be displayed indicating the problem to the user.

Registering a cyclic function
-----------------------------

To register a cyclic function, use something like this::

    struct donkey {
        struct cyclic_info cyclic;
        void (*say)(const char *s);
    };

    static void cyclic_demo(struct cyclic_info *c)
    {
        struct donkey *donkey = container_of(c, struct donkey, cyclic);

        donkey->say("Are we there yet?");
    }

    int donkey_init(void)
    {
        struct donkey *donkey;

        /* Initialize donkey ... */

        /* Register demo cyclic function */
        cyclic_register(&donkey->cyclic, cyclic_demo, 10 * 1000, "cyclic_demo");
        
        return 0;
    }

This will register the function `cyclic_demo()` to be periodically
executed all 10ms.

How is this cyclic functionality integrated /  executed?
--------------------------------------------------------

The cyclic infrastructure integrates cyclic_run(), the main function
responsible for calling all registered cyclic functions, into the
common schedule() function. This guarantees that cyclic_run() is
executed very often, which is necessary for the cyclic functions to
get scheduled and executed at their configured periods.

Idempotence
-----------

Both the cyclic_register() and cyclic_unregister() functions are safe
to call on any struct cyclic_info, regardless of whether that instance
is already registered or not.

More specifically, calling cyclic_unregister() with a cyclic_info
which is not currently registered is a no-op, while calling
cyclic_register() with a cyclic_info which is currently registered
results in it being automatically unregistered, and then registered
with the new callback function and timeout parameters.
