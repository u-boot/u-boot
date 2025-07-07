.. SPDX-License-Identifier: GPL-2.0+

.. toctree::
   :maxdepth: 1

Binman Tests
============

.. contents::
   :depth: 2
   :local:

There is some material on writing tests in the main Binman documentation
(see :doc:`package/index`). This short guide is separate so people don't
feel they have to read as much.

Code and output is mostly included verbatim, which makes the doc longer, but
avoids its becoming confusing when the output or referenced code changes in the
future.

Purpose
-------

The main purpose of tests in Binman is to make sure that Binman actually does
what it is supposed to. Various people contribute code, refactoring is done
over time, but U-Boot users (developers, SoC vendors, board vendors) rely on
Binman producing images which function correctly. Without tests, a one-line
change could unintentionally break a corner-case and the problem might not be
noticed for months. Debugging an image-generation problem with a board you
don't have can be very hard.

A secondary purpose is productivity. U-Boot contributors are busy and often
have too much on their plate. Trying to figure out why their patch broke
some other vendor's workflow can be very time-consuming and frustrating. By
building in tests from the start, this is largely avoided. If your change has
full test coverage and doesn't break any test, all is well and no one can
complain.

A lessor purpose is to document what Binman actually does. If a test covers a
feature, it works. If there is no test coverage, no one can say for sure
whether it works in all expected situations, certainly not wihout manual
effort.

In fact, strictly speaking it isn't completely clear what 'works' even means in
the case where these is no test to cover the code. We are often left guessing
as to what the documentation means, what was actually intended, etc.

Finally, code-coverage helps to remove 'zombie code', copied from elsewhere
because it looks reasonable, but not actually needed. The same situation arises
in silicon-chip design, where a part of the chip is not validated. If it isn't
validated, it can be assumed not to work, either now or later, so it is best to
remove that logic to avoid it causing problems.

Setting up
----------

Binman tests use various utility programs. Most of these are documented in
:doc:`../build/gcc`. But some are SoC-specific. To fetch these, tell Binman to
fetch or build any missing tools:

.. code-block:: bash

    $ binman tool -f missing

When this completes successfully, you can list the tools. You should see
something like this:

