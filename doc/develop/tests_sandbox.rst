.. SPDX-License-Identifier: GPL-2.0+

Tests Under the Hood
====================

Running sandbox tests directly
------------------------------

Typically tests are run using the pytest suite. Running pytests on sandbox is
easy and always gets things right. For example some tests require files to be
set up before they can work.

But it is also possible to run some sandbox tests directly. For example, this
runs the dm_test_gpio() test which you can find in test/dm/gpio.c::

   $ ./u-boot -T -c "ut dm gpio"


   U-Boot 2021.01

   Model: sandbox
   DRAM:  128 MiB
   WDT:   Started with servicing (60s timeout)
   MMC:   mmc2: 2 (SD), mmc1: 1 (SD), mmc0: 0 (SD)
   In:    serial
   Out:   vidconsole
   Err:   vidconsole
   Model: sandbox
   SCSI:
   Net:   eth0: eth@10002000, eth5: eth@10003000, eth3: sbe5, eth6: eth@10004000
   Test: dm_test_gpio: gpio.c
   Test: dm_test_gpio: gpio.c (flat tree)
   Failures: 0

The -T option tells the U-Boot sandbox to run with the 'test' devicetree
(test.dts) instead of -D which selects the normal sandbox.dts - this is
necessary because many tests rely on nodes or properties in the test devicetree.
If you try running tests without -T then you may see failures, like::

   $ ./u-boot -c "ut dm gpio"


   U-Boot 2021.01

   DRAM:  128 MiB
   WDT:   Not found!
   MMC:
   In:    serial
   Out:   serial
   Err:   serial
   SCSI:
   Net:   No ethernet found.
   Please run with test device tree:
       ./u-boot -d arch/sandbox/dts/test.dtb
   Test: dm_test_gpio: gpio.c
   test/dm/gpio.c:37, dm_test_gpio(): 0 == gpio_lookup_name("b4", &dev, &offset, &gpio): Expected 0x0 (0), got 0xffffffea (-22)
   Test: dm_test_gpio: gpio.c (flat tree)
   test/dm/gpio.c:37, dm_test_gpio(): 0 == gpio_lookup_name("b4", &dev, &offset, &gpio): Expected 0x0 (0), got 0xffffffea (-22)
   Failures: 2

The message above should provide a hint if you forget to use the -T flag. Even
running with -D will produce different results.

You can easily use gdb on these tests, without needing --gdbserver::

   $ gdb u-boot --args -T -c "ut dm gpio"
   ...
   (gdb) break dm_test_gpio
   Breakpoint 1 at 0x1415bd: file test/dm/gpio.c, line 37.
   (gdb) run -T -c "ut dm gpio"
   Starting program: u-boot -T -c "ut dm gpio"
   Test: dm_test_gpio: gpio.c

   Breakpoint 1, dm_test_gpio (uts=0x5555558029a0 <global_dm_test_state>)
       at files/test/dm/gpio.c:37
   37		ut_assertok(gpio_lookup_name("b4", &dev, &offset, &gpio));
   (gdb)

You can then single-step and look at variables as needed.

