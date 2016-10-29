

class RigidBody:
    def addLocation(self, locationTuple):
        if not hasattr(self, "gridLocation"):
            self.gridLocation = []
        self.gridLocation.append(locationTuple)
        
    def removeLocation(self, locationToRemove):
        for i in range(len(locationToRemove)):
            location = self.gridLocation[i]
            if location[0] == locationToRemove[0] and location[1] == locationToRemove[1]:
                del self.gridLocation[i]
                return
        raise Exception()