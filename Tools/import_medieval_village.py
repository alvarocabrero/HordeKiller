# Copyright (c) 2026 Álvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.
"""Imports Quaternius's Medieval Village MegaKit (Standard version, CC0) into the project.

Download the kit from https://quaternius.itch.io/medieval-village-megakit, unzip it and point the
HK_MEDIEVAL_KIT_DIR environment variable at the unzipped "Medieval Village MegaKit[Standard]" folder.
Then run this script without opening the editor window:

    UnrealEditor-Cmd.exe <path>\\HordeKiller.uproject -run=pythonscript -script=<path>\\Tools\\import_medieval_village.py

It imports the textures and the models into Content/ThirdParty/Quaternius/MedievalVillage, creates the
materials the models ask for and gives every model collision. The kit's own material setups are for
other engines, so the materials are rebuilt here from three small parent materials.

Assets that already exist are left as they are, so the script can be run again after a partial import.
"""

import os

import unreal

KIT_DIR = os.environ.get("HK_MEDIEVAL_KIT_DIR", "")

ROOT = "/Game/ThirdParty/Quaternius/MedievalVillage"
MESH_FOLDER = ROOT + "/Meshes"
TEXTURE_FOLDER = ROOT + "/Textures"
MATERIAL_FOLDER = ROOT + "/Materials"

# Material name -> (base colour, normal, roughness) texture names. The roughness textures keep roughness
# in their green channel, both the plain ones and the packed occlusion / roughness / metallic ones.
SURFACES = {
    "MI_WoodTrim": ("T_WoodTrim_BaseColor", "T_WoodTrim_Normal", "T_WoodTrim_Roughness"),
    "MI_WoodTrim_Wear": ("T_WoodTrim_BaseColor", "T_WoodTrim_Normal", "T_WoodTrim_Roughness"),
    "MI_RockTrim": ("T_RockTrim_BaseColor", "T_RockTrim_Normal", "T_RockTrim_ORM"),
    "MI_Brick": ("T_Brick_BaseColor", "T_Brick_Normal", "T_Brick_Roughness"),
    "MI_RedBrick": ("T_RedBrick_BaseColor", "T_Brick_Normal", "T_Brick_Roughness"),
    "MI_UnevenBrick": ("T_UnevenBrick_BaseColor", "T_UnevenBrick_Normal", "T_UnevenBrick_Roughness"),
    "MI_Plaster": ("T_Plaster_BaseColor", "T_Plaster_Normal", "T_Plaster_ORM"),
    "MI_RoundTiles": ("T_RoundTiles_BaseColor", "T_RoundTiles_Normal", "T_RoundTiles_Roughness"),
    "MI_MetalOrnaments": ("T_MetalOrnaments_BaseColor", None, "T_MetalOrnaments_Roughness"),
}
METALLIC = {"MI_MetalOrnaments": 1.0}

# Texture name -> folder inside the kit. The normal maps in "Textures" are the ones made for Unreal;
# the ones next to the glTF files use the other convention for the green channel.
TEXTURES = {}
for _base, _normal, _rough in SURFACES.values():
    TEXTURES[_base] = "glTF"
    TEXTURES[_rough] = "glTF"
    if _normal:
        TEXTURES[_normal] = "Textures"
TEXTURES["T_VineLeaf_png"] = "glTF"

eal = unreal.EditorAssetLibrary
mel = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()


def log(message):
    unreal.log("HKVillageImport: {}".format(message))


def import_textures():
    """Imports the kit's textures and marks the non-colour ones as linear data."""
    tasks = []
    for name, folder in TEXTURES.items():
        if eal.does_asset_exist("{}/{}".format(TEXTURE_FOLDER, name)):
            continue
        task = unreal.AssetImportTask()
        task.filename = os.path.join(KIT_DIR, folder, name + ".png")
        task.destination_path = TEXTURE_FOLDER
        task.automated = True
        task.save = False
        tasks.append(task)
    if tasks:
        tools.import_asset_tasks(tasks)

    for name in TEXTURES:
        texture = unreal.load_asset("{}/{}".format(TEXTURE_FOLDER, name))
        if name.endswith("_Normal"):
            texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
            texture.set_editor_property("srgb", False)
        elif name.endswith("_Roughness") or name.endswith("_ORM"):
            texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
            texture.set_editor_property("srgb", False)
        eal.save_loaded_asset(texture)
    log("textures ready: {}".format(len(TEXTURES)))


