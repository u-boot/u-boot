#
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# To run a single test, change to this directory, and:
#
#    python -m unittest func_test.TestFunctional.testHelp

from optparse import OptionParser
import os
import shutil
import struct
import sys
import tempfile
import unittest

import binman
import cmdline
import command
import control
import entry
import fdt_select
import fdt_util
import tools
import tout

# Contents of test files, corresponding to different entry types
U_BOOT_DATA         = '1234'
U_BOOT_IMG_DATA     = 'img'
U_BOOT_SPL_DATA     = '567'
BLOB_DATA           = '89'
ME_DATA             = '0abcd'
VGA_DATA            = 'vga'
U_BOOT_DTB_DATA     = 'udtb'
X86_START16_DATA    = 'start16'
U_BOOT_NODTB_DATA   = 'nodtb with microcode pointer somewhere in here'

class TestFunctional(unittest.TestCase):
    """Functional tests for binman

    Most of these use a sample .dts file to build an image and then check
    that it looks correct. The sample files are in the test/ subdirectory
    and are numbered.

    For each entry type a very small test file is created using fixed
    string contents. This makes it easy to test that things look right, and
    debug problems.

    In some cases a 'real' file must be used - these are also supplied in
    the test/ diurectory.
    """
    @classmethod
    def setUpClass(self):
        # Handle the case where argv[0] is 'python'
        self._binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        self._binman_pathname = os.path.join(self._binman_dir, 'binman')

        # Create a temporary directory for input files
        self._indir = tempfile.mkdtemp(prefix='binmant.')

        # Create some test files
        TestFunctional._MakeInputFile('u-boot.bin', U_BOOT_DATA)
        TestFunctional._MakeInputFile('u-boot.img', U_BOOT_IMG_DATA)
        TestFunctional._MakeInputFile('spl/u-boot-spl.bin', U_BOOT_SPL_DATA)
        TestFunctional._MakeInputFile('blobfile', BLOB_DATA)
        TestFunctional._MakeInputFile('u-boot.dtb', U_BOOT_DTB_DATA)
        TestFunctional._MakeInputFile('u-boot-nodtb.bin', U_BOOT_NODTB_DATA)
        self._output_setup = False

    @classmethod
    def tearDownClass(self):
        """Remove the temporary input directory and its contents"""
        if self._indir:
            shutil.rmtree(self._indir)
        self._indir = None

    def setUp(self):
        # Enable this to turn on debugging output
        # tout.Init(tout.DEBUG)
        command.test_result = None

    def tearDown(self):
        """Remove the temporary output directory"""
        tools._FinaliseForTest()

    def _RunBinman(self, *args, **kwargs):
        """Run binman using the command line

        Args:
            Arguments to pass, as a list of strings
            kwargs: Arguments to pass to Command.RunPipe()
        """
        result = command.RunPipe([[self._binman_pathname] + list(args)],
                capture=True, capture_stderr=True, raise_on_error=False)
        if result.return_code and kwargs.get('raise_on_error', True):
            raise Exception("Error running '%s': %s" % (' '.join(args),
                            result.stdout + result.stderr))
        return result

    def _DoBinman(self, *args):
        """Run binman using directly (in the same process)

        Args:
            Arguments to pass, as a list of strings
        Returns:
            Return value (0 for success)
        """
        (options, args) = cmdline.ParseArgs(list(args))
        options.pager = 'binman-invalid-pager'
        options.build_dir = self._indir

        # For testing, you can force an increase in verbosity here
        # options.verbosity = tout.DEBUG
        return control.Binman(options, args)

    def _DoTestFile(self, fname):
        """Run binman with a given test file

        Args:
            fname: Device tree source filename to use (e.g. 05_simple.dts)
        """
        return self._DoBinman('-p', '-I', self._indir,
                              '-d', self.TestFile(fname))

    def _SetupDtb(self, fname, outfile='u-boot.dtb'):
        if not self._output_setup:
            tools.PrepareOutputDir(self._indir, True)
            self._output_setup = True
        dtb = fdt_util.EnsureCompiled(self.TestFile(fname))
        with open(dtb) as fd:
            data = fd.read()
            TestFunctional._MakeInputFile(outfile, data)

    def _DoReadFile(self, fname, use_real_dtb=False):
        """Run binman and return the resulting image

        This runs binman with a given test file and then reads the resulting
        output file. It is a shortcut function since most tests need to do
        these steps.

        Raises an assertion failure if binman returns a non-zero exit code.

        Args:
            fname: Device tree source filename to use (e.g. 05_simple.dts)
            use_real_dtb: True to use the test file as the contents of
                the u-boot-dtb entry. Normally this is not needed and the
                test contents (the U_BOOT_DTB_DATA string) can be used.
                But in some test we need the real contents.
        """
        # Use the compiled test file as the u-boot-dtb input
        if use_real_dtb:
            self._SetupDtb(fname)

        try:
            retcode = self._DoTestFile(fname)
            self.assertEqual(0, retcode)

            # Find the (only) image, read it and return its contents
            image = control.images['image']
            fname = tools.GetOutputFilename('image.bin')
            self.assertTrue(os.path.exists(fname))
            with open(fname) as fd:
                return fd.read()
        finally:
            # Put the test file back
            if use_real_dtb:
                TestFunctional._MakeInputFile('u-boot.dtb', U_BOOT_DTB_DATA)

    @classmethod
    def _MakeInputFile(self, fname, contents):
        """Create a new test input file, creating directories as needed

        Args:
            fname: Filenaem to create
            contents: File contents to write in to the file
        Returns:
            Full pathname of file created
        """
        pathname = os.path.join(self._indir, fname)
        dirname = os.path.dirname(pathname)
        if dirname and not os.path.exists(dirname):
            os.makedirs(dirname)
        with open(pathname, 'wb') as fd:
            fd.write(contents)
        return pathname

    @classmethod
    def TestFile(self, fname):
        return os.path.join(self._binman_dir, 'test', fname)

    def AssertInList(self, grep_list, target):
        """Assert that at least one of a list of things is in a target

        Args:
            grep_list: List of strings to check
            target: Target string
        """
        for grep in grep_list:
            if grep in target:
                return
        self.fail("Error: '%' not found in '%s'" % (grep_list, target))

    def CheckNoGaps(self, entries):
        """Check that all entries fit together without gaps

        Args:
            entries: List of entries to check
        """
        pos = 0
        for entry in entries.values():
            self.assertEqual(pos, entry.pos)
            pos += entry.size

    def testRun(self):
        """Test a basic run with valid args"""
        result = self._RunBinman('-h')

    def testFullHelp(self):
        """Test that the full help is displayed with -H"""
        result = self._RunBinman('-H')
        help_file = os.path.join(self._binman_dir, 'README')
        self.assertEqual(len(result.stdout), os.path.getsize(help_file))
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testFullHelpInternal(self):
        """Test that the full help is displayed with -H"""
        try:
            command.test_result = command.CommandResult()
            result = self._DoBinman('-H')
            help_file = os.path.join(self._binman_dir, 'README')
        finally:
            command.test_result = None

    def testHelp(self):
        """Test that the basic help is displayed with -h"""
        result = self._RunBinman('-h')
        self.assertTrue(len(result.stdout) > 200)
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    # Not yet available.
    def testBoard(self):
        """Test that we can run it with a specific board"""
        self._SetupDtb('05_simple.dts', 'sandbox/u-boot.dtb')
        TestFunctional._MakeInputFile('sandbox/u-boot.bin', U_BOOT_DATA)
        result = self._DoBinman('-b', 'sandbox')
        self.assertEqual(0, result)

    def testNeedBoard(self):
        """Test that we get an error when no board ius supplied"""
        with self.assertRaises(ValueError) as e:
            result = self._DoBinman()
        self.assertIn("Must provide a board to process (use -b <board>)",
                str(e.exception))

    def testMissingDt(self):
        """Test that an invalid device tree file generates an error"""
        with self.assertRaises(Exception) as e:
            self._RunBinman('-d', 'missing_file')
        # We get one error from libfdt, and a different one from fdtget.
        self.AssertInList(["Couldn't open blob from 'missing_file'",
                           'No such file or directory'], str(e.exception))

    def testBrokenDt(self):
        """Test that an invalid device tree source file generates an error

        Since this is a source file it should be compiled and the error
        will come from the device-tree compiler (dtc).
        """
        with self.assertRaises(Exception) as e:
            self._RunBinman('-d', self.TestFile('01_invalid.dts'))
        self.assertIn("FATAL ERROR: Unable to parse input tree",
                str(e.exception))

    def testMissingNode(self):
        """Test that a device tree without a 'binman' node generates an error"""
        with self.assertRaises(Exception) as e:
            self._DoBinman('-d', self.TestFile('02_missing_node.dts'))
        self.assertIn("does not have a 'binman' node", str(e.exception))

    def testEmpty(self):
        """Test that an empty binman node works OK (i.e. does nothing)"""
        result = self._RunBinman('-d', self.TestFile('03_empty.dts'))
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testInvalidEntry(self):
        """Test that an invalid entry is flagged"""
        with self.assertRaises(Exception) as e:
            result = self._RunBinman('-d',
                                     self.TestFile('04_invalid_entry.dts'))
        #print e.exception
        self.assertIn("Unknown entry type 'not-a-valid-type' in node "
                "'/binman/not-a-valid-type'", str(e.exception))

    def testSimple(self):
        """Test a simple binman with a single file"""
        data = self._DoReadFile('05_simple.dts')
        self.assertEqual(U_BOOT_DATA, data)

    def testDual(self):
        """Test that we can handle creating two images

        This also tests image padding.
        """
        retcode = self._DoTestFile('06_dual_image.dts')
        self.assertEqual(0, retcode)

        image = control.images['image1']
        self.assertEqual(len(U_BOOT_DATA), image._size)
        fname = tools.GetOutputFilename('image1.bin')
        self.assertTrue(os.path.exists(fname))
        with open(fname) as fd:
            data = fd.read()
            self.assertEqual(U_BOOT_DATA, data)

        image = control.images['image2']
        self.assertEqual(3 + len(U_BOOT_DATA) + 5, image._size)
        fname = tools.GetOutputFilename('image2.bin')
        self.assertTrue(os.path.exists(fname))
        with open(fname) as fd:
            data = fd.read()
            self.assertEqual(U_BOOT_DATA, data[3:7])
            self.assertEqual(chr(0) * 3, data[:3])
            self.assertEqual(chr(0) * 5, data[7:])

    def testBadAlign(self):
        """Test that an invalid alignment value is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('07_bad_align.dts')
        self.assertIn("Node '/binman/u-boot': Alignment 23 must be a power "
                      "of two", str(e.exception))

    def testPackSimple(self):
        """Test that packing works as expected"""
        retcode = self._DoTestFile('08_pack.dts')
        self.assertEqual(0, retcode)
        self.assertIn('image', control.images)
        image = control.images['image']
        entries = image._entries
        self.assertEqual(5, len(entries))

        # First u-boot
        self.assertIn('u-boot', entries)
        entry = entries['u-boot']
        self.assertEqual(0, entry.pos)
        self.assertEqual(len(U_BOOT_DATA), entry.size)

        # Second u-boot, aligned to 16-byte boundary
        self.assertIn('u-boot-align', entries)
        entry = entries['u-boot-align']
        self.assertEqual(16, entry.pos)
        self.assertEqual(len(U_BOOT_DATA), entry.size)

        # Third u-boot, size 23 bytes
        self.assertIn('u-boot-size', entries)
        entry = entries['u-boot-size']
        self.assertEqual(20, entry.pos)
        self.assertEqual(len(U_BOOT_DATA), entry.contents_size)
        self.assertEqual(23, entry.size)

        # Fourth u-boot, placed immediate after the above
        self.assertIn('u-boot-next', entries)
        entry = entries['u-boot-next']
        self.assertEqual(43, entry.pos)
        self.assertEqual(len(U_BOOT_DATA), entry.size)

        # Fifth u-boot, placed at a fixed position
        self.assertIn('u-boot-fixed', entries)
        entry = entries['u-boot-fixed']
        self.assertEqual(61, entry.pos)
        self.assertEqual(len(U_BOOT_DATA), entry.size)

        self.assertEqual(65, image._size)

    def testPackExtra(self):
        """Test that extra packing feature works as expected"""
        retcode = self._DoTestFile('09_pack_extra.dts')

        self.assertEqual(0, retcode)
        self.assertIn('image', control.images)
        image = control.images['image']
        entries = image._entries
        self.assertEqual(5, len(entries))

        # First u-boot with padding before and after
        self.assertIn('u-boot', entries)
        entry = entries['u-boot']
        self.assertEqual(0, entry.pos)
        self.assertEqual(3, entry.pad_before)
        self.assertEqual(3 + 5 + len(U_BOOT_DATA), entry.size)

        # Second u-boot has an aligned size, but it has no effect
        self.assertIn('u-boot-align-size-nop', entries)
        entry = entries['u-boot-align-size-nop']
        self.assertEqual(12, entry.pos)
        self.assertEqual(4, entry.size)

        # Third u-boot has an aligned size too
        self.assertIn('u-boot-align-size', entries)
        entry = entries['u-boot-align-size']
        self.assertEqual(16, entry.pos)
        self.assertEqual(32, entry.size)

        # Fourth u-boot has an aligned end
        self.assertIn('u-boot-align-end', entries)
        entry = entries['u-boot-align-end']
        self.assertEqual(48, entry.pos)
        self.assertEqual(16, entry.size)

        # Fifth u-boot immediately afterwards
        self.assertIn('u-boot-align-both', entries)
        entry = entries['u-boot-align-both']
        self.assertEqual(64, entry.pos)
        self.assertEqual(64, entry.size)

        self.CheckNoGaps(entries)
        self.assertEqual(128, image._size)

    def testPackAlignPowerOf2(self):
        """Test that invalid entry alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('10_pack_align_power2.dts')
        self.assertIn("Node '/binman/u-boot': Alignment 5 must be a power "
                      "of two", str(e.exception))

    def testPackAlignSizePowerOf2(self):
        """Test that invalid entry size alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('11_pack_align_size_power2.dts')
        self.assertIn("Node '/binman/u-boot': Alignment size 55 must be a "
                      "power of two", str(e.exception))

    def testPackInvalidAlign(self):
        """Test detection of an position that does not match its alignment"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('12_pack_inv_align.dts')
        self.assertIn("Node '/binman/u-boot': Position 0x5 (5) does not match "
                      "align 0x4 (4)", str(e.exception))

    def testPackInvalidSizeAlign(self):
        """Test that invalid entry size alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('13_pack_inv_size_align.dts')
        self.assertIn("Node '/binman/u-boot': Size 0x5 (5) does not match "
                      "align-size 0x4 (4)", str(e.exception))

    def testPackOverlap(self):
        """Test that overlapping regions are detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('14_pack_overlap.dts')
        self.assertIn("Node '/binman/u-boot-align': Position 0x3 (3) overlaps "
                      "with previous entry '/binman/u-boot' ending at 0x4 (4)",
                      str(e.exception))

    def testPackEntryOverflow(self):
        """Test that entries that overflow their size are detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('15_pack_overflow.dts')
        self.assertIn("Node '/binman/u-boot': Entry contents size is 0x4 (4) "
                      "but entry size is 0x3 (3)", str(e.exception))

    def testPackImageOverflow(self):
        """Test that entries which overflow the image size are detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('16_pack_image_overflow.dts')
        self.assertIn("Image '/binman': contents size 0x4 (4) exceeds image "
                      "size 0x3 (3)", str(e.exception))

    def testPackImageSize(self):
        """Test that the image size can be set"""
        retcode = self._DoTestFile('17_pack_image_size.dts')
        self.assertEqual(0, retcode)
        self.assertIn('image', control.images)
        image = control.images['image']
        self.assertEqual(7, image._size)

    def testPackImageSizeAlign(self):
        """Test that image size alignemnt works as expected"""
        retcode = self._DoTestFile('18_pack_image_align.dts')
        self.assertEqual(0, retcode)
        self.assertIn('image', control.images)
        image = control.images['image']
        self.assertEqual(16, image._size)

    def testPackInvalidImageAlign(self):
        """Test that invalid image alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('19_pack_inv_image_align.dts')
        self.assertIn("Image '/binman': Size 0x7 (7) does not match "
                      "align-size 0x8 (8)", str(e.exception))

    def testPackAlignPowerOf2(self):
        """Test that invalid image alignment is detected"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('20_pack_inv_image_align_power2.dts')
        self.assertIn("Image '/binman': Alignment size 131 must be a power of "
                      "two", str(e.exception))

    def testImagePadByte(self):
        """Test that the image pad byte can be specified"""
        data = self._DoReadFile('21_image_pad.dts')
        self.assertEqual(U_BOOT_SPL_DATA + (chr(0xff) * 9) + U_BOOT_DATA, data)

    def testImageName(self):
        """Test that image files can be named"""
        retcode = self._DoTestFile('22_image_name.dts')
        self.assertEqual(0, retcode)
        image = control.images['image1']
        fname = tools.GetOutputFilename('test-name')
        self.assertTrue(os.path.exists(fname))

        image = control.images['image2']
        fname = tools.GetOutputFilename('test-name.xx')
        self.assertTrue(os.path.exists(fname))

    def testBlobFilename(self):
        """Test that generic blobs can be provided by filename"""
        data = self._DoReadFile('23_blob.dts')
        self.assertEqual(BLOB_DATA, data)

    def testPackSorted(self):
        """Test that entries can be sorted"""
        data = self._DoReadFile('24_sorted.dts')
        self.assertEqual(chr(0) * 5 + U_BOOT_SPL_DATA + chr(0) * 2 +
                         U_BOOT_DATA, data)

    def testPackZeroPosition(self):
        """Test that an entry at position 0 is not given a new position"""
        with self.assertRaises(ValueError) as e:
            self._DoTestFile('25_pack_zero_size.dts')
        self.assertIn("Node '/binman/u-boot-spl': Position 0x0 (0) overlaps "
                      "with previous entry '/binman/u-boot' ending at 0x4 (4)",
                      str(e.exception))

    def testPackUbootDtb(self):
        """Test that a device tree can be added to U-Boot"""
        data = self._DoReadFile('26_pack_u_boot_dtb.dts')
        self.assertEqual(U_BOOT_NODTB_DATA + U_BOOT_DTB_DATA, data)
