import random
import operator
import math

from Helper import RenderMode
from Helper import Direction

class Cell:
    OMEGA = 1.0
    C = 1.0 # lattice scaling, 1.0 should be ok
    SPEEDS = [tuple([-C, -C]), tuple([0, -C]), tuple([C, -C]),
              tuple([-C, 0]), tuple([0, 0]), tuple([C, 0]),
              tuple([-C, C]), tuple([0, C]), tuple([C, C])]
    
    WEIGHTS = [1.0/36.0, 1.0/9.0, 1.0/36.0,
               1.0/9.0, 4.0/9.0, 1.0/9.0,
               1.0/36.0, 1.0/9.0, 1.0/36.0]

    MAX_DIR_MASS = 1.5
    MAX_TOTAL_MASS = 5.0
    
    def __init__(self, x, y):
        self.x = x
        self.y = y
        
        massDensity = 1.0
        momentumDensity = [0.0, 0.0]
        self.population = self.calculateEquilibiumDistribution(massDensity, momentumDensity)
#         self.population = [x for x in Cell.WEIGHTS]
    def velocityToMomentum(self, v, massDensity):
        return [x / massDensity for x in v]
    
    @staticmethod
    def getSerializationId():
        raise NotImplementedError()
  
    def setupNeighbors(self, neighbors):
        self.neighbors = neighbors

    def stream(self, bufferArray):
        # print "pop:", self.population
        for dir in Direction.DIR_ITER:
            if self.population[dir] == 0:
                continue
            neighborCell = self.neighbors[dir]
            bufferNeighborCell = bufferArray[neighborCell.x][neighborCell.y]
            bufferNeighborCell.addPopulation(dir, self.population[dir])

    def getMassDensity(self):
        massDensity = sum([mass for mass in self.population])
        return massDensity

    def getMomentumDensity(self):
        momentumDensity = [0.0, 0.0]
        for dir in Direction.DIR_ITER:
            momentumDensity[0] += self.population[dir] * Cell.SPEEDS[dir][0]
            momentumDensity[1] += self.population[dir] * Cell.SPEEDS[dir][1]
        return momentumDensity

    def calculateEquilibiumDistribution(self, massDensity, momentumDensity):
        equilibrium = []
        
        dim = 2
        u = [momentumDensity[i] / massDensity for i in range(dim)]
        uu = sum([u[i] * u[i] for i in range(dim)])
        c2 = Cell.C * Cell.C
        c4 = c2 * c2
        forth = -1.0 * (3.0 * uu) / (2.0 * c2)
        
        for dir in Direction.DIR_ITER:
            cu = sum([Cell.SPEEDS[dir][i] * u[i] for i in range(dim)])

            second = (3.0 * cu) / (c2)
            third = (9.0 * cu * cu) / (2.0 * c4)
            
            if dir == Direction.C:
                equilibrium.append((4.0/9.0) * massDensity * (1.0 + forth))
            elif dir == Direction.N or dir == Direction.E or dir == Direction.S or dir == Direction.W:
                equilibrium.append((1.0/9.0) * massDensity * (1.0 + second + third + forth))
            else:
                equilibrium.append((1.0/36.0) * massDensity * (1.0 + second + third + forth))

        return equilibrium
    
    def collide(self):
        totalMass = sum(self.population)
        #print "totalMass:", totalMass

        if totalMass == 0.0:
            return 
        
        # scale
        scaled = [(1 - Cell.OMEGA) * x for x in self.population]

        massDensity = self.getMassDensity()        
        momentumDensity = self.getMomentumDensity()
        equilibrium = self.calculateEquilibiumDistribution(massDensity, momentumDensity)
        equilibrium = [Cell.OMEGA * x for x in equilibrium]
        
        
#         if self.x == 2 and self.y == 2:
#         print self.population
#         print self.x, self.y, "massD:", massDensity, "momD:", momentumDensity #, "equilibrium:", equilibrium

#         massDensity = self.getMassDensity()        
#         momentumDensity = self.getMomentumDensity()
#         print "massD after:", massDensity, "momD after:", momentumDensity
        
        self.population = map(operator.add, scaled, equilibrium)
        #self.check(self.population)
        self.correct()

    def check(self, population):
        for i in population:
            if i < 0.0:
                print "i:", i, "(warning)"
            #assert(i >= 0.0)
            if i > Cell.MAX_TOTAL_MASS:
                print "i:", i, "(warning)"
            #assert(i <= 9.0)                

    def correct(self):
        for i in Direction.DIR_ITER:
#             if self.population[i] > Cell.MAX_DIR_MASS:
#                 self.population[i] = Cell.MAX_DIR_MASS
            if self.population[i] < 0.0:
                self.population[i] = 0.0                
                
    def addPopulation(self, dir, newPopulation):
        self.population[dir] += newPopulation

    def clear(self):
        for dir in Direction.DIR_ITER:
            self.population[dir] = 0

    def totalPop(self):
        sum = 0
        for popDir in self.population:
            sum += popDir
        return sum
            
    def getColor(self, renderMode):
        if renderMode == RenderMode.DRAW_MASS:
            pop = self.totalPop()
            if pop > Cell.MAX_TOTAL_MASS:
                pop = Cell.MAX_TOTAL_MASS
            elif pop < 0.0:
                pop = 0.0
            
            value = 255 * math.sqrt(pop / Cell.MAX_TOTAL_MASS)
            return tuple([value, 0, 255 - value])
        elif renderMode == RenderMode.DRAW_VELOCITY:
            mom = self.getMomentumDensity()
            mom = math.sqrt(mom[0] * mom[0] + mom[1] * mom[1])
            mom = mom / math.sqrt(2.0) 
#             print "mom:", mom
            if mom > 0.5:
                print "except:", mom
            mom = 255 * mom * 2      
            if mom > 255:
                mom = 255
            elif mom < 0:
                mom = 0
            return tuple([mom, 0, 0])
        elif renderMode == RenderMode.DRAW_COMBINED:
            pop = self.totalPop()
            
            if pop > Cell.MAX_TOTAL_MASS:
                pop = Cell.MAX_TOTAL_MASS
            elif pop < 0.0:
                pop = 0.0
            
            value = 255 * math.sqrt(pop / Cell.MAX_TOTAL_MASS)
                
            mom = self.getMomentumDensity()
            mom = math.sqrt(mom[0] * mom[0] + mom[1] * mom[1])
            mom = mom / math.sqrt(2.0)
            if mom > 1.0:
                print "except:", mom
            mom = 255 * mom      
            if mom > 255:
                mom = 255
            elif mom < 0:
                mom = 0
                
            return tuple([value, mom, 255 - value])
    
    def source(self):
        pass

    def drain(self):
        pass