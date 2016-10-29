import pygame
import random
from WaterCell import WaterCell
from Helper import Direction

from SourceCell import SourceCell
from DrainCell import DrainCell
from BorderCell import BorderCell
from RigidBodyCell import RigidBodyCell

class Grid:
    def __init__(self, *args):
        if len(args) == 2:
            self.gridX = args[0]
            self.gridY = args[1]
            random.seed()
            self.randomSeed = random.random()
            self.gridArray = self.createGrid()
            self.bufferArray = self.createGrid()
        elif len(args) == 1:
            f = open(args[0], "r")
            states = ["READX", "READY", "READGRID"]
            state = "READX"
            curY = 0
            for line in f:
                if state == "READX":
                    self.gridX = int(line.lstrip().rstrip())
                    state = "READY"
                elif state == "READY":
                    self.gridY = int(line.lstrip().rstrip())
                    self.gridArray = [[None for y in range(self.gridY)] for x in range(self.gridX)]
                    self.bufferArray = [[None for y in range(self.gridY)] for x in range(self.gridX)]
                    state = "READGRID"
                elif state == "READGRID":                  
                    splitted = line.split(" ")
                    splitted = [x.lstrip().rstrip() for x in splitted]
                    curX = 0
                    for serializationId in splitted:
                        print curX, curY, "\"" + serializationId + "\"",
                        if serializationId == WaterCell.getSerializationId():
                            print "-> Water"
                            self.gridArray[curX][curY] = WaterCell(curX, curY)
                            self.bufferArray[curX][curY] = WaterCell(curX, curY)
                        elif serializationId == SourceCell.getSerializationId():
                            print "-> Source"
                            self.gridArray[curX][curY] = SourceCell(curX, curY)
                            self.bufferArray[curX][curY] = SourceCell(curX, curY)     
                        elif serializationId == DrainCell.getSerializationId():
                            print "-> Drain"
                            self.gridArray[curX][curY] = DrainCell(curX, curY)
                            self.bufferArray[curX][curY] = DrainCell(curX, curY)   
                        elif serializationId == BorderCell.getSerializationId():
                            print "-> Border"
                            self.gridArray[curX][curY] = BorderCell(curX, curY)
                            self.bufferArray[curX][curY] = BorderCell(curX, curY)   
                        else:
                            raise NotImplementedError()         
                        curX += 1
                    curY += 1
        else:
            raise NotImplementedError()
        print "file read"
        self.setupAllNeighbors(self.gridArray)
        self.setupAllNeighbors(self.bufferArray)
    
    def createGrid(self):
        random.seed(self.randomSeed)
        grid = [[WaterCell(x, y) for y in range(self.gridY)] for x in range(self.gridX)]

        # create border cells
        for y in xrange(0, self.gridY):
            grid[0][y] = BorderCell(0, y)
            grid[self.gridX - 1][y] = BorderCell(self.gridX - 1, y)
    
        # create source cells
        # for y in xrange(1, 3):
        #     grid[1][y] = SourceCell(1, y)
#         grid[1][1] = SourceCell(1, 1)
#         grid[1][self.gridY - 2] = SourceCell(1, 1)        
#     
#         # # create drain cells
#         # for y in xrange(self.gridY - 3, self.self.gridY - 1):
#         #     grid[self.gridX - 1][y] = DrainCell(self.gridX - 1, y)
#         grid[self.gridX - 2][self.gridY - 2] = DrainCell(self.gridX - 2, self.gridY - 2)
#         grid[1][self.gridY - 2] = DrainCell(self.gridX - 2, self.gridY - 2)
#     
        # create border cells
        for y in xrange(0, self.gridY):
            grid[0][y] = BorderCell(0, y)
            grid[self.gridX - 1][y] = BorderCell(self.gridX - 1, y)
     
        # create border cells
        for x in xrange(0, self.gridX):
            grid[x][0] = BorderCell(x, 0)
            grid[x][self.gridY - 1] = BorderCell(x, self.gridY - 1)        
    
        
        # grid[2][4] = BorderCell(2, 4)
        # grid[3][4] = BorderCell(3, 4)
    
