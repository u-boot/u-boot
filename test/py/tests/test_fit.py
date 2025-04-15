# SPDX-License-Identifier:	GPL-2.0+
# Copyright (c) 2013, Google Inc.
#
# Sanity check of the FIT handling in U-Boot

import os
import pytest
import struct
import utils
import fit_util

# Define a base ITS which we can adjust using % and a dictionary
base_its = '''
/dts-v1/;

/ {
        description = "Chrome OS kernel image with one or more FDT blobs";
        #address-cells = <1>;

        images {
                kernel-1 {
                        data = /incbin/("%(kernel)s");
                        type = "kernel";
                        arch = "sandbox";
                        os = "linux";
                        compression = "%(compression)s";
                        load = <0x40000>;
                        entry = <0x8>;
                };
                kernel-2 {
                        data = /incbin/("%(loadables1)s");
                        type = "kernel";
                        arch = "sandbox";
                        os = "linux";
                        compression = "none";
                        %(loadables1_load)s
                        entry = <0x0>;
                };
                fdt-1 {
                        description = "snow";
                        data = /incbin/("%(fdt)s");
                        type = "flat_dt";
                        arch = "sandbox";
                        %(fdt_load)s
                        compression = "%(compression)s";
                        signature-1 {
                                algo = "sha1,rsa2048";
                                key-name-hint = "dev";
                        };
                };
                ramdisk-1 {
                        description = "snow";
                        data = /incbin/("%(ramdisk)s");
                        type = "ramdisk";
                        arch = "sandbox";
                        os = "linux";
                        %(ramdisk_load)s
                        compression = "%(compression)s";
                };
                ramdisk-2 {
                        description = "snow";
                        data = /incbin/("%(loadables2)s");
                        type = "ramdisk";
                        arch = "sandbox";
                        os = "linux";
                        %(loadables2_load)s
                        compression = "none";
                };
        };
        configurations {
                default = "conf-1";
                conf-1 {
                        kernel = "kernel-1";
                        fdt = "fdt-1";
                        %(ramdisk_config)s
                        %(loadables_config)s
                };
        };
};
'''

# Define a base FDT - currently we don't use anything in this
base_fdt = '''
/dts-v1/;

/ {
	#address-cells = <1>;
	#size-cells = <0>;

	model = "Sandbox Verified Boot Test";
	compatible = "sandbox";

	binman {
	};

	reset@0 {
		compatible = "sandbox,reset";
		reg = <0>;
	};
};
'''

