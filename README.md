# HordeKiller

A horde shooter base for Unreal Engine 5.6, written entirely in C++. Waves of enemies chase you around an arena and you take them down with a weapon that fires physics-driven projectiles.

The project contains no art assets of its own: everything is built from the engine's basic shapes. Its assets are an arena level, two Blueprints for the player and the enemy, a horde config and two materials, all of which can be edited in the editor.

## Features

- **Physics projectile weapon.** Every shot is a simulated rigid body launched with an impulse. Projectiles drop with gravity, bounce off the floor and walls, and knock enemies back.
- **Two-hit enemies.** Enemies have 2 health and each projectile deals 1 damage. They turn from red to orange after the first hit.
- **Chasing horde.** Enemies run straight at the player, spread out around each other and deal damage on contact.
- **Data-driven waves.** A horde generator spawns waves in a ring around the player. The waves are described in a data asset, and each level can have its own.
- **Health and game over.** The player has 100 health; on death the level restarts after a short delay.
- **Canvas HUD.** Crosshair, health, wave number, enemies alive and kill count.
- **Arena level.** `Test_HKArenaMap`, an 80 x 80 m floor with four walls, lighting, a player start and a horde generator.

## Requirements

- Windows 10 or 11, 64-bit.
- Unreal Engine 5.6, installed through the Epic Games Launcher.
- Visual Studio 2022 with these workloads:
  - Game development with C++
  - Desktop development with C++
  - .NET desktop development
- Git and Git LFS.

## Getting started

1. Clone the repository:

   ```
   git clone https://github.com/alvarocabrero/HordeKiller.git
   cd HordeKiller
   git lfs install --local
   ```

2. Generate the Visual Studio solution. Either right-click `HordeKiller.uproject` and choose **Generate Visual Studio project files**, or run:

   ```
   "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%CD%\HordeKiller.uproject" -game -rocket -progress
   ```

3. Open `HordeKiller.sln`, select the **Development Editor** configuration and the **Win64** platform, set `HordeKiller` as the start-up project and press **F5**. This builds the game module and opens the editor.

   To build from the command line instead:

   ```
   "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" HordeKillerEditor Win64 Development -Project="%CD%\HordeKiller.uproject" -WaitMutex
   ```

4. In the editor, press **Play**. No further setup is needed.

Adjust the engine path in the commands above if Unreal Engine is installed somewhere else.

## Controls

| Input | Action |
| --- | --- |
| W / A / S / D | Move |
| Mouse | Look and aim |
| Left mouse button | Fire (hold for automatic fire) |
| Space | Jump |

## Tuning

