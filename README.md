# Civilization 4 Realism Invictus Customizations
Customizations for Civilization 4 Realism Invictus

## Prerequisites
These modifications were made for Realism Invictus Git commit [XXX]

## Customizations

### AI does not Raze big Cities
Modified function to prevent the AI to raze big cities

```	def canRazeCity(self,argsList):
		iRazingPlayer, pCity = argsList
		pRazingPlayer = gc.getPlayer(iRazingPlayer)

		# Can raze city if size smaller equal 4 or player is human
		if pCity.getPopulation() <= 4 or pRazingPlayer.isHuman():
			return True
		
		return False
```

### Adapted World Size Huge Settings
Adapted some settings of the world to prevent unwanted effects
1. iPerCityResearchCostModifier lowered from 9 to 1 as with many cities, the research takes far too long time to complete, like 40 turns
2. iDistanceMaintenancePercent lowered from 10 to 9 to make the players and computers to have more cities far away
3. iNumCitiesMaintenancePercent lowered from 25 to 50 to make players have more cities

```
			<!--<iPerCityResearchCostModifier>9</iPerCityResearchCostModifier>-->
			<iPerCityResearchCostModifier>1</iPerCityResearchCostModifier>
			<iTradeProfitPercent>30</iTradeProfitPercent>
			<!--<iDistanceMaintenancePercent>100</iDistanceMaintenancePercent>-->
			<iDistanceMaintenancePercent>90</iDistanceMaintenancePercent>
			<!--<iNumCitiesMaintenancePercent>25</iNumCitiesMaintenancePercent>-->
			<iNumCitiesMaintenancePercent>20</iNumCitiesMaintenancePercent>
```			
