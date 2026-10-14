# Parkour foundation (UE 5.8)

## Safe staging assets

- Character: `/Game/Blueprints/BP_PlayerCharacter_Parkour`
- GameMode: `/Game/Blueprints/GameMode_Parkour`
- Active `GameMode_1` and the map override have not been changed.
- `BP_PlayerCharacter_Parkour` keeps the Game Animation Sample camera, mesh, animation setup and the current PlayerState-owned GAS bridge.

To test in the editor, open `Lvl_ThirdPerson` and temporarily select `GameMode_Parkour` under **World Settings > GameMode Override**. Restore the previous override after testing if the parkour staging pawn should not remain active.

## Native classes

- `UGameProjectSummer26ParkourMovementComponent` owns custom parkour movement and its network saved-move foundation.
- `UGameProjectSummer26ParkourTraversalComponent` performs read-only vault/mantle and wall-run queries.
- `UGameProjectSummer26ParkourSettings` contains trace and movement tuning defaults and can later be supplied as a Data Asset.
- `AGameProjectSummer26ParkourCharacter` installs the custom movement and traversal components without replacing the sample presentation setup.

The initial custom movement mode implements slope-aware sliding. Hanging, wall running and wall climbing currently fail safely back to falling and must not be enabled before their movement physics and server validation are implemented.

## Important integration boundary

The copied sample character still contains `AC_TraversalLogic`. Do not connect native parkour actions to input while that component also controls the same action. Migrate one action at a time and disable the corresponding legacy path first.

Recommended next slice:

1. Route Ctrl through a GAS slide ability/tag.
2. Make the ability request `SetWantsToSlide` on the custom movement component.
3. Disable the old PlayerState fallback slide only for the parkour pawn.
4. Validate start, deceleration, crouch hold/release and ability cancellation in PIE.
5. Add server-side request validation before multiplayer use.

## Traversal GAS bridge

The first traversal GAS integration is now present:

- `UGameProjectSummer26TraversalAbility` owns activation and the `State.Movement.Traversing` lifetime.
- `AGameProjectSummer26ParkourCharacter::HandleTraversalInputPressed` sends one `Input.Ability.Traversal` pulse.
- The native character reflects the copied sample's `GetTraversalCheckInputs` return struct into `AC_TraversalLogic.TryTraversalAction` without duplicating the sample Blueprint structs in C++.
- A failed traversal request falls back to normal `ACharacter::Jump`.
- Sprint is cancelled when traversal starts; sprint, dodge and fallback slide are blocked while traversal is active.
- Chooser, montage playback and Motion Warping remain owned by the proven Game Animation Sample component.

Blueprint input migration:

1. Test `Handle Traversal Input Pressed` from the temporary P Pressed event after disconnecting the direct `Try Traversal Action` execution path.
2. After that passes, disconnect both old IA_Jump Started/Triggered execution paths.
3. Connect IA_Jump Started to `Handle Traversal Input Pressed`.
4. Keep the old nodes disconnected until normal jump, vault, low mantle and high mantle have all passed PIE testing.

## Verification completed

- `GameProjectSummer26Editor Win64 Development` builds successfully.
- Automation test `GameProjectSummer26.Parkour.Foundation` succeeds.
- Headless runtime test loads `GameMode_Parkour_C` and spawns `BP_PlayerCharacter_Parkour_C`.
- The PlayerState GAS bridge initializes the staging pawn with two abilities and Health, Stamina, Mana and Endurance at 100.
- The existing GAS status widget is added.
- The runtime log contains no fatal, Blueprint runtime, linker, class or load errors.

Headless verification does not prove animation quality. Motion Matching, montage selection, Motion Warping targets and visible UI layout still require an editor PIE test.