def texture_parameter(material, name, texture, sampler, y):
    node = mel.create_material_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -700, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("texture", texture)
    node.set_editor_property("sampler_type", sampler)
    return node


def tiled_uvs(material):
    """Texture coordinates multiplied by a "Tiling" parameter, so a large surface can repeat the texture."""
    coords = mel.create_material_expression(material, unreal.MaterialExpressionTextureCoordinate, -1200, 0)
    tiling = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -1200, 150)
    tiling.set_editor_property("parameter_name", "Tiling")
    tiling.set_editor_property("default_value", 1.0)
    scaled = mel.create_material_expression(material, unreal.MaterialExpressionMultiply, -1000, 50)
    mel.connect_material_expressions(coords, "", scaled, "A")
    mel.connect_material_expressions(tiling, "", scaled, "B")
    return scaled


def create_surface_parent():
    """Opaque parent material: base colour, normal map and roughness from textures."""
    path = MATERIAL_FOLDER + "/M_HKVillageSurface"
    if eal.does_asset_exist(path):
        return unreal.load_asset(path)

    material = tools.create_asset("M_HKVillageSurface", MATERIAL_FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property("two_sided", True)
    uvs = tiled_uvs(material)

    base = texture_parameter(material, "BaseColor", unreal.load_asset(TEXTURE_FOLDER + "/T_WoodTrim_BaseColor"),
                             unreal.MaterialSamplerType.SAMPLERTYPE_COLOR, -300)
    normal = texture_parameter(material, "Normal", unreal.load_asset("/Engine/EngineMaterials/DefaultNormal"),
                               unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL, 0)
    rough = texture_parameter(material, "Roughness", unreal.load_asset(TEXTURE_FOLDER + "/T_WoodTrim_Roughness"),
                              unreal.MaterialSamplerType.SAMPLERTYPE_MASKS, 300)
    for node in (base, normal, rough):
        mel.connect_material_expressions(uvs, "", node, "UVs")

    metallic = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -700, 600)
    metallic.set_editor_property("parameter_name", "Metallic")
    metallic.set_editor_property("default_value", 0.0)

    mel.connect_material_property(base, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(normal, "RGB", unreal.MaterialProperty.MP_NORMAL)
    mel.connect_material_property(rough, "G", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(metallic, "", unreal.MaterialProperty.MP_METALLIC)
    mel.recompile_material(material)
    eal.save_loaded_asset(material)
    return material


def create_foliage_material():
    """Masked material for the vines: the leaf texture cuts the shape and a flat green colours it."""
    path = MATERIAL_FOLDER + "/MI_Vine"
    if eal.does_asset_exist(path):
        return

    material = tools.create_asset("MI_Vine", MATERIAL_FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property("two_sided", True)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED)
    leaf = texture_parameter(material, "Leaf", unreal.load_asset(TEXTURE_FOLDER + "/T_VineLeaf_png"),
                             unreal.MaterialSamplerType.SAMPLERTYPE_COLOR, 0)
    color = mel.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, -700, -200)
    color.set_editor_property("constant", unreal.LinearColor(0.10, 0.35, 0.0, 1.0))
    mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(leaf, "A", unreal.MaterialProperty.MP_OPACITY_MASK)
    mel.recompile_material(material)
    eal.save_loaded_asset(material)


def create_glass_material():
    """Dark, glossy and opaque: enough for windows nobody looks through."""
    path = MATERIAL_FOLDER + "/MI_WindowGlass"
    if eal.does_asset_exist(path):
        return

    material = tools.create_asset("MI_WindowGlass", MATERIAL_FOLDER, unreal.Material, unreal.MaterialFactoryNew())
    color = mel.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, -400, 0)
    color.set_editor_property("constant", unreal.LinearColor(0.02, 0.04, 0.06, 1.0))
    rough = mel.create_material_expression(material, unreal.MaterialExpressionConstant, -400, 200)
    rough.set_editor_property("r", 0.15)
    mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.recompile_material(material)
    eal.save_loaded_asset(material)


