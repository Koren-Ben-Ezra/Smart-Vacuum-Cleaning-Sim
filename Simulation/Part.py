from enum import Enum
import pygame
import random
from math import ceil
# Basic settings
BLOCK_SIZE = 30
BLOCK_MARGIN = 1
DIRT_SIZE = 3
MAX_DIRT_IN_BLOCK = 3

WIDTH, HEIGHT = 1290, 720
WALL_SGN = 'W'
DOCKING_SGN = 'D'
CLEAN_SGN = ' '


class Step(Enum):
    North = 'N'  
    East = 'E'  
    South = 'S'  
    West = 'W'  
    Stay = 's'  
    Finish = 'F'  

class Direction(Enum):
    North = 0
    East = 1
    South = 2
    West = 3
    
# Part Type
class PT(Enum):
    Blank = 0
    Robot = 1
    Wall = 2
    EdgeWall = 3
    Docking = 4
    Background = 5
    
WALL_PARTS = (PT.Wall, PT.EdgeWall)

W_BLOCKS, H_BLOCKS = WIDTH // BLOCK_SIZE, HEIGHT // BLOCK_SIZE

PART_COLOR = {
    "background": pygame.Color(29, 89, 1),
    "text": pygame.Color(241, 229, 209),
    "frame" : pygame.Color(196, 202, 202),
    "robotFrame": pygame.Color(65, 65, 67),
    "robot": pygame.Color(168, 170, 169),
    "docking": pygame.Color(160, 112, 188), 
    "blank": pygame.Color(170,159,137),
    "edgeWall": pygame.Color(156, 69, 49), 
    "wall": pygame.Color(192, 141, 122), 
    "dirt": pygame.Color(47, 79, 79)
}

class Part:
        
        xMid = None     # relative to the robot's position
        yMid = None     # relative to the robot's position
        xLeft = None    # relative to the frame
        yTop = None     # relative to the frame
        screen = None
        dockingPart = None
        
        def __init__(self, x:int ,y:int, pt:PT, dirtLevel: int = -1):
            self.x = x
            self.y = y
            self.dirtLevel = dirtLevel
            self.pt = pt
            self.dirt = []
        
        def calcCoorOnScreen(self):
            return (self.x + self.xLeft) * BLOCK_SIZE, (self.y + self.yTop) * BLOCK_SIZE

        @classmethod
        def staticVariableInit(cls, robotPart, screen, rows, cols):
            cls.screen = screen
            cls.dockingPart.x = robotPart.x
            cls.dockingPart.y = robotPart.y
            
            cls.xMid = robotPart.x
            cls.yMid = robotPart.y
            cls.xLeft = W_BLOCKS // 2 - robotPart.x
            cls.yTop = H_BLOCKS // 2 - robotPart.y
        
        def draw(self):
            raise NotImplementedError("Subclass must implement abstract method")

                         
class PTWall(Part):
    def __init__(self, x: int, y: int):
        super().__init__(x, y, PT.Wall)
        
    def draw(self):
        x, y = self.calcCoorOnScreen()
        w = h = BLOCK_SIZE - 2*BLOCK_MARGIN
        pygame.draw.rect(self.screen, PART_COLOR.get("frame"), 
                        (x, y, BLOCK_SIZE, BLOCK_SIZE))
        pygame.draw.rect(self.screen, PART_COLOR.get("wall"), 
                        (x + BLOCK_MARGIN, y + BLOCK_MARGIN, w, h))
    
class PTEdgeWall(Part):
    def __init__(self, x: int, y: int):
        super().__init__(x, y, PT.EdgeWall)
    
    def draw(self):
        w = h = BLOCK_SIZE - 2*BLOCK_MARGIN
        x, y = self.calcCoorOnScreen()
        pygame.draw.rect(self.screen, PART_COLOR.get("frame"), 
                        (x, y, BLOCK_SIZE, BLOCK_SIZE))
        pygame.draw.rect(self.screen, PART_COLOR.get("edgeWall"),
                        (x + BLOCK_MARGIN, y + BLOCK_MARGIN, w, h))

class PTRobot(Part):
    def __init__(self, x: int, y: int):
        super().__init__(x, y, PT.Robot)
        
    def draw(self):
        padding = 3 * BLOCK_MARGIN
        w = h = BLOCK_SIZE - 2*padding - 2*BLOCK_MARGIN
        x, y = self.calcCoorOnScreen()

        x += padding
        y += padding
        pygame.draw.rect(self.screen, PART_COLOR.get("robotFrame"), 
                        (x, y, BLOCK_SIZE - 2 * padding, BLOCK_SIZE - 2 * padding))
        pygame.draw.rect(self.screen, PART_COLOR.get("robot"), 
                        (x + BLOCK_MARGIN, y + BLOCK_MARGIN, w, h))

class PTBlank(Part):
    def __init__(self, x: int, y: int, dirtLevel: int):
        super().__init__(x, y, PT.Blank, dirtLevel)
        self._initDirt()
        
    def _decideRandomCoor(self):
        return random.randint(0, BLOCK_SIZE - 1 - DIRT_SIZE), random.randint(0, BLOCK_SIZE - 1 - DIRT_SIZE)
    
    def _initDirt(self):
        for _ in range(ceil(self.dirtLevel / MAX_DIRT_IN_BLOCK)):
            self.dirt.append(self._decideRandomCoor())
    
    def draw(self):
        x, y = self.calcCoorOnScreen()
        pygame.draw.rect(self.screen, PART_COLOR.get("blank"), 
                        (x, y, BLOCK_SIZE, BLOCK_SIZE))
        
        for i in range(ceil(self.dirtLevel / MAX_DIRT_IN_BLOCK)):
            x_d, y_d = self.dirt[i]
            pygame.draw.rect(self.screen,  PART_COLOR["dirt"],
                            (x + x_d, y + y_d, DIRT_SIZE, DIRT_SIZE))
            
            
    
class PTDocking(Part):
    def __init__(self, x: int, y: int):
        super().__init__(x, y, PT.Docking)
    
    def draw(self):
        padding = BLOCK_MARGIN
        w = h = BLOCK_SIZE - 2 * padding - 2 * BLOCK_MARGIN
        x, y = self.calcCoorOnScreen()
        pygame.draw.rect(self.screen, PART_COLOR.get("blank"), 
                        (x, y, BLOCK_SIZE, BLOCK_SIZE))
        
        x += padding
        y += padding
        pygame.draw.rect(self.screen, PART_COLOR.get("frame"), 
                        (x, y, BLOCK_SIZE - 2*padding, BLOCK_SIZE- 2*padding))
        pygame.draw.rect(self.screen, PART_COLOR.get("docking"), 
                        (x + BLOCK_MARGIN, y + BLOCK_MARGIN, w, h))    