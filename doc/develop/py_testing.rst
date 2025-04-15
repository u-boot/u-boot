U-Boot pytest suite
===================

Introduction
------------

This tool aims to test U-Boot by executing U-Boot shell commands using the
console interface. A single top-level script exists to execute or attach to the
U-Boot console, run the entire script of tests against it, and summarize the
results. Advantages of this approach are:

- Testing is performed in the same way a user or script would interact with
  U-Boot; there can be no disconnect.
- There is no need to write or embed test-related code into U-Boot itself.
  It is asserted that writing test-related code in Python is simpler and more
  flexible than writing it all in C. But see :doc:`tests_writing` for caveats
  and more discussion / analysis.
- It is reasonably simple to interact with U-Boot in this way.

Requirements
------------

The test suite is implemented using pytest. Interaction with the U-Boot console
involves executing some binary and interacting with its stdin/stdout. You will
need to implement various "hook" scripts that are called by the test suite at
the appropriate time.

In order to run the test suite at a minimum we require that both Python 3 and
pip for Python 3 are installed. All of the required python modules are
described in the requirements.txt file in the /test/py/ directory and can be
installed via the command

.. code-block:: bash

   pip install -r requirements.txt

In order to execute certain tests on their supported platforms other tools
will be required. The following is an incomplete list:

* gdisk
* dfu-util
* dtc
* openssl
* e2fsprogs
* util-linux
* coreutils
* dosfstools
* efitools
* mount
* mtools
* sbsigntool
* udisks2

Please use the appropriate commands for your distribution to match these tools
up with the package that provides them.

The test script supports either:

- Executing a sandbox port of U-Boot on the local machine as a sub-process,
  and interacting with it over stdin/stdout.
- Executing an external "hook" scripts to flash a U-Boot binary onto a
  physical board, attach to the board's console stream, and reset the board.
  Further details are described later.

The usage of the command 'sudo' is not allowed in tests. Using elevated
priviledges can lead to security concerns. Furthermore not all users may have
administrator rights. Therefore the command 'sudo' must not be used in tests.
To create disk images we have helper functions located in
`test/py/tests/fs_helper.py` which shall be used in any tests that require
creating disk images.

Using `virtualenv` to provide requirements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The recommended way to run the test suite, in order to ensure reproducibility
is to use `virtualenv` to set up the necessary environment.  This can be done
via the following commands:


.. code-block:: console

    $ cd /path/to/u-boot
    $ sudo apt-get install python3 python3-virtualenv
    $ virtualenv -p /usr/bin/python3 venv
    $ . ./venv/bin/activate
    $ pip install -r test/py/requirements.txt

Testing sandbox
---------------

To run the test suite on the sandbox port (U-Boot built as a native user-space
application), simply execute:

.. code-block:: bash

    ./test/py/test.py --bd sandbox --build

The `--bd` option tells the test suite which board type is being tested. This
lets the test suite know which features the board has, and hence exactly what
can be tested.

The `--build` option tells U-Boot to compile U-Boot. Alternatively, you may
omit this option and build U-Boot yourself, in whatever way you choose, before
running the test script.

The test script will attach to U-Boot, execute all valid tests for the board,
then print a summary of the test process. A complete log of the test session
will be written to `${build_dir}/test-log.html`. This is best viewed in a web
browser, but may be read directly as plain text, perhaps with the aid of the
`html2text` utility.

If sandbox crashes (e.g. with a segfault) you will see message like this::


    test/py/spawn.py:171: in expect
        c = os.read(self.fd, 1024).decode(errors='replace')
    E   ValueError: U-Boot exited with signal 11 (Signals.SIGSEGV)


Controlling output
~~~~~~~~~~~~~~~~~~

By default a short backtrace is reported. If you would like a longer one,
pass ``--tb=long`` when running the test. See the pytest documentation for
more options.

Running tests in parallel
~~~~~~~~~~~~~~~~~~~~~~~~~

Note: Not all tests can run in parallel at present, so the usual approach is
to just run those that can.

First install support for parallel tests::

    sudo apt install python3-pytest-xdist

or:::

    pip3 install pytest-xdist

Then run the tests in parallel using the -n flag::

    test/py/test.py -B sandbox --build --build-dir /tmp/b/sandbox -q -k \
        'not slow and not bootstd and not spi_flash' -n16

You can also use `make pcheck` to run all tests in parallel. This uses a maximum
of 16 threads, since the setup time is significant and there are under 1000
tests.

