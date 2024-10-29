#pragma warning(push, 3)
#include <API/ARK/Ark.h>
#include "json.hpp"
#pragma warning(pop)
#pragma comment(lib, "ArkApi.lib")

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <limits>
#include <string>

nlohmann::json config;

// Dawn time in seconds, about 04:30 ingame
constexpr float dawnTime = 16200.0F;
// Night start time in seconds, about 21:50 ingame
constexpr float nightTime = 78600.0F;
constexpr float midnight = 0.0F;

bool isNight = false;
bool isEventTime = false;
bool isEventStarted = false;
bool isEventEnded = false;
bool isNightNotified = false;

float paintingChance = 0.5F;
float paintingSpecialChance = 0.15F;

float difficulty = 5.0F;

// Dodo Wyvern spawns coords (position and rotation vectors).
TArray<std::pair<FVector, FRotator>> spawnCoords{};

TArray<int> packEventColorsSet{};
TArray<int> packSpecialColorsSet{};

APrimalDinoCharacter* dodoWyvern{ nullptr };
constexpr int zombiePackSize = 4;
TArray<APrimalDinoCharacter*> zombiePack;

constexpr auto const dodoWyvernBP{ "Blueprint'/Game/ScorchedEarth/Dinos/DodoWyvern/DodoWyvern_Character_BP.DodoWyvern_Character_BP'" };
constexpr auto const fireZombieBP{ "Blueprint'/Game/ScorchedEarth/Dinos/Wyvern/Wyvern_Character_BP_ZombieFire.Wyvern_Character_BP_ZombieFire'" };
constexpr auto const lightningZombieBP{ "Blueprint'/Game/ScorchedEarth/Dinos/Wyvern/Wyvern_Character_BP_ZombieLightning.Wyvern_Character_BP_ZombieLightning'" };
constexpr auto const poisonZombieBP{ "Blueprint'/Game/ScorchedEarth/Dinos/Wyvern/Wyvern_Character_BP_ZombiePoison.Wyvern_Character_BP_ZombiePoison'" };

FString nightMessage;
FString midnightMessage;
FString victorMessage;
FString defeatMessage;
const FLinearColor displayColor{ 0.949F, 0.431F, 0.133F, 0.95F };
constexpr float displayScale = 3.0F;
constexpr float displayTime = 8.0F;

bool logDebug = true;
std::ofstream debugLogFile;

void debugLog( const std::string& info )
{
    if( logDebug )
    {
        debugLogFile << info << std::endl;
    }
}

//void debugLog( const std::wstring& info )
//{
//    debugLog(ArkApi::Tools::Utf8Encode( info ) );
//}

enum class ZombieType : std::uint8_t
{
    Fire,
    Lightning,
    Poison,
    LastInvalid // Used for outer limit
};

void initLocations()
{
    auto& placements = config["Placements"];
    for( auto& entry : placements )
    {
        spawnCoords.Add( { FVector( entry["x"],
                                    entry["y"],
                                    entry["z"] ),
                         FRotator( 0.0F,
                                   entry["yaw"],
                                   0.0F ) } );
    }
}

void initColors()
{
    // Read event colors.
    try
    {
        auto& eventColors = config["EventColors"];
        for( auto& entry : eventColors )
        {
            packEventColorsSet.Add( entry );
        }
    }
    catch( const std::exception& )
    {
        debugLog( "WARNING: Failed to read event colors, default to 0 (no colors)" );
        packEventColorsSet.Empty();
        packEventColorsSet.Add( 0 );
    }
    // Read special colors
    try
    {
        auto& specialColors = config["SpecialColors"];
        for( auto& entry : specialColors )
        {
            packSpecialColorsSet.Add( entry );
        }
    }
    catch( const std::exception& )
    {
        debugLog( "WARNING: Failed to read special colors, default to 0 (no colors)" );
        packEventColorsSet.Empty();
        packEventColorsSet.Add( 0 );
    }
}

template<typename T>
T getRandomValue( T minValue,
                  T outerLimit )
{
    do
    {
        // If you don't like this for science, go and use any (slower) engine of <random>!
        auto value = static_cast<T>( std::rand() / ( ( RAND_MAX + 1u ) / static_cast<unsigned>( outerLimit ) ) );
        if( value >= minValue && value < outerLimit )
        {
            return value;
        }
    }
    while( true );
}

