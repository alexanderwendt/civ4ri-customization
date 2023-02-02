# Civilization 4 Realism Invictus Customizations
Customizations for Civilization 4 Realism Invictus

## Prerequisites
These modifications were made for a Realism Invictus 3.5 (Realism Invictus 3.5 (2020-02-28) Setup (Full).exe) around the 2020-08-21 with hotfixes 1-7 applied.


Start settings for the game
Difficulty: 
* Monarch

Victory: 
* All except Time and Cultural

Settings:
* Raging barbarians
* No technology brokering
* No revolutions
* No barbarian civs
* Holy city migration
* No dynamic city naming
* Protect valuable units
* AI plays to win

Multiplayer
* Take over AI


## Customizations

### AI does not Raze big Cities
Modified function to prevent the AI to raze big cities. In ```.\Realism Invictus\Assets\Python\CvGameUtils.py``` 
enhance the method in the way you wish, e.g. that only cities with population < 6 can be destroyed.

```	
def canRazeCity(self,argsList):
	iRazingPlayer, pCity = argsList
	pRazingPlayer = gc.getPlayer(iRazingPlayer)

	# Can raze city if size smaller equal 4 or player is human
	if pCity.getPopulation() <= 4 or pRazingPlayer.isHuman():
		return True

	return False
```

To be able to raze cities, the settings of BUG have to be modified
In  ```.\Realism Invictus\Assets\Python\Components\BUG\BugGameUtils.py``` 
search the following line and replace ```True``` with ```None``` to be able to activate that function. 

```self._setDefault("canRazeCity", None)```

The reason can be found here https://forums.civfanatics.com/threads/making-cvgameutils-modular.323808/. "If your class doesn't define canTrain() BUG will use the one from CvGameUtils. 
Actually, BUG will use the registered default if there is one to avoid calling the functions in CvGameUtils. This is why it is so very important that you do not modify the original 
CvGameUtils as your modifications will be ignored."

Additional information: https://forums.civfanatics.com/threads/customizing-300bc-scenario.443668/

### Adapted World Settings
Adapted some settings of the world to prevent unwanted effects. File 
```.\Realism Invictus\Assets\XML\GameInfo\CIV4WorldInfo.xml```

Changes
1. Set target number of cities higher to encourage the AI to build more cities
2. iPerCityResearchCostModifier lowered from 9 to 1 as with many cities, the research takes far too long time to complete, like 40 turns
3. iDistanceMaintenancePercent lowered from 10 to 9 to make the players and computers to have more cities far away
4. iNumCitiesMaintenancePercent lowered from 25 to 50 to make players have more cities

For ```<Type>WORLDSIZE_HUGE</Type>``` do the following modifications

```
<!-- iTargetNumCities>6</iTargetNumCities -->
<iTargetNumCities>8</iTargetNumCities>
...
<!--<iPerCityResearchCostModifier>9</iPerCityResearchCostModifier>-->
<iPerCityResearchCostModifier>1</iPerCityResearchCostModifier>
<iTradeProfitPercent>30</iTradeProfitPercent>
<!--<iDistanceMaintenancePercent>100</iDistanceMaintenancePercent>-->
<iDistanceMaintenancePercent>90</iDistanceMaintenancePercent>
<!--<iNumCitiesMaintenancePercent>25</iNumCitiesMaintenancePercent>-->
<iNumCitiesMaintenancePercent>20</iNumCitiesMaintenancePercent>
```			
### Adapted Handicap Settings
To help the AI to expand to other continents, we lowered the penalty for distance and number of cities.
File ```.\Realism Invictus\Assets\XML\GameInfo\CIV4HandicapInfo.xml```

For ```<Type>HANDICAP_MONARCH</Type>```

```
<!-- iDistanceMaintenancePercent>90</iDistanceMaintenancePercent -->
<iDistanceMaintenancePercent>80</iDistanceMaintenancePercent>
<!-- iNumCitiesMaintenancePercent>80</iNumCitiesMaintenancePercent -->
<iNumCitiesMaintenancePercent>75</iNumCitiesMaintenancePercent>
```


## Tips and Tricks to Get the Game running Smoothly
I play several multiplayer games on the Worldmap Huge with 52 civilizations. We are 3-4 players, who play over Steam. Although it is tough, we managed to play the game to the modern era with 
only moderate rate of crashes. Here is my experience about how to handle multiplayer problems.

