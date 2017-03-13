#!/usr/bin/python
#
# Copyright (c) 2013, Google Inc.
#
# Sanity check of the FIT handling in U-Boot
#
# SPDX-License-Identifier:	GPL-2.0+
#
# To run this:
#
# make O=sandbox sandbox_config
# make O=sandbox
# ./test/image/test-fit.py -u sandbox/u-boot
#
# Note: The above testing requires the Python development package, typically
# called python-devel or something similar.

import doctest
from optparse import OptionParser
import os
import shutil
import struct
import sys
import tempfile

# Enable printing of all U-Boot output
DEBUG = True

# The 'command' library in patman is convenient for running commands
base_path = os.path.dirname(sys.argv[0])
patman = os.path.join(base_path, '../../tools/patman')
sys.path.append(patman)

import command

# Define a base ITS which we can adjust using % and a dictionary
base_its = '''
/dts-v1/;

/ {
        description = "Chrome OS kernel image with one or more FDT blobs";
        #address-cells = <1>;

        images {
                kernel@1 {
                        data = /incbin/("%(kernel)s");
                        type = "kernel";
                        arch = "sandbox";
                        os = "linux";
                        compression = "none";
                        load = <0x40000>;
                        entry = <0x8>;
                };
                kernel@2 {
                        data = /incbin/("%(loadables1)s");
                        type = "kernel";
                        arch = "sandbox";
                        os = "linux";
                        compression = "none";
                        %(loadables1_load)s
                        entry = <0x0>;
                };
                fdt@1 {
                        description = "snow";
                        data = /incbin/("u-boot.dtb");
                        type = "flat_dt";
                        arch = "sandbox";
                        %(fdt_load)s
                        compression = "none";
                        signature@1 {
                                algo = "sha1,rsa2048";
                                key-name-hint = "dev";
                        };
                };
                ramdisk@1 {
                        description = "snow";
                        data = /incbin/("%(ramdisk)s");
                        type = "ramdisk";
                        arch = "sandbox";
                        os = "linux";
                        %(ramdisk_load)s
                        compression = "none";
                };
                ramdisk@2 {
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
                default = "conf@1";
                conf@1 {
                        kernel = "kernel@1";
                        fdt = "fdt@1";
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
        model = "Sandbox Verified Boot Test";
        compatible = "sandbox";

	reset@0 {
		compatible = "sandbox,reset";
	};

};
'''

# This is the U-Boot script that is run for each test. First load the FIT,
# then run the 'bootm' command, then save out memory from the places where
# we expect 'bootm' to write things. Then quit.
base_script = '''
sb load hostfs 0 %(fit_addr)x %(fit)s
fdt addr %(fit_addr)x
bootm start %(fit_addr)x
bootm loados
sb save hostfs 0 %(kernel_addr)x %(kernel_out)s %(kernel_size)x
sb save hostfs 0 %(fdt_addr)x %(fdt_out)s %(fdt_size)x
sb save hostfs 0 %(ramdisk_addr)x %(ramdisk_out)s %(ramdisk_size)x
sb save hostfs 0 %(loadables1_addr)x %(loadables1_out)s %(loadables1_size)x
sb save hostfs 0 %(loadables2_addr)x %(loadables2_out)s %(loadables2_size)x
reset
'''

def debug_stdout(stdout):
    if DEBUG:
        print stdout

def make_fname(leaf):
    """Make a temporary filename

    Args:
        leaf: Leaf name of file to create (within temporary directory)
    Return:
        Temporary filename
    """
    global base_dir

    return os.path.join(base_dir, leaf)

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
    with open(fname, 'r') as fd:
        return fd.read()

def make_dtb():
    """Make a sample .dts file and compile it to a .dtb

    Returns:
        Filename of .dtb file created
    """
    src = make_fname('u-boot.dts')
    dtb = make_fname('u-boot.dtb')
    with open(src, 'w') as fd:
        print >>fd, base_fdt
    command.Output('dtc', src, '-O', 'dtb', '-o', dtb)
    return dtb

def make_its(params):
    """Make a sample .its file with parameters embedded

    Args:
        params: Dictionary containing parameters to embed in the %() strings
    Returns:
        Filename of .its file created
    """
    its = make_fname('test.its')
    with open(its, 'w') as fd:
        print >>fd, base_its % params
    return its

def make_fit(mkimage, params):
    """Make a sample .fit file ready for loading

    This creates a .its script with the selected parameters and uses mkimage to
    turn this into a .fit image.

    Args:
        mkimage: Filename of 'mkimage' utility
        params: Dictionary containing parameters to embed in the %() strings
    Return:
        Filename of .fit file created
    """
    fit = make_fname('test.fit')
    its = make_its(params)
    command.Output(mkimage, '-f', its, fit)
    with open(make_fname('u-boot.dts'), 'w') as fd:
        print >>fd, base_fdt
    return fit

