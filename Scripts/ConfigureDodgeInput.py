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
changed = 0

for mapping in mappings:
    path = action_path(mapping)
    current_key = key_name(mapping)
    if "IA_Crouch" in path and current_key == "C":
        replacement_key = unreal.Key()
        replacement_key.set_editor_property("key_name", "X")
        mapping.set_editor_property("key", replacement_key)
        changed += 1

if changed:
    mapping_data.set_editor_property("mappings", mappings)
    context.set_editor_property("default_key_mappings", mapping_data)
    if not unreal.EditorAssetLibrary.save_loaded_asset(context, only_if_is_dirty=False):
        raise RuntimeError("Failed to save {}".format(MAPPING_CONTEXT_PATH))

verified_context = unreal.load_asset(MAPPING_CONTEXT_PATH)
verified_data = verified_context.get_editor_property("default_key_mappings")
verified_mappings = list(verified_data.get_editor_property("mappings"))
crouch_c = 0
crouch_x = 0

for mapping in verified_mappings:
    path = action_path(mapping)
    current_key = key_name(mapping)
    if current_key in ("C", "X"):
        unreal.log_warning("DODGE_INPUT_VERIFY key={} action={}".format(current_key, path))
    if "IA_Crouch" in path and current_key == "C":
        crouch_c += 1
    if "IA_Crouch" in path and current_key == "X":
        crouch_x += 1

if crouch_c != 0 or crouch_x < 1:
    raise RuntimeError(
        "Crouch remap verification failed: crouch_c={} crouch_x={}".format(crouch_c, crouch_x)
    )

unreal.log_warning(
    "DODGE_INPUT_DONE changed={} crouch_c={} crouch_x={}".format(changed, crouch_c, crouch_x)
)
