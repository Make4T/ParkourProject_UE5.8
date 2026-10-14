import unreal


MAPPING_CONTEXT_PATH = "/Game/Input/IMC_Sandbox"
CROUCH_ACTION_PATH = "/Game/Input/IA_Crouch"


def key_name(mapping):
    key = mapping.get_editor_property("key")
    return str(key.get_editor_property("key_name"))


def action_path(mapping):
    action = mapping.get_editor_property("action")
    return action.get_path_name() if action else ""


context = unreal.load_asset(MAPPING_CONTEXT_PATH)
if not context:
    raise RuntimeError("Could not load {}".format(MAPPING_CONTEXT_PATH))

mapping_data = context.get_editor_property("default_key_mappings")
mappings = list(mapping_data.get_editor_property("mappings"))
kept_mappings = []
removed = 0

for mapping in mappings:
    path = action_path(mapping)
    current_key = key_name(mapping)
    if "IA_Crouch" in path and current_key in ("C", "X"):
        unreal.log_warning("SLIDE_INPUT_REMOVE key={} action={}".format(current_key, path))
        removed += 1
        continue
    kept_mappings.append(mapping)

if removed:
    mapping_data.set_editor_property("mappings", kept_mappings)
    context.set_editor_property("default_key_mappings", mapping_data)

crouch_action = unreal.load_asset(CROUCH_ACTION_PATH)
if not crouch_action:
    raise RuntimeError("Could not load {}".format(CROUCH_ACTION_PATH))

current_mappings = list(context.get_editor_property("default_key_mappings").get_editor_property("mappings"))
has_left_control = any(
    "IA_Crouch" in action_path(mapping) and key_name(mapping) == "LeftControl"
    for mapping in current_mappings
)
if not has_left_control:
    left_control_key = unreal.Key()
    left_control_key.set_editor_property("key_name", "LeftControl")
    context.map_key(crouch_action, left_control_key)

if not unreal.EditorAssetLibrary.save_loaded_asset(context, only_if_is_dirty=False):
    raise RuntimeError("Failed to save {}".format(MAPPING_CONTEXT_PATH))

verified_context = unreal.load_asset(MAPPING_CONTEXT_PATH)
verified_data = verified_context.get_editor_property("default_key_mappings")
verified_mappings = list(verified_data.get_editor_property("mappings"))
old_crouch_mappings = []
left_control_crouch = 0

for mapping in verified_mappings:
    path = action_path(mapping)
    current_key = key_name(mapping)
    if "IA_Crouch" in path and current_key in ("C", "X"):
        old_crouch_mappings.append((current_key, path))
    if "IA_Crouch" in path and current_key == "LeftControl":
        left_control_crouch += 1

if old_crouch_mappings:
    raise RuntimeError("Old C/X crouch mappings remain: {}".format(old_crouch_mappings))
if left_control_crouch != 1:
    raise RuntimeError("Expected one LeftControl crouch mapping, found {}".format(left_control_crouch))

unreal.log_warning(
    "SLIDE_INPUT_DONE removed={} old_crouch_mappings={} left_control_crouch={}".format(
        removed, len(old_crouch_mappings), left_control_crouch
    )
)
