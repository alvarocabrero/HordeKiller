# Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.
"""Creates the project's Blueprint classes from their C++ parents.

Blueprints are binary assets, so they cannot be written by hand. This script builds them with the
editor's Python API and is meant to be run without opening the editor window:

    UnrealEditor-Cmd.exe <path>\\HordeKiller.uproject -run=pythonscript -script=<path>\\Tools\\create_blueprints.py

It is safe to run again: Blueprints that already exist are left untouched, so values edited in the
editor are never overwritten. The game module must be compiled first, because the parent classes
are looked up in it.
"""

import unreal

# (Blueprint name, content folder, C++ parent class path).
# A class path is "/Script/<ModuleName>.<ClassName without its A/U prefix>".
BLUEPRINTS = [
    ("BP_HKPlayer", "/Game/Blueprints/Characters", "/Script/HordeKiller.HKPlayer"),
    ("BP_HKEnemy", "/Game/Blueprints/Characters", "/Script/HordeKiller.HKEnemy"),
]


def create_blueprint(name, folder, parent_path):
    """Creates one Blueprint deriving from the given C++ class and saves it. Returns True on success."""
    asset_path = "{}/{}".format(folder, name)

    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log("HKBlueprints: {} already exists, skipped".format(asset_path))
        return True

    parent_class = unreal.load_class(None, parent_path)
    if parent_class is None:
        unreal.log_error("HKBlueprints: parent class {} not found; build the project first".format(parent_path))
        return False

    # The factory is what the editor itself uses for "New Blueprint Class"; the parent is its only input.
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, folder, unreal.Blueprint, factory)
    if blueprint is None:
        unreal.log_error("HKBlueprints: could not create {}".format(asset_path))
        return False

    # Assets created from script only exist in memory until they are saved.
    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint):
        unreal.log_error("HKBlueprints: could not save {}".format(asset_path))
        return False

    unreal.log("HKBlueprints: created {} (parent {})".format(asset_path, parent_path))
    return True


def main():
    results = [create_blueprint(*entry) for entry in BLUEPRINTS]
    unreal.log("HKBlueprints: finished, {} of {} ok".format(sum(results), len(results)))


main()
