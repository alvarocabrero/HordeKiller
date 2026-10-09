# Contributing to HordeKiller

## Setting up

Follow the steps in the [README](README.md#getting-started) to clone, generate project files and build. Run `git lfs install --local` once after cloning: `.uasset` and `.umap` files are stored with Git LFS.

## Making changes

1. Create a branch from `main` with a short descriptive name, for example `feature/enemy-navigation` or `fix/projectile-spawn`.
2. Keep each commit to one logical change, with a subject line in the imperative mood ("Add enemy navigation", not "Added" or "Adds").
3. Before opening a pull request, make sure the editor target builds:

   ```
   "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" HordeKillerEditor Win64 Development -Project="%CD%\HordeKiller.uproject" -WaitMutex
   ```

   Then play the game in the editor and check that the change works and that waves still start, enemies still die in two hits and the player can still die and restart.
4. Add an entry to [CHANGELOG.md](CHANGELOG.md) under "Unreleased".

## Code style

- Follow Epic's [C++ coding standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine): tabs for indentation, PascalCase names, and the `A` / `U` / `F` / `E` / `b` prefixes.
- Name every type created for this project with `HK` right after the Unreal type prefix: `AHKPlayer` for an actor, `UHKHealthComponent` for an object or component, `FHKWaveConfig` for a struct, `EHKEnemyState` for an enum, `IHKDamageable` for an interface. Name the files after the type without that first letter (`HKPlayer.h`, `HKPlayer.cpp`).
- Make every new actor class subscribe to the actor manager: call `UHKActorManager::Register(this)` in `BeginPlay` and `UHKActorManager::Unregister(this)` in `EndPlay`, unless a parent class already does.
- In headers, keep functions and variables in separate access blocks, with all the functions first and all the variables at the end. Repeat the access specifier for each block, in this order: `public:` functions, `protected:` functions, `private:` functions, then `public:` variables, `protected:` variables, `private:` variables. Leave out the blocks a class does not need.
- Do not repeat the class or owner name in member variable names: in `UHKAnimInstanceHuman` it is `GroundSpeed`, not `HumanGroundSpeed`.
- Write all code, comments and documentation in English.
- Document every class, function and property in the headers with `/** ... */` comments. Property comments become tooltips in the editor, so state the unit (cm, cm/s, seconds).
- In `.cpp` files, keep comments short: one line saying why, only where the code does not make it obvious. The detailed explanations belong in the header.
- Expose gameplay values as `UPROPERTY(EditAnywhere)` instead of hard-coding them, and list them in the README's tuning tables.
- Start each source file with the copyright line used by the existing files.
- Put new classes in the folder for their gameplay area (`Animation`, `Characters`, `Weapons`, `Game`, `Hordes`, `Managers`, `UI`), keeping the header next to its `.cpp`, and include them by their path from the module folder.

## What not to commit

Generated files are ignored by `.gitignore` and should stay out of the repository: `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/` and `*.sln`.

Epic's content must stay out of the repository too. `Content/Characters/Mannequins` is ignored for that reason: those assets are licensed by Epic under the Unreal Engine EULA and cannot be redistributed here. Do not add them, or any other engine or Marketplace content, to a commit. Third-party assets keep their original names; the `HK` prefix is only for what is made for this project.

## License

By contributing you agree that your contributions are released under the project's [MIT License](LICENSE).