#         # place random border in the domain
#         randomBorders = 50
#         for i in xrange(randomBorders):
#             x = random.randint(0, self.gridX - 1)
#             y = random.randint(0, self.gridY - 1)
#             grid[x][y] = BorderCell(x, y)
    
    
#         for i in xrange(1, 20):
#             grid[i][5] = BorderCell(i, 5)
        self.setupAllNeighbors(grid)

        return grid
    
    def setupAllNeighbors(self, grid):
        print self.gridX
        print self.gridY
        #setup neighbors
        for y in xrange(0, self.gridY):
            for x in xrange(0, self.gridX):
                print x, y
                neighbors = [None] * Direction.DIRS
                curDir = 0
                for yy in xrange(y - 1, y + 2, 1):
                    for xx in xrange(x - 1, x + 2, 1):
                        if yy < 0 or yy >= self.gridY:
                            yy = yy % self.gridY
                        if xx < 0 or xx >= self.gridX:
                            xx = xx % self.gridX
                        #print "pos:", str([x, y]), "neighbor:", str([xx, yy])
                        neighbors[curDir] = grid[xx][yy]
                        curDir += 1
                grid[x][y].setupNeighbors(neighbors)
    
    def stream(self):    
        for y in xrange(0, self.gridY):
            for x in xrange(0, self.gridX):
                self.bufferArray[x][y].clear()
    
        for y in xrange(0, self.gridY):
            for x in xrange(0, self.gridX):
                self.gridArray[x][y].stream(self.bufferArray)            
        
    def collide(self):
        for y in xrange(0, self.gridY):
            for x in xrange(0, self.gridX):
                self.gridArray[x][y].collide()
    
    def source(self):
        for y in xrange(0, self.gridY):
            for x in xrange(0, self.gridX):
                self.gridArray[x][y].source()
    
    def drain(self):
        for y in xrange(0, self.gridY):
            for x in xrange(0, self.gridX):
                self.gridArray[x][y].drain()                        
    
    def doLatticeBoltzmannStep(self):
        # switch array (for buffering)
        temp = self.gridArray
        self.gridArray = self.bufferArray
        self.bufferArray = temp
    
        totalMassBefore = 0.0
        for y in xrange(0, self.gridY):
            for x in xrange(0, self.gridX):
                if isinstance(self.gridArray[x][y], WaterCell):
                    totalMassBefore += self.gridArray[x][y].getMassDensity()
        
        self.collide()
        self.source()
        self.drain()
        self.stream()
    
        totalMassAfter = 0.0
        for y in xrange(0, self.gridY):
            for x in xrange(0, self.gridX):
                if isinstance(self.gridArray[x][y], WaterCell):
                    totalMassAfter += self.bufferArray[x][y].getMassDensity()
        print "totalMassBefore:", totalMassBefore
        print "totalMassAfter:", totalMassAfter        
       
    def regenerateNeighbors(self, array, xNew, yNew):
        #setup neighbors
        for y in xrange(yNew - 1, yNew + 2):
            if y < 0 or y >= self.gridY:
                continue
            for x in xrange(xNew - 1, xNew + 2):
                if x < 0 or x >= self.gridX:
                    continue
                neighbors = [None] * Direction.DIRS
                curDir = 0
                for yy in xrange(y - 1, y + 2, 1):
                    for xx in xrange(x - 1, x + 2, 1):
                        if yy < 0 or yy >= self.gridY:
                            yy = yy % self.gridY
                        if xx < 0 or xx >= self.gridX:
                            xx = xx % self.gridX
                        neighbors[curDir] = array[xx][yy]
                        curDir += 1
                array[x][y].setupNeighbors(neighbors) 
                   
    def replaceByBorder(self, xNew, yNew):
        print "replacing by border"
        if not isinstance(self.gridArray[xNew][yNew], BorderCell):
            self.gridArray[xNew][yNew] = BorderCell(xNew, yNew)
            self.bufferArray[xNew][yNew] = BorderCell(xNew, yNew)
            self.regenerateNeighbors(self.gridArray, xNew, yNew)
            self.regenerateNeighbors(self.bufferArray, xNew, yNew)
    
    def replaceByWater(self, xNew, yNew):
        print "replacing by water"
        if not isinstance(self.gridArray[xNew][yNew], WaterCell):
            self.gridArray[xNew][yNew] = WaterCell(xNew, yNew)
            self.bufferArray[xNew][yNew] = WaterCell(xNew, yNew)
            self.regenerateNeighbors(self.gridArray, xNew, yNew)
            self.regenerateNeighbors(self.bufferArray, xNew, yNew)
        
    def replaceBySource(self, xNew, yNew):
        print "replacing by source"
        if not isinstance(self.gridArray[xNew][yNew], SourceCell):
            self.gridArray[xNew][yNew] = SourceCell(xNew, yNew)
            self.bufferArray[xNew][yNew] = SourceCell(xNew, yNew)
            self.regenerateNeighbors(self.gridArray, xNew, yNew)
            self.regenerateNeighbors(self.bufferArray, xNew, yNew)
    
    def replaceByDrain(self, xNew, yNew):
        print "replacing by drain"
        if not isinstance(self.gridArray[xNew][yNew], DrainCell):
            self.gridArray[xNew][yNew] = DrainCell(xNew, yNew)
            self.bufferArray[xNew][yNew] = DrainCell(xNew, yNew)
            self.regenerateNeighbors(self.gridArray, xNew, yNew)
            self.regenerateNeighbors(self.bufferArray, xNew, yNew)
            
    def getArray(self):
        return self.gridArray
   
    def isTraversible(self, cornerX, cornerY, size):
        for x in range(cornerX, cornerX + size):
            for y in range(cornerY, cornerY + size):
                if y < 0 or y >= self.gridY:
                    y = y % self.gridY
                if x < 0 or x >= self.gridX:
                    x = x % self.gridX
                    
                if not isinstance(self.gridArray[x][y], WaterCell):
                    return False
        return True
    
    def dumpToFile(self, fileName):
        f = open(fileName, "w")
        f.write(str(self.gridX) + "\n")
        f.write(str(self.gridY) + "\n")
        for y in xrange(0, self.gridY):
            first = True
            for x in xrange(0, self.gridX):
                if first:
                    first = False
                else:
                    f.write(" ")
                f.write(self.gridArray[x][y].getSerializationId())
            f.write("\n")
        f.close()
        print "dumped to file!"
        
    def setRigidBody(self, rigidBody, x, y):
        rigidBodyCell = RigidBodyCell(rigidBody, self.gridArray[x][y], x, y)
        bufferRigidBodyCell = RigidBodyCell(rigidBody, self.bufferArray[x][y], x, y)
        
        self.gridArray[x][y] = rigidBodyCell
        self.bufferArray[x][y] = bufferRigidBodyCell
        
        self.regenerateNeighbors(self.gridArray, x, y)
        self.regenerateNeighbors(self.bufferArray, x, y)
        
        rigidBody.addLocation(tuple([x, y]))
    
    def unsetRigidBody(self, rigidBody, x, y):
        self.gridArray[x][y] = self.gridArray[x][y].getReplacingCell()
        self.bufferArray[x][y] = self.bufferArray[x][y].getReplacingCell()
        
        self.regenerateNeighbors(self.gridArray, x, y)
        self.regenerateNeighbors(self.bufferArray, x, y)
        
        rigidBody.removeLocation(tuple([x, y]))    