def make_kernel(filename, text):
    """Make a sample kernel with test data

    Args:
        filename: the name of the file you want to create
    Returns:
        Full path and filename of the kernel it created
    """
    fname = make_fname(filename)
    data = ''
    for i in range(100):
        data += 'this %s %d is unlikely to boot\n' % (text, i)
    with open(fname, 'w') as fd:
        print >>fd, data
    return fname

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
        print >>fd, data
    return fname

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
        text: Text to check (each line separated by \n)
        match: String to search for
    Return:
        String containing unmatched portion of line
    Exceptions:
        ValueError: If match is not found

    >>> find_matching('first line:10\\nsecond_line:20', 'first line:')
    '10'
    >>> find_matching('first line:10\\nsecond_line:20', 'second line')
    Traceback (most recent call last):
      ...
    ValueError: Test aborted
    >>> find_matching('first line:10\\nsecond_line:20', 'second_line:')
    '20'
    """
    for line in text.splitlines():
        pos = line.find(match)
        if pos != -1:
            return line[:pos] + line[pos + len(match):]

    print "Expected '%s' but not found in output:"
    print text
    raise ValueError('Test aborted')

def set_test(name):
    """Set the name of the current test and print a message

    Args:
        name: Name of test
    """
    global test_name

    test_name = name
    print name

def fail(msg, stdout):
    """Raise an error with a helpful failure message

    Args:
        msg: Message to display
    """
    print stdout
    raise ValueError("Test '%s' failed: %s" % (test_name, msg))

def run_fit_test(mkimage, u_boot):
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
    global test_name

    # Set up invariant files
    control_dtb = make_dtb()
    kernel = make_kernel('test-kernel.bin', 'kernel')
    ramdisk = make_ramdisk('test-ramdisk.bin', 'ramdisk')
    loadables1 = make_kernel('test-loadables1.bin', 'lenrek')
    loadables2 = make_ramdisk('test-loadables2.bin', 'ksidmar')
    kernel_out = make_fname('kernel-out.bin')
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
    }

    # Make a basic FIT and a script to load it
    fit = make_fit(mkimage, params)
    params['fit'] = fit
    cmd = base_script % params

    # First check that we can load a kernel
    # We could perhaps reduce duplication with some loss of readability
    set_test('Kernel load')
    stdout = command.Output(u_boot, '-d', control_dtb, '-c', cmd)
    debug_stdout(stdout)
    if read_file(kernel) != read_file(kernel_out):
        fail('Kernel not loaded', stdout)
    if read_file(control_dtb) == read_file(fdt_out):
        fail('FDT loaded but should be ignored', stdout)
    if read_file(ramdisk) == read_file(ramdisk_out):
        fail('Ramdisk loaded but should not be', stdout)

    # Find out the offset in the FIT where U-Boot has found the FDT
    line = find_matching(stdout, 'Booting using the FDT blob at ')
    fit_offset = int(line, 16) - params['fit_addr']
    fdt_magic = struct.pack('>L', 0xd00dfeed)
    data = read_file(fit)

    # Now find where it actually is in the FIT (skip the first word)
    real_fit_offset = data.find(fdt_magic, 4)
    if fit_offset != real_fit_offset:
        fail('U-Boot loaded FDT from offset %#x, FDT is actually at %#x' %
                (fit_offset, real_fit_offset), stdout)

    # Now a kernel and an FDT
    set_test('Kernel + FDT load')
    params['fdt_load'] = 'load = <%#x>;' % params['fdt_addr']
    fit = make_fit(mkimage, params)
    stdout = command.Output(u_boot, '-d', control_dtb, '-c', cmd)
    debug_stdout(stdout)
    if read_file(kernel) != read_file(kernel_out):
        fail('Kernel not loaded', stdout)
    if read_file(control_dtb) != read_file(fdt_out):
        fail('FDT not loaded', stdout)
    if read_file(ramdisk) == read_file(ramdisk_out):
        fail('Ramdisk loaded but should not be', stdout)

    # Try a ramdisk
    set_test('Kernel + FDT + Ramdisk load')
    params['ramdisk_config'] = 'ramdisk = "ramdisk@1";'
    params['ramdisk_load'] = 'load = <%#x>;' % params['ramdisk_addr']
    fit = make_fit(mkimage, params)
    stdout = command.Output(u_boot, '-d', control_dtb, '-c', cmd)
    debug_stdout(stdout)
    if read_file(ramdisk) != read_file(ramdisk_out):
        fail('Ramdisk not loaded', stdout)

    # Configuration with some Loadables
    set_test('Kernel + FDT + Ramdisk load + Loadables')
    params['loadables_config'] = 'loadables = "kernel@2", "ramdisk@2";'
    params['loadables1_load'] = 'load = <%#x>;' % params['loadables1_addr']
    params['loadables2_load'] = 'load = <%#x>;' % params['loadables2_addr']
    fit = make_fit(mkimage, params)
    stdout = command.Output(u_boot, '-d', control_dtb, '-c', cmd)
    debug_stdout(stdout)
    if read_file(loadables1) != read_file(loadables1_out):
        fail('Loadables1 (kernel) not loaded', stdout)
    if read_file(loadables2) != read_file(loadables2_out):
        fail('Loadables2 (ramdisk) not loaded', stdout)

def run_tests():
    """Parse options, run the FIT tests and print the result"""
    global base_path, base_dir

    # Work in a temporary directory
    base_dir = tempfile.mkdtemp()
    parser = OptionParser()
    parser.add_option('-u', '--u-boot',
            default=os.path.join(base_path, 'u-boot'),
            help='Select U-Boot sandbox binary')
    parser.add_option('-k', '--keep', action='store_true',
            help="Don't delete temporary directory even when tests pass")
    parser.add_option('-t', '--selftest', action='store_true',
            help='Run internal self tests')
    (options, args) = parser.parse_args()

    # Find the path to U-Boot, and assume mkimage is in its tools/mkimage dir
    base_path = os.path.dirname(options.u_boot)
    mkimage = os.path.join(base_path, 'tools/mkimage')

    # There are a few doctests - handle these here
    if options.selftest:
        doctest.testmod()
        return

    title = 'FIT Tests'
    print title, '\n', '=' * len(title)

    run_fit_test(mkimage, options.u_boot)

    print '\nTests passed'
    print 'Caveat: this is only a sanity check - test coverage is poor'

    # Remove the temporary directory unless we are asked to keep it
    if options.keep:
        print "Output files are in '%s'" % base_dir
    else:
        shutil.rmtree(base_dir)

run_tests()
