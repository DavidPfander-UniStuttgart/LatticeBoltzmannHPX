# uncompyle6 version 2.9.3
# Python bytecode 2.7 (62211)
# Decompiled from: Python 2.7.6 (default, Jun 22 2015, 17:58:13) 
# [GCC 4.8.2]
# Embedded file name: /home/pfandedd/git/FA2015Prep/RigidBodyCell.py
# Compiled at: 2015-09-30 09:03:57
from Cell import Cell
from Helper import Direction

class RigidBodyCell(Cell):

    def __init__(self, rigidBody, replacingCell, x, y):
        Cell.__init__(self, x, y)
        self.replacingCell = replacingCell

    def collide(self):
        pass

    def getColor(self, renderMode):
        return tuple([128, 128, 128])

    @staticmethod
    def getSerializationId():
        return 'R'

    def getReplacingCell(self):
        return self.replacingCell
# okay decompiling RigidBodyCell.pyc
