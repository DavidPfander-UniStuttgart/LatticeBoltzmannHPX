
def posToGrid(renderWidth, renderHeight, mouseX, mouseY):
    x = int(mouseX / renderWidth)
    y = int(mouseY / renderHeight)
    return x, y

class MouseConfiguration:
    PLACE_WATER = 0
    PLACE_BORDER = 1
    PLACE_SOURCE = 2
    PLACE_DRAIN = 3
    PLACE_CELLBODY = 4
    PLACE_SQUAREBODY = 5
    
class RenderMode:
    DRAW_MASS = 0
    DRAW_VELOCITY = 1
    DRAW_COMBINED = 2
    
class Direction:
    NW = 0
    N = 1
    NE = 2
    W = 3
    C = 4
    E = 5
    SW = 6
    S = 7
    SE = 8

    DIRS = 9
    DIR_ITER = range(9)