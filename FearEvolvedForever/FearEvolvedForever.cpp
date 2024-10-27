#pragma warning(push, 3)
#include <API/ARK/Ark.h>
#pragma warning(pop)
#pragma comment(lib, "ArkApi.lib")

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <limits>
#include <string>

#include "json.hpp"

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


// Dodo Wyvern spawns coords (position and rotation vectors).
TArray<std::pair<FVector, FRotator>> spawnCoords{};

TArray<int> packEventColorsSet{};
TArray<int> packSpecialColorsSet{};


APrimalDinoCharacter* dodoWyvern{ nullptr };
constexpr int zombiePackSize = 4;
TArray<APrimalDinoCharacter*> zombiePack;

constexpr auto const dodoWyvernBP_str{ "Blueprint'/Game/ScorchedEarth/Dinos/DodoWyvern/DodoWyvern_Character_BP.DodoWyvern_Character_BP'" };
constexpr auto const fireZombieBP{ "Blueprint'/Game/ScorchedEarth/Dinos/Wyvern/Wyvern_Character_BP_ZombieFire.Wyvern_Character_BP_ZombieFire'" };
constexpr auto const lightningZombieBP{ "Blueprint'/Game/ScorchedEarth/Dinos/Wyvern/Wyvern_Character_BP_ZombieLightning.Wyvern_Character_BP_ZombieLightning'" };
constexpr auto const poisonZombieBP{ "Blueprint'/Game/ScorchedEarth/Dinos/Wyvern/Wyvern_Character_BP_ZombiePoison.Wyvern_Character_BP_ZombiePoison'" };

std::ofstream debugLogFile;

void debugLog( const std::string& info )
{
    debugLogFile << info << std::endl;
}

enum class ZombieType : std::uint8_t
{
    Fire,
    Lightning,
    Poison,
    LastInvalid // Used for outer limit
};

void initLocations()
{
    auto placements = config["Placements"];
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
    try
    {
        auto eventColors = config["EventColors"];
        for( auto& entry : eventColors )
        {
            packEventColorsSet.Add( entry );
        }
    }
    catch( const std::exception& )
    {
        debugLog( "Failed to read event colors, default to 0 (no colors)" );
        packEventColorsSet.Empty();
        packEventColorsSet.Add( 0 );
    }

    try
    {
        auto specialColors = config["SpecialColors"];
        for( auto& entry : specialColors )
        {
            packSpecialColorsSet.Add( entry );
        }
    }
    catch( const std::exception& )
    {
        debugLog( "Failed to read special colors, default to 0 (no colors)" );
        packEventColorsSet.Empty();
        packEventColorsSet.Add( 0 );
    }
}

template<typename T>
T getRandomValue( T minValue, T outerLimit )
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
    return getRandomValue( static_cast<T>( 0 ), outerLimit );
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

void paintZombie( APrimalDinoCharacter* zombie )
{
    constexpr auto bigEnough = 10000;
    int extraction = getRandomValue( 1, 10000 );

    // 1.0 : 10000 = paintingChance : x
    auto paintingChanceLimit = static_cast<int>( bigEnough * paintingChance );
    // 1.0 : 10000 = paintingSpecialChance : x
    auto paintingSpecialChanceLimit = static_cast<int>( bigEnough * paintingSpecialChance );
    paintingSpecialChanceLimit += paintingChanceLimit;

    // Zombie painting chance: [0, paintingChanceLimit]
    // Zomie special painting chance: [paintingChanceLimit+1, paintingSpecialChanceLimit]
    // No painting: (paintingSpecialChanceLimit, bigEnough]
    constexpr int colorRegionsCount = 6;
    if( extraction <= paintingChanceLimit )
    {
        debugLog( "Zombie will have event painting!" );
        for( int i{}; i < colorRegionsCount; ++i )
        {
            zombie->ForceUpdateColorSets( i, packEventColorsSet[getRandomValue( packEventColorsSet.Num() )] );
        }
    }
    else if( extraction <= paintingSpecialChanceLimit )
    {
        debugLog( "Zombie will have special painting!" );
        for( int i{}; i < colorRegionsCount; ++i )
        {
            zombie->ForceUpdateColorSets( i, packSpecialColorsSet[getRandomValue( packSpecialColorsSet.Num() )] );
        }
    }
    else
    {
        debugLog( "Zombie will not have any painting at all!" );
    }
}

// add some locationoffset to the zombie pack.
void packOffset( FVector& location,
                 int packIndex )
{
    switch( packIndex )
    {
    case 0:
        location.X += 5000.0F;
        location.Y += 5000.0F;
        location.Z += 5000.0F;
        break;
    case 1:
        location.X -= 5000.0F;
        location.Y += 5000.0F;
        location.Z += 5000.0F;
        break;
    case 2:
        location.X += 5000.0F;
        location.Y += 5000.0F;
        location.Z -= 5000.0F;
        break;
    case 3:
        location.X -= 5000.0F;
        location.Y += 5000.0F;
        location.Z -= 5000.0F;
        break;
    default:
        debugLog( "WARNING: invalid pack index on pack offset computation! Index was: " + std::to_string( packIndex ) );
        break;
    }
}