Note that the `test-log.html` output does not work correctly at present with
parallel testing. All the threads write to it at once, so it is garbled.

Note that the `tools/` tests still run each tool's tests once after the other,
although within that, they do run in parallel. So for example, the buildman
tests run in parallel, then the binman tests run in parallel. There would be a
significant advantage to running them all in parallel together, but that would
require a large amount of refactoring, e.g. with more use of pytest fixtures.
The code-coverage tests are omitted since they cannot run in parallel due to a
Python limitation.


Testing under a debugger
~~~~~~~~~~~~~~~~~~~~~~~~

If you need to run sandbox under a debugger, you may pass the command-line
option `--gdbserver COMM`. This causes two things to happens:

- Instead of running U-Boot directly, it will be run under gdbserver, with
  debug communication via the channel `COMM`. You can attach a debugger to the
  sandbox process in order to debug it. See `man gdbserver` and the example
  below for details of valid values for `COMM`.
- All timeouts in tests are disabled, allowing U-Boot an arbitrary amount of
  time to execute commands. This is useful if U-Boot is stopped at a breakpoint
  during debugging.

A usage example is:

Window 1:

.. code-block:: bash

    ./test/py/test.py --bd sandbox --gdbserver localhost:1234

Window 2:

.. code-block:: bash

    gdb ./build-sandbox/u-boot -ex 'target remote localhost:1234'

Alternatively, you could leave off the `-ex` option and type the command
manually into gdb once it starts.

You can use any debugger you wish, as long as it speaks the gdb remote
protocol, or any graphical wrapper around gdb.

Some tests deliberately cause the sandbox process to exit, e.g. to test the
reset command, or sandbox's CTRL-C handling. When this happens, you will need
to attach the debugger to the new sandbox instance. If these tests are not
relevant to your debugging session, you can skip them using pytest's -k
command-line option; see the next section.

Command-line options
--------------------

--board-type, --bd, -B
  set the type of the board to be tested. For example, `sandbox` or `seaboard`.

--board-identity`, --id
  sets the identity of the board to be tested. This allows differentiation
  between multiple instances of the same type of physical board that are
  attached to the same host machine. This parameter is not interpreted by th
  test script in any way, but rather is simply passed to the hook scripts
  described below, and may be used in any site-specific way deemed necessary.

--build
  indicates that the test script should compile U-Boot itself before running
  the tests. If using this option, make sure that any environment variables
  required by the build process are already set, such as `$CROSS_COMPILE`.

--buildman
  indicates that `--build` should use buildman to build U-Boot. There is no need
  to set $CROSS_COMPILE` in this case since buildman handles it.

--build-dir
  sets the directory containing the compiled U-Boot binaries. If omitted, this
  is `${source_dir}/build-${board_type}`.

--result-dir
  sets the directory to write results, such as log files, into.
  If omitted, the build directory is used.

--persistent-data-dir
  sets the directory used to store persistent test data. This is test data that
  may be re-used across test runs, such as file-system images.

--timing
  shows a histogram of test duration, at the end of the run. The columns are:

  Duration
      the duration-bucket that this test was in

  Total
      total time of all tests in this bucket

  Number of tests
      graph showing the number of tests in this bucket, with the actual number
      shown at the end

  Example::

    Duration :   Total  | Number of tests
    ======== : =======  |========================================
       <20ms :   418ms  |## 23
       <30ms :    9.1s  |######################################## 347
       <40ms :   10.0s  |################################# 294
       <50ms :    3.1s  |####### 69
       <75ms :    2.6s  |#### 43
      <100ms :    1.7s  |## 19
      <200ms :    3.0s  |## 22
      <300ms :    1.7s  | 7
      <400ms :   675ms  | 2
      <500ms :    2.2s  | 5
      <750ms :    8.3s  |# 13
       <1.0s :    1.6s  | 2
       <2.0s :    9.4s  | 7
       <3.0s :    2.4s  | 1
       <7.5s :    6.1s  | 1

`pytest` also implements a number of its own command-line options. Commonly used
options are mentioned below. Please see `pytest` documentation for complete
details. Execute `py.test --version` for a brief summary. Note that U-Boot's
test.py script passes all command-line arguments directly to `pytest` for
processing.

