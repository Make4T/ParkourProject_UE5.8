import unreal


SOURCE_CHARACTER = "/Game/Blueprints/SandboxCharacter_CMC"
TARGET_CHARACTER = "/Game/Blueprints/BP_PlayerCharacter_1"
SOURCE_GAME_MODE = "/Game/Blueprints/GM_Sandbox"
TARGET_GAME_MODE = "/Game/Blueprints/GameMode_1"
PLAYER_STATE_CLASS_PATH = "/Script/GameProjectSummer26.GameProjectSummer26PlayerState"


def log(message):
    unreal.log_warning("GASP_GAS_SETUP " + str(message))


def duplicate_or_load(source_path, target_path):
    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        log("Using existing target asset {}".format(target_path))
        return unreal.load_asset(target_path)

    duplicated = unreal.EditorAssetLibrary.duplicate_asset(source_path, target_path)
    if not duplicated:
        raise RuntimeError("Failed to duplicate {} to {}".format(source_path, target_path))

    log("Duplicated {} -> {}".format(source_path, target_path))
    return duplicated


character_blueprint = duplicate_or_load(SOURCE_CHARACTER, TARGET_CHARACTER)
game_mode_blueprint = duplicate_or_load(SOURCE_GAME_MODE, TARGET_GAME_MODE)

if not isinstance(character_blueprint, unreal.Blueprint):
    raise RuntimeError("{} is not a Blueprint".format(TARGET_CHARACTER))

if not isinstance(game_mode_blueprint, unreal.Blueprint):
    raise RuntimeError("{} is not a Blueprint".format(TARGET_GAME_MODE))

player_state_class = unreal.load_class(None, PLAYER_STATE_CLASS_PATH)
if not player_state_class:
    raise RuntimeError("Could not load {}".format(PLAYER_STATE_CLASS_PATH))

game_mode_cdo = unreal.get_default_object(game_mode_blueprint.generated_class())
game_mode_cdo.set_editor_property("default_pawn_class", character_blueprint.generated_class())
game_mode_cdo.set_editor_property("player_state_class", player_state_class)

if hasattr(unreal, "BlueprintEditorLibrary"):
    unreal.BlueprintEditorLibrary.compile_blueprint(character_blueprint)
    unreal.BlueprintEditorLibrary.compile_blueprint(game_mode_blueprint)

if not unreal.EditorAssetLibrary.save_loaded_asset(character_blueprint, only_if_is_dirty=False):
    raise RuntimeError("Failed to save {}".format(TARGET_CHARACTER))

if not unreal.EditorAssetLibrary.save_loaded_asset(game_mode_blueprint, only_if_is_dirty=False):
    raise RuntimeError("Failed to save {}".format(TARGET_GAME_MODE))

verified_character = unreal.load_asset(TARGET_CHARACTER)
verified_game_mode = unreal.load_asset(TARGET_GAME_MODE)
verified_cdo = unreal.get_default_object(verified_game_mode.generated_class())

log("VERIFY character={} generated={}".format(
    verified_character.get_path_name(),
    verified_character.generated_class().get_path_name()))
log("VERIFY game_mode={} generated={}".format(
    verified_game_mode.get_path_name(),
    verified_game_mode.generated_class().get_path_name()))
log("VERIFY default_pawn_class={}".format(
    verified_cdo.get_editor_property("default_pawn_class").get_path_name()))
log("VERIFY player_controller_class={}".format(
    verified_cdo.get_editor_property("player_controller_class").get_path_name()))
log("VERIFY player_state_class={}".format(
    verified_cdo.get_editor_property("player_state_class").get_path_name()))
log("DONE")