### Process Lasso
Use Process Lasso to free up memory at a certain point to reduce the risk of memory allocation failures.
How to use:
1. Install Process Lasso and buy it to have the effect longer than 30 days
2. Start Civ4 RI
3. Go to process lasso and press Process Lasso->right click on Civ4BeyondtheSword.exe->Set Watchdog Advanced Rules
4. Create a new rule with the following settings: 
- Process match: civ4beyondsword.exe
- for virtual memoryGreater than 825 megabytes
- for 30 seconds
- then trim virtual memory
- virtual memory metric to use: Any memory metric
5. Go to options->Memory->Enable Smart Trim

### Must DOs to prevent Crashes of the Game
The following setps increases the memory available for the application:
1. Download the 4GB Patch from https://ntcore.com/files/4gb_patch.zip (Website https://ntcore.com/?page_id=371) and apply it to 
```C:\Games\Steam\steamapps\common\Sid Meier's Civilization IV Beyond the Sword\Beyond the Sword\Civ4BeyondSword.exe```
2. Increase memory for the application with ```call bcdedit /set IncreaseUserVa 3072```, which is put into a script here ```.\scrips\civ4_preparation.bat```


### MAF Failure and other Graphic Problems
The following things help here:
1. Get a computer with a very good graphic card and much graphic card memory (not normal RAM) to lower the probablity for these kinds of errors
2. Lower the resolution to minimum in full screen mode
3. Option: single unit graphics
4. Option: graphical paging
5. CivilizationIV.ini: Paging out units and unit animations 
6. CivlizationIV.ini: Play in windowed mode and not in full screen

### Out of Sync Errors Unnoticed
The problem was that there emerged lots of out of sync errors early in the game. The worst thing was that it is not always noticeable that out of sync occurs. After some time, you see different things than other players. We play one very fast computer, two „normal“ computers and a slow laptop. It seems that the different speeds at which the computers are able to start a new turn cause frequent out of sync errors. 

#### Mitigation 1 
The idea is to slow down the loading of the turn at the fast computers. This is done in CivilizationIV.ini an efficient way by adding logging to the game. To slow down the fastest computer the most, please set the following in CivilizationIV.ini:
```
; Create a dump file if the application crashes
GenerateCrashDumps = 1

; Enable the logging system
LoggingEnabled = 1

; Enable synchronization logging
SynchLog = 1

; Overwrite old network and message logs
OverwriteLogs = 1

; Enable rand event logging
RandLog = 1

; Enable message logging
MessageLog = 1
```

We started by setting this to all computers and almost all out of sync immediately disappeared. At a later stage, as computers get slower, try to deactivate some of the logging options to speed up the slowest computer. The goal is that all computer finish turns with as little difference as possible.

### Out of Sync With Red, Blinking Message
Usually, at the beginning oft he turns, one computer takes pretty long to finish and then, an out of sync occurs.

#### Mitigation 1 
Like above, activate the logs

#### Mitigation 2
Restart all computers. Usually, out of sync happens very irregular. Some times it happens four times within an hour and then four hours not. Restarting all computers seem to help here.

#### Mitigation 3
In case the OOS seems to occur due to one of the slower computers, change or reduce the graphics resolution. Some resolutions do not seem to work very well on all computers. Also in windowed mode, the OOS still occur. Try another resolution. In the worst case, use the lowest possible resolution.

### Timeout of Turn Loading
If you play with a really slow computer, it is the last to start the new turn. There seems to be a timeout, which causes the connection to break after some time.

#### Mitigation 1
If logging is on due to sync errors, deactive some of the loggers to see an increase in starting speed.

#### Mitigation 2
Buy a new and faster computer. Thr 10y old game requires more resources than many new games.

### Fast Loading of Save Games in Multiplayer Mode
If the save game of the host is put in the ```.\My Games\beyond the sword\Saves\multi\auto```, no loading of a save game in the game is necessary and players save 5min at the loading.
In ```./scripts/Copy_Civ4_Save.bat``` the bat file is used to copy the current savegame over Dropbox from the host to all other multiplayers to prevent the really long loading times 
in the game. Each player uses it on his/her local machine.

