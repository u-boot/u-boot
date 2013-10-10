# Copyright (c) 2012 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

import re

class Board:
    """A particular board that we can build"""
    def __init__(self, status, arch, cpu, soc, vendor, board_name, target, options):
        """Create a new board type.

        Args:
            status: define whether the board is 'Active' or 'Orphaned'
            arch: Architecture name (e.g. arm)
            cpu: Cpu name (e.g. arm1136)
            soc: Name of SOC, or '' if none (e.g. mx31)
            vendor: Name of vendor (e.g. armltd)
            board_name: Name of board (e.g. integrator)
            target: Target name (use make <target>_config to configure)
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
                while len(fields) < 8:
                    fields.append('')
                if len(fields) > 8:
                    fields = fields[:8]

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
        argres = {}
        for arg in args:
            result[arg] = 0
            argres[arg] = re.compile(arg)
        result['all'] = 0

        for board in self._boards:
            if args:
                for arg in args:
                    argre = argres[arg]
                    match = False
                    for prop in board.props:
                        match = argre.match(prop)
                        if match:
                            break
                    if match:
                        if not board.build_it:
                            board.build_it = True
                            result[arg] += 1
                            result['all'] += 1
            else:
                board.build_it = True
                result['all'] += 1

        return result
