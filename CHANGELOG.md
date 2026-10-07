# Changelog

All notable changes to this project are listed here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## Unreleased

### Added

- MIT license, README, contributing guide and this changelog.
- Detailed comments on all source and configuration files.
- Blueprints `BP_HKCharacter` and `BP_HKEnemy`, derived from the C++ player and enemy classes, and the script that generates them (`Tools/create_blueprints.py`). The game mode uses them when present and falls back to the C++ classes otherwise.

### Changed

- Source files are grouped into `Characters`, `Weapons`, `Game` and `UI` folders.
- The player and the enemy now inherit from a new abstract base class, `AHKHuman`, which holds their shared health, damage and death logic. Gameplay values are unchanged.
- Project classes are renamed to use an `HK` prefix after the Unreal type prefix (`AHordeKillerCharacter` is now `AHKCharacter`, and so on), and their files are renamed to match. The default game mode setting in `Config/DefaultEngine.ini` points to `HKGameMode`.

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