Every gameplay value is an editable property. For the player and the enemy, edit them in the Class Defaults of `BP_HKCharacter` and `BP_HKEnemy` (see [Blueprints](#blueprints)); a value set there overrides the C++ default. For the other classes, change the defaults in the headers. Distances are in centimetres, speeds in centimetres per second and times in seconds.

### Weapon and player (`AHKCharacter`)

| Property | Default | Meaning |
| --- | --- | --- |
| `FireInterval` | 0.2 | Time between shots while fire is held |
| `MuzzleDistance` | 100 | Distance in front of the camera where projectiles spawn |
| `MaxHealth` | 100 | Starting health (property inherited from `AHKHuman`) |
| `ProjectileClass` | `AHKProjectile` | Projectile spawned on each shot |

### Projectile (`AHKProjectile`)

| Property | Default | Meaning |
| --- | --- | --- |
| `LaunchSpeed` | 5000 | Speed the projectile is fired at |
| `Damage` | 1 | Damage dealt to an enemy |
| `MinDamageSpeed` | 600 | Speed below which a projectile no longer deals damage |
| `KnockbackStrength` | 350 | Speed given to the enemy that is hit |

### Enemy (`AHKEnemy`)

| Property | Default | Meaning |
| --- | --- | --- |
| `MaxHealth` | 2 | Starting health (inherited from `AHKHuman`); with `Damage` at 1 this is two shots |
| `MoveSpeed` | 380 | Running speed (the player runs at 600) |
| `AttackDamage` | 10 | Damage dealt to the player per attack |
| `AttackRange` | 110 | Distance at which an attack lands |
| `AttackCooldown` | 1 | Time between attacks |
| `HealthyColor` / `WoundedColor` | red / orange | Body colour before and after the first hit |

### Waves (`UHKHordeConfig` data asset)

Waves are configured in a data asset, not in code. See [Horde generator](#horde-generator). The values below are those of the default asset, `DA_HKHorde_Default`.

| Property | Default | Meaning |
| --- | --- | --- |
| `Waves` | one wave of 6 `BP_HKEnemy` | List of waves; each wave is a list of enemy class and count |
| `TimeBetweenWaves` | 3 | Pause before the first wave and between waves |
| `bEndless` | true | Whether the last wave keeps repeating after the list ends |
| `EndlessEnemiesAddedPerWave` | 4 | Enemies added on each repetition in endless mode |
| `SpawnRadiusMin` / `SpawnRadiusMax` | 1500 / 2500 | Ring around the player where enemies spawn |

### Game (`AHKGameMode`)

| Property | Default | Meaning |
| --- | --- | --- |
| `DefaultHordeConfig` | `DA_HKHorde_Default` | Horde used in levels without their own horde generator |
| `RestartDelay` | 3 | Time between the player's death and the restart |

The arena's size and layout are not properties: they are the actors placed in the level. See [Level](#level).

## Level

The game is played in `Content/Maps/Test/Test_HKArenaMap`, a regular level asset. It is the map the editor opens and the one the game loads, as set in `Config/DefaultEngine.ini`. It contains:

| Actor | Purpose |
| --- | --- |
| `Arena_Floor` | 80 x 80 m floor, with its top surface at height 0 |
| `Arena_Wall_XPos`, `XNeg`, `YPos`, `YNeg` | Four walls, 4 m high |
| `HordeGenerator` | The level's horde generator, with `DA_HKHorde_Default` and a spawn area kept 2 m inside the walls |
| `PlayerStart` | Where the player appears |
| `DirectionalLight`, `SkyLight`, `SkyAtmosphere`, `ExponentialHeightFog`, `VolumetricCloud`, `SM_SkySphere` | Sky and lighting, taken from the engine's default level template |

Nothing in the level is created by code. To change the arena, open the level in the editor and move, scale, add or delete actors. The floor and walls are engine cubes using the materials `MI_HKArenaFloor` and `MI_HKArenaWall` from `Content/Materials`.

To add another level, create it in the editor, give it a floor, a `PlayerStart` and an **HK Horde Generator** with that level's horde config, and open it or set it as the default map.

`Tools/create_arena_level.py` generated `Test_HKArenaMap` and its materials. It does nothing if the level already exists, so it never overwrites edits; it is only useful to recreate the level after deleting it.

## Blueprints

The player and the enemy each have a Blueprint that derives from their C++ class:

| Blueprint | Parent class | Location |
| --- | --- | --- |
| `BP_HKCharacter` | `AHKCharacter` | `Content/Blueprints/Characters` |
| `BP_HKEnemy` | `AHKEnemy` | `Content/Blueprints/Characters` |

They contain no logic of their own. They exist so that values and components can be changed in the editor without recompiling: open one, select **Class Defaults** and edit the properties listed under [Tuning](#tuning), or select a component to swap its mesh or material.

The game mode uses these Blueprints automatically. If one is missing, it falls back to the C++ class, so the game still runs. The Output Log shows which classes are in use on the line starting with `LogHordeKiller: Player class`.

The Blueprints were generated with `Tools/create_blueprints.py`. To recreate a Blueprint that has been deleted, build the project and run:

```
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%CD%\HordeKiller.uproject" -run=pythonscript -script="%CD%\Tools\create_blueprints.py"
```

The script skips Blueprints that already exist, so it never overwrites changes made in the editor.

Blueprint files (`.uasset`) are stored with Git LFS. Run `git lfs install --local` after cloning, otherwise they are downloaded as small pointer files and the game falls back to the C++ classes.

## Project structure

```
HordeKiller.uproject          Project descriptor (engine version, modules, plugins)
Config/                       Engine, game and input settings
Content/
  Blueprints/Characters/
    BP_HKCharacter            Blueprint of the player (parent AHKCharacter)
    BP_HKEnemy                Blueprint of the enemy (parent AHKEnemy)
  Data/Hordes/
    DA_HKHorde_Default        Default horde config (data asset of class UHKHordeConfig)
  Maps/Test/
    Test_HKArenaMap            The arena level, used for testing
  Materials/
    MI_HKArenaFloor           Material of the arena floor
    MI_HKArenaWall            Material of the arena walls
Tools/
  create_blueprints.py        Script that generates the Blueprints from the C++ classes
  create_horde_configs.py     Script that generates the horde config data assets
  create_arena_level.py       Script that generates the arena level and its materials
Source/
  HordeKiller.Target.cs       Build target for the standalone game
  HordeKillerEditor.Target.cs Build target for the editor
  HordeKiller/
    HordeKiller.Build.cs      Module build rules and dependencies
    HordeKiller.h / .cpp      Module registration
    Characters/
      HKHuman.*               Base class of player and enemy: health, damage, death
      HKCharacter.*           Player: movement, input, weapon
      HKEnemy.*               Enemy: chase, attack
    Weapons/
      HKProjectile.*          Physics projectile
    Game/
      HKGameMode.*            Player and HUD classes, game over, restart
    Hordes/
      HKHordeGenerator.*      Actor that spawns the waves and tracks their progress
      HKHordeConfig.*         Data asset class that describes a horde's waves
    Managers/
      HKActorManager.*        Registry of the actors in the match, with static access
    UI/
      HKHUD.*                 Crosshair and on-screen counters
```

Every class written for this project is named with the Unreal type prefix followed by `HK`, such as `AHKCharacter`, and its files drop the type prefix (`HKCharacter.h`). The module itself keeps the full name, `HordeKiller`.

Source files are grouped by gameplay area, with each header next to its `.cpp`. Includes are written relative to the module folder, for example `#include "Characters/HKCharacter.h"`.

### Class hierarchy

```
ACharacter (engine)
  AHKHuman           Abstract. Health, TakeDamage, IsDead, HandleDamaged / HandleDeath hooks
    AHKCharacter     The player
    AHKEnemy         A horde enemy
```

`AHKHuman` owns the damage flow. Subclasses do not override `TakeDamage`; they override `HandleDamaged` (a hit they survive) and `HandleDeath` (health reached zero) to add their own reaction.

## Horde generator

Everything about waves is split in two:

- **`AHKHordeGenerator`** is an actor that holds the functionality: the countdown between waves, picking spawn points, spawning enemies, counting how many are alive and killed, and deciding when the horde is over.
- **`UHKHordeConfig`** is a data asset that holds the configuration: which waves there are and how they spawn. The generator reads it from its `Config` property.

### Configuring a horde

A horde config contains a list of waves. Each wave is a list of groups, and each group is an enemy class and a count, so a wave can mix several kinds of enemy. For example:

```
Waves
  [0] EnemyGroups: BP_HKEnemy x 6
  [1] EnemyGroups: BP_HKEnemy x 10
  [2] EnemyGroups: BP_HKEnemy x 8, BP_HKFastEnemy x 4
```

Waves play in order. When the list ends, `bEndless` decides what happens: if it is on, the last wave repeats forever with `EndlessEnemiesAddedPerWave` more enemies each time; if it is off, the horde is complete and the HUD shows "ALL WAVES CLEARED".

The default asset is `Content/Data/Hordes/DA_HKHorde_Default`. As generated by the script it has a single wave of 6 enemies in endless mode adding 4 per wave, which gives 6, 10, 14, 18 and so on; open it in the editor to see or change its current values.

### A different horde for each level

1. In the Content Browser, choose **Add > Miscellaneous > Data Asset**, pick **HK Horde Config** and name it, for example `DA_HKHorde_Level2`.
2. Open it and fill in its waves and settings.
3. Open the level, drag an **HK Horde Generator** actor into it and place it on the floor at the centre of the play area.
4. In the generator's Details panel, set **Config** to the new asset. Set **Spawn Area Half Size** to keep spawns inside the play area; 0 means no limit.

A level that has a generator uses that generator and its config; `Test_HKArenaMap` has one. A level without one gets a generator spawned by the game mode at the world origin, using the game mode's `DefaultHordeConfig` and no spawn area limit.

### Generator properties

| Property | Default | Meaning |
| --- | --- | --- |
| `Config` | none | Horde config asset this generator plays |
| `bAutoStart` | true | Start the horde when play begins; turn off to call `StartHorde()` from code |
| `SpawnAreaHalfSize` | 0 | Half the side of the square spawn area centred on the generator; 0 is unlimited |
| `SpawnHeightOffset` | 100 | Height above the generator at which enemies spawn |

From code, the generator in use is available as `AHKGameMode::GetHordeGenerator()`, and exposes `StartHorde()`, `StopHorde()`, `GetCurrentWave()`, `GetEnemiesAlive()`, `GetKills()` and `IsHordeComplete()`. Its events are written to the Output Log under `LogHKHorde`.

`Tools/create_horde_configs.py` generated the default asset and can recreate it; run it the same way as the Blueprint script. It skips assets that already exist.

## Actor manager

`UHKActorManager` keeps a reference to the actors that take part in the match and makes them available through static functions, from any class and without needing a pointer to the manager.

Actors subscribe themselves. Each actor class of the project registers in `BeginPlay` and unregisters in `EndPlay`:

```cpp
void AHKProjectile::BeginPlay()
{
	Super::BeginPlay();
	UHKActorManager::Register(this);
}

void AHKProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UHKActorManager::Unregister(this);
	Super::EndPlay(EndPlayReason);
}
```

Any class can then query the registry:

```cpp
#include "Managers/HKActorManager.h"

TArray<AHKEnemy*> Enemies = UHKActorManager::GetActors<AHKEnemy>();     // all enemies alive
AHKCharacter* Player      = UHKActorManager::GetFirstActor<AHKCharacter>(); // the player, or nullptr
TArray<AActor*> Everything = UHKActorManager::GetAllActors();
int32 Total                = UHKActorManager::GetActorCount();
```

| Function | Returns |
| --- | --- |
| `Register(Actor)` | Nothing. Adds the actor; called by the actor itself in `BeginPlay` |
| `Unregister(Actor)` | Nothing. Removes the actor; called by the actor itself in `EndPlay` |
| `GetActors<T>()` | Every registered actor of class `T`, including subclasses and Blueprints |
| `GetFirstActor<T>()` | The oldest registered actor of class `T`, or `nullptr` |
| `GetAllActors()` | Every registered actor |
| `GetActorCount()` | Number of registered actors |
| `Get()` | The manager itself, or `nullptr` if no game is running |

Things to know:

- **It only holds actors that subscribed.** Those are the project's own classes: the player and the enemies (through their shared base class `AHKHuman`), the projectiles, the game mode and the HUD. Engine actors with no code of ours, such as lights, the player controller or the arena's floor and walls, are not in the list.
- **A new actor class must subscribe too.** Add the two calls shown above to its `BeginPlay` and `EndPlay`, unless it inherits from a class that already does. An actor that does not register is invisible to the manager.
- **It lives as long as the level.** It is a world subsystem, created with the game world and destroyed with it, so the list starts empty after a level load or restart.
- **The static functions refer to the game that is running.** Outside a running game, such as in the editor before pressing Play, they return empty results.
- **Queries scan the whole list.** That is fine for occasional use; code that needs the result every frame should store it.

The functions are C++ only for now; they are not exposed to Blueprint graphs.

## How it works

- **Projectiles** are static mesh spheres with physics simulation, hit events and continuous collision detection enabled. Firing applies a single velocity-change impulse along the camera's aim direction; after that the physics engine moves them. On hitting an enemy at speed they apply damage and knockback and are destroyed.
- **Enemies** are characters possessed by an AI controller. Each frame they add movement input towards the player and attack when within range.
- **The horde generator** spawns each wave described in its config asset, counts living enemies and starts the next wave when the count reaches zero.
- **The level** provides the arena, the lighting, the player start and the horde generator.
- **The game mode** makes sure the level has a horde generator and restarts the level when the player dies.
- **Input** uses Enhanced Input. The actions and the mapping context are created at runtime unless assets are assigned in a child Blueprint of the character.

The source files are commented in detail and are the best reference for the specifics.

## Known limitations

- Enemies do not use navigation. They walk in a straight line towards the player and will get stuck on obstacles if any are added.
- All visuals are placeholder shapes; there are no models, animations, effects or sounds.
- There is no menu, pause screen or score saving.
- Single-player only.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Changes are listed in [CHANGELOG.md](CHANGELOG.md).

## License

The source code and configuration in this repository are released under the [MIT License](LICENSE).

That license covers this project's own files only. Unreal Engine is not part of this repository and is licensed separately by Epic Games under the [Unreal Engine End User License Agreement](https://www.unrealengine.com/eula). You need to accept that agreement to build and run the project.
