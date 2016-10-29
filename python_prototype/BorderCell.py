from Cell import Cell
from Helper import Direction

class BorderCell(Cell):    
    def collide(self):
        next = [0] * Direction.DIRS
        next[Direction.NW] = self.population[Direction.SE]
        next[Direction.N] = self.population[Direction.S]
        next[Direction.NE] = self.population[Direction.SW]
        next[Direction.W] = self.population[Direction.E]
        next[Direction.E] = self.population[Direction.W]
        next[Direction.SW] = self.population[Direction.NE]
        next[Direction.S] = self.population[Direction.N]
        next[Direction.SE] = self.population[Direction.NW]
        self.population = next
#          self.population = [-1.0 * self.population[i] for i in Direction.DIR_ITER]
        
#     def stream(self, bufferArray):
#         pass

    def getColor(self, renderMode):
        return tuple([128, 128, 128])
    
    @staticmethod
    def getSerializationId():
        return "B"