APrimalDinoCharacter* spwanDodoWyvern()
{
    FString dodoWyvernBP{ dodoWyvernBP_str };
    UClass* dodoWyvernClass = UVictoryCore::BPLoadClass( &dodoWyvernBP );
    if( !dodoWyvernClass )
    {
        debugLog( "WARNING: Cannot load Dodo Wyvern class!" );
        return nullptr;
    }
    FActorSpawnParameters spawnParams;
    FVector* location{};
    FRotator* rotation{};
    pickSpawn( location, rotation );
    auto dodoWyvernChar = static_cast<APrimalDinoCharacter*>( ArkApi::GetApiUtils().GetWorld()->SpawnActor( dodoWyvernClass,
                                                                                                            location,
                                                                                                            rotation,
                                                                                                            &spawnParams ) );
    if( !dodoWyvernChar )
    {
        debugLog( "WARNING: Cannot spawn Dodo Wyvern!" );
        return nullptr;
    }
    // Spawn the 4 zombie wyvern pack around Dodo Wyvern.
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
        level += 38;
        zombiePack[packIndex] = APrimalDinoCharacter::SpawnDino( ArkApi::GetApiUtils().GetWorld(),
                                                                 subclass,
                                                                 *location,
                                                                 *rotation,
                                                                 5.0F,
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
            return dodoWyvernChar;
        }
        debugLog( "INFO: spawned a Zombie Wyvern of the above type of level " + std::to_string( level * 5 ) );
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
            // TODO: send message in chat about the night started + time ingame!
            debugLog( "Time: " + dbgTimeStr );
            debugLog( "It's night time!" );
        }
        // Check if is event-time.
        if( true == isEventTime )
        {
            // Check if event didn't yet start.
            if( false == isEventStarted )
            {
                debugLog( "Time: " + dbgTimeStr );
                debugLog( "Try to spawn Dodo Wyvern.." );
                dodoWyvern = spwanDodoWyvern();
                // TODO: send message in chat about the Dodo Wyvern appeared!
                isEventStarted = true;
                isEventEnded = false;
                debugLog( "Event started, Dodo Wyvern spanwed!" );

            }
            // Event started and did not end yet.
            else if( false == isEventEnded )
            {
                auto ptr = GetWeakReference( dodoWyvern );
                // If Dodo Wyvern has been slayed, event ends, notify in chat.
                if( nullptr == ptr )
                {
                    debugLog( "Time: " + dbgTimeStr );
                    debugLog( "DodoWwyvern has been slayed!" );
                    // TODO: send message in chat about the Dodo Wyvern appeared!
                    isEventEnded = true;
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
                    debugLog( "Time: " + dbgTimeStr );
                    debugLog( "WARNING: Night time ended, Dodo Wyvern not slayed and cannot find it! If the plugin just loaded, don't panic, it's OK" );
                }
                else
                {
                    debugLog( "Time: " + dbgTimeStr );
                    debugLog( "Night time ended, Dodo Wyvern fled away!" );
                    dodoWyvern->Destroy( false, true );
                    debugLog( "Destroy called on Dodo Wyvern" );
                    // TODO: send message in chat about the Dodo Wyvern fled away!
                }
                for( int i = 0; i < zombiePack.Num(); ++i )
                {
                    auto zombiePtr = GetWeakReference( zombiePack[i] );
                    if( nullptr == zombiePtr )
                    {
                        std::string info = "No zombie found for zombie pack #" + std::to_string( i );
                        debugLog( info );
                    }
                    else
                    {
                        zombiePtr->Destroy( false, true );
                        debugLog( "Destroy called for zombie pack #" + std::to_string( i ) );
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
    const std::string config_path = ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/FearEvolvedForever/Config.json";
    std::ifstream file{ config_path };
    if( !file.is_open() )
    {
        throw std::runtime_error( "Can't open Config.json" );
    }
    file >> config;
    float colorChance = std::numeric_limits<float>::infinity();
    float specialChance = std::numeric_limits<float>::infinity();
    try
    {
        colorChance = config["EventColorsChance"];
        debugLog( "Json EventColorsChance: " + std::to_string( colorChance ) );
        if( colorChance >= 1.0F )
        {
            colorChance = 1.0F;
        }
        else if( colorChance < 0.0F )
        {
            colorChance = 0.0F;
        }
        specialChance = config["SpecialColorsChance"];
        debugLog( "Json SpecialColorsChance: " + std::to_string( specialChance ) );
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
            debugLog( "Summed color chance values greater than 1.0! Let's keep the biggest value, and clamp the smaller" );
            colorChance >= specialChance
                ? specialChance = 1.0F - ( colorChance + specialChance )
                : colorChance = 1.0F - ( colorChance + specialChance );
        }
    }
    catch( const std::exception& )
    {
        debugLog( "Invalid color chance values, reverting to default (0.5 for EventColorsChance and 0.15 for SpecialColorsChance)" );
    }
    if( colorChance != std::numeric_limits<float>::infinity() )
    {
        paintingChance = colorChance;
        debugLog( "Zombie pack color spawn override chance set to " + std::to_string( paintingChance ) );
    }
    else
    {
        debugLog( "Invalid Zombie pack color spawn override chance, reverting to " + std::to_string( paintingChance ) );
    }
    if( specialChance != std::numeric_limits<float>::infinity() )
    {
        paintingSpecialChance = specialChance;
        debugLog( "Zombie pack color spawn override chance set to " + std::to_string( paintingSpecialChance ) );
    }
    else
    {
        debugLog( "Invalid Zombie pack special color spawn override chance, reverting to " + std::to_string( paintingSpecialChance ) );
    }
    file.close();
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
        debugLogFile.open( "_FearEvolved_forever_debugLog.txt", std::ios::app );
        debugLog( "================================================================================" );
        debugLog( "Fear Evolved Forever Plugin loaded" );
        debugLog( "================================================================================" );
        load();
        break;
    case DLL_PROCESS_DETACH:
        debugLog( "Plugin Unloading" );
        debugLogFile.close();
        unload();
        break;
    }
    return TRUE;
}
