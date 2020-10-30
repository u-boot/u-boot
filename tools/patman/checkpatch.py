# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

import collections
import os
import re
import sys

from patman import command
from patman import gitutil
from patman import terminal
from patman import tools

def FindCheckPatch():
    top_level = gitutil.GetTopLevel()
    try_list = [
        os.getcwd(),
        os.path.join(os.getcwd(), '..', '..'),
        os.path.join(top_level, 'tools'),
        os.path.join(top_level, 'scripts'),
        '%s/bin' % os.getenv('HOME'),
        ]
    # Look in current dir
    for path in try_list:
        fname = os.path.join(path, 'checkpatch.pl')
        if os.path.isfile(fname):
            return fname

    # Look upwwards for a Chrome OS tree
    while not os.path.ismount(path):
        fname = os.path.join(path, 'src', 'third_party', 'kernel', 'files',
                'scripts', 'checkpatch.pl')
        if os.path.isfile(fname):
            return fname
        path = os.path.dirname(path)

    sys.exit('Cannot find checkpatch.pl - please put it in your ' +
             '~/bin directory or use --no-check')

def CheckPatch(fname, verbose=False, show_types=False):
    """Run checkpatch.pl on a file.

    Args:
        fname: Filename to check
        verbose: True to print out every line of the checkpatch output as it is
            parsed
        show_types: Tell checkpatch to show the type (number) of each message

    Returns:
        namedtuple containing:
            ok: False=failure, True=ok
            problems: List of problems, each a dict:
                'type'; error or warning
                'msg': text message
                'file' : filename
                'line': line number
            errors: Number of errors
            warnings: Number of warnings
            checks: Number of checks
            lines: Number of lines
            stdout: Full output of checkpatch
    """
    fields = ['ok', 'problems', 'errors', 'warnings', 'checks', 'lines',
              'stdout']
    result = collections.namedtuple('CheckPatchResult', fields)
    result.ok = False
    result.errors, result.warnings, result.checks = 0, 0, 0
    result.lines = 0
    result.problems = []
    chk = FindCheckPatch()
    item = {}
    args = [chk, '--no-tree']
    if show_types:
        args.append('--show-types')
    result.stdout = command.Output(*args, fname, raise_on_error=False)
    #pipe = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    #stdout, stderr = pipe.communicate()

    # total: 0 errors, 0 warnings, 159 lines checked
    # or:
    # total: 0 errors, 2 warnings, 7 checks, 473 lines checked
    emacs_prefix = '(?:[0-9]{4}.*\.patch:[0-9]+: )?'
    emacs_stats = '(?:[0-9]{4}.*\.patch )?'
    re_stats = re.compile(emacs_stats +
                          'total: (\\d+) errors, (\d+) warnings, (\d+)')
    re_stats_full = re.compile(emacs_stats +
                               'total: (\\d+) errors, (\d+) warnings, (\d+)'
                               ' checks, (\d+)')
    re_ok = re.compile('.*has no obvious style problems')
    re_bad = re.compile('.*has style problems, please review')
    type_name = '([A-Z_]+:)?'
    re_error = re.compile('ERROR:%s (.*)' % type_name)
    re_warning = re.compile(emacs_prefix + 'WARNING:%s (.*)' % type_name)
    re_check = re.compile('CHECK:%s (.*)' % type_name)
    re_file = re.compile('#(\d+): (FILE: ([^:]*):(\d+):)?')
    re_note = re.compile('NOTE: (.*)')
    re_new_file = re.compile('new file mode .*')
    indent = ' ' * 6
    for line in result.stdout.splitlines():
        if verbose:
            print(line)

        # A blank line indicates the end of a message
        if not line:
            if item:
                result.problems.append(item)
                item = {}
            continue
        if re_note.match(line):
            continue
        # Skip lines which quote code
        if line.startswith(indent):
            continue
        # Skip code quotes
        if line.startswith('+'):
            continue
        if re_new_file.match(line):
            continue
        match = re_stats_full.match(line)
        if not match:
            match = re_stats.match(line)
        if match:
            result.errors = int(match.group(1))
            result.warnings = int(match.group(2))
            if len(match.groups()) == 4:
                result.checks = int(match.group(3))
                result.lines = int(match.group(4))
            else:
                result.lines = int(match.group(3))
            continue
        elif re_ok.match(line):
            result.ok = True
            continue
        elif re_bad.match(line):
            result.ok = False
            continue
        err_match = re_error.match(line)
        warn_match = re_warning.match(line)
        file_match = re_file.match(line)
        check_match = re_check.match(line)
        subject_match = line.startswith('Subject:')
        if err_match:
            item['cptype'] = err_match.group(1)
            item['msg'] = err_match.group(2)
            item['type'] = 'error'
        elif warn_match:
            item['cptype'] = warn_match.group(1)
            item['msg'] = warn_match.group(2)
            item['type'] = 'warning'
        elif check_match:
            item['cptype'] = check_match.group(1)
            item['msg'] = check_match.group(2)
            item['type'] = 'check'
        elif file_match:
            err_fname = file_match.group(3)
            if err_fname:
                item['file'] = err_fname
                item['line'] = int(file_match.group(4))
            else:
                item['file'] = '<patch>'
                item['line'] = int(file_match.group(1))
        elif subject_match:
            item['file'] = '<patch subject>'
            item['line'] = None
        else:
            print('bad line "%s", %d' % (line, len(line)))

    return result

def GetWarningMsg(col, msg_type, fname, line, msg):
    '''Create a message for a given file/line

    Args:
        msg_type: Message type ('error' or 'warning')
        fname: Filename which reports the problem
        line: Line number where it was noticed
        msg: Message to report
    '''
    if msg_type == 'warning':
        msg_type = col.Color(col.YELLOW, msg_type)
    elif msg_type == 'error':
        msg_type = col.Color(col.RED, msg_type)
    elif msg_type == 'check':
        msg_type = col.Color(col.MAGENTA, msg_type)
    line_str = '' if line is None else '%d' % line
    return '%s:%s: %s: %s\n' % (fname, line_str, msg_type, msg)

def CheckPatches(verbose, args):
    '''Run the checkpatch.pl script on each patch'''
    error_count, warning_count, check_count = 0, 0, 0
    col = terminal.Color()

    for fname in args:
        result = CheckPatch(fname, verbose)
        if not result.ok:
            error_count += result.errors
            warning_count += result.warnings
            check_count += result.checks
            print('%d errors, %d warnings, %d checks for %s:' % (result.errors,
                    result.warnings, result.checks, col.Color(col.BLUE, fname)))
            if (len(result.problems) != result.errors + result.warnings +
                    result.checks):
                print("Internal error: some problems lost")
            for item in result.problems:
                sys.stderr.write(
                    GetWarningMsg(col, item.get('type', '<unknown>'),
                        item.get('file', '<unknown>'),
                        item.get('line', 0), item.get('msg', 'message')))
            print
            #print(stdout)
    if error_count or warning_count or check_count:
        str = 'checkpatch.pl found %d error(s), %d warning(s), %d checks(s)'
        color = col.GREEN
        if warning_count:
            color = col.YELLOW
        if error_count:
            color = col.RED
        print(col.Color(color, str % (error_count, warning_count, check_count)))
        return False
    return True
