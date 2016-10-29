import pygame

from CellBody import CellBody
from SquareBody import SquareBody

class RigidBodyManager:
    def __init__(self, grid, screen, renderWidth, renderHeight):
        self.grid = grid
        self.screen = screen
        self.renderWidth = renderWidth
        self.renderHeight = renderHeight
        self.rigidBodies = []
        
    def addRigidBody(self, newRigidBody):
        
        # cannot stack rigid bodies
        for rigidBody in self.rigidBodies:
            if newRigidBody.x == rigidBody.x and newRigidBody.y == rigidBody.y:
                return
        self.rigidBodies.append(newRigidBody)
    
    def update(self):
        for rigidBody in self.rigidBodies:
            rigidBody.update()
            
    def draw(self, renderMode):
        # draw rigid bodies
        for rigidBody in self.rigidBodies:
            color = rigidBody.getColor(renderMode)
            if isinstance(rigidBody, CellBody):
                posX, posY = rigidBody.getGridPos()
                myRect = pygame.Rect(posX * self.renderWidth, posY * self.renderHeight, self.renderWidth, self.renderHeight)
                
                posStart = [(posX + 0.5) * self.renderWidth, (posY + 0.5) * self.renderHeight]
                forceSum = rigidBody.getLastForceSum()
                print "fS:", forceSum
                forceScaling = 100.0
                posEnd = [posStart[0] + forceScaling * forceSum[0], posStart[1] + forceScaling * forceSum[1]]
                print "pE:", [abs(posStart[i] - posEnd[i]) for i in range(2)]
                lineColor = tuple([255, 255, 255])
                pygame.draw.line(self.screen, lineColor, posStart, posEnd)
                dotColor = tuple([0, 0, 0])
                pygame.draw.rect(self.screen, dotColor, pygame.Rect(posStart, (2, 2)))
                
            elif isinstance(rigidBody, SquareBody):
                cornerX = rigidBody.cornerX
                cornerY = rigidBody.cornerY
                myRect = pygame.Rect(cornerX * self.renderWidth, cornerY * self.renderHeight, self.renderWidth * rigidBody.size, self.renderHeight * rigidBody.size)                        
            pygame.draw.rect(self.screen, color, myRect)
