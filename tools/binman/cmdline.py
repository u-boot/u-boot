# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Command-line parser for binman
#

from argparse import ArgumentParser

def ParseArgs(argv):
    """Parse the binman command-line arguments

    Args:
        argv: List of string arguments
    Returns:
        Tuple (options, args) with the command-line options and arugments.
            options provides access to the options (e.g. option.debug)
            args is a list of string arguments
    """
    if '-H' in argv:
        argv.append('build')

    epilog = '''Binman creates and manipulate images for a board from a set of binaries. Binman is
controlled by a description in the board device tree.'''

    parser = ArgumentParser(epilog=epilog)
    parser.add_argument('-B', '--build-dir', type=str, default='b',
        help='Directory containing the build output')
    parser.add_argument('-D', '--debug', action='store_true',
        help='Enabling debugging (provides a full traceback on error)')
    parser.add_argument('-H', '--full-help', action='store_true',
        default=False, help='Display the README file')
    parser.add_argument('--toolpath', type=str, action='append',
        help='Add a path to the directories containing tools')
    parser.add_argument('-T', '--threads', type=int,
          default=None, help='Number of threads to use (0=single-thread)')
    parser.add_argument('--test-section-timeout', action='store_true',
          help='Use a zero timeout for section multi-threading (for testing)')
    parser.add_argument('-v', '--verbosity', default=1,
        type=int, help='Control verbosity: 0=silent, 1=warnings, 2=notices, '
        '3=info, 4=detail, 5=debug')

    subparsers = parser.add_subparsers(dest='cmd')
    subparsers.required = True

    build_parser = subparsers.add_parser('build', help='Build firmware image')
    build_parser.add_argument('-a', '--entry-arg', type=str, action='append',
            help='Set argument value arg=value')
    build_parser.add_argument('-b', '--board', type=str,
            help='Board name to build')
    build_parser.add_argument('-d', '--dt', type=str,
            help='Configuration file (.dtb) to use')
    build_parser.add_argument('--fake-dtb', action='store_true',
            help='Use fake device tree contents (for testing only)')
    build_parser.add_argument('--fake-ext-blobs', action='store_true',
            help='Create fake ext blobs with dummy content (for testing only)')
    build_parser.add_argument('-i', '--image', type=str, action='append',
            help='Image filename to build (if not specified, build all)')
    build_parser.add_argument('-I', '--indir', action='append',
            help='Add a path to the list of directories to use for input files')
    build_parser.add_argument('-m', '--map', action='store_true',
        default=False, help='Output a map file for each image')
    build_parser.add_argument('-M', '--allow-missing', action='store_true',
        default=False, help='Allow external blobs to be missing')
    build_parser.add_argument('-n', '--no-expanded', action='store_true',
            help="Don't use 'expanded' versions of entries where available; "
                 "normally 'u-boot' becomes 'u-boot-expanded', for example")
    build_parser.add_argument('-O', '--outdir', type=str,
        action='store', help='Path to directory to use for intermediate and '
        'output files')
    build_parser.add_argument('-p', '--preserve', action='store_true',\
        help='Preserve temporary output directory even if option -O is not '
             'given')
    build_parser.add_argument('-u', '--update-fdt', action='store_true',
        default=False, help='Update the binman node with offset/size info')
    build_parser.add_argument('--update-fdt-in-elf', type=str,
        help='Update an ELF file with the output dtb: infile,outfile,begin_sym,end_sym')

    entry_parser = subparsers.add_parser('entry-docs',
        help='Write out entry documentation (see entries.rst)')

    list_parser = subparsers.add_parser('ls', help='List files in an image')
    list_parser.add_argument('-i', '--image', type=str, required=True,
                             help='Image filename to list')
    list_parser.add_argument('paths', type=str, nargs='*',
                             help='Paths within file to list (wildcard)')

    extract_parser = subparsers.add_parser('extract',
                                           help='Extract files from an image')
    extract_parser.add_argument('-i', '--image', type=str, required=True,
                                help='Image filename to extract')
    extract_parser.add_argument('-f', '--filename', type=str,
                                help='Output filename to write to')
    extract_parser.add_argument('-O', '--outdir', type=str, default='',
        help='Path to directory to use for output files')
    extract_parser.add_argument('paths', type=str, nargs='*',
                                help='Paths within file to extract (wildcard)')
    extract_parser.add_argument('-U', '--uncompressed', action='store_true',
        help='Output raw uncompressed data for compressed entries')

    replace_parser = subparsers.add_parser('replace',
                                           help='Replace entries in an image')
    replace_parser.add_argument('-C', '--compressed', action='store_true',
        help='Input data is already compressed if needed for the entry')
    replace_parser.add_argument('-i', '--image', type=str, required=True,
                                help='Image filename to update')
    replace_parser.add_argument('-f', '--filename', type=str,
                                help='Input filename to read from')
    replace_parser.add_argument('-F', '--fix-size', action='store_true',
        help="Don't allow entries to be resized")
    replace_parser.add_argument('-I', '--indir', type=str, default='',
        help='Path to directory to use for input files')
    replace_parser.add_argument('-m', '--map', action='store_true',
        default=False, help='Output a map file for the updated image')
    replace_parser.add_argument('paths', type=str, nargs='*',
                                help='Paths within file to replace (wildcard)')

    test_parser = subparsers.add_parser('test', help='Run tests')
    test_parser.add_argument('-P', '--processes', type=int,
        help='set number of processes to use for running tests')
    test_parser.add_argument('-T', '--test-coverage', action='store_true',
        default=False, help='run tests and check for 100%% coverage')
    test_parser.add_argument('-X', '--test-preserve-dirs', action='store_true',
        help='Preserve and display test-created input directories; also '
             'preserve the output directory if a single test is run (pass test '
             'name at the end of the command line')
    test_parser.add_argument('tests', nargs='*',
                             help='Test names to run (omit for all)')

    return parser.parse_args(argv)
