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
- Write all code, comments and documentation in English.
- Document every class, function and property in the headers with `/** ... */` comments. Property comments become tooltips in the editor, so state the unit (cm, cm/s, seconds).
- In `.cpp` files, comment the reason behind anything that is not obvious from the code itself.
- Expose gameplay values as `UPROPERTY(EditAnywhere)` instead of hard-coding them, and list them in the README's tuning tables.
- Start each source file with the copyright line used by the existing files.

## What not to commit

Generated files are ignored by `.gitignore` and should stay out of the repository: `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/` and `*.sln`.

## License

By contributing you agree that your contributions are released under the project's [MIT License](LICENSE).