template<typename T>
T getRandomValue( T outerLimit )
{
    return getRandomValue( static_cast<T>( 0 ),
                           outerLimit );
}

void pickSpawn( FVector*& position,
                FRotator*& rotation )
{
    auto randomIndex = getRandomValue( spawnCoords.Num() );
    position = &spawnCoords[randomIndex].first;
    rotation = &spawnCoords[randomIndex].second;
    std::string info{ "INFO: picked location at index " };
    info += std::to_string( randomIndex );
    info += " having coords:";
    info += " " + std::to_string( spawnCoords[randomIndex].first.X );
    info += " " + std::to_string( spawnCoords[randomIndex].first.Y );
    info += " " + std::to_string( spawnCoords[randomIndex].first.Z );
    debugLog( info );
}

// Paints Zombie Wyverns according to config.
void paintZombie( APrimalDinoCharacter* zombie )
{
    constexpr auto bigEnough = 10000;
    int extraction = getRandomValue( 1,
                                     bigEnough );
    // 1.0 : 10000 = paintingChance : x
    auto paintingChanceLimit = static_cast<int>( bigEnough * paintingChance );
    // 1.0 : 10000 = paintingSpecialChance : x
    auto paintingSpecialChanceLimit = static_cast<int>( bigEnough * paintingSpecialChance );
    paintingSpecialChanceLimit += paintingChanceLimit;
    constexpr int colorRegionsCount = 6;
    // Zombie painting chance: [0, paintingChanceLimit]
    // Zomie special painting chance: [paintingChanceLimit+1, paintingSpecialChanceLimit]
    // No painting: (paintingSpecialChanceLimit, bigEnough]
    if( extraction <= paintingChanceLimit )
    {
        debugLog( "INFO: Zombie will have event painting!" );
        for( int i{}; i < colorRegionsCount; ++i )
        {
            zombie->ForceUpdateColorSets( i,
                                          packEventColorsSet[getRandomValue( packEventColorsSet.Num() )] );
        }
    }
    else if( extraction <= paintingSpecialChanceLimit )
    {
        debugLog( "INFO: Zombie will have special painting!" );
        for( int i{}; i < colorRegionsCount; ++i )
        {
            zombie->ForceUpdateColorSets( i,
                                          packSpecialColorsSet[getRandomValue( packSpecialColorsSet.Num() )] );
        }
    }
    else
    {
        debugLog( "INFO: Zombie will not have any painting at all!" );
    }
}

// Adds some offset for the Zombie pack from the Dodo Wyvern spawn point.
void packOffset( FVector& location,
                 int packIndex )
{
    // Offset is in UE units, 1UE unit ~ 1cm.
    constexpr float offset = 5000.0F;
    switch( packIndex )
    {
    case 0:
        location.X += offset;
        location.Y += offset;
        location.Z += offset;
        break;
    case 1:
        location.X -= offset;
        location.Y += offset;
        location.Z += offset;
        break;
    case 2:
        location.X += offset;
        location.Y += offset;
        location.Z -= offset;
        break;
    case 3:
        location.X -= offset;
        location.Y += offset;
        location.Z -= offset;
        break;
    default:
        debugLog( "WARNING: invalid pack index on pack offset computation! Index was: " + std::to_string( packIndex ) );
        break;
    }
}

