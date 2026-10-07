# Changelog

All notable changes to this project are listed here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## Unreleased

### Added

- MIT license, README, contributing guide and this changelog.
- Detailed comments on all source and configuration files.
- Blueprints `BP_HKPlayer` and `BP_HKEnemy`, derived from the C++ player and enemy classes, and the script that generates them (`Tools/create_blueprints.py`). The game mode uses them when present and falls back to the C++ classes otherwise.
- `UHKActorManager`, a world subsystem that keeps a reference to the actors in the match and exposes them through static functions. Actors subscribe themselves in `BeginPlay` and unsubscribe in `EndPlay`.
- `AHKHordeGenerator`, an actor that holds all wave functionality, and `UHKHordeConfig`, a data asset that describes a horde's waves. Each level can have its own config. The default asset, `DA_HKHorde_Default`, reproduces the previous waves (6, 10, 14...), and `Tools/create_horde_configs.py` generates it.
- `Test_HKArenaMap` (in `Content/Maps/Test`), a test level asset with the arena, lighting, player start and horde generator, set as the default map, plus its two materials and the script that generates them (`Tools/create_arena_level.py`).
- Humanoid models for the player (Quinn) and the enemies (Manny), using Epic's mannequins and their locomotion animations. The assets are copied from the engine by `Tools/install_mannequins.py` and are not stored in the repository; without them the characters fall back to their placeholder shapes.
- `UHKAnimInstanceHuman`, a C++ base animation instance for the player and the enemies, and `ABP_HKHuman`, the animation Blueprint that derives from it. The Blueprint started as a copy of the engine's `ABP_Unarmed` and was rewired by hand to read the C++ variables; `Tools/create_anim_blueprint.py` can recreate the unwired copy if it is deleted.
- `Setup.bat`, a one-step setup to run after cloning: it finds the engine, downloads the Git LFS files, generates the solution, builds the editor target and installs the character models.
- Ragdoll deaths: a killed enemy falls as a physics ragdoll and its corpse is removed after `CorpseLifetime` (10 seconds).
- `UPhysicsControlComponent` on `AHKEnemy`, with the engine's PhysicsControl plugin enabled, for future partial ragdolls and hit reactions.
- `UHKEnemyPool`, a pool owned by the horde generator that creates enemies in advance and reuses them. It is sized, per enemy class, to the largest total of one wave plus the next.

### Changed

- The player class is now `AHKPlayer` (it was `AHKCharacter`), in `HKPlayer.h` / `.cpp`, and its Blueprint is `BP_HKPlayer`. A class redirect in `Config/DefaultEngine.ini` keeps assets saved with the old name loading.

- Enemies are no longer spawned per wave or destroyed on death; they are taken from and returned to the pool. Projectiles ignore dead enemies for damage.

- The arena is no longer built by code. `AHKGameMode` lost its arena generation and arena properties; the floor and walls are actors in `Test_HKArenaMap`.
- Wave logic and wave settings moved out of `AHKGameMode` into the horde generator and its config asset. The game mode keeps the restart, and spawns a generator in levels that do not have one.
- Source files are grouped into `Characters`, `Weapons`, `Game` and `UI` folders.
- The player and the enemy now inherit from a new abstract base class, `AHKHuman`, which holds their shared health, damage and death logic. Gameplay values are unchanged.
- Project classes are renamed to use an `HK` prefix after the Unreal type prefix (`AHordeKillerCharacter` is now `AHKPlayer`, and so on), and their files are renamed to match. The default game mode setting in `Config/DefaultEngine.ini` points to `HKGameMode`.

## 0.1.0 - 2026-10-06

### Added

- C++ project for Unreal Engine 5.6 with a single game module.
- First-person player character with Enhanced Input bindings created in code (WASD, mouse, Space, left mouse button).
- Weapon that fires physics-simulated projectiles launched with an impulse.
- Enemies that chase the player, attack on contact and die after two projectile hits.
- Wave system that spawns progressively larger waves around the player.
- Player health, game over and automatic level restart.
- Canvas HUD with crosshair, health, wave, enemies alive and kills.
- Arena (floor and four walls) generated at the start of play from engine basic shapes.