# This is the U-Boot script that is run for each test. First load the FIT,
# then run the 'bootm' command, then save out memory from the places where
# we expect 'bootm' to write things. Then quit.
base_script = '''
host load hostfs 0 %(fit_addr)x %(fit)s
fdt addr %(fit_addr)x
bootm start %(fit_addr)x
bootm loados
host save hostfs 0 %(kernel_addr)x %(kernel_out)s %(kernel_size)x
host save hostfs 0 %(fdt_addr)x %(fdt_out)s %(fdt_size)x
host save hostfs 0 %(ramdisk_addr)x %(ramdisk_out)s %(ramdisk_size)x
host save hostfs 0 %(loadables1_addr)x %(loadables1_out)s %(loadables1_size)x
host save hostfs 0 %(loadables2_addr)x %(loadables2_out)s %(loadables2_size)x
'''

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('fit_signature')
@pytest.mark.requiredtool('dtc')
def test_fit(ubman):
    def make_fname(leaf):
        """Make a temporary filename

        Args:
            leaf: Leaf name of file to create (within temporary directory)
        Return:
            Temporary filename
        """
        return os.path.join(ubman.config.build_dir, leaf)

    def filesize(fname):
        """Get the size of a file

        Args:
            fname: Filename to check
        Return:
            Size of file in bytes
        """
        return os.stat(fname).st_size

    def read_file(fname):
        """Read the contents of a file

        Args:
            fname: Filename to read
        Returns:
            Contents of file as a string
        """
        with open(fname, 'rb') as fd:
            return fd.read()

    def make_ramdisk(filename, text):
        """Make a sample ramdisk with test data

        Returns:
            Filename of ramdisk created
        """
        fname = make_fname(filename)
        data = ''
        for i in range(100):
            data += '%s %d was seldom used in the middle ages\n' % (text, i)
        with open(fname, 'w') as fd:
            print(data, file=fd)
        return fname

    def make_compressed(filename):
        utils.run_and_log(ubman, ['gzip', '-f', '-k', filename])
        return filename + '.gz'

    def find_matching(text, match):
        """Find a match in a line of text, and return the unmatched line portion

        This is used to extract a part of a line from some text. The match string
        is used to locate the line - we use the first line that contains that
        match text.

        Once we find a match, we discard the match string itself from the line,
        and return what remains.

        TODO: If this function becomes more generally useful, we could change it
        to use regex and return groups.

        Args:
            text: Text to check (list of strings, one for each command issued)
            match: String to search for
        Return:
            String containing unmatched portion of line
        Exceptions:
            ValueError: If match is not found

        >>> find_matching(['first line:10', 'second_line:20'], 'first line:')
        '10'
        >>> find_matching(['first line:10', 'second_line:20'], 'second line')
        Traceback (most recent call last):
          ...
        ValueError: Test aborted
        >>> find_matching('first line:10\', 'second_line:20'], 'second_line:')
        '20'
        >>> find_matching('first line:10\', 'second_line:20\nthird_line:30'],
                          'third_line:')
        '30'
        """
        __tracebackhide__ = True
        for line in '\n'.join(text).splitlines():
            pos = line.find(match)
            if pos != -1:
                return line[:pos] + line[pos + len(match):]

        pytest.fail("Expected '%s' but not found in output")

    def check_equal(expected_fname, actual_fname, failure_msg):
        """Check that a file matches its expected contents

        This is always used on out-buffers whose size is decided by the test
        script anyway, which in some cases may be larger than what we're
        actually looking for. So it's safe to truncate it to the size of the
        expected data.

        Args:
            expected_fname: Filename containing expected contents
            actual_fname: Filename containing actual contents
            failure_msg: Message to print on failure
        """
        expected_data = read_file(expected_fname)
        actual_data = read_file(actual_fname)
        if len(expected_data) < len(actual_data):
            actual_data = actual_data[:len(expected_data)]
        assert expected_data == actual_data, failure_msg

    def check_not_equal(expected_fname, actual_fname, failure_msg):
        """Check that a file does not match its expected contents

        Args:
            expected_fname: Filename containing expected contents
            actual_fname: Filename containing actual contents
            failure_msg: Message to print on failure
        """
        expected_data = read_file(expected_fname)
        actual_data = read_file(actual_fname)
        assert expected_data != actual_data, failure_msg

    def run_fit_test(mkimage):
        """Basic sanity check of FIT loading in U-Boot

        TODO: Almost everything:
          - hash algorithms - invalid hash/contents should be detected
          - signature algorithms - invalid sig/contents should be detected
          - compression
          - checking that errors are detected like:
                - image overwriting
                - missing images
                - invalid configurations
                - incorrect os/arch/type fields
                - empty data
                - images too large/small
                - invalid FDT (e.g. putting a random binary in instead)
          - default configuration selection
          - bootm command line parameters should have desired effect
          - run code coverage to make sure we are testing all the code
        """
        # Set up invariant files
        control_dtb = fit_util.make_dtb(ubman, base_fdt, 'u-boot')
        kernel = fit_util.make_kernel(ubman, 'test-kernel.bin', 'kernel')
        ramdisk = make_ramdisk('test-ramdisk.bin', 'ramdisk')
        loadables1 = fit_util.make_kernel(ubman, 'test-loadables1.bin', 'lenrek')
        loadables2 = make_ramdisk('test-loadables2.bin', 'ksidmar')
        kernel_out = make_fname('kernel-out.bin')
        fdt = make_fname('u-boot.dtb')
        fdt_out = make_fname('fdt-out.dtb')
        ramdisk_out = make_fname('ramdisk-out.bin')
        loadables1_out = make_fname('loadables1-out.bin')
        loadables2_out = make_fname('loadables2-out.bin')

        # Set up basic parameters with default values
        params = {
            'fit_addr' : 0x1000,

            'kernel' : kernel,
            'kernel_out' : kernel_out,
            'kernel_addr' : 0x40000,
            'kernel_size' : filesize(kernel),

            'fdt' : fdt,
            'fdt_out' : fdt_out,
            'fdt_addr' : 0x80000,
            'fdt_size' : filesize(control_dtb),
            'fdt_load' : '',

            'ramdisk' : ramdisk,
            'ramdisk_out' : ramdisk_out,
            'ramdisk_addr' : 0xc0000,
            'ramdisk_size' : filesize(ramdisk),
            'ramdisk_load' : '',
            'ramdisk_config' : '',

            'loadables1' : loadables1,
            'loadables1_out' : loadables1_out,
            'loadables1_addr' : 0x100000,
            'loadables1_size' : filesize(loadables1),
            'loadables1_load' : '',

            'loadables2' : loadables2,
            'loadables2_out' : loadables2_out,
            'loadables2_addr' : 0x140000,
            'loadables2_size' : filesize(loadables2),
            'loadables2_load' : '',

            'loadables_config' : '',
            'compression' : 'none',
        }

        # Make a basic FIT and a script to load it
        fit = fit_util.make_fit(ubman, mkimage, base_its, params)
        params['fit'] = fit
        cmd = base_script % params

        # First check that we can load a kernel
        # We could perhaps reduce duplication with some loss of readability
        ubman.config.dtb = control_dtb
        ubman.restart_uboot()
        with ubman.log.section('Kernel load'):
            output = ubman.run_command_list(cmd.splitlines())
            check_equal(kernel, kernel_out, 'Kernel not loaded')
            check_not_equal(control_dtb, fdt_out,
                            'FDT loaded but should be ignored')
            check_not_equal(ramdisk, ramdisk_out,
                            'Ramdisk loaded but should not be')

            # Find out the offset in the FIT where U-Boot has found the FDT
            line = find_matching(output, 'Booting using the fdt blob at ')
            fit_offset = int(line, 16) - params['fit_addr']
            fdt_magic = struct.pack('>L', 0xd00dfeed)
            data = read_file(fit)

            # Now find where it actually is in the FIT (skip the first word)
            real_fit_offset = data.find(fdt_magic, 4)
            assert fit_offset == real_fit_offset, (
                  'U-Boot loaded FDT from offset %#x, FDT is actually at %#x' %
                  (fit_offset, real_fit_offset))

            # Check if bootargs strings substitution works
            output = ubman.run_command_list([
                'env set bootargs \\\"\'my_boot_var=${foo}\'\\\"',
                'env set foo bar',
                'bootm prep',
                'env print bootargs'])
            assert 'bootargs="my_boot_var=bar"' in output, "Bootargs strings not substituted"

        # Now a kernel and an FDT
        with ubman.log.section('Kernel + FDT load'):
            params['fdt_load'] = 'load = <%#x>;' % params['fdt_addr']
            fit = fit_util.make_fit(ubman, mkimage, base_its, params)
            ubman.restart_uboot()
            output = ubman.run_command_list(cmd.splitlines())
            check_equal(kernel, kernel_out, 'Kernel not loaded')
            check_equal(control_dtb, fdt_out, 'FDT not loaded')
            check_not_equal(ramdisk, ramdisk_out,
                            'Ramdisk loaded but should not be')

        # Try a ramdisk
        with ubman.log.section('Kernel + FDT + Ramdisk load'):
            params['ramdisk_config'] = 'ramdisk = "ramdisk-1";'
            params['ramdisk_load'] = 'load = <%#x>;' % params['ramdisk_addr']
            fit = fit_util.make_fit(ubman, mkimage, base_its, params)
            ubman.restart_uboot()
            output = ubman.run_command_list(cmd.splitlines())
            check_equal(ramdisk, ramdisk_out, 'Ramdisk not loaded')

        # Configuration with some Loadables
        with ubman.log.section('Kernel + FDT + Ramdisk load + Loadables'):
            params['loadables_config'] = 'loadables = "kernel-2", "ramdisk-2";'
            params['loadables1_load'] = ('load = <%#x>;' %
                                         params['loadables1_addr'])
            params['loadables2_load'] = ('load = <%#x>;' %
                                         params['loadables2_addr'])
            fit = fit_util.make_fit(ubman, mkimage, base_its, params)
            ubman.restart_uboot()
            output = ubman.run_command_list(cmd.splitlines())
            check_equal(loadables1, loadables1_out,
                        'Loadables1 (kernel) not loaded')
            check_equal(loadables2, loadables2_out,
                        'Loadables2 (ramdisk) not loaded')

        # Kernel, FDT and Ramdisk all compressed
        with ubman.log.section('(Kernel + FDT + Ramdisk) compressed'):
            params['compression'] = 'gzip'
            params['kernel'] = make_compressed(kernel)
            params['fdt'] = make_compressed(fdt)
            params['ramdisk'] = make_compressed(ramdisk)
            fit = fit_util.make_fit(ubman, mkimage, base_its, params)
            ubman.restart_uboot()
            output = ubman.run_command_list(cmd.splitlines())
            check_equal(kernel, kernel_out, 'Kernel not loaded')
            check_equal(control_dtb, fdt_out, 'FDT not loaded')
            check_not_equal(ramdisk, ramdisk_out, 'Ramdisk got decompressed?')
            check_equal(ramdisk + '.gz', ramdisk_out, 'Ramdist not loaded')


    # We need to use our own device tree file. Remember to restore it
    # afterwards.
    old_dtb = ubman.config.dtb
    try:
        mkimage = ubman.config.build_dir + '/tools/mkimage'
        run_fit_test(mkimage)
    finally:
        # Go back to the original U-Boot with the correct dtb.
        ubman.config.dtb = old_dtb
        ubman.restart_uboot()