// Spawns Dodo Wyvern and 4 high level Zombie Wyvern pack guard with random difficulty and colors according to config.
APrimalDinoCharacter* spwanDodoWyvern()
{
    // Spawn Dodo Wyvern.
    FString dodoWyvernBPPath{ dodoWyvernBP };
    UClass* dodoWyvernClass = UVictoryCore::BPLoadClass( &dodoWyvernBPPath );
    if( !dodoWyvernClass )
    {
        debugLog( "WARNING: Cannot load Dodo Wyvern class!" );
        return nullptr;
    }
    FVector* location{};
    FRotator* rotation{};
    pickSpawn( location,
               rotation );
    auto dodoWyvernChar = APrimalDinoCharacter::SpawnDino( ArkApi::GetApiUtils().GetWorld(),
                                                           TSubclassOf<APrimalDinoCharacter>( dodoWyvernClass ),
                                                           *location,
                                                           *rotation,
                                                           difficulty,
                                                           getRandomValue( 39 ),
                                                           false,
                                                           true,
                                                           0,
                                                           false,
                                                           1.0F,
                                                           0,
                                                           false );
    if( !dodoWyvernChar )
    {
        debugLog( "WARNING: Cannot spawn Dodo Wyvern!" );
        return nullptr;
    }
    // Spawn the 4 Zombie Wyverns pack around Dodo Wyvern.
    for( int packIndex = 0; packIndex < zombiePackSize; ++packIndex )
    {
        auto randomType = getRandomValue( ZombieType::LastInvalid );
        FString zombieBP;
        switch( randomType )
        {
        case ZombieType::Fire:
            zombieBP = fireZombieBP;
            debugLog( "INFO: a Zombie Fire Wyvern will spawn in the pack" );
            break;
        case ZombieType::Lightning:
            zombieBP = lightningZombieBP;
            debugLog( "INFO: a Zombie Lightning Wyvern will spawn in the pack" );
            break;
        case ZombieType::Poison:
            zombieBP = poisonZombieBP;
            debugLog( "INFO: a Zombie Poison Wyvern will spawn in the pack" );
            break;
        }
        UClass* zombieClass = UVictoryCore::BPLoadClass( &zombieBP );
        if( !zombieClass )
        {
            debugLog( "WARNING: Cannot load Zombie Wyvern being " + zombieBP.ToString() );
            return nullptr;
        }
        packOffset( *location,
                    packIndex );
        TSubclassOf<APrimalDinoCharacter> subclass( zombieClass );
        int level = getRandomValue( 39 );
        // Zombie Pack guard will be high level spawns, ie over max wild value (default for Wyverns is difficulty * 38)
        level += 38;
        zombiePack[packIndex] = APrimalDinoCharacter::SpawnDino( ArkApi::GetApiUtils().GetWorld(),
                                                                 subclass,
                                                                 *location,
                                                                 *rotation,
                                                                 difficulty,
                                                                 level,
                                                                 false,
                                                                 true,
                                                                 0,
                                                                 false,
                                                                 1.0F,
                                                                 0,
                                                                 false );
        if( !zombiePack[packIndex] )
        {
            debugLog( "WARNING: Cannot spawn Zombie Wyvern being " + zombieBP.ToString() );
            dodoWyvernChar->BeginPlay();
            return dodoWyvernChar;
        }
        debugLog( "INFO: spawned a Zombie Wyvern of the above type with level " + std::to_string( level * 5 ) );
        zombiePack[packIndex]->BeginPlay();
        paintZombie( zombiePack[packIndex] );
    }
    dodoWyvernChar->BeginPlay();
    return dodoWyvernChar;
}

/*
void Tick( float DeltaSeconds )
{
    NativeCall<void, float>( this, "AShooterGameState.Tick", DeltaSeconds );
}
*/
DECLARE_HOOK( AShooterGameState_Tick, void, AShooterGameState*, float );

