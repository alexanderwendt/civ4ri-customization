# Civilization 4 Realism Invictus Customizations
Customizations for Civilization 4 Realism Invictus

## Prerequisites
These modifications were made for a Realism Invictus 3.72 r5469 (Realism Invictus 3.72 (2025-02-24) Setup (Full).exe).


Start settings for the game
Difficulty: 
* Monarch

Victory: 
* All except Time and Cultural

Settings:
* Raging barbarians
* No technology brokering 	(Allow tech trade, but only your own techs)
* No revolutions			
* No barbarian civs
* Holy city migration
* No dynamic City Naming 	(No extra city renaming as we use to rename our cities by ourself)
* No dynmaic civ name changes
* Protect valuable units
* AI plays to win
* Influence Driven War
* Switch leaders every era
* Military alliances

Multiplayer
* Take over AI

In game settings:
*Automated workers leave fortifications

civilization4.ini properties
```
; Specify the number of turns between autoSaves.  0 means no autosave.
AutoSaveInterval = 1
```


## Customizations
Following customizations were applied to the game for improved balance. 

### AI does not Raze big Cities
A problem is that the AI often razes cities with size 20 in a single turn as he conquers it. This is a 
modified function to prevent the AI to raze big cities. In ```.\Realism Invictus\Assets\Python\CvGameUtils.py``` 
enhance the method in the way you wish, e.g. that only cities with population <= 4 can be razed or human always have the 
choice to raze a city.

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

```
self._setDefault("canRazeCity", None)
```

The reason can be found here https://forums.civfanatics.com/threads/making-cvgameutils-modular.323808/. "If your class doesn't define canTrain() BUG will use the one from CvGameUtils. 
Actually, BUG will use the registered default if there is one to avoid calling the functions in CvGameUtils. This is why it is so very important that you do not modify the original 
CvGameUtils as your modifications will be ignored."

Additional information: https://forums.civfanatics.com/threads/customizing-300bc-scenario.443668/

### OBSOLETE Adapted World Settings
Adapted some settings of the world to prevent unwanted effects. File 
```.\Realism Invictus\Assets\XML\GameInfo\CIV4WorldInfo.xml```

Changes
1. Set target number of cities higher to encourage the AI to build more cities
2. iPerCityResearchCostModifier lowered from 9 to 1 as with many cities, the research takes far too long time to complete, like 40 turns
3. iDistanceMaintenancePercent lowered from 100 to 80 to make the players and computers to have more cities far away
4. iNumCitiesMaintenancePercent lowered from 35 to 15 to make players have more cities
5. iColonyMaintenancePercent lowered from 30 to 20 to make players have more cities

For ```<Type>WORLDSIZE_HUGE</Type>``` do the following modifications:

```
<!-- AW CUSTOM CHANGES BEGIN -->
<!-- iTargetNumCities>7</iTargetNumCities -->
<iTargetNumCities>8</iTargetNumCities>
<!-- AW CUSTOM CHANGES END -->
```

and further down in the XML 

```
<!-- AW CUSTOM CHANGES BEGIN -->
<!-- <iResearchPercent>92</iResearchPercent> -->
<iResearchPercent>92</iResearchPercent>
<!--<iPerCityResearchCostModifier>9</iPerCityResearchCostModifier>-->
<iPerCityResearchCostModifier>2</iPerCityResearchCostModifier>
<iTradeProfitPercent>30</iTradeProfitPercent>
<!--<iDistanceMaintenancePercent>100</iDistanceMaintenancePercent>-->
<iDistanceMaintenancePercent>80</iDistanceMaintenancePercent>
<!--<iNumCitiesMaintenancePercent>35</iNumCitiesMaintenancePercent>-->
<iNumCitiesMaintenancePercent>15</iNumCitiesMaintenancePercent>
<!--<iColonyMaintenancePercent>30</iColonyMaintenancePercent>-->
<iColonyMaintenancePercent>20</iColonyMaintenancePercent>
<!-- AW CUSTOM CHANGES END -->
```

### OBSOLETE Adapted Handicap Settings
To help the AI to expand to other continents, we lowered the penalty for distance and number of cities.
File ```.\Realism Invictus\Assets\XML\GameInfo\CIV4HandicapInfo.xml``` (copy the original to *.xml.orig)

For ```<Type>HANDICAP_MONARCH</Type>```

replace the following values 
```
<!-- AW CUSTOM CHANGES BEGIN -->
<!-- iDistanceMaintenancePercent>90</iDistanceMaintenancePercent -->
<iDistanceMaintenancePercent>75</iDistanceMaintenancePercent>
<!-- iNumCitiesMaintenancePercent>85</iNumCitiesMaintenancePercent -->
<iNumCitiesMaintenancePercent>70</iNumCitiesMaintenancePercent>
<!-- AW CUSTOM CHANGES END -->
```

