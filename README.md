# HordeKiller

A horde shooter base for Unreal Engine 5.6, written entirely in C++. Waves of enemies chase you around an arena and you take them down with a weapon that fires physics-driven projectiles.

The project contains no art assets of its own. The arena, the weapon and the projectiles are built from the engine's basic shapes, and the characters use Epic's mannequins, which are copied from the engine installation instead of being stored in the repository. Its own assets are an arena level, two Blueprints for the player and the enemy, a horde config and two materials, all of which can be edited in the editor.

## Features

- **Physics projectile weapon.** Every shot is a simulated rigid body launched with an impulse. Projectiles drop with gravity, bounce off the floor and walls, and knock enemies back.
- **Two-hit enemies.** Enemies have 2 health and each projectile deals 1 damage. They turn from red to orange after the first hit.
- **Ragdoll deaths.** A killed enemy falls as a physics ragdoll, pushed by the shot, and its corpse is removed after 10 seconds.
- **Pooled enemies.** Enemies are created in advance and reused between waves instead of being spawned and destroyed.
- **Humanoid characters.** The player and the enemies use Epic's mannequins with walk and run animations.
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
   ```

2. Run `Setup.bat`, by double-clicking it or from a terminal. Make sure the Unreal editor is closed.

3. Open `HordeKiller.uproject` and press **Play**, or open `HordeKiller.sln` to work on the code.

`Setup.bat` does everything needed to work with the project, and can be run again at any time; each step skips what is already done:

| Step | What it does |
| --- | --- |
| 1 | Finds the Unreal Engine 5.6 installation |
| 2 | Downloads the Git LFS files (Blueprints, levels, data assets) |
| 3 | Generates the Visual Studio solution |
| 4 | Builds the editor target (`HordeKillerEditor`, Win64, Development) |
| 5 | Copies Epic's mannequins from the engine and generates the animation Blueprint, which are not stored in the repository |

It finds the engine through the Epic Games Launcher's list of installed engines. If your engine is somewhere it cannot find, pass the folder: `Setup.bat "D:\Engines\UE_5.6"`, or set the `UE_ROOT` environment variable.

### Manual setup

The same steps by hand, in case the script cannot be used. Adjust the engine path if Unreal Engine is installed somewhere else.

1. Download the Git LFS files:

   ```
   git lfs install --local
   git lfs pull
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

