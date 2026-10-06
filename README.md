# HordeKiller

A horde shooter base for Unreal Engine 5.6, written entirely in C++. Waves of enemies chase you around an arena and you take them down with a weapon that fires physics-driven projectiles.

The project contains no art or Blueprint assets. The arena, the enemies, the weapon, the HUD and the key bindings are all created from code using the engine's basic shapes, so it runs as soon as it compiles.

## Features

- **Physics projectile weapon.** Every shot is a simulated rigid body launched with an impulse. Projectiles drop with gravity, bounce off the floor and walls, and knock enemies back.
- **Two-hit enemies.** Enemies have 2 health and each projectile deals 1 damage. They turn from red to orange after the first hit.
- **Chasing horde.** Enemies run straight at the player, spread out around each other and deal damage on contact.
- **Endless waves.** Each wave spawns in a ring around the player and is larger than the previous one.
- **Health and game over.** The player has 100 health; on death the level restarts after a short delay.
- **Canvas HUD.** Crosshair, health, wave number, enemies alive and kill count.
- **Generated arena.** An 80 x 80 m floor with four walls, built at the start of play.

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

Every gameplay value is an editable property. Change the defaults in the headers, or create a child Blueprint of the class and edit them in the Details panel. Distances are in centimetres, speeds in centimetres per second and times in seconds.

### Weapon and player (`AHordeKillerCharacter`)

| Property | Default | Meaning |
| --- | --- | --- |
| `FireInterval` | 0.2 | Time between shots while fire is held |
| `MuzzleDistance` | 100 | Distance in front of the camera where projectiles spawn |
| `MaxHealth` | 100 | Starting health (property inherited from `AHuman`) |
| `ProjectileClass` | `AHordeKillerProjectile` | Projectile spawned on each shot |

### Projectile (`AHordeKillerProjectile`)

| Property | Default | Meaning |
| --- | --- | --- |
| `LaunchSpeed` | 5000 | Speed the projectile is fired at |
| `Damage` | 1 | Damage dealt to an enemy |
| `MinDamageSpeed` | 600 | Speed below which a projectile no longer deals damage |
| `KnockbackStrength` | 350 | Speed given to the enemy that is hit |

### Enemy (`AHordeKillerEnemy`)

| Property | Default | Meaning |
| --- | --- | --- |
| `MaxHealth` | 2 | Starting health (inherited from `AHuman`); with `Damage` at 1 this is two shots |
| `MoveSpeed` | 380 | Running speed (the player runs at 600) |
| `AttackDamage` | 10 | Damage dealt to the player per attack |
| `AttackRange` | 110 | Distance at which an attack lands |
| `AttackCooldown` | 1 | Time between attacks |
| `HealthyColor` / `WoundedColor` | red / orange | Body colour before and after the first hit |

### Waves and arena (`AHordeKillerGameMode`)

| Property | Default | Meaning |
| --- | --- | --- |
| `FirstWaveEnemies` | 6 | Enemies in the first wave |
| `EnemiesAddedPerWave` | 4 | Extra enemies in each following wave |
| `TimeBetweenWaves` | 3 | Pause before the first wave and between waves |
| `SpawnRadiusMin` / `SpawnRadiusMax` | 1500 / 2500 | Ring around the player where enemies spawn |
| `RestartDelay` | 3 | Time between the player's death and the restart |
| `bBuildArena` | true | Whether to generate the arena |
| `ArenaHalfSize` | 4000 | Half the side of the square arena |
| `ArenaWallHeight` | 400 | Height of the walls |
| `ArenaFloorZ` | 5 | Height of the floor surface |

## Project structure

```
HordeKiller.uproject          Project descriptor (engine version, modules, plugins)
Config/                       Engine, game and input settings
Content/                      Empty; the project has no assets yet
Source/
  HordeKiller.Target.cs       Build target for the standalone game
  HordeKillerEditor.Target.cs Build target for the editor
  HordeKiller/
    HordeKiller.Build.cs      Module build rules and dependencies
    HordeKiller.h / .cpp      Module registration
    Characters/
      Human.*                 Base class of player and enemy: health, damage, death
      HordeKillerCharacter.*  Player: movement, input, weapon
      HordeKillerEnemy.*      Enemy: chase, attack
    Weapons/
      HordeKillerProjectile.* Physics projectile
    Game/
      HordeKillerGameMode.*   Arena generation, waves, counters, restart
    UI/
      HordeKillerHUD.*        Crosshair and on-screen counters
```

Source files are grouped by gameplay area, with each header next to its `.cpp`. Includes are written relative to the module folder, for example `#include "Characters/HordeKillerCharacter.h"`.

### Class hierarchy

```
ACharacter (engine)
  AHuman                      Abstract. Health, TakeDamage, IsDead, HandleDamaged / HandleDeath hooks
    AHordeKillerCharacter     The player
    AHordeKillerEnemy         A horde enemy
```

`AHuman` owns the damage flow. Subclasses do not override `TakeDamage`; they override `HandleDamaged` (a hit they survive) and `HandleDeath` (health reached zero) to add their own reaction.

## How it works

- **Projectiles** are static mesh spheres with physics simulation, hit events and continuous collision detection enabled. Firing applies a single velocity-change impulse along the camera's aim direction; after that the physics engine moves them. On hitting an enemy at speed they apply damage and knockback and are destroyed.
- **Enemies** are characters possessed by an AI controller. Each frame they add movement input towards the player and attack when within range.
- **The game mode** counts living enemies and starts the next wave when the count reaches zero.
- **Input** uses Enhanced Input. The actions and the mapping context are created at runtime unless assets are assigned in a child Blueprint of the character.

The source files are commented in detail and are the best reference for the specifics.

## Known limitations

- Enemies do not use navigation. They walk in a straight line towards the player and will get stuck on obstacles if any are added.
- All visuals are placeholder shapes; there are no models, animations, effects or sounds.
- The project has no map of its own. It uses the engine's default template map and builds the arena on top of it.
- There is no menu, pause screen or score saving.
- Single-player only.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). Changes are listed in [CHANGELOG.md](CHANGELOG.md).

## License

The source code and configuration in this repository are released under the [MIT License](LICENSE).

That license covers this project's own files only. Unreal Engine is not part of this repository and is licensed separately by Epic Games under the [Unreal Engine End User License Agreement](https://www.unrealengine.com/eula). You need to accept that agreement to build and run the project.
