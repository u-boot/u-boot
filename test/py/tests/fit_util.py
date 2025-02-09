# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC

"""Common utility functions for FIT tests"""

import os

import utils

def make_fname(ubman, basename):
    """Make a temporary filename

    Args:
        ubman (ConsoleBase): ubman to use
        basename (str): Base name of file to create (within temporary directory)
    Return:
        Temporary filename
    """

    return os.path.join(ubman.config.build_dir, basename)

def make_its(ubman, base_its, params, basename='test.its'):
    """Make a sample .its file with parameters embedded

    Args:
        ubman (ConsoleBase): ubman to use
        base_its (str): Template text for the .its file, typically containing
            %() references
        params (dict of str): Parameters to embed in the %() strings
        basename (str): base name to write to (will be placed in the temp dir)
    Returns:
        str: Filename of .its file created
    """
    its = make_fname(ubman, basename)
    with open(its, 'w', encoding='utf-8') as outf:
        print(base_its % params, file=outf)
    return its

def make_fit(ubman, mkimage, base_its, params, basename='test.fit', base_fdt=None):
    """Make a sample .fit file ready for loading

    This creates a .its script with the selected parameters and uses mkimage to
    turn this into a .fit image.

    Args:
        ubman (ConsoleBase): ubman to use
        mkimage (str): Filename of 'mkimage' utility
        base_its (str): Template text for the .its file, typically containing
            %() references
        params (dict of str): Parameters to embed in the %() strings
        basename (str): base name to write to (will be placed in the temp dir)
    Return:
        Filename of .fit file created
    """
    fit = make_fname(ubman, basename)
    its = make_its(ubman, base_its, params)
    utils.run_and_log(ubman, [mkimage, '-f', its, fit])
    if base_fdt:
        with open(make_fname(ubman, 'u-boot.dts'), 'w') as fd:
            fd.write(base_fdt)
    return fit

def make_kernel(ubman, basename, text):
    """Make a sample kernel with test data

    Args:
        ubman (ConsoleBase): ubman to use
        basename (str): base name to write to (will be placed in the temp dir)
        text (str): Contents of the kernel file (will be repeated 100 times)
    Returns:
        str: Full path and filename of the kernel it created
    """
    fname = make_fname(ubman, basename)
    data = ''
    for i in range(100):
        data += f'this {text} {i} is unlikely to boot\n'
    with open(fname, 'w', encoding='utf-8') as outf:
        print(data, file=outf)
    return fname

def make_dtb(ubman, base_fdt, basename):
    """Make a sample .dts file and compile it to a .dtb

    Returns:
        ubman (ConsoleBase): ubman to use
        Filename of .dtb file created
    """
    src = make_fname(ubman, f'{basename}.dts')
    dtb = make_fname(ubman, f'{basename}.dtb')
    with open(src, 'w', encoding='utf-8') as outf:
        outf.write(base_fdt)
    utils.run_and_log(ubman, ['dtc', src, '-O', 'dtb', '-o', dtb])
    return dtb