-k
  selects which tests to run. The default is to run all known tests. This
  option takes a single argument which is used to filter test names. Simple
  logical operators are supported. For example:

  - `'-k ums'` runs only tests with "ums" in their name.
  - `'-k ut_dm'` runs only tests with "ut_dm" in their name. Note that in this
    case, "ut_dm" is a parameter to a test rather than the test name. The full
    test name is e.g. "test_ut[ut_dm_leak]".
  - `'-k not reset'` runs everything except tests with "reset" in their name.
  - `'-k ut or hush'` runs only tests with "ut" or "hush" in their name.
  - `'-k not (ut or hush)'` runs everything except tests with "ut" or "hush" in
    their name.

-s
  prevents pytest from hiding a test's stdout. This allows you to see
  U-Boot's console log in real time on pytest's stdout.

Testing real hardware
---------------------

The tools and techniques used to interact with real hardware will vary
radically between different host and target systems, and the whims of the user.
For this reason, the test suite does not attempt to directly interact with real
hardware in any way. Rather, it executes a standardized set of "hook" scripts
via `$PATH`. These scripts implement certain actions on behalf of the test
suite. This keeps the test suite simple and isolated from system variances
unrelated to U-Boot features.

Hook scripts
~~~~~~~~~~~~

Environment variables
'''''''''''''''''''''

The following environment variables are set when running hook scripts:

- `UBOOT_BOARD_TYPE` the board type being tested.
- `UBOOT_BOARD_IDENTITY` the board identity being tested, or `na` if none was
  specified.
- `UBOOT_SOURCE_DIR` the U-Boot source directory.
- `UBOOT_TEST_PY_DIR` the full path to `test/py/` in the source directory.
- `UBOOT_BUILD_DIR` the U-Boot build directory.
- `UBOOT_RESULT_DIR` the test result directory.
- `UBOOT_PERSISTENT_DATA_DIR` the test persistent data directory.

u-boot-test-console
'''''''''''''''''''

This script provides access to the U-Boot console. The script's stdin/stdout
should be connected to the board's console. This process should continue to run
indefinitely, until killed. The test suite will run this script in parallel
with all other hooks.

This script may be implemented e.g. by executing `cu`, `kermit`, `conmux`, etc.
via exec().

If you are able to run U-Boot under a hardware simulator such as QEMU, then
you would likely spawn that simulator from this script. However, note that
`u-boot-test-reset` may be called multiple times per test script run, and must
cause U-Boot to start execution from scratch each time. Hopefully your
simulator includes a virtual reset button! If not, you can launch the
simulator from `u-boot-test-reset` instead, while arranging for this console
process to always communicate with the current simulator instance.

u-boot-test-flash
'''''''''''''''''

Prior to running the test suite against a board, some arrangement must be made
so that the board executes the particular U-Boot binary to be tested. Often
this involves writing the U-Boot binary to the board's flash ROM. The test
suite calls this hook script for that purpose.

This script should perform the entire flashing process synchronously; the
script should only exit once flashing is complete, and a board reset will
cause the newly flashed U-Boot binary to be executed.

It is conceivable that this script will do nothing. This might be useful in
the following cases:

- Some other process has already written the desired U-Boot binary into the
  board's flash prior to running the test suite.
- The board allows U-Boot to be downloaded directly into RAM, and executed
  from there. Use of this feature will reduce wear on the board's flash, so
  may be preferable if available, and if cold boot testing of U-Boot is not
  required. If this feature is used, the `u-boot-test-reset` script should
  perform this download, since the board could conceivably be reset multiple
  times in a single test run.

It is up to the user to determine if those situations exist, and to code this
hook script appropriately.

This script will typically be implemented by calling out to some SoC- or
board-specific vendor flashing utility.

