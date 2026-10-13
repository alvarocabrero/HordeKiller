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
- Editor MCP server: the `ModelContextProtocol` and `AllToolsets` engine plugins are enabled for the editor, so an AI assistant can drive the running editor.
- Enemy attack animation: `AHKEnemy` plays `AttackAnimation` on its body each time it attacks. `AS_HKEnemy_Attack`, a punch keyed for this project on the mannequin's Control Rig, is available for it; `LS_HKEnemy_Attack` is the Level Sequence it was exported from.
- Zombie animations for the enemies, from Quaternius's Universal Animation Library 2 (CC0), retargeted to the mannequin skeleton and stored in `Content/ThirdParty/Quaternius`: an idle, a walk and a scratch. Enemies use them through `ABP_HKZombie`, which blends between the zombie idle and a jog depending on whether the enemy is moving, and the scratch is their default `AttackAnimation`. The jog, and a sprint that is not used yet, come from the first Universal Animation Library (CC0).
- `Test_HKVillageMap` (in `Content/Maps/Test`), a second test level: a 40 m paved square enclosed by houses, built from Quaternius's Medieval Village MegaKit (CC0). The kit is stored in `Content/ThirdParty/Quaternius/MedievalVillage`; `Tools/import_medieval_village.py` imports it and builds its materials, and `Tools/create_village_level.py` generates the level.

### Changed

- Groundwork for a single animation Blueprint shared by the player and the enemies: `UHKAnimLayerInstance`, a C++ base class for animation layers that gives them the main instance's state, and `AnimLayerClass` on `AHKHuman`, the layer Blueprint each character links onto its animation Blueprint when play begins.
- New Blueprints `BP_HKProjectile`, `BP_HKGameMode`, `BP_HKHUD` and `BP_HKHordeGenerator`. The project's game mode is now `BP_HKGameMode`, which names the player and HUD Blueprints and the default horde config; `AHKGameMode` and `AHKProjectile` no longer set classes, assets or physics values in their constructors.
- The constructors of `AHKHuman`, `AHKPlayer` and `AHKEnemy` only create components now. Everything a Blueprint can set (capsule size, movement, camera boom, weapon mesh, AI possession, health, projectile class) is set in `BP_HKPlayer` and `BP_HKEnemy`, on the engine's own fields. `CameraDistance`, `CameraOffset` and `MoveSpeed` are gone: the camera boom's arm length and offset and the movement component's walk speed replace them.
- The offset that stands a character's model on its capsule is set on the Mesh component of `BP_HKPlayer` and `BP_HKEnemy` instead of in code, and the enemy's placeholder cylinder is gone: without the models installed, characters have no visible body.
- The body model, the animation Blueprint and the enemy's attack animation are no longer named in C++. The model and the animation Blueprint are assigned on the Mesh component of `BP_HKPlayer` and `BP_HKEnemy`, in its own Skeletal Mesh Asset and Anim Class fields; `AHKHuman` no longer has `BodyModel` and `BodyAnimClass` properties. The attack animation is set in the Class Defaults of `BP_HKEnemy`.
- The player is now seen in third person: a camera on a boom behind and over the right shoulder replaces the first-person camera, the body is visible, and shots leave the chest towards the point under the crosshair.
- Migrated from Unreal Engine 5.6 to 5.8. The build targets use `BuildSettingsVersion.V7`. The 5.6 version of the project is kept on the `ue-5.6` branch.
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