### Adapt Research Pace During the Game
Often, it turns out that the research is not balanced, and especially it is noticable in the late game. In multiplayer 
games it is not possible to create a new map from the save game to fix it. Therefore, the research rate has to be "fixed" 
in the python scripts instead. It can be done by doing the following. Check that at the start of the game the 
option "set ahead of time research penalty" is activated. In the code, disable the ahead of time penalty and adapt it. 
This is done in the file ```.\Realism Invictus\Assets\Python\Components\AheadOfTime.py```. Replace the values with appropriate
values in the matrix.

```
Eras = {
0:	[-10000,	10,20,30,40,50,50],
1:	[-2000,		0,10,20,30,40,40],	## 2000 BC as a start date for that era, and then a list of all the tech cost modifiers
2:	[-1200,		0,5,20,30,40,40],	
3:	[0,			0,0,10,20,30,40],		
4:	[400,		0,0,5,20,30,30],
5:	[1100,		0,0,0,10,20,30],	
6:	[1400,		0,0,0,10,20,20],	
7:	[1600,		0,0,0,0,10,20],	
8:	[1750,		0,0,0,0,5,10],		
9:	[1910,		0,0,0,0,0,10],	
10:	[1945,		0,0,0,0,0,0]
}
```

In ```CvInfos.cpp``` the ahead of time has the following formula: 

```
iResearchCost = (m_iResearchCost * (100 + m_iAheadOfTime ) / 100);
```

It means that 0 is no change, +100 is the double effort and -100 is no effort for a research.

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
6. CivilizationIV.ini: Play in windowed mode and not in full screen

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

## How to Debug and Test a Game

## core dll compilation
In ```./compile-dll```, files necessary for the compilation of the realism invictus core dll can be found. How to use it:
1. Download and install Visual Studio 2022 Community version
2. Extract the deps.7z to a folder \[(FOLDER\]
3. Open ```CvGameCoreDLL.2012.sln``` in Visual Studio
4. In ```Makefile.settings.mk```, adapt the following paths
- ```TOOLKIT=\[(FOLDER\]\Microsoft Visual C++ Toolkit 2003```
- ```PSDK=\[(FOLDER\]\Microsoft Platform SDK```
5. Execute build for Release

## Pakbuilder Script
In ```./pakbuilder-script```, there is a script for building the pak files for RI. NOTE: To be able to create links, you need to start the powershell as administrator.
It creates the PAK files and copies all other asset files correctly to a mod folder to use.

## Python stubs
in ```./python-stubs```, python stubs are added. To apply them to PyCharm or any other IDE, include the stubs in the build.
In PyCharm, use Settings->Project structure and add all folders with python files to content root.

## Handling GFC error: failed to initialize the primary control theme
If you get into "GFC error: failed to initialize the primary control theme", the path in "C:\Games\Steam\steamapps\common\Sid Meier's Civilization IV Beyond the Sword\Beyond the Sword\Mods\Realism Invictus\Assets\XML\Art\CIV4ArtDefines_Misc.xml"
is wrong. It should match your file. Set the file path correct to Realism Invictus:
"C:\Games\Steam\steamapps\common\Sid Meier's Civilization IV Beyond the Sword\Beyond the Sword\Mods\Realism Invictus\Resource\Realism Invictus.thm" and here, set the content to
```
// *** Control Bitmap Theme file

// Set the resource 
resource_path "Mods/Realism Invictus/Resource";

// Setup common properties
include "Mods/Realism Invictus/Resource/Themes/Realism Invictus/Civ4Theme.thm";
```
Rename Directory
"C:\Games\Steam\steamapps\common\Sid Meier's Civilization IV Beyond the Sword\Beyond the Sword\Mods\Realism Invictus\Resource\Themes\Realism Invictus"

```
<MiscArtInfo>
			<Type>DEFAULT_THEME_NAME</Type>
			<Path>Mods/Realism Invictus/Resource/Realism Invictus.thm</Path>
			<fScale>0.0</fScale>
			<NIF>None</NIF>
			<KFM>None</KFM>
		</MiscArtInfo>
```

Source: https://www.moddb.com/games/civilization-iv/downloads/civ4bts-ffh2-game-of-thrones-mod

# Additional Guides
DLL complile: https://forums.civfanatics.com/threads/the-easiest-way-to-compile-a-new-dll.608137/
Debug console commands: https://forums.civfanatics.com/threads/tutorial-in-game-cheats-modding-debugging-using-console-commands.145278/
Debug console commands: https://www.liveabout.com/civilization-iv-cheats-pc-3401827#:~:text=Open%20the%20developer%20console%20with,enter%20the%20code%20or%20hotkey.
Learn how to mod the SDK: https://forums.civfanatics.com/threads/best-way-to-learn-how-to-mod-the-sdk.677705/

Python mock extensions: https://github.com/civ4-mp/pbmod/tree/master/tests/Pylint_for_Civ4/Civ4PythonApi
Python mock extensions 2: https://github.com/civ4-mp/pbmod/tree/master

