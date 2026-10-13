# Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.
"""Creates the village test level: a paved square enclosed by houses built from the Medieval Village kit.

Run it without opening the editor window, after import_medieval_village.py and create_horde_configs.py:

    UnrealEditor-Cmd.exe <path>\\HordeKiller.uproject -run=pythonscript -script=<path>\\Tools\\create_village_level.py

It does nothing if the level already exists, so a level edited in the editor is never overwritten.

The enemies steer straight at the player without path finding, so the square is kept open: the houses
stand around it and only a few small props are inside. The houses cannot be entered; invisible walls
along their fronts keep everyone in the square.
"""

import math
import random

import unreal

LEVEL_PATH = "/Game/Maps/Test/Test_HKVillageMap"
# Engine level that provides sky, sun, fog and a player start.
TEMPLATE_PATH = "/Engine/Maps/Templates/Template_Default"
HORDE_CONFIG_PATH = "/Game/Data/Hordes/DA_HKHorde_Default"
KIT = "/Game/ThirdParty/Quaternius/MedievalVillage"
GROUND_MATERIAL_FOLDER = "/Game/Materials"
CUBE = "/Engine/BasicShapes/Cube"

# All sizes in cm. The kit is built on a 2 m grid with 3 m storeys.
MODULE = 200.0
STOREY = 300.0
SQUARE_HALF_SIZE = 2000.0
GROUND_HALF_SIZE = 3600.0
FLOOR_Z = 0.0  # Height of the walkable surface.
BLOCKER_HEIGHT = 1000.0
BLOCKER_THICKNESS = 100.0

# Houses along one side of the square, left to right: (width, length, storeys), width and length in metres.
# The widths add up to the side of the square, and each width x length pair has a roof in the kit.
SIDE_HOUSES = [(6, 8, 2), (8, 8, 1), (4, 6, 2), (8, 10, 2), (6, 8, 1), (8, 8, 2)]

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
meshes = {}


def mesh(name):
    if name not in meshes:
        meshes[name] = unreal.load_asset("{}/Meshes/{}".format(KIT, name))
        if not meshes[name]:
            raise RuntimeError("missing kit mesh {}; run import_medieval_village.py first".format(name))
    return meshes[name]


def place(name, folder, x, y, z, yaw=0.0, collide=True):
    """Spawns one kit piece at a world position."""
    # Rotator takes roll, pitch, yaw.
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0.0, 0.0, yaw))
    actor.set_folder_path(folder)
    component = actor.static_mesh_component
    component.set_static_mesh(mesh(name))
    if not collide:
        component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    return actor


class House:
    """Places kit pieces in the house's own space: X across the front, -Y towards the street."""

    def __init__(self, label, center_x, center_y, yaw):
        self.folder = "Village/{}".format(label)
        self.center = (center_x, center_y)
        self.yaw = yaw
        self.cos = math.cos(math.radians(yaw))
        self.sin = math.sin(math.radians(yaw))

    def add(self, name, x, y, z, yaw=0.0, collide=True):
        world_x = self.center[0] + x * self.cos - y * self.sin
        world_y = self.center[1] + x * self.sin + y * self.cos
        return place(name, self.folder, world_x, world_y, FLOOR_Z + z, self.yaw + yaw, collide)


