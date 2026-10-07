# Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.
"""Creates the arena level and the two materials it uses.

Levels are binary assets, so they cannot be written by hand. This script builds the level with the
editor's Python API and is meant to be run without opening the editor window:

    UnrealEditor-Cmd.exe <path>\\HordeKiller.uproject -run=pythonscript -script=<path>\\Tools\\create_arena_level.py

It does nothing if the level already exists, so a level edited in the editor is never overwritten.
The game module must be compiled first and the default horde config must exist (see
create_horde_configs.py).
"""

import unreal

LEVEL_PATH = "/Game/Maps/Test/Test_L_HKArena"
# Engine level that provides sky, sun, fog and a player start.
TEMPLATE_PATH = "/Engine/Maps/Templates/Template_Default"
MATERIAL_FOLDER = "/Game/Materials"
HORDE_CONFIG_PATH = "/Game/Data/Hordes/DA_HKHorde_Default"

# All sizes in cm.
ARENA_HALF_SIZE = 4000.0
WALL_HEIGHT = 400.0
THICKNESS = 100.0
FLOOR_Z = 0.0  # Height of the walkable surface.

CUBE = "/Engine/BasicShapes/Cube"
BASE_MATERIAL = "/Engine/BasicShapes/BasicShapeMaterial"


def create_material(name, color):
    """Creates (or loads) an instance of the engine's basic material with the given colour."""
    path = "{}/{}".format(MATERIAL_FOLDER, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.EditorAssetLibrary.load_asset(path)

    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, MATERIAL_FOLDER, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    unreal.MaterialEditingLibrary.set_material_instance_parent(material, unreal.load_asset(BASE_MATERIAL))
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(material, "Color", color)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def spawn_block(actors, label, center, size, material):
    """Spawns a box made from the engine cube. 'center' and 'size' are (x, y, z) in cm."""
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*center))
    actor.set_actor_label(label)
    # The cube is 100 cm per side.
    actor.set_actor_scale3d(unreal.Vector(size[0] / 100.0, size[1] / 100.0, size[2] / 100.0))
    component = actor.static_mesh_component
    component.set_static_mesh(unreal.load_asset(CUBE))
    component.set_material(0, material)
    return actor


def main():
    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        unreal.log("HKLevel: {} already exists, skipped".format(LEVEL_PATH))
        return

    floor_material = create_material("MI_HKArenaFloor", unreal.LinearColor(0.25, 0.27, 0.3, 1.0))
    wall_material = create_material("MI_HKArenaWall", unreal.LinearColor(0.12, 0.13, 0.16, 1.0))

    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not levels.new_level_from_template(LEVEL_PATH, TEMPLATE_PATH):
        unreal.log_error("HKLevel: could not create {} from {}".format(LEVEL_PATH, TEMPLATE_PATH))
        return

    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    # The template brings its own small floor; the arena replaces it. Its sky sphere is kept.
    has_player_start = False
    for actor in actors.get_all_level_actors():
        if isinstance(actor, unreal.StaticMeshActor) and actor.get_actor_label() == "Floor":
            unreal.log("HKLevel: removing template actor {}".format(actor.get_actor_label()))
            actors.destroy_actor(actor)
        elif isinstance(actor, unreal.PlayerStart):
            has_player_start = True

    side = ARENA_HALF_SIZE * 2.0
    spawn_block(actors, "Arena_Floor", (0.0, 0.0, FLOOR_Z - THICKNESS / 2.0), (side, side, THICKNESS), floor_material)

    # Walls stand on the floor, just outside its edge.
    wall_z = FLOOR_Z + WALL_HEIGHT / 2.0
    offset = ARENA_HALF_SIZE + THICKNESS / 2.0
    spawn_block(actors, "Arena_Wall_XPos", (offset, 0.0, wall_z), (THICKNESS, side, WALL_HEIGHT), wall_material)
    spawn_block(actors, "Arena_Wall_XNeg", (-offset, 0.0, wall_z), (THICKNESS, side, WALL_HEIGHT), wall_material)
    spawn_block(actors, "Arena_Wall_YPos", (0.0, offset, wall_z), (side, THICKNESS, WALL_HEIGHT), wall_material)
    spawn_block(actors, "Arena_Wall_YNeg", (0.0, -offset, wall_z), (side, THICKNESS, WALL_HEIGHT), wall_material)

    if not has_player_start:
        start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0.0, 0.0, FLOOR_Z + 100.0))
        start.set_actor_label("PlayerStart")

    # The horde generator of this level, on the floor at the centre, with its own config.
    generator_class = unreal.load_class(None, "/Script/HordeKiller.HKHordeGenerator")
    generator = actors.spawn_actor_from_class(generator_class, unreal.Vector(0.0, 0.0, FLOOR_Z))
    generator.set_actor_label("HordeGenerator")
    generator.set_editor_property("config", unreal.load_asset(HORDE_CONFIG_PATH))
    # Keep spawns 2 m inside the walls.
    generator.set_editor_property("spawn_area_half_size", ARENA_HALF_SIZE - 200.0)

    if not levels.save_current_level():
        unreal.log_error("HKLevel: could not save {}".format(LEVEL_PATH))
        return

    summary = ", ".join(sorted("{} ({})".format(a.get_actor_label(), a.get_class().get_name())
                               for a in actors.get_all_level_actors()))
    unreal.log("HKLevel: created {} with actors: {}".format(LEVEL_PATH, summary))


main()
