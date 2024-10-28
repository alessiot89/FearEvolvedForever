# FearEvolvedForever

Attempt to re-implement Fear Evolved mechanics for Scorched EArth

# Instructions .. aka, how does it work?
At middnight on a random location a Dodo Wyvern and extra 4 high level zombie wyvern minions will spawn on SE map.
You have time until dawn (4:30 AM) to slay Dodo Wyvern. Once slayed all wild Zombie Wyverns already spawned on the map will be claimable (including the 4 high levels if still alive).
At 4:30 Dodo Wyvern will flee away until next middnight.

# HOW to make it work
Dodo Wyvern assets is still present in game files of SE map but not loaded and referenced in-gane. This means it will FAIL to spawn properly, even with code (unless you tell me how to hack the dino assets reference table).
The only workaround that will make the plugin to WORK correctly is:
1. Start the server with the plugin
2. Wait or force (use cheat settimeofday) the mid-night: a Dodo Wyvern will attempt to spawn but will be invisible and not in play-state.
3. Immidiately save the game and restart the server
4. Dodo Wyvern and its additional pack will spawn at every mid-night as expected.
5. You can add more Zombie Wyvern to claim overriding the spawn entries container (in Game.ini) as follow:
```
ConfigAddNPCSpawnEntriesContainer=(NPCSpawnEntriesContainerClassString="SE_DinoSpawnEntriesWyvern_C",NPCSpawnEntries=((AnEntryName="zombie_fire",EntryWeight=0.500000,NPCsToSpawnStrings=("Wyvern_Character_BP_ZombieFire_C")),(AnEntryName="zombie_lightning",EntryWeight=0.500000,NPCsToSpawnStrings=("Wyvern_Character_BP_ZombieLightning_C")),(AnEntryName="zombie_poison",EntryWeight=0.500000,NPCsToSpawnStrings=("Wyvern_Character_BP_ZombiePoison_C"))),NPCSpawnLimits=((NPCClassString="Wyvern_Character_BP_ZombieFire_C",MaxPercentageOfDesiredNumToAllow=1.000000),(NPCClassString="Wyvern_Character_BP_ZombieLightning_C",MaxPercentageOfDesiredNumToAllow=1.000000),(NPCClassString="Wyvern_Character_BP_ZombiePoison_C",MaxPercentageOfDesiredNumToAllow=1.000000)))
```
You can read more about this in the official Wiki https://ark.wiki.gg/wiki/Server_configuration#Creature_Spawn_related

## TODO:
* Add more and better spawn locations