def build_house(label, center_x, center_y, yaw, width_m, length_m, storeys, rng, roof=None):
    """Builds a closed house: walls with a door and windows on the front, corner posts, gables and roof."""
    house = House(label, center_x, center_y, yaw)
    half_w = width_m * 50.0
    half_l = length_m * 50.0
    across = int(width_m // 2)
    along = int(length_m // 2)
    door_slot = rng.randrange(across)

    for storey in range(storeys):
        z = storey * STOREY
        # Stone below, plaster above.
        style = "UnevenBrick" if storey == 0 else "Plaster"

        for i in range(across):
            x = -half_w + MODULE / 2.0 + MODULE * i
            # Front wall: the outside of a wall piece is its +Y side, so the front ones are turned around.
            if storey == 0 and i == door_slot:
                house.add("Wall_{}_Door_Flat".format(style), x, -half_l, z, 180.0)
                # The door leaf hinges at its origin and is about 1.1 m wide.
                house.add("Door_2_Flat", x + 51.0, -half_l, z, 180.0)
            elif rng.random() < 0.6:
                house.add("Wall_{}_Window_Wide_Flat".format(style), x, -half_l, z, 180.0)
                house.add("Window_Wide_Flat1", x, -half_l, z, 180.0, collide=False)
                if rng.random() < 0.5:
                    house.add("WindowShutters_Wide_Flat_Open", x, -half_l, z, 180.0, collide=False)
            else:
                house.add("Wall_{}_Straight".format(style), x, -half_l, z, 180.0)
            house.add("Wall_{}_Straight".format(style), x, half_l, z, 0.0)

        for i in range(along):
            y = -half_l + MODULE / 2.0 + MODULE * i
            house.add("Wall_{}_Straight".format(style), -half_w, y, z, 90.0)
            house.add("Wall_{}_Straight".format(style), half_w, y, z, -90.0)

        for x in (-half_w, half_w):
            for y in (-half_l, half_l):
                house.add("Corner_Exterior_Wood", x, y, z, collide=False)

    top = storeys * STOREY
    if roof:
        house.add(roof, 0.0, 0.0, top)
    else:
        # The roof's ridge runs front to back, with a gable wall under each end.
        house.add("Roof_RoundTiles_{}x{}".format(width_m, length_m), 0.0, 0.0, top)
        house.add("Roof_Front_Brick{}".format(width_m), 0.0, -half_l, top, 180.0)
        house.add("Roof_Front_Brick{}".format(width_m), 0.0, half_l, top, 0.0)


def create_ground_material():
    """The kit's cobblestone, repeated every 2 m across the whole ground."""
    name = "MI_HKVillageGround"
    path = "{}/{}".format(GROUND_MATERIAL_FOLDER, name)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)

    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, GROUND_MATERIAL_FOLDER, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    library = unreal.MaterialEditingLibrary
    library.set_material_instance_parent(material, unreal.load_asset(KIT + "/Materials/M_HKVillageSurface"))
    for parameter, texture in (("BaseColor", "T_UnevenBrick_BaseColor"), ("Normal", "T_UnevenBrick_Normal"),
                               ("Roughness", "T_UnevenBrick_Roughness")):
        library.set_material_instance_texture_parameter_value(
            material, parameter, unreal.load_asset("{}/Textures/{}".format(KIT, texture)))
    library.set_material_instance_scalar_parameter_value(material, "Tiling", GROUND_HALF_SIZE * 2.0 / MODULE)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def spawn_block(label, center, size, material=None, hidden=False):
    """Spawns a box made from the engine cube. 'center' and 'size' are (x, y, z) in cm."""
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*center))
    actor.set_actor_label(label)
    actor.set_folder_path("Village/Ground")
    # The cube is 100 cm per side.
    actor.set_actor_scale3d(unreal.Vector(size[0] / 100.0, size[1] / 100.0, size[2] / 100.0))
    component = actor.static_mesh_component
    component.set_static_mesh(unreal.load_asset(CUBE))
    if material:
        component.set_material(0, material)
    if hidden:
        # Still collides; it only stops being drawn.
        component.set_visibility(False)
    return actor


def build_side(side, yaw, rng):
    """Lines one side of the square with houses, their fronts on the edge of the square."""
    cursor = -SQUARE_HALF_SIZE
    for index, (width_m, length_m, storeys) in enumerate(SIDE_HOUSES):
        along = cursor + width_m * 50.0
        cursor += width_m * 100.0
        depth = SQUARE_HALF_SIZE + length_m * 50.0
        # 'along' runs left to right as seen from the square, 'depth' goes away from it.
        cos = math.cos(math.radians(yaw))
        sin = math.sin(math.radians(yaw))
        build_house("{}_{}".format(side, index + 1), along * cos - depth * sin, along * sin + depth * cos,
                    yaw, width_m, length_m, storeys, rng)


