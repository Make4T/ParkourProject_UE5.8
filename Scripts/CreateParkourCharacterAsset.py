import unreal


SOURCE_CHARACTER = "/Game/Blueprints/BP_PlayerCharacter_1"
TARGET_CHARACTER = "/Game/Blueprints/BP_PlayerCharacter_Parkour"
PARKOUR_PARENT_CLASS = "/Script/GameProjectSummer26.GameProjectSummer26ParkourCharacter"
PARKOUR_MOVEMENT_CLASS = "/Script/GameProjectSummer26.GameProjectSummer26ParkourMovementComponent"
SOURCE_GAME_MODE = "/Game/Blueprints/GameMode_1"
TARGET_GAME_MODE = "/Game/Blueprints/GameMode_Parkour"


def log(message):
    unreal.log_warning("PARKOUR_CHARACTER_SETUP " + str(message))


source = unreal.load_asset(SOURCE_CHARACTER)
if not isinstance(source, unreal.Blueprint):
    raise RuntimeError("Source is not a Blueprint: {}".format(SOURCE_CHARACTER))

parent_class = unreal.load_class(None, PARKOUR_PARENT_CLASS)
if not parent_class:
    raise RuntimeError("Could not load native parent class {}".format(PARKOUR_PARENT_CLASS))

if unreal.EditorAssetLibrary.does_asset_exist(TARGET_CHARACTER):
    parkour_blueprint = unreal.load_asset(TARGET_CHARACTER)
    log("Using existing target asset {}".format(TARGET_CHARACTER))
else:
    parkour_blueprint = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_CHARACTER, TARGET_CHARACTER)
    if not parkour_blueprint:
        raise RuntimeError("Failed to duplicate {} to {}".format(SOURCE_CHARACTER, TARGET_CHARACTER))
    log("Duplicated {} -> {}".format(SOURCE_CHARACTER, TARGET_CHARACTER))

if not isinstance(parkour_blueprint, unreal.Blueprint):
    raise RuntimeError("Target is not a Blueprint: {}".format(TARGET_CHARACTER))

current_parent = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(parkour_blueprint)
if current_parent != parent_class:
    unreal.BlueprintEditorLibrary.reparent_blueprint(parkour_blueprint, parent_class)
    log("Reparented {} to {}".format(TARGET_CHARACTER, PARKOUR_PARENT_CLASS))

unreal.BlueprintEditorLibrary.compile_blueprint(parkour_blueprint)
if not unreal.EditorAssetLibrary.save_loaded_asset(parkour_blueprint, only_if_is_dirty=False):
    raise RuntimeError("Failed to save {}".format(TARGET_CHARACTER))

verified = unreal.load_asset(TARGET_CHARACTER)
verified_parent = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(verified)
if verified_parent != parent_class:
    raise RuntimeError("Parent verification failed: {}".format(verified_parent.get_path_name()))

cdo = unreal.get_default_object(verified.generated_class())
movement = cdo.get_editor_property("character_movement")
movement_class_path = movement.get_class().get_path_name() if movement else "None"
if movement_class_path != PARKOUR_MOVEMENT_CLASS:
    raise RuntimeError(
        "Movement component verification failed: expected {}, got {}".format(
            PARKOUR_MOVEMENT_CLASS, movement_class_path
        )
    )

traversal_component = cdo.get_editor_property("parkour_traversal_component")
if not traversal_component:
    raise RuntimeError("Native ParkourTraversalComponent is missing from the target CDO")

log("VERIFY parent={}".format(verified_parent.get_path_name()))
log("VERIFY movement={}".format(movement_class_path))
log("VERIFY traversal={}".format(traversal_component.get_class().get_path_name()))

source_game_mode = unreal.load_asset(SOURCE_GAME_MODE)
if not isinstance(source_game_mode, unreal.Blueprint):
    raise RuntimeError("Source GameMode is not a Blueprint: {}".format(SOURCE_GAME_MODE))

if unreal.EditorAssetLibrary.does_asset_exist(TARGET_GAME_MODE):
    parkour_game_mode = unreal.load_asset(TARGET_GAME_MODE)
    log("Using existing target asset {}".format(TARGET_GAME_MODE))
else:
    parkour_game_mode = unreal.EditorAssetLibrary.duplicate_asset(SOURCE_GAME_MODE, TARGET_GAME_MODE)
    if not parkour_game_mode:
        raise RuntimeError("Failed to duplicate {} to {}".format(SOURCE_GAME_MODE, TARGET_GAME_MODE))
    log("Duplicated {} -> {}".format(SOURCE_GAME_MODE, TARGET_GAME_MODE))

if not isinstance(parkour_game_mode, unreal.Blueprint):
    raise RuntimeError("Target GameMode is not a Blueprint: {}".format(TARGET_GAME_MODE))

game_mode_cdo = unreal.get_default_object(parkour_game_mode.generated_class())
game_mode_cdo.set_editor_property("default_pawn_class", verified.generated_class())
unreal.BlueprintEditorLibrary.compile_blueprint(parkour_game_mode)
if not unreal.EditorAssetLibrary.save_loaded_asset(parkour_game_mode, only_if_is_dirty=False):
    raise RuntimeError("Failed to save {}".format(TARGET_GAME_MODE))

verified_game_mode = unreal.load_asset(TARGET_GAME_MODE)
verified_game_mode_cdo = unreal.get_default_object(verified_game_mode.generated_class())
verified_pawn_class = verified_game_mode_cdo.get_editor_property("default_pawn_class")
if verified_pawn_class != verified.generated_class():
    raise RuntimeError(
        "GameMode pawn verification failed: {}".format(verified_pawn_class.get_path_name())
    )

log("VERIFY game_mode={}".format(verified_game_mode.get_path_name()))
log("VERIFY default_pawn={}".format(verified_pawn_class.get_path_name()))
log("DONE")
