# Copyright (c) 2012 The Chromium OS Authors.
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

class Board:
    """A particular board that we can build"""
    def __init__(self, target, arch, cpu, board_name, vendor, soc, options):
        """Create a new board type.

        Args:
            target: Target name (use make <target>_config to configure)
            arch: Architecture name (e.g. arm)
            cpu: Cpu name (e.g. arm1136)
            board_name: Name of board (e.g. integrator)
            vendor: Name of vendor (e.g. armltd)
            soc: Name of SOC, or '' if none (e.g. mx31)
            options: board-specific options (e.g. integratorcp:CM1136)
        """
        self.target = target
        self.arch = arch
        self.cpu = cpu
        self.board_name = board_name
        self.vendor = vendor
        self.soc = soc
        self.props = [self.target, self.arch, self.cpu, self.board_name,
                      self.vendor, self.soc]
        self.options = options
        self.build_it = False


class Boards:
    """Manage a list of boards."""
    def __init__(self):
        # Use a simple list here, sinc OrderedDict requires Python 2.7
        self._boards = []

    def AddBoard(self, board):
        """Add a new board to the list.

        The board's target member must not already exist in the board list.

        Args:
            board: board to add
        """
        self._boards.append(board)

    def ReadBoards(self, fname):
        """Read a list of boards from a board file.

        Create a board object for each and add it to our _boards list.

        Args:
            fname: Filename of boards.cfg file
        """
        with open(fname, 'r') as fd:
            for line in fd:
                if line[0] == '#':
                    continue
                fields = line.split()
                if not fields:
                    continue
                for upto in range(len(fields)):
                    if fields[upto] == '-':
                        fields[upto] = ''
                while len(fields) < 7:
                    fields.append('')

                board = Board(*fields)
                self.AddBoard(board)


    def GetList(self):
        """Return a list of available boards.

        Returns:
            List of Board objects
        """
        return self._boards

    def GetDict(self):
        """Build a dictionary containing all the boards.

        Returns:
            Dictionary:
                key is board.target
                value is board
        """
        board_dict = {}
        for board in self._boards:
            board_dict[board.target] = board
        return board_dict

    def GetSelectedDict(self):
        """Return a dictionary containing the selected boards

        Returns:
            List of Board objects that are marked selected
        """
        board_dict = {}
        for board in self._boards:
            if board.build_it:
                board_dict[board.target] = board
        return board_dict

    def GetSelected(self):
        """Return a list of selected boards

        Returns:
            List of Board objects that are marked selected
        """
        return [board for board in self._boards if board.build_it]

    def GetSelectedNames(self):
        """Return a list of selected boards

        Returns:
            List of board names that are marked selected
        """
        return [board.target for board in self._boards if board.build_it]

    def SelectBoards(self, args):
        """Mark boards selected based on args

        Args:
            List of strings specifying boards to include, either named, or
            by their target, architecture, cpu, vendor or soc. If empty, all
            boards are selected.

        Returns:
            Dictionary which holds the number of boards which were selected
            due to each argument, arranged by argument.
        """
        result = {}
        for arg in args:
            result[arg] = 0
        result['all'] = 0

        for board in self._boards:
            if args:
                for arg in args:
                    if arg in board.props:
                        if not board.build_it:
                            board.build_it = True
                            result[arg] += 1
                            result['all'] += 1
            else:
                board.build_it = True
                result['all'] += 1

        return result