def main():
    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        unreal.log("HKVillage: {} already exists, skipped".format(LEVEL_PATH))
        return

    ground_material = create_ground_material()

    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not levels.new_level_from_template(LEVEL_PATH, TEMPLATE_PATH):
        unreal.log_error("HKVillage: could not create {} from {}".format(LEVEL_PATH, TEMPLATE_PATH))
        return

    # The template brings its own small floor; the village ground replaces it. Its sky sphere is kept.
    has_player_start = False
    for actor in actors.get_all_level_actors():
        if isinstance(actor, unreal.StaticMeshActor) and actor.get_actor_label() == "Floor":
            actors.destroy_actor(actor)
        elif isinstance(actor, unreal.PlayerStart):
            has_player_start = True

    side = GROUND_HALF_SIZE * 2.0
    spawn_block("Village_Ground", (0.0, 0.0, FLOOR_Z - 50.0), (side, side, 100.0), ground_material)

    # Invisible walls just behind the house fronts, overlapping at the corners.
    blocker_z = FLOOR_Z + BLOCKER_HEIGHT / 2.0
    offset = SQUARE_HALF_SIZE + BLOCKER_THICKNESS / 2.0 + 40.0
    span = SQUARE_HALF_SIZE * 2.0 + 400.0
    spawn_block("Village_Blocker_XPos", (offset, 0.0, blocker_z), (BLOCKER_THICKNESS, span, BLOCKER_HEIGHT), hidden=True)
    spawn_block("Village_Blocker_XNeg", (-offset, 0.0, blocker_z), (BLOCKER_THICKNESS, span, BLOCKER_HEIGHT), hidden=True)
    spawn_block("Village_Blocker_YPos", (0.0, offset, blocker_z), (span, BLOCKER_THICKNESS, BLOCKER_HEIGHT), hidden=True)
    spawn_block("Village_Blocker_YNeg", (0.0, -offset, blocker_z), (span, BLOCKER_THICKNESS, BLOCKER_HEIGHT), hidden=True)

    # Fixed seed: the same village every time the script runs.
    rng = random.Random(1337)

    # A house faces the street with its -Y side, so each side of the square turns its houses to look in.
    build_side("North", 0.0, rng)
    build_side("East", -90.0, rng)
    build_side("South", 180.0, rng)
    build_side("West", 90.0, rng)

    # A three-storey tower closes each corner.
    corner = SQUARE_HALF_SIZE + 200.0
    for label, x, y in (("NE", corner, corner), ("NW", -corner, corner), ("SE", corner, -corner), ("SW", -corner, -corner)):
        build_house("Tower_{}".format(label), x, y, 0.0, 4, 4, 3, rng, roof="Roof_Tower_RoundTiles")

    # A few props near the edges. They are small, so enemies running at the player slide around them.
    props = [
        ("Prop_Wagon", 1500.0, 1450.0, 35.0),
        ("Prop_Wagon", -1550.0, -1300.0, -110.0),
        ("Prop_Crate", 1650.0, -1700.0, 10.0),
        ("Prop_Crate", 1770.0, -1690.0, 35.0),
        ("Prop_Crate", 1700.0, -1580.0, -20.0),
        ("Prop_Crate", -1750.0, 1650.0, 5.0),
        ("Prop_Crate", -1640.0, 1720.0, 50.0),
        ("Prop_Crate", -600.0, 1800.0, 15.0),
        ("Prop_Crate", 900.0, -1820.0, -30.0),
    ]
    for name, x, y, yaw in props:
        place(name, "Village/Props", x, y, FLOOR_Z, yaw)
    # A second crate on top of one of the stacks.
    place("Prop_Crate", "Village/Props", 1700.0, -1660.0, FLOOR_Z + 106.0, 25.0)

    if not has_player_start:
        start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0.0, 0.0, FLOOR_Z + 100.0))
        start.set_actor_label("PlayerStart")

    # The horde generator of this level, on the ground at the centre, with the default waves.
    generator_class = unreal.load_class(None, "/Script/HordeKiller.HKHordeGenerator")
    generator = actors.spawn_actor_from_class(generator_class, unreal.Vector(0.0, 0.0, FLOOR_Z))
    generator.set_actor_label("HordeGenerator")
    generator.set_editor_property("config", unreal.load_asset(HORDE_CONFIG_PATH))
    # Keep spawns 2 m inside the house fronts.
    generator.set_editor_property("spawn_area_half_size", SQUARE_HALF_SIZE - 200.0)

    if not levels.save_current_level():
        unreal.log_error("HKVillage: could not save {}".format(LEVEL_PATH))
        return

    unreal.log("HKVillage: created {} with {} actors".format(LEVEL_PATH, len(actors.get_all_level_actors())))


main()
