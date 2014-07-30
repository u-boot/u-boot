#!/usr/bin/env python
#
# Copyright (C) 2014, Masahiro Yamada <yamada.m@jp.panasonic.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

"""
A wrapper script to adjust Kconfig for U-Boot

The biggest difference between Linux Kernel and U-Boot in terms of the
board configuration is that U-Boot has to configure multiple boot images
per board: Normal, SPL, TPL.
We need to expand the functions of Kconfig to handle multiple boot
images.

Instead of touching various parts under the scripts/kconfig/ directory,
pushing necessary adjustments into this single script would be better
for code maintainance. All the make targets related to the configuration
(make %config) should be invoked via this script.

Let's see what is different from the original Kconfig.

- config, menuconfig, etc.

The commands 'make config', 'make menuconfig', etc. are used to create
or modify the .config file, which stores configs for Normal boot image.

The location of the one for SPL, TPL image is spl/.config, tpl/.config,
respectively. Use 'make spl/config', 'make spl/menuconfig', etc.
to create or modify the spl/.config file, which contains configs
for SPL image.
Do likewise for the tpl/.config file.
The generic syntax for SPL, TPL configuration is
'make <target_image>/<config_command>'.

- silentoldconfig

The command 'make silentoldconfig' updates .config, if necessary, and
additionally updates include/generated/autoconf.h and files under
include/configs/ directory. In U-Boot, it should do the same things for
SPL, TPL images for boards supporting them.
Depending on whether CONFIG_SPL, CONFIG_TPL is defined or not,
'make silentoldconfig' iterates three times at most changing the target
directory.

To sum up, 'make silentoldconfig' possibly updates
  - .config, include/generated/autoconf.h, include/config/*
  - spl/.config, spl/include/generated/autoconf.h, spl/include/config/*
    (in case CONFIG_SPL=y)
  - tpl/.config, tpl/include/generated/autoconf.h, tpl/include/config/*
    (in case CONFIG_TPL=y)

- defconfig, <board>_defconfig

The command 'make <board>_defconfig' creates a new .config based on the
file configs/<board>_defconfig. The command 'make defconfig' is the same
but the difference is it uses the file specified with KBUILD_DEFCONFIG
environment.

We need to create .config, spl/.config, tpl/.config for boards where SPL
and TPL images are supported. One possible solution for that is to have
multiple defconfig files per board, but it would produce duplication
among the defconfigs.
The approach chosen here is to expand the feature and support
conditional definition in defconfig, that is, each line in defconfig
files has the form of:
<condition>:<macro definition>

The '<condition>:' prefix specifies which image the line is valid for.
The '<condition>:' is one of:
  None  - the line is valid only for Normal image
  S:    - the line is valid only for SPL image
  T:    - the line is valid only for TPL image
  ST:   - the line is valid for SPL and TPL images
  +S:   - the line is valid for Normal and SPL images
  +T:   - the line is valid for Normal and TPL images
  +ST:  - the line is valid for Normal, SPL and SPL images

So, if neither CONFIG_SPL nor CONFIG_TPL is defined, the defconfig file
has no '<condition>:' part and therefore has the same form of that of
Linux Kernel.

In U-Boot, for example, a defconfig file can be written like this:

  CONFIG_FOO=100
  S:CONFIG_FOO=200
  T:CONFIG_FOO=300
  ST:CONFIG_BAR=y
  +S:CONFIG_BAZ=y
  +T:CONFIG_QUX=y
  +ST:CONFIG_QUUX=y

The defconfig above is parsed by this script and internally divided into
three temporary defconfig files.

  - Temporary defconfig for Normal image
     CONFIG_FOO=100
     CONFIG_BAZ=y
     CONFIG_QUX=y
     CONFIG_QUUX=y

  - Temporary defconfig for SPL image
     CONFIG_FOO=200
     CONFIG_BAR=y
     CONFIG_BAZ=y
     CONFIG_QUUX=y

  - Temporary defconfig for TPL image
     CONFIG_FOO=300
     CONFIG_BAR=y
     CONFIG_QUX=y
     CONFIG_QUUX=y

They are passed to scripts/kconfig/conf, each is used for generating
.config, spl/.config, tpl/.config, respectively.

- savedefconfig

This is the reverse operation of 'make defconfig'.
If neither CONFIG_SPL nor CONFIG_TPL is defined in the .config file,
it works as 'make savedefconfig' in Linux Kernel: create the minimal set
of config based on the .config and save it into 'defconfig' file.

If CONFIG_SPL or CONFIG_TPL is defined, the common lines among .config,
spl/.config, tpl/.config are coalesced together and output to the file
'defconfig' in the form like:

  CONFIG_FOO=100
  S:CONFIG_FOO=200
  T:CONFIG_FOO=300
  ST:CONFIG_BAR=y
  +S:CONFIG_BAZ=y
  +T:CONFIG_QUX=y
  +ST:CONFIG_QUUX=y

This can be used as an input of 'make <board>_defconfig' command.
"""

