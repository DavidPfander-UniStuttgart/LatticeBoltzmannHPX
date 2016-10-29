from Cell import Cell
from Helper import Direction

class DrainCell(Cell):    
    def drain(self):
#         self.population = [x for x in Cell.WEIGHTS]
        mass = 1.0
        momentum = self.velocityToMomentum([0.0, 0.0], mass)
        self.population = self.calculateEquilibiumDistribution(mass, momentum)
        
#         for dir in Direction.DIR_ITER:
# #             self.population[dir] = self.WEIGHTS[dir]
#             mass = 1.0
#             momentum = self.velocityToMomentum([0.0, 0.0], mass)
#             equilibrium = self.calculateEquilibiumDistribution(mass, momentum)
#             self.population[dir] = equilibrium[dir]

    def collide(self):
        pass

    @staticmethod
    def getSerializationId():
        return "D"