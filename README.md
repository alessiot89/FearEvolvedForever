# FearEvolvedForever
Re-implements Fear Evolved Dodo Wyvern event boss fight mechanics for Scorched Earth

## How does it work?
At midnight on a random location a Dodo Wyvern and extra 4 high level zombie wyvern minions will spawn on Scorched Earth map.
You have time until dawn (5:15 AM) to slay Dodo Wyvern. Once slayed all wild Zombie Wyverns already spawned on the map will be claimable (including the 4 high levels if still alive).
At 5:30 Dodo Wyvern will flee away until next middnight.
In the Config.json file it is possible to customize some settings such difficulty, Zombie Pack color chances, two custom color sets, announcement message texts, toggle the debug log and finally change the possible spawn locations of the Dodo Wyvern.

## Add extra Zombie Wyverns on the map
You can add more Zombie Wyvern to claim (mostly around the trench area) overriding the spawn entries container (in Game.ini) as follow:
```
ConfigAddNPCSpawnEntriesContainer=(NPCSpawnEntriesContainerClassString="SE_DinoSpawnEntriesWyvern_C",NPCSpawnEntries=((AnEntryName="zombie_fire",EntryWeight=0.500000,NPCsToSpawnStrings=("Wyvern_Character_BP_ZombieFire_C")),(AnEntryName="zombie_lightning",EntryWeight=0.500000,NPCsToSpawnStrings=("Wyvern_Character_BP_ZombieLightning_C")),(AnEntryName="zombie_poison",EntryWeight=0.500000,NPCsToSpawnStrings=("Wyvern_Character_BP_ZombiePoison_C"))),NPCSpawnLimits=((NPCClassString="Wyvern_Character_BP_ZombieFire_C",MaxPercentageOfDesiredNumToAllow=1.000000),(NPCClassString="Wyvern_Character_BP_ZombieLightning_C",MaxPercentageOfDesiredNumToAllow=1.000000),(NPCClassString="Wyvern_Character_BP_ZombiePoison_C",MaxPercentageOfDesiredNumToAllow=1.000000)))
```
Adjust EntryWeight and MaxPercentageOfDesiredNumToAllow at your desire (you can read more about this in the official Wiki https://ark.wiki.gg/wiki/Server_configuration#Creature_Spawn_related)

## Warning
Please note that randomly forcing timings with console admin commands in an illogical way may break the plugin logic.