void hook_AShooterGameState_Tick( AShooterGameState* gameState,
                                  float deltaSeconds )
{
    const float currentTime = gameState->DayTimeField();
    const std::string dbgTimeStr = std::to_string( currentTime );
    // Check day-time.
    if( ( currentTime >= dawnTime ) and ( currentTime < nightTime ) )
    {
        isNight = false;
        isEventTime = false;
    }
    else // ( ( currentTime >= nightTime ) or ( currentTime <= dawnTime ) )
    {
        isNight = true;
        isEventTime = false;
        // Check event time.
        if( ( currentTime >= midnight ) and ( currentTime < dawnTime ) )
        {
            isEventTime = true;
        }
    }
    // Check if it is night.
    if( true == isNight )
    {
        // Check if night just started and needs to be notified.
        if( ( false == isEventTime ) and ( false == isNightNotified ) )
        {
            isNightNotified = true;
            ArkApi::GetApiUtils().SendNotificationToAll( displayColor,
                                                         displayScale,
                                                         displayTime,
                                                         nullptr, nightMessage.GetCharArray().GetData() );
            ArkApi::GetApiUtils().SendServerMessageToAll( displayColor,
                                                          nightMessage.GetCharArray().GetData() );
            debugLog( "INFO: Time " + dbgTimeStr + ", it's night time!" );
        }
        // Check if is event-time.
        if( true == isEventTime )
        {
            // Check if event didn't yet start.
            if( false == isEventStarted )
            {
                debugLog( "INFO: Time " + dbgTimeStr + ", try to spawn Dodo Wyvern.." );
                dodoWyvern = spwanDodoWyvern();
                isEventStarted = true;
                isEventEnded = false;
                ArkApi::GetApiUtils().SendNotificationToAll( displayColor,
                                                             displayScale,
                                                             displayTime,
                                                             nullptr, midnightMessage.GetCharArray().GetData() );
                ArkApi::GetApiUtils().SendServerMessageToAll( displayColor,
                                                              midnightMessage.GetCharArray().GetData() );
                debugLog( "INFO: Event started, Dodo Wyvern spanwed!" );
            }
            // Event started and did not end yet.
            else if( false == isEventEnded )
            {
                auto ptr = GetWeakReference( dodoWyvern );
                // If Dodo Wyvern has been slayed, event ends, notify in chat.
                // Note: pointer is null only if corpse has been removed.
                if( nullptr == ptr or ptr->IsDead() )
                {
                    debugLog( "INFO: Time " + dbgTimeStr + ", DodoWwyvern has been slayed" );
                    isEventEnded = true;
                    ArkApi::GetApiUtils().SendNotificationToAll( displayColor,
                                                                 displayScale,
                                                                 displayTime,
                                                                 nullptr, victorMessage.GetCharArray().GetData() );
                    ArkApi::GetApiUtils().SendServerMessageToAll( displayColor,
                                                                  victorMessage.GetCharArray().GetData() );
                }
            }
        }
    }
    // Event time is over.
    else if( false == isNight )
    {
        // Check if event is still running.
        if( false == isEventEnded )
        {
            // Dodo Wyvern was not slayed, must be removed from map (as well it's guard pack).
            {
                auto ptr = GetWeakReference( dodoWyvern );
                if( nullptr == ptr )
                {
                    debugLog( "WARNING: Time: " + dbgTimeStr + ", Night time ended, Dodo Wyvern not slayed and cannot find it! If the plugin just loaded, don't panic, it's OK" );
                }
                else
                {
                    debugLog( "INFO: Time: " + dbgTimeStr + ", night time ended, Dodo Wyvern fled away!" );
                    dodoWyvern->Destroy( false,
                                         true );
                    debugLog( "INFO: Destroy called on Dodo Wyvern" );
                    ArkApi::GetApiUtils().SendNotificationToAll( displayColor,
                                                                 displayScale,
                                                                 displayTime,
                                                                 nullptr, defeatMessage.GetCharArray().GetData() );
                    ArkApi::GetApiUtils().SendServerMessageToAll( displayColor,
                                                                  defeatMessage.GetCharArray().GetData() );
                }
                for( int i = 0; i < zombiePack.Num(); ++i )
                {
                    auto zombiePtr = GetWeakReference( zombiePack[i] );
                    if( nullptr == zombiePtr )
                    {
                        debugLog( "INFO: No character found for zombie pack #" + std::to_string( i ) );
                    }
                    else
                    {
                        zombiePtr->Destroy( false,
                                            true );
                        debugLog( "INFO: Destroy called for zombie pack #" + std::to_string( i ) );
                    }
                }
            }
        }
        isEventStarted = false;
        isEventEnded = true;
        isNightNotified = false;
    }
    return( AShooterGameState_Tick_original( gameState,
                                             deltaSeconds ) );
}

