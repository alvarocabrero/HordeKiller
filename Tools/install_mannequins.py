# Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.
"""Copies Epic's mannequin characters from the engine installation into this project.

The humanoid models used by the player and the enemies (Manny and Quinn, with their animations) are
Epic Games content. They ship with every Unreal Engine installation, but they are not stored in this
repository: Content/Characters/Mannequins is ignored by git. Each developer runs this script once
after cloning to copy them from their own engine:

    UnrealEditor-Cmd.exe <path>\\HordeKiller.uproject -run=pythonscript -script=<path>\\Tools\\install_mannequins.py

Without these files the game still runs; the characters fall back to their placeholder shapes.
Files that already exist in the project are left untouched.
"""

import os
import shutil

import unreal

# Where the engine keeps the mannequins, relative to the engine folder.
SOURCE = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.engine_dir()),
                      "..", "Templates", "TemplateResources", "High", "Characters", "Content", "Mannequins")
# The assets refer to each other as /Game/Characters/Mannequins/..., so they must keep this location.
DESTINATION = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()),
                           "Characters", "Mannequins")


def main():
    source = os.path.normpath(SOURCE)
    if not os.path.isdir(source):
        unreal.log_error("HKMannequins: {} not found in this engine installation".format(source))
        return

    copied = 0
    skipped = 0
    for folder, _, files in os.walk(source):
        target_folder = os.path.join(DESTINATION, os.path.relpath(folder, source))
        os.makedirs(target_folder, exist_ok=True)
        for name in files:
            target = os.path.join(target_folder, name)
            if os.path.exists(target):
                skipped += 1
                continue
            shutil.copy2(os.path.join(folder, name), target)
            copied += 1

    unreal.log("HKMannequins: copied {} files to {} ({} already present)".format(copied, DESTINATION, skipped))


main()
