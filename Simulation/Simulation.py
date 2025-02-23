import pygame
from sys import argv
from Part import *
# Settings
FPS = 10

class Simulation:

    def __init__(self, width: int, height: int):
        pygame.init()
        self.running = True
        self.screen = pygame.display.set_mode((width, height))
        self.font = pygame.font.SysFont(None, 40)
        self.clock = pygame.time.Clock()
        self.centerPosition = [W_BLOCKS // 2, H_BLOCKS // 2]     # (x, y)
        
        self.description = None
        self.maxSteps = None
        self.steps = 0
        self.maxBattery = None
        self.battery = None
        self.rows = None
        self.cols = None
        self.houseStructure = None
        
        self.numSteps = None
        self.finalDirtLeft = None
        self.dirtLeft = None
        self.status = None
        self.totalSteps = None
        
        self.iteration = 0
        
        self.robotPart = None
        self.dockingPart = None
        self.score = None
        
    def _updatePart(self, sgn: str, row: int, col: int):
        
        if sgn == WALL_SGN:
            self.houseStructure[row][col] = PTWall(col, row)
        
        elif sgn == DOCKING_SGN:
            self.houseStructure[row][col] = PTDocking(col, row)
            
            self.dockingPart = self.houseStructure[row][col]
            Part.dockingPart = self.dockingPart
        
            self.robotPart = PTRobot(col, row)

        else:
            dirtLevel = int(sgn) if '0' <= sgn <= '9' else 0
            self.houseStructure[row][col] = PTBlank(col, row, dirtLevel)
            
    def _loadHouseStructure(self, houseLines):
        
        for row in range(self.rows - 2):
            if row < len(houseLines):
                line = houseLines[row]
            else:
                line = " "
            for col in range(self.cols - 2):
                
                if col < len(line):
                    self._updatePart(line[col], row + 1, col + 1)

                else:
                    self._updatePart(CLEAN_SGN, row+1, col+1)
            
        for row in range(self.rows):
            for col in range(self.cols):
                if row == 0 or row == self.rows - 1 or col == 0 or col == self.cols - 1:
                    self.houseStructure[row][col] = PTEdgeWall(col, row)

    def _loadInputFile(self, filename):
        with open(filename, 'r') as file:       
            lines = file.readlines()
            self.description = lines[0].strip()
            pygame.display.set_caption(self.description)
            self.maxSteps = int(lines[1].split("=")[1].strip())
            self.maxBattery = int(lines[2].split("=")[1].strip())
            self.battery = int(self.maxBattery)
            self.rows = 2 + int(lines[3].split("=")[1].strip())
            self.cols = 2 + int(lines[4].split("=")[1].strip())
            self.houseStructure = [[None for col in range(self.cols)] for row in range(self.rows)]
            
            self._loadHouseStructure(lines[5:])
            
            # update xLeft and yTop to achive the center of the screen
            Part.staticVariableInit(self.robotPart ,self.screen, self.rows, self.cols)
            
            
    def _loadOutputFile(self, filename):
        with open(filename, 'r') as file:       
            lines = file.readlines()
            self.numSteps = int(lines[0].split("=")[1].strip())
            self.finalDirtLeft = int(lines[1].split("=")[1].strip())
            self.status = lines[2].split("=")[1].strip()
            self.score = int(lines[4].split("=")[1].strip())
            self.totalSteps = lines[6].strip()
        
    def _updateDirtLeft(self):
        self.dirtLeft = 0
        for parts in self.houseStructure:
            for part in parts:
                if isinstance(part, PTBlank):
                    self.dirtLeft += part.dirtLevel
    
    def _updateNextPosition(self, step: Step):
        if step == Step.North.value:
            self.robotPart.y -= 1
        elif step == Step.East.value:
            self.robotPart.x += 1
        elif step == Step.South.value:
            self.robotPart.y += 1
        elif step == Step.West.value:
            self.robotPart.x -= 1
        elif step == Step.Stay.value:
            pass
        elif step == Step.Finish.value:
            pass
        
    def _tryToClean(self):
        row, col = self.robotPart.y, self.robotPart.x
        if self.houseStructure[row][col].dirtLevel > 0:
            self.houseStructure[row][col].dirtLevel -= 1
            self.dirtLeft -= 1
            
    def _tryToCharge(self):
        if self._atDocking():
            self.battery = ceil(min(self.battery + self.maxBattery / 20, self.maxBattery))
            
    def _executeStep(self):

        if self.iteration == len(self.totalSteps):
            return False

        step = self.totalSteps[self.iteration]
        
        if step == Step.Finish.value:
            return False
        
        elif step == Step.Stay.value:
            self._tryToCharge()
            self._tryToClean()
        
        else:
            self._updateNextPosition(step)
        
        if not self._atDocking():
            self.battery -= 1

        self.iteration += 1
        return True
    
    def _atDocking(self):
        return self.robotPart.x == self.dockingPart.x and self.robotPart.y == self.dockingPart.y
    
    def _shiftFrameIfNeeded(self):
        xLeftBorder = Part.xMid - W_BLOCKS // 2
        xRightBorder = Part.xMid + W_BLOCKS // 2 - 1
        yTopBorder = Part.yMid - H_BLOCKS // 2
        yBottomBorder = Part.yMid + H_BLOCKS // 2 - 1

        if self.robotPart.x == xLeftBorder:
            Part.xLeft += 1 
            Part.xMid -= 1 
        elif self.robotPart.x == xRightBorder:
            Part.xLeft -= 1 
            Part.xMid += 1 
        elif self.robotPart.y == yTopBorder:
            Part.yTop += 1 
            Part.yMid -= 1 
        elif self.robotPart.y == yBottomBorder:
            Part.yTop -= 1 
            Part.yMid += 1 
        
    def _drawHouseStructure(self):
        for parts in self.houseStructure:
            for part in parts:
                part.draw()
    
    def _drawUI(self):
        def drawText(text: str, x, y):
            text_surface = self.font.render(text, True, PART_COLOR["text"])
            self.screen.blit(text_surface, (x, y))

        drawText(f"Step: {self.iteration}/{self.maxSteps}", 10, 10)
        drawText(f"Battery: {self.battery}/{self.maxBattery}", 10, 50)    
        drawText(f"Dirt: {self.dirtLeft}", 10, 90)    
    
    def _drawMsg(self, text: str):
        text_surface = self.font.render(text, True, PART_COLOR["text"])
        x = WIDTH//2 - text_surface.get_size()[0] // 2
        y = HEIGHT//6 - text_surface.get_size()[1] // 2
        self.screen.blit(text_surface, (x, y))    
    
    def _drawMsg2(self, text: str):
        text_surface = self.font.render(text, True, PART_COLOR["text"])
        x = WIDTH//2 - text_surface.get_size()[0] // 2
        y = HEIGHT//6 - 5 * text_surface.get_size()[1] // 2
        self.screen.blit(text_surface, (x, y)) 
        
    def _startMenu(self):
        
        self._drawMsg("Press any key to start the simulation")
        pygame.display.flip()

        pause = True
        quit = False
        while pause and not quit:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    quit = True

                if event.type == pygame.KEYDOWN:
                    pause = False
        
            self.clock.tick(FPS)
        
        return quit
    
    def run(self, inputName: str, outputName: str):
            
        self._loadInputFile(inputName)
        self._loadOutputFile(outputName)
        self._updateDirtLeft()

        once = True
        running = True
        quit = False
        while running and not quit:
            
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    quit = True
            
            # Simulation iteration #
            self.screen.fill(PART_COLOR["background"])
            self._shiftFrameIfNeeded()
            self._drawHouseStructure()
            self.robotPart.draw()
            self._drawUI()
            running = self._executeStep()
            # -------------------- #
            
            pygame.display.flip()
            self.clock.tick(FPS)

            if once:
                once = False
                quit = self._startMenu()


        once = True
        while not quit:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    quit = True
            
            if once:    
                once = False
                self._drawMsg(f"Score: {self.score}")
                self._drawMsg2(self.status)
                pygame.display.flip()
                
            self.clock.tick(FPS)

        pygame.quit()
    

def main():
    if len(argv) != 3:
        print("Error: Invalid simulation filename")
        return

    sim = Simulation(WIDTH, HEIGHT)
    sim.run(argv[1], argv[2])


if __name__ == "__main__":
    main()