import errno
import os
import re
import subprocess
import sys

# Constant variables
SUB_IMAGES = ('spl', 'tpl')
IMAGES = ('',) + SUB_IMAGES
SYMBOL_MAP = {'': '+', 'spl': 'S', 'tpl': 'T'}
PATTERN_SYMBOL = re.compile(r'(\+?)(S?)(T?):(.*)')

# Environment variables (should be defined in the top Makefile)
# .get('key', 'default_value') method is useful for standalone testing.
MAKE = os.environ.get('MAKE', 'make')
srctree = os.environ.get('srctree', '.')
KCONFIG_CONFIG = os.environ.get('KCONFIG_CONFIG', '.config')

# Useful shorthand
build = '%s -f %s/scripts/Makefile.build obj=scripts/kconfig %%s' % (MAKE, srctree)
autoconf = '%s -f %s/scripts/Makefile.autoconf obj=%%s %%s' % (MAKE, srctree)

### helper functions ###
def mkdirs(*dirs):
    """Make directories ignoring 'File exists' error."""
    for d in dirs:
        try:
            os.makedirs(d)
        except OSError as exception:
            # Ignore 'File exists' error
            if exception.errno != errno.EEXIST:
                raise

def rmfiles(*files):
    """Remove files ignoring 'No such file or directory' error."""
    for f in files:
        try:
            os.remove(f)
        except OSError as exception:
            # Ignore 'No such file or directory' error
            if exception.errno != errno.ENOENT:
                raise

def rmdirs(*dirs):
    """Remove directories ignoring 'No such file or directory'
    and 'Directory not empty' error.
    """
    for d in dirs:
        try:
            os.rmdir(d)
        except OSError as exception:
            # Ignore 'No such file or directory'
            # and 'Directory not empty' error
            if exception.errno != errno.ENOENT and \
               exception.errno != errno.ENOTEMPTY:
                raise

def error(msg):
    """Output the given argument to stderr and exit with return code 1."""
    print >> sys.stderr, msg
    sys.exit(1)

def run_command(command, callback_on_error=None):
    """Run the given command in a sub-shell (and exit if it fails).

    Arguments:
      command: A string of the command
      callback_on_error: Callback handler invoked just before exit
                         when the command fails (Default=None)
    """
    retcode = subprocess.call(command, shell=True)
    if retcode:
        if callback_on_error:
            callback_on_error()
        error("'%s' Failed" % command)

def run_make_config(cmd, objdir, callback_on_error=None):
    """Run the make command in a sub-shell (and exit if it fails).

    Arguments:
      cmd: Make target such as 'config', 'menuconfig', 'defconfig', etc.
      objdir: Target directory where the make command is run.
              Typically '', 'spl', 'tpl' for Normal, SPL, TPL image,
              respectively.
      callback_on_error: Callback handler invoked just before exit
                         when the command fails (Default=None)
    """
    # Linux expects defconfig files in arch/$(SRCARCH)/configs/ directory,
    # but U-Boot puts them in configs/ directory.
    # Give SRCARCH=.. to fake scripts/kconfig/Makefile.
    options = 'SRCARCH=.. KCONFIG_OBJDIR=%s' % objdir
    if objdir:
        options += ' KCONFIG_CONFIG=%s/%s' % (objdir, KCONFIG_CONFIG)
        mkdirs(objdir)
    run_command(build % cmd + ' ' + options, callback_on_error)

def get_enabled_subimages(ignore_error=False):
    """Parse .config file to detect if CONFIG_SPL, CONFIG_TPL is enabled
    and return a tuple of enabled subimages.

    Arguments:
      ignore_error: Specify the behavior when '.config' is not found;
                    Raise an exception if this flag is False.
                    Return a null tuple if this flag is True.

    Returns:
      A tuple of enabled subimages as follows:
        ()             if neither CONFIG_SPL nor CONFIG_TPL is defined
        ('spl',)       if CONFIG_SPL is defined but CONFIG_TPL is not
        ('spl', 'tpl') if both CONFIG_SPL and CONFIG_TPL are defined
    """
    enabled = ()
    match_patterns = [ (img, 'CONFIG_' + img.upper() + '=y\n')
                                                        for img in SUB_IMAGES ]
    try:
        f = open(KCONFIG_CONFIG)
    except IOError as exception:
        if not ignore_error or exception.errno != errno.ENOENT:
            raise
        return enabled
    with f:
        for line in f:
            for img, pattern in match_patterns:
                if line == pattern:
                    enabled += (img,)
    return enabled

