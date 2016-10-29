import math
from RigidBody import RigidBody
from WaterCell import WaterCell
from BorderCell import BorderCell

class CellBody(RigidBody):
    def __init__(self, grid, gridPosX, gridPosY, mass):
        self.grid = grid
        self.gridX = grid.gridX
        self.gridY = grid.gridY
        # precise location of regid body
        self.x = gridPosX
        self.y = gridPosY
        self.gridPosX = gridPosX
        self.gridPosY = gridPosY
        self.mass = mass
        self.speed = [0.0, 0.0]
        self.acceleration = [0.0, 0.0]
        self.grid.setRigidBody(self, gridPosX, gridPosY)
    
        self.lastForceSum = [0.0, 0.0]
    
    def update(self):
        
        forceSum = [0.0, 0.0]
        gridArray = self.grid.getArray()
        
        for neighborX in range(self.gridPosX - 1, self.gridPosX + 1 + 2):
            for neighborY in range(self.gridPosY - 1, self.gridPosY + 1 + 2):
                if neighborX == self.gridPosX and neighborY == self.gridPosY:
                    continue
                
                if neighborY < 0 or neighborY >= self.gridY:
                    neighborY = neighborY % self.gridY
                if neighborX < 0 or neighborX >= self.gridX:
                    neighborX = neighborX % self.gridX
                
                neighborCell = gridArray[neighborX][neighborY]
                if not isinstance(neighborCell, WaterCell) and not isinstance(neighborCell, BorderCell):
                    continue
                
                # 1D case:
                # - calculate momentum of both cells
                # - switch x parts (ignoring mass)
                # - 
                #
                
                cellMomentum = neighborCell.getMomentumDensity()
                cellMassDensity = neighborCell.getMassDensity()
                #         if cellMassDensity > 0.0:
                fluidSpeed = [cellMomentum[i] / cellMassDensity for i in range(2)]
        
                force = [cellMomentum[i] * cellMassDensity for i in range(2)]
#                 print "force:", force
                forceSum = [forceSum[i] + force[i] for i in range(2)]
        
#         cellMomentum = self.grid.getArray()[self.gridPosX][self.gridPosY].getMomentumDensity()
#         cellMassDensity = self.grid.getArray()[self.gridPosX][self.gridPosY].getMassDensity()
#         
#         fluidSpeed = [cellMomentum[i] / cellMassDensity for i in range(2)]
#         
#         force = [cellMomentum[i] * cellMassDensity for i in range(2)]

        self.lastForceSum = [forceSum[0] / self.mass, forceSum[1] / self.mass]

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
        newGridPosX = int(round(self.x))
        newGridPosY = int(round(self.y))
        
        if newGridPosY < 0 or newGridPosY >= self.gridY:
            newGridPosY = newGridPosY % self.gridY
        if newGridPosX < 0 or newGridPosX >= self.gridX:
            newGridPosX = newGridPosX % self.gridX
            
        if newGridPosX == self.gridPosX and newGridPosY == self.gridPosY:
            return

        print "pos update pre:", newGridPosX, " y:", newGridPosY
        
        if self.grid.isTraversible(newGridPosX, newGridPosY, 1):
            print "pos update:", newGridPosX, " y:", newGridPosY
            self.grid.unsetRigidBody(self, self.gridPosX, self.gridPosY)
            self.grid.setRigidBody(self, newGridPosX, newGridPosY)
            self.gridPosX = newGridPosX
            self.gridPosY = newGridPosY
        else:
            # if you bump into an obstacle, reset coord to last valid cell
            self.x = self.gridPosX
            self.y = self.gridPosY
            # bounce back
            bounceDirection = [-1.0 * (newGridPosX - self.gridPosX), -1.0 * (newGridPosY - self.gridPosY)]
            
            self.speed[0] = bounceDirection[0] * self.speed[0]
            self.speed[1] = bounceDirection[1] * self.speed[1]
            
    def getGridPos(self):
        return self.gridPosX, self.gridPosY
    
    def getColor(self, renderMode):
        return tuple([70, 70, 70])
    
    def getLastForceSum(self):
        return self.lastForceSum