4. Optional, but recommended: copy the humanoid character models from your engine installation into the project. Close the editor first, then run:

   ```
   "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%CD%\HordeKiller.uproject" -run=pythonscript -script="%CD%\Tools\install_mannequins.py"
   ```

   Then generate the animation Blueprint that drives them:

   ```
   "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%CD%\HordeKiller.uproject" -run=pythonscript -script="%CD%\Tools\create_anim_blueprint.py"
   ```

   Without this step the game still runs, with placeholder shapes instead of the models. See [Character models](#character-models).

5. Open the editor and press **Play**.

## Controls

| Input | Action |
| --- | --- |
| W / A / S / D | Move |
| Mouse | Look and aim |
| Left mouse button | Fire (hold for automatic fire) |
| Space | Jump |

## Tuning

Every gameplay value is an editable property. For the player and the enemy, edit them in the Class Defaults of `BP_HKPlayer` and `BP_HKEnemy` (see [Blueprints](#blueprints)); a value set there overrides the C++ default. For the other classes, change the defaults in the headers. Distances are in centimetres, speeds in centimetres per second and times in seconds.

### Weapon and player (`AHKPlayer`)

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
| `CorpseLifetime` | 10 | Time the corpse stays after death before it is removed |
| `DeathImpulse` | 500 | Speed given to the ragdoll at death, away from what killed the enemy |

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

## Character models

The player and the enemies are humanoids: Epic's mannequins, **Quinn** for the player and **Manny** for the enemies, driven by the animation Blueprint `ABP_HKHuman`, which gives them idle, walk, run and jump animations.

- **Enemies** are tinted red, and turn orange after the first hit, using the `HealthyColor` and `WoundedColor` properties.
- **The player** is seen in first person, so the body is hidden from the player's own camera and only casts its shadow.

### Where the models come from

The mannequins are Epic Games content, covered by the Unreal Engine EULA and not by this project's license. They are **not stored in this repository**: `Content/Characters` is listed in `.gitignore`. They ship with every engine installation, and `Tools/install_mannequins.py` copies them from there into the project (about 125 MB). `Setup.bat` runs it for you, as described in [Getting started](#getting-started). It never overwrites files that are already there.

### Animation

Animation is split between a C++ class and a Blueprint:

- **`UHKAnimInstanceHuman`** (`Source/HordeKiller/Animation`) is the base animation instance for the player and the enemies. Every frame it reads the owning `AHKHuman` and fills these variables, visible in Blueprint under the **Human** category:

  | Variable | Meaning |
  | --- | --- |
  | `Velocity` | Velocity in world space, in cm/s |
  | `GroundSpeed` | Horizontal speed, in cm/s |
  | `Direction` | Angle between travel and facing direction, in degrees (-180 to 180) |
  | `bShouldMove` | Moving on purpose: above `MoveSpeedThreshold` and accelerating |
  | `bIsFalling` | In the air |
  | `bIsDead` | Health has reached zero |

- **`ABP_HKHuman`** (`Content/Characters/Animation`) is the animation Blueprint used by both. Its parent class is `UHKAnimInstanceHuman`.

Animation graphs cannot be built from a script, so `ABP_HKHuman` is not written from scratch: `Tools/create_anim_blueprint.py` copies the engine's `ABP_Unarmed` and re-parents the copy to `UHKAnimInstanceHuman`. Since it derives from Epic's asset, it is kept out of the repository like the mannequins and generated locally.

As generated, its graph still works the way Epic's does: its own event graph computes its own variables and the anim graph reads those. Epic's Blueprint uses some of the same names as the C++ class, so when the copy is re-parented the editor renames the Blueprint's clashing variables by adding `_0` (`GroundSpeed_0`, `Direction_0`) and reports it as a warning; that is expected. The C++ variables are computed alongside and are available in the graph, but nothing reads them yet.

To finish moving the logic to C++, open `ABP_HKHuman` in the editor, replace each use of the Blueprint's own variables in the anim graph and its transitions with the C++ variable from the **Human** category, then delete the event graph nodes and the Blueprint's own variables. This has to be done by hand, and because the Blueprint is not stored in the repository, it has to be done on each machine.

The C++ variables used to be named with a `Human` prefix (`HumanGroundSpeed` and so on). `Config/DefaultEngine.ini` has redirects from those names, so a Blueprint that was wired to them keeps working.

If the models are not installed, the game falls back to placeholders: enemies appear as coloured cylinders and the player has no body. Nothing else changes.

### Using other models

The model is set by two properties of `AHKHuman`, inherited by the player and the enemy and editable in the Class Defaults of `BP_HKPlayer` and `BP_HKEnemy`:

| Property | Default | Meaning |
| --- | --- | --- |
| `BodyModel` | `SKM_Manny_Simple` (enemy), `SKM_Quinn_Simple` (player) | Skeletal mesh shown as the body |
| `BodyAnimClass` | `ABP_HKHuman` | Animation Blueprint that drives it; must match the model's skeleton |

The tint is applied to a material parameter named `Paint Tint`, which is what the mannequin materials use. A different model needs a material with that parameter for the enemy colours to show.

## Blueprints

The player and the enemy each have a Blueprint that derives from their C++ class:

| Blueprint | Parent class | Location |
| --- | --- | --- |
| `BP_HKPlayer` | `AHKPlayer` | `Content/Blueprints/Characters` |
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
Setup.bat                     One-step setup after cloning
HordeKiller.uproject          Project descriptor (engine version, modules, plugins)
Config/                       Engine, game and input settings
Content/
  Blueprints/Characters/
    BP_HKPlayer            Blueprint of the player (parent AHKPlayer)
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
  install_mannequins.py       Script that copies Epic's mannequins from the engine (not in git)
  create_anim_blueprint.py    Script that generates ABP_HKHuman from the engine's ABP (not in git)
Source/
  HordeKiller.Target.cs       Build target for the standalone game
  HordeKillerEditor.Target.cs Build target for the editor
  HordeKiller/
    HordeKiller.Build.cs      Module build rules and dependencies
    HordeKiller.h / .cpp      Module registration
    Animation/
      HKAnimInstanceHuman.*   Base animation instance of player and enemy
    Characters/
      HKHuman.*               Base class of player and enemy: health, damage, death, body model
      HKPlayer.*           Player: movement, input, weapon
      HKEnemy.*               Enemy: chase, attack
    Weapons/
      HKProjectile.*          Physics projectile
    Game/
      HKGameMode.*            Player and HUD classes, game over, restart
    Hordes/
      HKHordeGenerator.*      Actor that spawns the waves and tracks their progress
      HKHordeConfig.*         Data asset class that describes a horde's waves
      HKEnemyPool.*           Pool that creates enemies in advance and reuses them
    Managers/
      HKActorManager.*        Registry of the actors in the match, with static access
    UI/
      HKHUD.*                 Crosshair and on-screen counters
```

Every class written for this project is named with the Unreal type prefix followed by `HK`, such as `AHKPlayer`, and its files drop the type prefix (`HKPlayer.h`). The module itself keeps the full name, `HordeKiller`.

Source files are grouped by gameplay area, with each header next to its `.cpp`. Includes are written relative to the module folder, for example `#include "Characters/HKPlayer.h"`.

### Class hierarchy

```
ACharacter (engine)
  AHKHuman           Abstract. Health, TakeDamage, IsDead, HandleDamaged / HandleDeath hooks
    AHKPlayer     The player
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

### Enemy pool

The generator does not spawn an enemy actor each time one is needed. It owns a pool, `UHKEnemyPool`, that creates the enemies in advance, hands them out when a wave starts and takes them back when their corpse is removed. A reused enemy is the same actor, reset: full health, standing, original colour.

How many are created:

- **At the start**, for each enemy class, the largest total of one wave plus the wave after it, over all the waves in the config. Two consecutive waves are counted because the corpses of one wave are still in the level, for `CorpseLifetime` seconds, when the next wave arrives.
- **In endless mode**, where waves keep growing, the pool is topped up to the same rule before each wave: the wave that is starting plus the following one.
- **If it still runs out**, it creates the missing enemies on the spot and writes a warning to the Output Log under `LogHKEnemyPool`. This happens when waves are cleared so quickly that corpses of more than two waves are in the level at once.

Enemies waiting in the pool are hidden, do not collide or tick, and are parked below the level. They are also unregistered from the [actor manager](#actor-manager), so `UHKActorManager::GetActors<AHKEnemy>()` returns only enemies that are in play, corpses included.

### Enemy death

When an enemy's health reaches zero it stops moving and attacking, its capsule stops colliding, and its body becomes a ragdoll: the bones are simulated by the physics engine using the mannequin's physics asset (`PA_Mannequin`), with an initial push away from whatever killed it. Projectiles no longer damage it, but still collide with it. After `CorpseLifetime` seconds the enemy returns to the pool.

Without the character models installed there is no ragdoll: the placeholder enemy returns to the pool as soon as it dies.

`AHKEnemy` also has a `UPhysicsControlComponent`, from the engine's PhysicsControl plugin, which is experimental in Unreal Engine 5.6. It is there for partial ragdolls and physical hit reactions, and has no controls set up yet; the death ragdoll does not use it.

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
AHKPlayer* Player      = UHKActorManager::GetFirstActor<AHKPlayer>(); // the player, or nullptr
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

- **It only holds actors that subscribed.** Those are the project's own classes: the player and the enemies (through their shared base class `AHKHuman`), the projectiles, the horde generator, the game mode and the HUD. Enemies waiting in the pool are not listed. Engine actors with no code of ours, such as lights, the player controller or the arena's floor and walls, are not in the list.
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
- Characters use the stock mannequins with locomotion animations only: there are no attack or hit animations. The arena and the weapon are placeholder shapes, and there are no effects or sounds.
- The player has no first-person arms or weapon model; the weapon is a floating bar.
- There is no menu, pause screen or score saving.
- Single-player only.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Changes are listed in [CHANGELOG.md](CHANGELOG.md).

## License

The source code and configuration in this repository are released under the [MIT License](LICENSE).

That license covers this project's own files only. Unreal Engine is not part of this repository and is licensed separately by Epic Games under the [Unreal Engine End User License Agreement](https://www.unrealengine.com/eula). You need to accept that agreement to build and run the project.