def do_silentoldconfig(cmd):
    """Run 'make silentoldconfig' for all the enabled images.

    Arguments:
      cmd: should always be a string 'silentoldconfig'
    """
    run_make_config(cmd, '')
    subimages = get_enabled_subimages()
    for obj in subimages:
        mkdirs(os.path.join(obj, 'include', 'config'),
               os.path.join(obj, 'include', 'generated'))
        run_make_config(cmd, obj)
    remove_auto_conf = lambda : rmfiles('include/config/auto.conf')
    # If the following part failed, include/config/auto.conf should be deleted
    # so 'make silentoldconfig' will be re-run on the next build.
    run_command(autoconf %
                ('include', 'include/autoconf.mk include/autoconf.mk.dep'),
                remove_auto_conf)
    # include/config.h has been updated after 'make silentoldconfig'.
    # We need to touch include/config/auto.conf so it gets newer
    # than include/config.h.
    # Otherwise, 'make silentoldconfig' would be invoked twice.
    os.utime('include/config/auto.conf', None)
    for obj in subimages:
        run_command(autoconf % (obj + '/include',
                                obj + '/include/autoconf.mk'),
                    remove_auto_conf)

def do_tmp_defconfig(output_lines, img):
    """Helper function for do_board_defconfig().

    Write the defconfig contents into a file '.tmp_defconfig' and
    invoke 'make .tmp_defconfig'.

    Arguments:
      output_lines: A sequence of defconfig lines of each image
      img: Target image. Typically '', 'spl', 'tpl' for
           Normal, SPL, TPL images, respectively.
    """
    TMP_DEFCONFIG = '.tmp_defconfig'
    TMP_DIRS = ('arch', 'configs')
    defconfig_path = os.path.join('configs', TMP_DEFCONFIG)
    mkdirs(*TMP_DIRS)
    with open(defconfig_path, 'w') as f:
        f.write(''.join(output_lines[img]))
    cleanup = lambda: (rmfiles(defconfig_path), rmdirs(*TMP_DIRS))
    run_make_config(TMP_DEFCONFIG, img, cleanup)
    cleanup()

def do_board_defconfig(cmd):
    """Run 'make <board>_defconfig'.

    Arguments:
      cmd: should be a string '<board>_defconfig'
    """
    defconfig_path = os.path.join(srctree, 'configs', cmd)
    output_lines = dict([ (img, []) for img in IMAGES ])
    with open(defconfig_path) as f:
        for line in f:
            m = PATTERN_SYMBOL.match(line)
            if m:
                for idx, img in enumerate(IMAGES):
                    if m.group(idx + 1):
                        output_lines[img].append(m.group(4) + '\n')
                continue
            output_lines[''].append(line)
    do_tmp_defconfig(output_lines, '')
    for img in get_enabled_subimages():
        do_tmp_defconfig(output_lines, img)

def do_defconfig(cmd):
    """Run 'make defconfig'.

    Arguments:
      cmd: should always be a string 'defconfig'
    """
    KBUILD_DEFCONFIG = os.environ['KBUILD_DEFCONFIG']
    print "*** Default configuration is based on '%s'" % KBUILD_DEFCONFIG
    do_board_defconfig(KBUILD_DEFCONFIG)

def do_savedefconfig(cmd):
    """Run 'make savedefconfig'.

    Arguments:
      cmd: should always be a string 'savedefconfig'
    """
    DEFCONFIG = 'defconfig'
    # Continue even if '.config' does not exist
    subimages = get_enabled_subimages(True)
    run_make_config(cmd, '')
    output_lines = []
    prefix = {}
    with open(DEFCONFIG) as f:
        for line in f:
            output_lines.append(line)
            prefix[line] = '+'
    for img in subimages:
        run_make_config(cmd, img)
        unmatched_lines = []
        with open(DEFCONFIG) as f:
            for line in f:
                if line in output_lines:
                    index = output_lines.index(line)
                    output_lines[index:index] = unmatched_lines
                    unmatched_lines = []
                    prefix[line] += SYMBOL_MAP[img]
                else:
                    ummatched_lines.append(line)
                    prefix[line] = SYMBOL_MAP[img]
    with open(DEFCONFIG, 'w') as f:
        for line in output_lines:
            if prefix[line] == '+':
                f.write(line)
            else:
                f.write(prefix[line] + ':' + line)

def do_others(cmd):
    """Run the make command other than 'silentoldconfig', 'defconfig',
    '<board>_defconfig' and 'savedefconfig'.

    Arguments:
      cmd: Make target in the form of '<target_image>/<config_command>'
           The field '<target_image>/' is typically empty, 'spl/', 'tpl/'
           for Normal, SPL, TPL images, respectively.
           The field '<config_command>' is make target such as 'config',
           'menuconfig', etc.
    """
    objdir, _, cmd = cmd.rpartition('/')
    run_make_config(cmd, objdir)

cmd_list = {'silentoldconfig': do_silentoldconfig,
            'defconfig': do_defconfig,
            'savedefconfig': do_savedefconfig}

def main():
    cmd = sys.argv[1]
    if cmd.endswith('_defconfig'):
        do_board_defconfig(cmd)
    else:
        func = cmd_list.get(cmd, do_others)
        func(cmd)

if __name__ == '__main__':
    main()
