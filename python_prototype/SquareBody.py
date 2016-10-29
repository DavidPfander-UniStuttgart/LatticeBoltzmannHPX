import math
from RigidBody import RigidBody

class SquareBody(RigidBody):
    def __init__(self, gridX, gridY, cornerX, cornerY, size, mass):
        self.gridX = gridX
        self.gridY = gridY
        # precise location of regid body
        self.x = cornerX + 0.5 * size
        self.y = cornerY + 0.5 * size
        self.cornerX = cornerX
        self.cornerY = cornerY
        self.mass = mass
        self.size = size
        self.speed = [0.0, 0.0]
        self.acceleration = [0.0, 0.0]
    
    def update(self, grid):
        forceSum = [0.0, 0.0]
        
        for neighborX in range(self.cornerX - 1, self.cornerX + self.size + 2):
            for neighborY in range(self.cornerY - 1, self.cornerY + self.size + 2):
                
                if neighborY < 0 or neighborY >= self.gridY:
                    neighborY = neighborY % self.gridY
                if neighborX < 0 or neighborX >= self.gridX:
                    neighborX = neighborX % self.gridX
                
                cellMomentum = grid.getArray()[neighborX][neighborY].getMomentumDensity()
                cellMassDensity = grid.getArray()[neighborX][neighborY].getMassDensity()
                #         if cellMassDensity > 0.0:
                fluidSpeed = [cellMomentum[i] / cellMassDensity for i in range(2)]
        
                force = [cellMomentum[i] * cellMassDensity for i in range(2)]
                forceSum = [forceSum[i] + force[i] for i in range(2)]

        print "location (old):", self.x, self.y
        
        # update acceleration
        self.acceleration[0] = (forceSum[0] / self.mass)
        self.acceleration[1] = (forceSum[1] / self.mass)
        print "accel:", self.acceleration
        # update speed
        self.speed[0] += self.acceleration[0]
        self.speed[1] += self.acceleration[1]
        
        bodySpeed = math.sqrt(self.speed[0] * self.speed[0] + self.speed[1] * self.speed[1])
        if bodySpeed > 1.0:
            self.speed = [ (1.0/bodySpeed) * s for s in self.speed]
        
        print "speed:", self.speed
        
        # update precise location   
        self.x += self.speed[0]
        self.y += self.speed[1]
        print "location:", self.x, self.y
        # round to grid coord
        newCornerX = int(round(self.x - 0.5 * self.size))
        newCornerY = int(round(self.y - 0.5 * self.size))
        
        if newCornerY < 0 or newCornerY >= self.gridY:
            newCornerY = newCornerY % self.gridY
        if newCornerX < 0 or newCornerX >= self.gridX:
            newCornerX = newCornerX % self.gridX

        print "pos update pre:", newCornerX, " y:", newCornerY
        
        if grid.isTraversible(newCornerX, newCornerY, self.size):
            print "pos update:", newCornerX, " y:", newCornerY
            self.cornerX = newCornerX
            self.cornerY = newCornerY
        else:
            # if you bump into an obstacle, reset coord to last valid cell
            self.x = self.cornerX + 0.5 * self.size
            self.y = self.cornerY + 0.5 * self.size
            # bounce back
            bounceDirection = [-1.0 * (newCornerX - self.cornerX), -1.0 * (newCornerY - self.cornerY)]
            
            self.speed[0] = bounceDirection[0] * self.speed[0]
            self.speed[1] = bounceDirection[1] * self.speed[1]
            
    def getGridPos(self):
        return self.cornerX, self.cornerY
    
    def getColor(self, renderMode):
        return tuple([90, 90, 90])