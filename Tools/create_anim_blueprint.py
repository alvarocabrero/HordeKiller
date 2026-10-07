# Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.
"""Creates ABP_HKHuman, the animation Blueprint shared by the player and the enemies.

Animation graphs cannot be built from script, so the Blueprint is not created empty. It starts as a
copy of the engine's unarmed locomotion Blueprint (ABP_Unarmed, part of Epic's mannequin content) and
is then re-parented to the project's C++ class UHKAnimInstanceHuman. Because it derives from Epic's
asset, it is kept out of the repository like the mannequins themselves, and each developer generates
it locally. Run install_mannequins.py first, then:

    UnrealEditor-Cmd.exe <path>\\HordeKiller.uproject -run=pythonscript -script=<path>\\Tools\\create_anim_blueprint.py

It does nothing if ABP_HKHuman already exists, so changes made to it in the editor are never
overwritten. The game module must be compiled first.
"""

import unreal

SOURCE = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"
TARGET = "/Game/Characters/Animation/ABP_HKHuman"
PARENT_CLASS = "/Script/HordeKiller.HKAnimInstanceHuman"


def main():
    if unreal.EditorAssetLibrary.does_asset_exist(TARGET):
        unreal.log("HKAnim: {} already exists, skipped".format(TARGET))
        return

    if not unreal.EditorAssetLibrary.does_asset_exist(SOURCE):
        unreal.log_error("HKAnim: {} not found; run install_mannequins.py first".format(SOURCE))
        return

    parent = unreal.load_class(None, PARENT_CLASS)
    if parent is None:
        unreal.log_error("HKAnim: class {} not found; build the project first".format(PARENT_CLASS))
        return

    blueprint = unreal.EditorAssetLibrary.duplicate_asset(SOURCE, TARGET)
    if blueprint is None:
        unreal.log_error("HKAnim: could not copy {} to {}".format(SOURCE, TARGET))
        return

    # Changes the Blueprint's parent from UAnimInstance to the project's class, then recompiles it.
    unreal.BlueprintEditorLibrary.reparent_blueprint(blueprint, parent)
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)

    if not unreal.EditorAssetLibrary.save_loaded_asset(blueprint):
        unreal.log_error("HKAnim: could not save {}".format(TARGET))
        return

    unreal.log("HKAnim: created {} (class {})".format(TARGET, blueprint.generated_class().get_name()))


main()
