#include <API/ARK/Ark.h>
#pragma comment(lib, "ArkApi.lib")

#include <fstream>

// Dawn time in seconds, about 04:30
constexpr float dawnTime = 16200.0F;
// Night time in seconds, about 21:50
constexpr float nightTime = 78600.0F;
constexpr float midnight = 0.0F;

bool isNight = false;
bool isEventTime = false;
bool isEventStarted = false;
bool isEventEnded = false;
bool isNightNotified = false;

// TODO: add location spawn coords.
TArray<FVector> locations{ { -199600.0F, 161824.0F, 38357.0F } };

APrimalDinoCharacter* dodoWyvern = nullptr;

std::ofstream debugLogFile;

void debugLog( const std::string& info )
{
    debugLogFile << info << std::endl;
}

APrimalDinoCharacter* spwanDodoWyvern()
{
    FString dodoWyvernBP = "Blueprint'/Game/ScorchedEarth/Dinos/DodoWyvern/DodoWyvern_Character_BP.DodoWyvern_Character_BP'";
    UClass* dodoWyvernClass = UVictoryCore::BPLoadClass( &dodoWyvernBP );
    if( !dodoWyvernClass )
    {
        debugLog( "Cannot load Dodo Wyvern class!" );
        return nullptr;
    }
    // TODO: add random location peaker.
    // TODO: add queen minion pack of high level zombies.
    FRotator rotation{};
    FActorSpawnParameters spawnParams;
    auto dino = static_cast<APrimalDinoCharacter*>( ArkApi::GetApiUtils().GetWorld()->SpawnActor( dodoWyvernClass, &locations[0], &rotation, &spawnParams ) );
    if( !dino )
    {
        debugLog( "Cannot spawn Dodo Wyvern!" );
        return nullptr;
    }
    dino->BeginPlay();
    return dino;
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
                //auto ptr = GetWeakReference( dodoWyvern.Get() );
                if( nullptr == ptr )
                {
                    debugLog( "Time: " + dbgTimeStr );
                    debugLog( "Night time ended, Dodo Wyvern not slayed but cannot find it!" );
                }
                else
                {
                    debugLog( "Time: " + dbgTimeStr );
                    debugLog( "Night time ended, Dodo Wyvern fleed away!" );
                    dodoWyvern->Destroy( false, true );
                    debugLog( "Destroy called" );
                }
            }
        }
        isEventStarted = false;
        isEventEnded = true;
        isNightNotified = false;
    }
    return( AShooterGameState_Tick_original( gameState, deltaSeconds ) );
}

void load()
{
    Log::Get().Init( "Fear Evolved Forever" );
    auto& hooks = ArkApi::GetHooks();
    hooks.SetHook( "AShooterGameState.Tick",
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
                       LPVOID /*lpvReserved*/ )
{
    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        debugLogFile.open( "debugLog.txt", std::ios::app );
        debugLog( "================================================================================" );
        debugLog( "Plugin loading" );
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
