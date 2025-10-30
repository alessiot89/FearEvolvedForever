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
ConfigAddNPCSpawnEntriesContainer=(NPCSpawnEntriesContainerClassString="SE_DinoSpawnEntriesWyvern_C",NPCSpawnEntries=((AnEntryName="zombie_fire",EntryWeight=0.400000,NPCsToSpawnStrings=("Wyvern_Character_BP_ZombieFire_C")),(AnEntryName="zombie_lightning",EntryWeight=0.400000,NPCsToSpawnStrings=("Wyvern_Character_BP_ZombieLightning_C")),(AnEntryName="zombie_poison",EntryWeight=0.400000,NPCsToSpawnStrings=("Wyvern_Character_BP_ZombiePoison_C"))),NPCSpawnLimits=((NPCClassString="Wyvern_Character_BP_ZombieFire_C",MaxPercentageOfDesiredNumToAllow=1.000000),(NPCClassString="Wyvern_Character_BP_ZombieLightning_C",MaxPercentageOfDesiredNumToAllow=1.000000),(NPCClassString="Wyvern_Character_BP_ZombiePoison_C",MaxPercentageOfDesiredNumToAllow=1.000000)))
```
Adjust EntryWeight and MaxPercentageOfDesiredNumToAllow at your desire (you can read more about this in the official Wiki https://ark.wiki.gg/wiki/Server_configuration#Creature_Spawn_related)

## Warning
Please note that randomly forcing timings with console admin commands in an illogical way may break the plugin logic.

## What about the rest of event features?
Themed dinos, candy supply drops, meteor shower, boss moon, decorations and the island DodoRex aren't working on PC version of ASE.

For Dodorex fight, a similar code-logic as this plugin could be used for recreating the fight in The Island Map.
For themed dinos, it is possible to add them patching the GlobalNPCRandomSpawnClassWeights in APrimalWorldSettings::PreGameplaySetup(), however I did not found a proper way to make Dinosaur Bones (they will be deleted at server resetart) as well decorations, meteor shower, boss moon and Candy supply drops to work properly as well to use such idems for crafting.

Colors still works enabling the event, as well crafting recipes are exposed, however since they require non-referenced itmes (Dinosaur Bone etc..) they aren't working in the right way by default settings.

Recipes can be overridden to use other items, as example you could use Cooked Prime Meat:
```
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItem_ChibiDino_RandomCraftable_FearEvolved_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=6.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemConsumable_UnlockEmote_Dance_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=10.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemConsumable_UnlockEmote_FEKnock_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=10.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemConsumable_UnlockEmote_FEScare_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=10.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemConsumable_UnlockEmote_Panic_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=10.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemConsumable_UnlockEmote_Zombie_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=10.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_Costume_Skeleton_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=20.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_Costume_Strawman_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=20.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_HeadlessHat_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_PumpkinHat_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_PumpkinHat_Dino_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_ScaryFaceMask_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_SwimShirt_Araneo_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_SwimShirt_Onyc_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_SwimShirt_Pumpkin_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_SwimShirt_Reaper_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_SwimShirt_VampireDodo_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_Underwear_Araneo_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_Underwear_Onyc_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_Underwear_Pumpkin_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_Underwear_Reaper_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_FE_Underwear_VampireDodo_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_HawaiianShirt_DodoZombies_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_HawaiianShirt_Pumpkin_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_HawaiianShirt_Reaper_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_HawaiianShirt_Skellies_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_HawaiianShirt_Tentacles_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
ConfigOverrideItemCraftingCosts=(ItemClassString="PrimalItemSkin_HawaiianShirt_ZombieWyvern_C",BaseCraftingResourceRequirements=((ResourceItemTypeString="PrimalItemConsumable_CookedPrimeMeat_C",BaseResourceRequirement=5.000000,bCraftingRequireExactResourceType=True)))
```
Note: the above crafting recipes needs the event to be enable con the config file.