.. code-block:: bash

    $ binman tool -l
    Name             Version      Description                Path
    ---------------  -----------  -------------------------  ------------------------------
    bootgen          ****** Bootg Xilinx Bootgen             /home/sglass/.binman-tools/bootgen
    bzip2            1.0.8        bzip2 compression          /usr/bin/bzip2
    cbfstool         unknown      Manipulate CBFS files      /home/sglass/bin/cbfstool
    fdt_add_pubkey   unknown      Generate image for U-Boot  /home/sglass/bin/fdt_add_pubkey
    fdtgrep          unknown      Grep devicetree files      /home/sglass/bin/fdtgrep
    fiptool          v2.11.0(rele Manipulate ATF FIP files   /home/sglass/.binman-tools/fiptool
    futility         v0.0.1-9f2e9 Chromium OS firmware utili /home/sglass/.binman-tools/futility
    gzip             1.12         gzip compression           /usr/bin/gzip
    ifwitool         unknown      Manipulate Intel IFWI file /home/sglass/.binman-tools/ifwitool
    lz4              v1.9.4       lz4 compression            /usr/bin/lz4
    lzma_alone       9.22 beta    lzma_alone compression     /usr/bin/lzma_alone
    lzop             v1.04        lzo compression            /usr/bin/lzop
    mkeficapsule     2024.10-rc5- mkeficapsule tool for gene /home/sglass/bin/mkeficapsule
    mkimage          2024.10-rc5- Generate image for U-Boot  /home/sglass/bin/mkimage
    openssl          3.0.13 30 Ja openssl cryptography toolk /usr/bin/openssl
    xz               5.4.5        xz compression             /usr/bin/xz
    zstd             v1.5.5       zstd compression           /usr/bin/zstd

The tools are written to ``~/.binman-tools`` so add that to your ``PATH``.
It's fine to have some of the tools elsewhere (e.g. ``~/bin``) so long as they
are up-to-date. This allows you use the version of the tools intended for
running tests.

Now you should be able to actually run the tests:

.. code-block:: bash

    $ binman test
    ======================== Running binman tests ========================
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ........
    ----------------------------------------------------------------------
    Ran 568 tests in 2.578s

    OK

If this doesn't work, see if you can have some missing tools. Check that the
dependencies are all there as above. If it is very slow, try installing
concurrencytest so that the tests run in parallel.

The next thing to set up is code coverage, using the -T flag:

.. code-block:: bash

    $ binman test -T
    ======================== Running binman tests ========================
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ......................................................................
    ........
    ----------------------------------------------------------------------
    Ran 568 tests in 17.367s

    OK

    99%
    Name                                                    Stmts   Miss  Cover
    ---------------------------------------------------------------------------
    tools/binman/__init__.py                                    0      0   100%
    tools/binman/bintool.py                                   263      0   100%
    tools/binman/btool/bootgen.py                              21      0   100%
    tools/binman/btool/btool_gzip.py                            5      0   100%
    tools/binman/btool/bzip2.py                                 5      0   100%
    tools/binman/btool/cbfstool.py                             24      0   100%
    tools/binman/btool/cst.py                                  15      4    73%
    tools/binman/btool/fdt_add_pubkey.py                       21      0   100%
    tools/binman/btool/fdtgrep.py                              26      0   100%
    tools/binman/btool/fiptool.py                              19      0   100%
    tools/binman/btool/futility.py                             19      0   100%
    tools/binman/btool/ifwitool.py                             22      0   100%
    tools/binman/btool/lz4.py                                  22      0   100%
    tools/binman/btool/lzma_alone.py                           34      0   100%
    tools/binman/btool/lzop.py                                  5      0   100%
    tools/binman/btool/mkeficapsule.py                         27      0   100%
    tools/binman/btool/mkimage.py                              23      0   100%
    tools/binman/btool/openssl.py                              42      0   100%
    tools/binman/btool/xz.py                                    5      0   100%
    tools/binman/btool/zstd.py                                  5      0   100%
    tools/binman/cbfs_util.py                                 376      0   100%
    tools/binman/cmdline.py                                    90      0   100%
    tools/binman/control.py                                   409      0   100%
    tools/binman/elf.py                                       241      0   100%
    tools/binman/entry.py                                     548      0   100%
    tools/binman/etype/alternates_fdt.py                       58      0   100%
    tools/binman/etype/atf_bl31.py                              5      0   100%
    tools/binman/etype/atf_fip.py                              67      0   100%
    tools/binman/etype/blob.py                                 49      0   100%
    tools/binman/etype/blob_dtb.py                             46      0   100%
    tools/binman/etype/blob_ext.py                              9      0   100%
    tools/binman/etype/blob_ext_list.py                        32      0   100%
    tools/binman/etype/blob_named_by_arg.py                     9      0   100%
    tools/binman/etype/blob_phase.py                           22      0   100%
    tools/binman/etype/cbfs.py                                101      0   100%
    tools/binman/etype/collection.py                           30      0   100%
    tools/binman/etype/cros_ec_rw.py                            5      0   100%
    tools/binman/etype/efi_capsule.py                          59      0   100%
    tools/binman/etype/efi_empty_capsule.py                    33      0   100%
    tools/binman/etype/encrypted.py                            34      0   100%
    tools/binman/etype/fdtmap.py                               62      0   100%
    tools/binman/etype/files.py                                35      0   100%
    tools/binman/etype/fill.py                                 13      0   100%
    tools/binman/etype/fit.py                                 311      0   100%
    tools/binman/etype/fmap.py                                 37      0   100%
    tools/binman/etype/gbb.py                                  37      0   100%
    tools/binman/etype/image_header.py                         53      0   100%
    tools/binman/etype/intel_cmc.py                             4      0   100%
    tools/binman/etype/intel_descriptor.py                     39      0   100%
    tools/binman/etype/intel_fit.py                            12      0   100%
    tools/binman/etype/intel_fit_ptr.py                        17      0   100%
    tools/binman/etype/intel_fsp.py                             4      0   100%
    tools/binman/etype/intel_fsp_m.py                           4      0   100%
    tools/binman/etype/intel_fsp_s.py                           4      0   100%
    tools/binman/etype/intel_fsp_t.py                           4      0   100%
    tools/binman/etype/intel_ifwi.py                           67      0   100%
    tools/binman/etype/intel_me.py                              4      0   100%
    tools/binman/etype/intel_mrc.py                             6      0   100%
    tools/binman/etype/intel_refcode.py                         6      0   100%
    tools/binman/etype/intel_vbt.py                             4      0   100%
    tools/binman/etype/intel_vga.py                             4      0   100%
    tools/binman/etype/mkimage.py                              84      0   100%
    tools/binman/etype/null.py                                  9      0   100%
    tools/binman/etype/nxp_imx8mcst.py                         78     59    24%
    tools/binman/etype/nxp_imx8mimage.py                       38      6    84%
    tools/binman/etype/opensbi.py                               5      0   100%
    tools/binman/etype/powerpc_mpc85xx_bootpg_resetvec.py       6      0   100%
    tools/binman/etype/pre_load.py                             76      0   100%
    tools/binman/etype/rockchip_tpl.py                          5      0   100%
    tools/binman/etype/scp.py                                   5      0   100%
    tools/binman/etype/section.py                             418      0   100%
    tools/binman/etype/tee_os.py                               31      0   100%
    tools/binman/etype/text.py                                 21      0   100%
    tools/binman/etype/ti_board_config.py                     139      0   100%
    tools/binman/etype/ti_dm.py                                 5      0   100%
    tools/binman/etype/ti_secure.py                            65      0   100%
    tools/binman/etype/ti_secure_rom.py                       117      0   100%
    tools/binman/etype/u_boot.py                                7      0   100%
    tools/binman/etype/u_boot_dtb.py                            9      0   100%
    tools/binman/etype/u_boot_dtb_with_ucode.py                51      0   100%
    tools/binman/etype/u_boot_elf.py                           19      0   100%
    tools/binman/etype/u_boot_env.py                           27      0   100%
    tools/binman/etype/u_boot_expanded.py                       4      0   100%
    tools/binman/etype/u_boot_img.py                            7      0   100%
    tools/binman/etype/u_boot_nodtb.py                          7      0   100%
    tools/binman/etype/u_boot_spl.py                            8      0   100%
    tools/binman/etype/u_boot_spl_bss_pad.py                   14      0   100%
    tools/binman/etype/u_boot_spl_dtb.py                        9      0   100%
    tools/binman/etype/u_boot_spl_elf.py                        8      0   100%
    tools/binman/etype/u_boot_spl_expanded.py                  12      0   100%
    tools/binman/etype/u_boot_spl_nodtb.py                      8      0   100%
    tools/binman/etype/u_boot_spl_pubkey_dtb.py                32      0   100%
    tools/binman/etype/u_boot_spl_with_ucode_ptr.py             8      0   100%
    tools/binman/etype/u_boot_tpl.py                            8      0   100%
    tools/binman/etype/u_boot_tpl_bss_pad.py                   14      0   100%
    tools/binman/etype/u_boot_tpl_dtb.py                        9      0   100%
    tools/binman/etype/u_boot_tpl_dtb_with_ucode.py             8      0   100%
    tools/binman/etype/u_boot_tpl_elf.py                        8      0   100%
    tools/binman/etype/u_boot_tpl_expanded.py                  12      0   100%
    tools/binman/etype/u_boot_tpl_nodtb.py                      8      0   100%
    tools/binman/etype/u_boot_tpl_with_ucode_ptr.py            12      0   100%
    tools/binman/etype/u_boot_ucode.py                         33      0   100%
    tools/binman/etype/u_boot_vpl.py                            8      0   100%
    tools/binman/etype/u_boot_vpl_bss_pad.py                   14      0   100%
    tools/binman/etype/u_boot_vpl_dtb.py                        9      0   100%
    tools/binman/etype/u_boot_vpl_elf.py                        8      0   100%
    tools/binman/etype/u_boot_vpl_expanded.py                  12      0   100%
    tools/binman/etype/u_boot_vpl_nodtb.py                      8      0   100%
    tools/binman/etype/u_boot_with_ucode_ptr.py                42      0   100%
    tools/binman/etype/vblock.py                               38      0   100%
    tools/binman/etype/x86_reset16.py                           7      0   100%
    tools/binman/etype/x86_reset16_spl.py                       7      0   100%
    tools/binman/etype/x86_reset16_tpl.py                       7      0   100%
    tools/binman/etype/x86_start16.py                           7      0   100%
    tools/binman/etype/x86_start16_spl.py                       7      0   100%
    tools/binman/etype/x86_start16_tpl.py                       7      0   100%
    tools/binman/etype/x509_cert.py                            71      0   100%
    tools/binman/etype/xilinx_bootgen.py                       72      0   100%
    tools/binman/fip_util.py                                  202      0   100%
    tools/binman/fmap_util.py                                  49      0   100%
    tools/binman/image.py                                     181      0   100%
    tools/binman/state.py                                     201      0   100%
    ---------------------------------------------------------------------------
    TOTAL                                                    5954     69    99%

    To get a report in 'htmlcov/index.html', type: python3-coverage html
    Coverage error: 99%, but should be 100%
    ValueError: Test coverage failure

Unfortunately the run failed. As it suggests, create a report:

.. code-block:: bash

    $ python3-coverage html
    Wrote HTML report to htmlcov/index.html

If you open that file in the browser, you can see which files are not reaching
100% and click on them. Here is ``nxp_imx8mimage.py``, for example:

.. code-block:: python

    43        # Generate mkimage configuration file similar to imx8mimage.cfg
    44        # and pass it to mkimage to generate SPL image for us here.
    45        cfg_fname = tools.get_output_filename('nxp.imx8mimage.cfg.%s' % uniq)
    46        with open(cfg_fname, 'w') as outf:
    47            print('ROM_VERSION v%d' % self.rom_version, file=outf)
    48            print('BOOT_FROM %s' % self.boot_from, file=outf)
    49            print('LOADER %s %#x' % (input_fname, self.loader_address), file=outf)
    50
    51        output_fname = tools.get_output_filename(f'cfg-out.{uniq}')
    52        args = ['-d', input_fname, '-n', cfg_fname, '-T', 'imx8mimage',
    53                output_fname]
    54        if self.mkimage.run_cmd(*args) is not None:
    55            return tools.read_file(output_fname)
    56        else:
    57            # Bintool is missing; just use the input data as the output
    58 x          self.record_missing_bintool(self.mkimage)
    59 x          return data
    60
    61    def SetImagePos(self, image_pos):
    62        # Customized SoC specific SetImagePos which skips the mkimage etype
    63        # implementation and removes the 0x48 offset introduced there. That
    64        # offset is only used for uImage/fitImage, which is not the case in
    65        # here.
    66        upto = 0x00
    67        for entry in super().GetEntries().values():
    68 x          entry.SetOffsetSize(upto, None)
    69
    70            # Give up if any entries lack a size
    71 x          if entry.size is None:
    72 x              return
    73 x          upto += entry.size
    74
    75        Entry_section.SetImagePos(self, image_pos)

Most of the file is covered, but the lines marked with ``x`` indicate missing
coverage. The will show up red in your browser.

What is a test?
---------------

A test is a function in ``ftest.py`` which uses an image description in
``tools/binman/test`` to perform some operations and exercise the code. Some
tests are just a few lines; some are more complicated.

Here is a simple test:

.. code-block:: python

    def testSimple(self):
        """Test a simple binman with a single file"""
        data = self._DoReadFile('005_simple.dts')
        self.assertEqual(U_BOOT_DATA, data)

This test tells Binman to build an image using the description. Then it checks
that the resulting image looks correct. The image description is:

.. code-block:: devicetree

    /dts-v1/;

    / {
        #address-cells = <1>;
        #size-cells = <1>;

        binman {
            u-boot {
            };
        };
    };

As you will know from the Binman documentation, this says that there is
one image and it contains the U-Boot binary. So this test builds an image
consisting of a U-Boot binary, then checks that it does indeed have just a
U-Boot binary in it.

Test data
---------

Using real binaries (like ``u-boot.bin``) to test Binman would be quite tedious.
Every output file would be large and it would be hard to tell by looking at the
output (e.g. with a hex dump) if a particular entry contains ``u-boot.bin`` or
``u-boot-spl.bin`` or something else.

Binman gets around this by using simple placeholders. Here is the placeholder
for u-boot.bin:

.. code-block:: python

    U_BOOT_DATA           = b'1234'

This is just bytes. So the test above checks that the output image contains
these four bytes. This makes verification fast for Binman and very easy for
humans.

Even the devicetree is a placeholder:

.. code-block:: python

    U_BOOT_DTB_DATA       = b'udtb'

But for some tests you need to use the real devicetree. In that case you can
use ``_DoReadFileRealDtb()``. See ``testUpdateFdtAll()`` for an example of how
to check the devicetree updated by Binman.

Test structure
--------------

Each test is designed to test just one thing. Binman tests are named according
to what they are testing. Individually they don't do very much, but as a whole
they test every line of code in Binman.

So ``testSimple()`` is designed to check that Binman can build the
simplest-possible image that isn't completely empty.

Another type of test is one which checks error-handling, for example:

.. code-block:: python

    def testFillNoSize(self):
        """Test for an fill entry type with no size"""
        with self.assertRaises(ValueError) as e:
            self._DoReadFile('070_fill_no_size.dts')
        self.assertIn("'fill' entry is missing properties: size",
                      str(e.exception))

This test deliberately tries to provoke an error. The image description is:

.. code-block:: devicetree

    // SPDX-License-Identifier: GPL-2.0+
    /dts-v1/;

    / {
        #address-cells = <1>;
        #size-cells = <1>;

        binman {
            size = <16>;
            fill {
                fill-byte = [ff];
            };
        };
    };

You can see that there is no size for the 'fill' entry, so we would expect
Binman to complain. The test checks that it actually does. It also checks the
error message produced by Binman. Sometimes you need to add several tests, each
with their own broken image description, in order to check all the error cases.

Sometimes you need to capture the console output of Binman, to check it is
correct. You can to this with ``terminal.capture()``, for example:

.. code-block:: python

    with terminal.capture() as (_, stderr):
        self._DoTestFile('071_gbb.dts', force_missing_bintools='futility',
                         entry_args=entry_args)
    err = stderr.getvalue()
    self.assertRegex(err, "Image 'image'.*missing bintools.*: futility")

The test collects the output and checks it with a regular expression. If you
need to see the test output (e.g. to debug it), you will have to remove that
capture line.

How to add a new test
---------------------

This section explains the process of writing a new test. It uses an example to
help with this, but your code will be different.

Generally you are adding a test because you are adding a new entry type
('etype'). So start by creating the shortest and simplest image-description you
can, which contains the new etype. Put it in a numbered file in
``tool/binman/test`` so that it comes last. All the numbers are unique and there
are no gaps.

Example from ``tools/binman/test/339_nxp_imx8.dts``:

.. code-block:: devicetree

    // SPDX-License-Identifier: GPL-2.0+

    /dts-v1/;

    / {
        #address-cells = <1>;
        #size-cells = <1>;

        binman {
            nxp-imx8mimage {
                args;    /* TODO: Needed by mkimage etype superclass */
                nxp,boot-from = "sd";
                nxp,rom-version = <1>;
                nxp,loader-address = <0x10>;
            };
        };
    };

Note that you should use tabs in the file, not spaces. You can see that this has
been cut down to the bare minimum, just enough to include the etype and the
arguments it needs. This is of course not a real image. It will not boot on
anything. But that's fine; we are just trying to test this one etype. Try not
to add any other sections and etypes unless they are absolutely essential for
your test to work. This helps others too: they don't need to understand the full
complexity of your etype just to read your test.

Then create your test by adding a new function at the end of ``ftest.py``:

.. code-block:: python

    def testNxpImx8Image(self):
        """Test that binman can produce an iMX8 image"""
        self._DoTestFile('339_nxp_imx8.dts')

This uses the test file that you created. It doesn't check anything, it just
runs the image description through binman.

Let's run it:

.. code-block:: bash

    $ binman test testNxpImx8Image
    ======================== Running binman tests ========================
    .
    ----------------------------------------------------------------------
    Ran 1 test in 0.242s

    OK

So the test passes. It doesn't really do a lot, but it does exercise the etype.
The next step is to update it to actually check the output:

.. code-block:: python

    def testNxpImx8Image(self):
        """Test that binman can produce an iMX8 image"""
        data = self._DoReadFile('339_nxp_imx8.dts')
        print('data', len(data))

The ``_DoReadFile()`` function is documented in the code. It returns the image
contents as the first part of a tuple.

Running this we see:

.. code-block:: bash

    data 2200

So it is producing a little over 8K of data. Your etype will be different, but
in any case you can add Python code to check that this data is actually correct,
based on your knowledge of your etype. Note that you should not be checking
whether the external tools (called 'bintools' in Binman) are actually working,
since presumably they have their own tests. You just need to check that the
image seems reasonable, e.g. is not empty, contains the expected sections, etc.

When your etype does use a bintool, it also needs tests, but generally it will
be tested by virtue of the etype test. This is because your etype must call the
bintool to create the image. Sometimes you might need to add a test for a
bintool error-condition, though.

Finishing code coverage
-----------------------

The objective is to have test-coverage for every line of code that you add to
Binman. So how can you tell? First, get a coverage report as described above.
Look through the output for any files which are not at 100%. Add more test cases
(image descriptions and new functions in ``ftest.py``) until you have covered
each line.

In the above example, here are some possible steps:

#. The first red bit is where the ``mkimage`` call returns None. This can be
   traced to ``Bintoolmkimage.mkimage()`` which calls
   ``Bintool.run_cmd_result()`` and ``None`` means that ``mkimage`` is missing.
   So the etype has code to handle that case, but it is never used. You can
   look for other examples of ``self.mkimage`` returning ``None`` - e.g.
   ``Entry_mkimage.BuildSectionData()`` does this. The clue for finding this is
   that the ``nxp-imx8mimage`` etype is based on ``Entry_mkimage``:

   .. code-block:: python

       class Entry_nxp_imx8mimage(Entry_mkimage):

   It must be tested somewhere...in this case ``testMkimage()`` doesn't do it,
   but ``testMkimageMissing()`` immediately below that does. So you can create a
   similar test, e.g.:

   .. code-block:: python

       def testNxpImx8ImageMkimageMissing(self):
           """Test that binman can produce an iMX8 image"""
           with terminal.capture() as (_, stderr):
               self._DoTestFile('339_nxp_imx8.dts',
                                force_missing_bintools='mkimage')
           err = stderr.getvalue()
           self.assertRegex(err, "Image 'image'.*missing bintools.*: mkimage")

   Note that this uses exactly the same image description as the first test.
   It just checks what happens when the tool is missing. Checking the coverage
   again, you will see that the first red bit has gone:

   .. code-block:: bash

       $ binman test -T
       $ python3-coverage html

#. The second red bit is for ``SetImagePos()``. You can see that it is iterating
   through the sub-entries inside the ``nxp-imx8mimage`` entry. In the case of
   the 339 file, there are no such entries, so this code inside the for() loop
   isn't used:

   .. code-block:: python

       def SetImagePos(self, image_pos):
        # Customized SoC specific SetImagePos which skips the mkimage etype
        # implementation and removes the 0x48 offset introduced there. That
        # offset is only used for uImage/fitImage, which is not the case in
        # here.
        upto = 0x00
        for entry in super().GetEntries().values():
            entry.SetOffsetSize(upto, None)

            # Give up if any entries lack a size
            if entry.size is None:
                return
            upto += entry.size

        Entry_section.SetImagePos(self, image_pos)

   The solution is to add an entry, e.g. in ``340_nxp_imx8_non_empty.dts``:

   .. code-block:: devicetree

       // SPDX-License-Identifier: GPL-2.0+

       /dts-v1/;

       / {
           #address-cells = <1>;
           #size-cells = <1>;

           binman {
               nxp-imx8mimage {
                   args;    /* TODO: Needed by mkimage etype superclass */
                   nxp,boot-from = "sd";
                   nxp,rom-version = <1>;
                   nxp,loader-address = <0x10>;

                   u-boot {
                   };
               };
           };
       };

   Now write a little test to use it:

   .. code-block:: python

       def testNxpImx8ImageNonEmpty(self):
           """Test that binman can produce an iMX8 image with something in it"""
            data = self._DoReadFile('340_nxp_imx8_non_empty.dts')
            # check data here

   With that, the second red bit goes away, because the for() loop is now used.

#. There is one more red bit left, the ``return`` in ``SetImagePos()``. The
   above effort got the for() loop to be executed, but it doesn't cover the
   ``return``. It might have been copied from some other etype, e.g. the mkimage
   one. See ``Entry_mkimage.SetImagePos()`` which contains the code:

   .. code-block:: python

       for entry in self.GetEntries().values():
           entry.SetOffsetSize(upto, None)

           # Give up if any entries lack a size
           if entry.size is None:
               return
           upto += entry.size

   But which test covers that code for mkimage? By figuring that out, you could
   use a similar technique. One way to find out is to delete the two lines in
   ``Entry_mkimage`` which check for entry.size being None and returning, then
   see what breaks with ``binman test``:

   .. code-block:: bash

       ERROR: binman.ftest.TestFunctional.testMkimageCollection (subunit.RemotedTestCase)
       binman.ftest.TestFunctional.testMkimageCollection
       ----------------------------------------------------------------------
       testtools.testresult.real._StringException: Traceback (most recent call last):
       TypeError: unsupported operand type(s) for +=: 'int' and 'NoneType'

       ======================================================================
       ERROR: binman.ftest.TestFunctional.testMkimageImage (subunit.RemotedTestCase)
       binman.ftest.TestFunctional.testMkimageImage
       ----------------------------------------------------------------------
       testtools.testresult.real._StringException: Traceback (most recent call last):
       TypeError: unsupported operand type(s) for +=: 'int' and 'NoneType'

       ======================================================================
       ERROR: binman.ftest.TestFunctional.testMkimageSpecial (subunit.RemotedTestCase)
       binman.ftest.TestFunctional.testMkimageSpecial
       ----------------------------------------------------------------------
       testtools.testresult.real._StringException: Traceback (most recent call last):
       TypeError: unsupported operand type(s) for +=: 'int' and 'NoneType'

   We can verify that you got the right test, by putting the lines back in and
   getting coverage for just that test:

   .. code-block:: bash

       binman test -T testMkimageCollection
       python3-coverage html

   You will see a lot of red since we are seeing test coverage just for one
   test, but if you look in ``mkimage.py`` at ``SetImagePos()`` you will see
   that the ``return`` is covered (i.e. it is marked green).

   Looking at the ``.dts`` files for each of these tests, none jumps out as
   being relevant to our case. It seems that this code just isn't needed, so the
   best solution is to delete those two lines from the function:

   .. code-block:: python

       def SetImagePos(self, image_pos):
           # Customized SoC specific SetImagePos which skips the mkimage etype
           # implementation and removes the 0x48 offset introduced there. That
           # offset is only used for uImage/fitImage, which is not the case in
           # here.
           upto = 0x00
           for entry in super().GetEntries().values():
               entry.SetOffsetSize(upto, None)
               upto += entry.size

           Entry_section.SetImagePos(self, image_pos)

We should check the updated code on a real build, to make sure it really
isn't needed, of course.

Now, the test coverage is complete!

If we later discover a case where those lines are needed, we can add the
lines back, along with a test for this case.

Getting help
------------

If you are stuck and cannot work out how to add test coverage for your entry
type, ask on the U-Boot mailing list, cc ``Simon Glass <sjg@chromium.org>`` or
on irc ``sjg1``