void ReadConfig()
{
    const std::string configPath = ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/FearEvolvedForever/Config.json";
    std::ifstream file{ configPath };
    if( !file.is_open() )
    {
        throw std::runtime_error( "Can't open Config.json" );
    }
    file >> config;
    file.close();
    const std::string logPath = ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/FearEvolvedForever/DebugLog.txt";
    // Check for debug log setting
    try
    {
        logDebug = config["DebugLog"];
        if( logDebug )
        {
            debugLogFile.open( logPath, std::ios::app );
            debugLog( "================================================================================" );
            debugLog( "Fear Evolved Forever Plugin loaded" );
            debugLog( "================================================================================" );
        }
    }
    catch( const std::exception& )
    {
        debugLogFile.open( logPath, std::ios::app );
        debugLog( "================================================================================" );
        debugLog( "Fear Evolved Forever Plugin loaded" );
        debugLog( "================================================================================" );
        debugLog( "WARNING: Failed to read DebugLog setting, reverted to default (enabled)" );
    }
    // Try to read Dodo Wyvern and pack difficulty.
    try
    {
        difficulty = config["Difficulty"];
        debugLog( "INFO: Plugin Difficulty: " + std::to_string( difficulty ) );
    }
    catch( const std::exception& )
    {
        difficulty = 5.0F;
        debugLog( "WARNING: Invalid difficulty value, reverting to default 5.0 (Official settings)" );
    }
    float colorChance = std::numeric_limits<float>::infinity();
    float specialChance = std::numeric_limits<float>::infinity();
    // Try to read pack color settings
    try
    {
        colorChance = config["EventColorsChance"];
        debugLog( "INFO: Json EventColorsChance: " + std::to_string( colorChance ) );
        if( colorChance >= 1.0F )
        {
            colorChance = 1.0F;
        }
        else if( colorChance < 0.0F )
        {
            colorChance = 0.0F;
        }
        specialChance = config["SpecialColorsChance"];
        debugLog( "INFO: Json SpecialColorsChance: " + std::to_string( specialChance ) );
        if( specialChance >= 1.0F )
        {
            specialChance = 1.0F;
            colorChance = 0.0F;
        }
        else if( specialChance < 0.0F )
        {
            specialChance = 0.0F;
        }
        if( ( colorChance + specialChance ) > 1.0F )
        {
            debugLog( "WARNING: Invalid color chance values, their sum cannot be greater than 1.0, reverting to default values (0.5 for EventColorsChance, 0.15 for SpecialColorsChance)" );
        }
    }
    catch( const std::exception& )
    {
        debugLog( "WARNING: Invalid color chance values, reverting to default values (0.5 for EventColorsChance, 0.15 for SpecialColorsChance)" );
    }
    if( colorChance != std::numeric_limits<float>::infinity() )
    {
        paintingChance = colorChance;
        debugLog( "INFO: Zombie pack event color spawn override chance set to " + std::to_string( paintingChance ) );
    }
    else
    {
        debugLog( "WARNING: Invalid Zombie pack color spawn override chance, reverting to " + std::to_string( paintingChance ) );
    }
    if( specialChance != std::numeric_limits<float>::infinity() )
    {
        paintingSpecialChance = specialChance;
        debugLog( "INFO: Zombie pack special color spawn override chance set to " + std::to_string( paintingSpecialChance ) );
    }
    else
    {
        debugLog( "WARNING: Invalid Zombie pack special color spawn override chance, reverting to " + std::to_string( paintingSpecialChance ) );
    }
    // Try to read custom event messages.
    try
    {
        nightMessage = ArkApi::Tools::Utf8Decode( config["NightMessage"]).c_str();
        midnightMessage = ArkApi::Tools::Utf8Decode(config["MidnightMessage"]).c_str();
        victorMessage = ArkApi::Tools::Utf8Decode(config["VictoryMessage"]).c_str();
        defeatMessage = ArkApi::Tools::Utf8Decode(config["DefeatMessage"]).c_str();
    }
    catch( const std::exception& )
    {
        nightMessage = "Trick or treat? Midnight is coming, better watch your back!";
        midnightMessage = "Are you a hero or a fool? Face your fear, Her Majesty Dodo Wyvern is here!";
        victorMessage = "Praise you hero, the monster has been defeated. Claim your prize before the fight will be repeated!";
        defeatMessage = "Today you were not fast or strong enough..This is rough, Dodo Wyvern fled away!";
        debugLog( "Invalid event messages, reverting to hardcoded default" );
    }
}

void load()
{
    Log::Get().Init( "Fear Evolved Forever" );
    try
    {
        ReadConfig();
        initColors();
        initLocations();
    }
    catch( const std::exception& error )
    {
        Log::GetLog()->error( error.what() );
        throw error;
    }
    std::srand( static_cast<unsigned>( std::time( nullptr ) ) );
    zombiePack.Init( nullptr,
                     zombiePackSize );
    ArkApi::GetHooks().SetHook( "AShooterGameState.Tick",
                                &hook_AShooterGameState_Tick,
                                &AShooterGameState_Tick_original );
}

void unload()
{
    ArkApi::GetHooks().DisableHook( "AShooterGameState.Tick",
                                    &hook_AShooterGameState_Tick );
}

BOOL APIENTRY DllMain( HINSTANCE /*hinstDLL*/,
                       DWORD fdwReason,
                       LPVOID /*reserved*/ )
{
    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        load();
        break;
    case DLL_PROCESS_DETACH:
        if( logDebug )
        {
            debugLog( "Plugin Unloading" );
            debugLogFile.close();
        }
        unload();
        break;
    }
    return TRUE;
}
