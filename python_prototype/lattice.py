#!/usr/bin/python

# to get all keys: print filter(lambda x:'K_' in x, dir(pygame))

import pygame
import random
import cProfile
import pstats
from Grid import Grid
from RigidBodyManager import RigidBodyManager

from CellBody import CellBody
from SquareBody import SquareBody
from WaterCell import WaterCell

from Helper import posToGrid
from Helper import MouseConfiguration
from Helper import RenderMode
import sys

def runGame():
    gridX = 50
    gridY = 50
    
    width = 1000
    height = 1000
    
    renderWidth = width / gridX
    renderHeight = height / gridY
    
    grid = None
    if len(sys.argv) == 1:
        print "creating new grid"
        grid = Grid(gridX, gridY)
    elif len(sys.argv) == 2:
        print "reading grid from file:", sys.argv[1] 
        grid = Grid(sys.argv[1])
    
    pygame.init()
    screen = pygame.display.set_mode((width, height))
    clock = pygame.time.Clock()
    done = False
    
    mouseMode = MouseConfiguration.PLACE_BORDER
    renderMode = RenderMode.DRAW_MASS
    
    rigidBodyManager = RigidBodyManager(grid, screen, renderWidth, renderHeight)
    
    while not done:
        
    # for curStep in range(2):
        print "step"
        pygame.display.set_caption("physics fps:" + str(clock.get_fps()))
        grid.doLatticeBoltzmannStep()
        
        rigidBodyManager.update()
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                done = True
            elif event.type == pygame.MOUSEMOTION or event.type == pygame.MOUSEBUTTONDOWN:
                if pygame.mouse.get_pressed()[0] or event.type == pygame.MOUSEBUTTONDOWN:
                    x, y = posToGrid(renderWidth, renderHeight, *event.pos)
                    if mouseMode == MouseConfiguration.PLACE_SOURCE:
                        grid.replaceBySource(x, y)
                    elif mouseMode == MouseConfiguration.PLACE_DRAIN:
                        grid.replaceByDrain(x, y)
                    elif mouseMode == MouseConfiguration.PLACE_WATER:
                        grid.replaceByWater(x, y)
                    elif mouseMode == MouseConfiguration.PLACE_BORDER:
                        grid.replaceByBorder(x, y)
                    elif mouseMode == MouseConfiguration.PLACE_CELLBODY:
#                         if not isinstance(grid.getArray()[x][y], WaterCell):
                        rigidBodyManager.addRigidBody(CellBody(grid, x, y, 8.0))
                    elif mouseMode == MouseConfiguration.PLACE_SQUAREBODY:
                        rigidBodyManager.addRigidBody(SquareBody(gridX, gridY, x, y, 2, 4.0))
                    else:
                        raise Exception("error: unknown mouse mode")
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_s:
                    mouseMode = MouseConfiguration.PLACE_SOURCE
                elif event.key == pygame.K_d:
                    mouseMode = MouseConfiguration.PLACE_DRAIN
                elif event.key == pygame.K_w:
                    mouseMode = MouseConfiguration.PLACE_WATER
                elif event.key == pygame.K_b:
                    mouseMode = MouseConfiguration.PLACE_BORDER
                elif event.key == pygame.K_c:
                    if mouseMode == MouseConfiguration.PLACE_CELLBODY:
                        mouseMode = MouseConfiguration.PLACE_SQUAREBODY 
                    else:                  
                        mouseMode = MouseConfiguration.PLACE_CELLBODY
                elif event.key == pygame.K_r:
                    if renderMode == RenderMode.DRAW_MASS:
                        renderMode = RenderMode.DRAW_VELOCITY
                    elif renderMode == RenderMode.DRAW_VELOCITY:
                        renderMode = RenderMode.DRAW_COMBINED
                    else:
                        renderMode = RenderMode.DRAW_MASS
                elif event.key == pygame.K_p:
                    grid.dumpToFile("dump.sc")
    
        # draw fluid
        gridArray = grid.getArray()
        for y in xrange(0, gridY):
            for x in xrange(0, gridX):
    
                cell = gridArray[x][y]
                color = cell.getColor(renderMode)
                pygame.draw.rect(screen, color, pygame.Rect(x * renderWidth, y * renderHeight, renderWidth, renderHeight))
                
#                 if isinstance(cell, WaterCell):
#                     speedScaling = 20
#                     speed = cell.getSpeed()
#                     posStart = [(x + 0.5) * renderWidth, (y + 0.5) * renderHeight]
#                     posEnd = [posStart[0] + speedScaling * speed[0], posStart[1] + speedScaling * speed[1]]
#                     color = tuple([255, 255, 255])
#                     pygame.draw.line(screen, color, posStart, posEnd)
#                     color = tuple([0, 0, 0])
#                     pygame.draw.rect(screen, color, pygame.Rect(posStart, (2, 2)))
                
#         # draw rigid bodies
#         for rigidBody in rigidBodyManager.rigidBodies:
#             posX, posY = rigidBody.getGridPos()
#             color = rigidBody.getColor(renderMode)
#             myRect = pygame.Rect(posX * renderWidth, posY * renderHeight, renderWidth, renderHeight)        
#             pygame.draw.rect(screen, color, myRect)
            
        rigidBodyManager.draw(renderMode)
            
        # draw ui
        mouseModeText = ""
        if mouseMode == MouseConfiguration.PLACE_BORDER:
            mouseModeText = "border"
        elif mouseMode == MouseConfiguration.PLACE_DRAIN:
            mouseModeText = "drain"
        elif mouseMode == MouseConfiguration.PLACE_SOURCE:
            mouseModeText = "source"
        elif mouseMode == MouseConfiguration.PLACE_WATER:
            mouseModeText = "water"
        elif mouseMode == MouseConfiguration.PLACE_CELLBODY:
            mouseModeText = "cellbody"
        elif mouseMode == MouseConfiguration.PLACE_SQUAREBODY:
            mouseModeText = "squarebody"
                            
        font = pygame.font.Font(None, 36)
        text = font.render(mouseModeText, 1, (10, 10, 10))
        textpos = text.get_rect(centerx=screen.get_width() / 2, centery=screen.get_width() - 25)
        screen.blit(text, textpos)
                
        pygame.display.flip()
    
        clock.tick(100)


# runGame()
cProfile.run("runGame()", "restats")
p = pstats.Stats('restats')
p.sort_stats('tottime').print_stats(10)
