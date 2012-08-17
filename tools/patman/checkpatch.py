# Copyright (c) 2011 The Chromium OS Authors.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

import command
import gitutil
import os
import re
import terminal

def FindCheckPatch():
    try_list = [
        os.getcwd(),
        os.path.join(os.getcwd(), '..', '..'),
        os.path.join(gitutil.GetTopLevel(), 'tools'),
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
    print 'Could not find checkpatch.pl'
    return None

def CheckPatch(fname, verbose=False):
    """Run checkpatch.pl on a file.

    Returns:
        4-tuple containing:
            result: False=failure, True=ok
            problems: List of problems, each a dict:
                'type'; error or warning
                'msg': text message
                'file' : filename
                'line': line number
            lines: Number of lines
    """
    result = False
    error_count, warning_count, lines = 0, 0, 0
    problems = []
    chk = FindCheckPatch()
    if not chk:
        raise OSError, ('Cannot find checkpatch.pl - please put it in your ' +
                '~/bin directory')
    item = {}
    stdout = command.Output(chk, '--no-tree', fname)
    #pipe = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    #stdout, stderr = pipe.communicate()

    # total: 0 errors, 0 warnings, 159 lines checked
    re_stats = re.compile('total: (\\d+) errors, (\d+) warnings, (\d+)')
    re_ok = re.compile('.*has no obvious style problems')
    re_bad = re.compile('.*has style problems, please review')
    re_error = re.compile('ERROR: (.*)')
    re_warning = re.compile('WARNING: (.*)')
    re_file = re.compile('#\d+: FILE: ([^:]*):(\d+):')

    for line in stdout.splitlines():
        if verbose:
            print line

        # A blank line indicates the end of a message
        if not line and item:
            problems.append(item)
            item = {}
        match = re_stats.match(line)
        if match:
            error_count = int(match.group(1))
            warning_count = int(match.group(2))
            lines = int(match.group(3))
        elif re_ok.match(line):
            result = True
        elif re_bad.match(line):
            result = False
        match = re_error.match(line)
        if match:
            item['msg'] = match.group(1)
            item['type'] = 'error'
        match = re_warning.match(line)
        if match:
            item['msg'] = match.group(1)
            item['type'] = 'warning'
        match = re_file.match(line)
        if match:
            item['file'] = match.group(1)
            item['line'] = int(match.group(2))

    return result, problems, error_count, warning_count, lines, stdout

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
    return '%s: %s,%d: %s' % (msg_type, fname, line, msg)

def CheckPatches(verbose, args):
    '''Run the checkpatch.pl script on each patch'''
    error_count = 0
    warning_count = 0
    col = terminal.Color()

    for fname in args:
        ok, problems, errors, warnings, lines, stdout = CheckPatch(fname,
                verbose)
        if not ok:
            error_count += errors
            warning_count += warnings
            print '%d errors, %d warnings for %s:' % (errors,
                    warnings, fname)
            if len(problems) != error_count + warning_count:
                print "Internal error: some problems lost"
            for item in problems:
                print GetWarningMsg(col, item['type'], item['file'],
                        item['line'], item['msg'])
            #print stdout
    if error_count != 0 or warning_count != 0:
        str = 'checkpatch.pl found %d error(s), %d warning(s)' % (
            error_count, warning_count)
        color = col.GREEN
        if warning_count:
            color = col.YELLOW
        if error_count:
            color = col.RED
        print col.Color(color, str)
        return False
    return True