def create_materials():
    parent = create_surface_parent()
    for name, (base, normal, rough) in SURFACES.items():
        if eal.does_asset_exist("{}/{}".format(MATERIAL_FOLDER, name)):
            continue
        instance = tools.create_asset(name, MATERIAL_FOLDER, unreal.MaterialInstanceConstant,
                                      unreal.MaterialInstanceConstantFactoryNew())
        mel.set_material_instance_parent(instance, parent)
        mel.set_material_instance_texture_parameter_value(
            instance, "BaseColor", unreal.load_asset("{}/{}".format(TEXTURE_FOLDER, base)))
        if normal:
            mel.set_material_instance_texture_parameter_value(
                instance, "Normal", unreal.load_asset("{}/{}".format(TEXTURE_FOLDER, normal)))
        mel.set_material_instance_texture_parameter_value(
            instance, "Roughness", unreal.load_asset("{}/{}".format(TEXTURE_FOLDER, rough)))
        mel.set_material_instance_scalar_parameter_value(instance, "Metallic", METALLIC.get(name, 0.0))
        eal.save_loaded_asset(instance)
    create_foliage_material()
    create_glass_material()
    log("materials ready")


def mesh_only_pipeline():
    """Import settings that bring in the model alone: the materials and textures are made by this script."""
    pipeline = unreal.InterchangeGenericAssetsPipeline()
    pipeline.set_editor_property("use_source_name_for_asset", True)
    materials = pipeline.get_editor_property("material_pipeline")
    materials.set_editor_property("import_materials", False)
    materials.get_editor_property("texture_pipeline").set_editor_property("import_textures", False)
    meshes = pipeline.get_editor_property("mesh_pipeline")
    meshes.set_editor_property("import_skeletal_meshes", False)
    meshes.set_editor_property("collision", False)
    meshes.set_editor_property("build_nanite", False)
    pipeline.get_editor_property("animation_pipeline").set_editor_property("import_animations", False)
    return pipeline


def import_meshes():
    manager = unreal.InterchangeManager.get_interchange_manager_scripted()
    folder = os.path.join(KIT_DIR, "glTF")
    names = sorted(f[:-5] for f in os.listdir(folder) if f.endswith(".gltf"))
    imported = 0
    for name in names:
        path = "{}/{}".format(MESH_FOLDER, name)
        if not eal.does_asset_exist(path):
            parameters = unreal.ImportAssetParameters()
            parameters.is_automated = True
            parameters.override_pipelines = [unreal.SoftObjectPath(mesh_only_pipeline().get_path_name())]
            source = unreal.InterchangeManager.create_source_data(os.path.join(folder, name + ".gltf"))
            manager.import_asset(MESH_FOLDER, source, parameters)
            imported += 1

        mesh = unreal.load_asset(path)
        if not mesh:
            unreal.log_error("HKVillageImport: {} was not imported".format(name))
            continue

        # The material slots are named after the kit's materials.
        for index, slot in enumerate(mesh.static_materials):
            material_path = "{}/{}".format(MATERIAL_FOLDER, slot.material_slot_name)
            if eal.does_asset_exist(material_path):
                mesh.set_material(index, unreal.load_asset(material_path))
            else:
                unreal.log_warning("HKVillageImport: {} uses unknown material {}".format(name, slot.material_slot_name))

        # The pieces are simple enough to collide with their own triangles.
        body = mesh.get_editor_property("body_setup")
        body.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        eal.save_loaded_asset(mesh)
    log("meshes ready: {} ({} imported now)".format(len(names), imported))


def main():
    if not os.path.isdir(os.path.join(KIT_DIR, "glTF")):
        unreal.log_error("HKVillageImport: set HK_MEDIEVAL_KIT_DIR to the unzipped kit folder (got '{}')".format(KIT_DIR))
        return
    import_textures()
    create_materials()
    import_meshes()
    log("done")


main()