u-boot-test-reset
'''''''''''''''''

Whenever the test suite needs to reset the target board, this script is
executed. This is guaranteed to happen at least once, prior to executing the
first test function. If any test fails, the test infra-structure will execute
this script again to restore U-Boot to an operational state before running the
next test function.

This script will likely be implemented by communicating with some form of
relay or electronic switch attached to the board's reset signal.

The semantics of this script require that when it is executed, U-Boot will
start running from scratch. If the U-Boot binary to be tested has been written
to flash, pulsing the board's reset signal is likely all this script needs to
do. However, in some scenarios, this script may perform other actions. For
example, it may call out to some SoC- or board-specific vendor utility in order
to download the U-Boot binary directly into RAM and execute it. This would
avoid the need for `u-boot-test-flash` to actually write U-Boot to flash, thus
saving wear on the flash chip(s).

Examples
''''''''

https://source.denx.de/u-boot/u-boot-test-hooks contains some working example hook
scripts, and may be useful as a reference when implementing hook scripts for
your platform. These scripts are not considered part of U-Boot itself.

Board-type-specific configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each board has a different configuration and behaviour. Many of these
differences can be automatically detected by parsing the `.config` file in the
build directory. However, some differences can't yet be handled automatically.

For each board, an optional Python module `u_boot_board_${board_type}` may exist
to provide board-specific information to the test script. Any global value
defined in these modules is available for use by any test function. The data
contained in these scripts must be purely derived from U-Boot source code.
Hence, these configuration files are part of the U-Boot source tree too.

Execution environment configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each user's hardware setup may enable testing different subsets of the features
implemented by a particular board's configuration of U-Boot. For example, a
U-Boot configuration may support USB device mode and USB Mass Storage, but this
can only be tested if a USB cable is connected between the board and the host
machine running the test script.

For each board, optional Python modules `u_boot_boardenv_${board_type}` and
`u_boot_boardenv_${board_type}_${board_identity}` may exist to provide
board-specific and board-identity-specific information to the test script. Any
global value defined in these modules is available for use by any test
function. The data contained in these is specific to a particular user's
hardware configuration. Hence, these configuration files are not part of the
U-Boot source tree, and should be installed outside of the source tree. Users
should set `$PYTHONPATH` prior to running the test script to allow these
modules to be loaded.

Board module parameter usage
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The test scripts rely on the following variables being defined by the board
module:

- none at present

U-Boot `.config` feature usage
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The test scripts rely on various U-Boot `.config` features, either directly in
order to test those features, or indirectly in order to query information from
the running U-Boot instance in order to test other features.

One example is that testing of the `md` command requires knowledge of a RAM
address to use for the test. This data is parsed from the output of the
`bdinfo` command, and hence relies on CONFIG_CMD_BDI being enabled.

For a complete list of dependencies, please search the test scripts for
instances of:

- `buildconfig.get(...`
- `@pytest.mark.buildconfigspec(...`
- `@pytest.mark.notbuildconfigspec(...`

Complete invocation example
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Assuming that you have installed the hook scripts into $HOME/ubtest/bin, and
any required environment configuration Python modules into $HOME/ubtest/py,
then you would likely invoke the test script as follows:

If U-Boot has already been built:

.. code-block:: bash

    PATH=$HOME/ubtest/bin:$PATH \
    PYTHONPATH=${HOME}/ubtest/py/${HOSTNAME}:${PYTHONPATH} \
    ./test/py/test.py --bd seaboard

If you want the test script to compile U-Boot for you too, then you likely
need to set `$CROSS_COMPILE` to allow this, and invoke the test script as
follows:

.. code-block:: bash

    CROSS_COMPILE=arm-none-eabi- \
    PATH=$HOME/ubtest/bin:$PATH \
    PYTHONPATH=${HOME}/ubtest/py/${HOSTNAME}:${PYTHONPATH} \
    ./test/py/test.py --bd seaboard --build

or, using buildman to handle it:

.. code-block:: bash

    PATH=$HOME/ubtest/bin:$PATH \
    PYTHONPATH=${HOME}/ubtest/py/${HOSTNAME}:${PYTHONPATH} \
    ./test/py/test.py --bd seaboard --build --buildman

Writing tests
-------------

Please refer to the pytest documentation for details of writing pytest tests.
Details specific to the U-Boot test suite are described below.

A test fixture named `ubman` should be used by each test function. This
provides the means to interact with the U-Boot console, and retrieve board and
environment configuration information.

The function `ubman.run_command()` executes a shell command on the
U-Boot console, and returns all output from that command. This allows
validation or interpretation of the command output. This function validates
that certain strings are not seen on the U-Boot console. These include shell
error messages and the U-Boot sign-on message (in order to detect unexpected
board resets). See the source of `console_base.py` for a complete list of
"bad" strings. Some test scenarios are expected to trigger these strings. Use
`ubman.disable_check()` to temporarily disable checking for specific
strings. See `test_unknown_cmd.py` for an example.

Board- and board-environment configuration values may be accessed as sub-fields
of the `ubman.config` object, for example
`ubman.config.ram_base`.

Build configuration values (from `.config`) may be accessed via the dictionary
`ubman.config.buildconfig`, with keys equal to the Kconfig variable
names.
