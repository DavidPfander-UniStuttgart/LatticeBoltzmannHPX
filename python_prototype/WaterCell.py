from Cell import Cell

from Helper import RenderMode
from Helper import Direction

class WaterCell(Cell):
    def getSpeed(self):
        momentum = self.getMomentumDensity()
        massDensity = self.getMassDensity()
        if massDensity == 0.0:
            return [0.0, 0.0]
        fluidSpeed = [momentum[i] / massDensity for i in range(2)]
        return fluidSpeed

    @staticmethod
    def getSerializationId():
        return "W"