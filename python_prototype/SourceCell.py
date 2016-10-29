import random
import operator
import math

from Cell import *

class SourceCell(Cell):    
    def source(self):
#         self.population = [x for x in Cell.WEIGHTS]
        self.population = [0.0] * Direction.DIRS
        for dir in Direction.DIR_ITER:
            mass = 1.0
            momentum = self.velocityToMomentum(Cell.SPEEDS[dir], mass)
            equilibrium = self.calculateEquilibiumDistribution(mass, momentum)
            self.population[dir] = equilibrium[dir]
#             self.population[dir] = 1.0

    def collide(self):
        pass

    @staticmethod
    def getSerializationId():
        return "S"