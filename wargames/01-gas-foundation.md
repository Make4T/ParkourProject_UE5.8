# WARGAME — UE 5.8 C++ GAS Foundation

## Mission status and rules of engagement

This document is a **wargame, not an execution report**. The GAS foundation has not been implemented, compiled, opened in Unreal Editor, or multiplayer-tested during this mission. Read-only recon was performed; every later result must be recorded by the executor as `PASS`, `FAIL`, `NOT RUN`, or `BLOCKED`, with a command, log excerpt, screenshot, or observed runtime value. A planned result is never evidence.

Mission objective: add one minimal, reusable multiplayer Gameplay Ability System vertical slice to `GameProjectSummer26`: PlayerState-owned ASC, Character avatar, six replicated attributes, native gameplay tags, tag-driven Enhanced Input, server-only duplicate-safe startup grants, and one ability that changes Oxygen from 100 to 90 exactly once per accepted activation.

Out of scope: broad UI, combat migration, inventory, AI abilities, save/load, new respawn mechanics, animation-sample repairs, production balancing, and unrelated refactors.

Authoritative source order:

1. UE 5.8 installed headers and Epic's current documentation.
2. Actual project source, config, Blueprint defaults, logs, and runtime observations.
3. The tutorial playlist only as a structural sequence; never copy an older API over UE 5.8 evidence.

## Recon Findings

### R1 — Project, engine, module, and change-safety baseline

**Observed:** project root is `D:\CRIMSONYEYS\UE5_8\GameProjectSummer26 5.8`; project file is `GameProjectSummer26.uproject`; the sole runtime module is `GameProjectSummer26`; the export macro is `GAMEPROJECTSUMMER26_API`. The project and targets declare UE 5.8 (`EngineAssociation: 5.8`, `EngineIncludeOrderVersion.Unreal5_8`, build settings V7). The associated launcher build is `D:\UE_5.8`, version 5.8.0, promoted changelist 55116800, branch `++UE5+Release-5.8`.

**Observed:** the VS Code workspace already contains a valid editor build task equivalent to:

```powershell
& 'D:\UE_5.8\Engine\Build\BatchFiles\Build.bat' GameProjectSummer26Editor Win64 Development 'D:\CRIMSONYEYS\UE5_8\GameProjectSummer26 5.8\GameProjectSummer26.uproject' -waitmutex
```

**Observed:** the directory is not a Git worktree (`git status` reports that it is not a repository). Therefore no clean-worktree claim is possible.

**RECON NEEDED — source-control baseline and overlap safety.** Before editing, attach/initialize the intended version control or create a timestamped, external recovery snapshot plus a SHA-256 manifest of every intended touched file. Record untracked and locally modified files by another trustworthy mechanism. Abort if a recoverable baseline cannot be established or if overlapping user changes cannot be separated safely.

### R2 — Untouched build and runtime baseline

**Observed, historical only:** `C:\Users\turun\AppData\Local\UnrealBuildTool\Log.txt` records a successful `GameProjectSummer26Editor` build using MSVC 14.50.35717 and Windows SDK 10.0.22621. This is useful environment evidence, not the required fresh pre-change baseline.

**Observed:** the newest saved PIE log contains repeated pre-existing `ABP_GenericRetarget` `Accessed None` errors in imported Game Animation Sample content. These appear in the sample's DefaultLevel path, not in the active ThirdPerson map. Do not repair or attribute them to GAS in this mission.

**RECON NEEDED — fresh untouched baseline.** Build the unmodified target with the exact command above, then run the active ThirdPerson map once. Save the complete build log and runtime log before any GAS edit. Use `Content/ThirdPerson/Lvl_ThirdPerson`, not the Game Animation Sample DefaultLevel, to isolate this mission.

### R3 — Existing GAS footprint and integration route

**Observed:** source/config/project searches found no `UAbilitySystemComponent`, `IAbilitySystemInterface`, `UAttributeSet`, `UGameplayAbility`, `UGameplayEffect`, `InitAbilityActorInfo`, or `GiveAbility`. Binary-content string searches likewise found no project GAS asset footprint. GameplayStateTree and GameplayInteractions content must not be mistaken for a GAS foundation.

**Decision:** take Fork A, Route B: create one coherent project GAS foundation. Do not create a second path later unless runtime recon disproves this finding.

### R4 — Active gameplay class chain

**Observed:** `DefaultEngine.ini` selects `/Game/ThirdPerson/Lvl_ThirdPerson` and `/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode`. Blueprint metadata and active class redirects indicate this expected chain:

```text
BP_ThirdPersonGameMode
  -> AGameProjectSummer26GameMode
  -> BP_ThirdPersonPlayerController / AGameProjectSummer26PlayerController
  -> default APlayerState (no project PlayerState currently configured)
  -> BP_ThirdPersonCharacter / AGameProjectSummer26Character
```

The Blueprint metadata still contains old `TP_ThirdPerson` script names, but active class redirects point to `GameProjectSummer26` classes.

**RECON NEEDED — exact runtime PIE class chain and map override.** Before editing, run ThirdPerson PIE and record the server and client class names for GameMode, PlayerController, PlayerState, and Pawn. Inspect `BP_ThirdPersonGameMode` Class Defaults for a PlayerState override. If the observed chain matches, add `AGameProjectSummer26PlayerState` and set it in the native GameMode constructor. If a Blueprint or map override selects different active classes, modify the actual active chain; do not wire GAS into dead classes.

### R5 — Existing attributes and writer inventory

**Observed:** the active ThirdPerson path has no Health, Oxygen, Sanity, or maximum counterparts. A separate Combat variant has local, non-replicated `CurrentHP` and `MaxHP` on its own character with direct damage/reset/UI writers. It is not the active ThirdPerson vertical slice.

| Attribute | Active owner/writers/readers | Decision | Collision risk |
|---|---|---|---|
| Health / MaxHealth | None | Add to GAS AttributeSet | Low in ThirdPerson; future Combat migration required |
| Oxygen / MaxOxygen | None | Add to GAS AttributeSet and use for test | Lowest |
| Sanity / MaxSanity | None | Add to GAS AttributeSet | Low |
| Combat CurrentHP / MaxHP | Combat variant character; damage/reset/life-bar code | Leave unchanged | High only if Combat is later activated with GAS |

**Decision:** Oxygen is the least-coupled test attribute. Do not migrate Combat HP or introduce a second writable Oxygen variable.

### R6 — ASC owner, avatar, and lifecycle

**Observed:** active ThirdPerson has no respawn path. Other variants destroy and respawn Pawns, so Pawn replacement exists in the project and future persistence is materially plausible.

**Decision:** take Fork B, Route A. `AGameProjectSummer26PlayerState` owns exactly one replicated project ASC and the project AttributeSet; the currently possessed `AGameProjectSummer26Character` is the avatar. The Character implements `IAbilitySystemInterface` only as a forwarding access point and never creates a second ASC.

Server actor-info initialization occurs in `PossessedBy`; client initialization occurs in `OnRep_PlayerState`. Re-possession refreshes owner/avatar actor info but must not duplicate startup effects or abilities.

### R7 — Enhanced Input route

**Observed:** the local PlayerController adds mapping contexts; the Character binds Enhanced Input actions for Jump, Move, and Look. There is no tag-based input layer. Current saved PIE settings are `PIE_Standalone`, one client, one process.

**Decision:** add a narrow input-tag bridge. Character input events submit pressed/released tags to the ASC; PlayerController `PostProcessInput` calls ASC `ProcessAbilityInput` once per input frame. This satisfies pressed/held/released processing without enabling Character Tick.

**RECON NEEDED — active mapping contexts and free test key.** Inspect the active PlayerController Blueprint defaults and `IMC_Default`. Confirm the context is applied to both local listen-server and remote-client controllers. Prefer `O` for `IA_TestOxygenPulse`; if occupied, use `P`; if both are occupied, choose one demonstrably unused key and record it. Do not replace an existing binding silently.

### R8 — Gameplay-tag source

**Observed:** `DefaultGameplayTags.ini` contains imported config tags for animation/motion systems but no project native tag definitions and no `Input.Ability.Test` or `Ability.Test.OxygenPulse`.

**Decision:** preserve config tags and add only these native C++ tags:

- `Input.Ability.Test`
- `Ability.Test.OxygenPulse`

UE 5.8 installed headers confirm that ability asset tags are set in a constructor with `SetAssetTags`, and spec input tags are read/written through `GetDynamicSpecSourceTags()`; the legacy mutable `AbilityTags`/`DynamicAbilityTags` names are deprecated.

### R9 — UE 5.8 compatibility ruling

| Concern | Tutorial-shaped assumption | UE 5.8 ruling |
|---|---|---|
| ASC location | Often Pawn/Character in samples | PlayerState is valid and preferred here for persistence; Pawn is avatar |
| Ability tags | Direct mutable `AbilityTags` | Use constructor-only `SetAssetTags` |
| Spec dynamic tags | Direct `DynamicAbilityTags` | Use `GetDynamicSpecSourceTags()` |
| Attribute values | Plain floats or DataTable min/max | Use `FGameplayAttributeData`; clamp in AttributeSet code; DataTable min/max do not clamp runtime values |
| Grants | Could appear in generic initialization | `GiveAbility` only on authority; duplicate-guard it |
| Cost | Direct subtraction in ability | Cost GameplayEffect applied by successful `CommitAbility` |
| Ability lifecycle | Activation body may simply return | Every path must call `EndAbility`; failed commit ends cancelled |
| Prediction | Local prediction assumed safe | Start Local Predicted only if two-player evidence is clean; fall back to Server Initiated on correction/duplication faults |

**RECON NEEDED — tutorial playlist episode-level compatibility.** The playlist could not be inspected reliably during recon. Before using any episode-specific code, record its engine version and compare each copied API to the installed UE 5.8 headers. The structural sequence may guide work; Epic documentation and compiled 5.8 APIs decide conflicts.

Primary references:

- [Gameplay Ability System Component and Gameplay Attributes](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-component-and-gameplay-attributes-in-unreal-engine)
- [Gameplay Attributes and Attribute Sets](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-attributes-and-attribute-sets-for-the-gameplay-ability-system-in-unreal-engine)
- [Using Gameplay Abilities](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-gameplay-abilities-in-unreal-engine)
- [Understanding the Gameplay Ability System](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-the-unreal-engine-gameplay-ability-system)

### R10 — Multiplayer test map and route

**Observed:** `Content/ThirdPerson/Lvl_ThirdPerson.umap` is the active, lightweight candidate and avoids known Game Animation Sample errors. Saved PIE settings are currently Standalone with one client. The number and placement of PlayerStarts and two-player behavior were not proven in read-only recon.

**RECON NEEDED — two-player PlayerStart and listen-server PIE route.** In the untouched baseline, inspect World Outliner PlayerStarts, select Play As Listen Server, two players, and separate windows if needed, then verify both players spawn independently and both accept movement/input. If that baseline fails, take Abort 2; do not hide it by adding a test map or bypassing normal startup during this mission.

## Planned file and asset surface

Names may change only to follow an already-observed project convention; record any deviation.

```text
GameProjectSummer26.uproject
Source/GameProjectSummer26/GameProjectSummer26.Build.cs
Source/GameProjectSummer26/GameProjectSummer26GameplayTags.h/.cpp
Source/GameProjectSummer26/AbilitySystem/GameProjectSummer26AbilitySystemComponent.h/.cpp
Source/GameProjectSummer26/AbilitySystem/GameProjectSummer26AttributeSet.h/.cpp
Source/GameProjectSummer26/AbilitySystem/GameProjectSummer26GameplayAbility.h/.cpp
Source/GameProjectSummer26/AbilitySystem/GameProjectSummer26DefaultAttributesEffect.h/.cpp
Source/GameProjectSummer26/AbilitySystem/GameProjectSummer26OxygenCostEffect.h/.cpp
Source/GameProjectSummer26/AbilitySystem/GameProjectSummer26TestOxygenPulseAbility.h/.cpp
Source/GameProjectSummer26/GameProjectSummer26PlayerState.h/.cpp
Source/GameProjectSummer26/GameProjectSummer26GameMode.h/.cpp
Source/GameProjectSummer26/GameProjectSummer26Character.h/.cpp
Source/GameProjectSummer26/GameProjectSummer26PlayerController.h/.cpp
Content/Input/Actions/IA_TestOxygenPulse.uasset
Content/Input/IMC_Default.uasset (or the runtime-proven active context)
Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.uasset
```

Do not touch Game Animation Sample assets, Combat variant HP, maps, menus, or unrelated Blueprints.

## Execution moves

### Move 1 — Freeze and prove the pre-change baseline

**Action:** Resolve R1, R2, R4, R7, and R10 before editing. Capture file hashes/recovery copies, exact class chain, active map/context, PlayerStart count, fresh Development Editor build, and untouched two-player Listen Server launch/movement result. Preserve logs under a location excluded from source changes.

**Expected observation:** clean fresh build; ThirdPerson is the active map; runtime classes match or clearly supersede the inferred chain; two players spawn and move; no GAS classes exist.

**Most likely failure:** build or two-player PIE already fails, or the active classes differ from inferred classes.

**Cause it signals:** pre-existing project fault, stale redirect/Blueprint override, bad PIE setup, or an unsafe change baseline.

**Counter-move:** isolate and record the pre-existing issue; adapt only to the runtime-proven class chain. Do not repair unrelated animation/sample faults. If baseline compilation, launch, two-player spawn/input, or recoverability remains unproven, abort.

**Fork trigger:** matching chain selects the planned files; differing chain selects Fork C. Existing GAS evidence selects Fork A Route A instead of creating new classes.

### Move 2 — Enable the plugin and module dependencies

**Action:** Enable `GameplayAbilities` in the `.uproject`. Add direct module dependencies `GameplayAbilities`, `GameplayTags`, and `GameplayTasks` to `GameProjectSummer26.Build.cs`; retain existing `EnhancedInput`. Regenerate project files only if UBT/VS Code discovery requires it, then build immediately.

**Expected observation:** UBT locates the installed runtime plugin and the unchanged game still builds.

**Most likely failure:** missing headers/modules, JSON error, or stale generated project data.

**Cause it signals:** descriptor syntax error, dependency placed in the wrong list, wrong engine association, or stale workspace metadata.

**Counter-move:** compare the descriptor with `D:\UE_5.8\Engine\Plugins\Runtime\GameplayAbilities\GameplayAbilities.uplugin`; remember GameplayTags and GameplayTasks are modules in that plugin, not separate `.uplugin` entries. Revert only this move if the build cannot be restored.

**Fork trigger:** plugin unavailable in the associated engine selects Abort 4; a build success permits Move 3.

### Move 3 — Define the native gameplay tags

**Action:** Add exported declarations and UE 5.8 native definitions for `Input.Ability.Test` and `Ability.Test.OxygenPulse`. Use a project namespace and `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG`. Do not duplicate or rewrite imported config tags. Build.

**Expected observation:** both tags resolve through `FGameplayTag::RequestGameplayTag` after module startup and appear in the editor tag browser.

**Most likely failure:** unresolved extern, duplicate tag warning, or tag unavailable during constructor execution.

**Cause it signals:** missing export macro/translation unit, duplicate native/config declaration, or unsafe static initialization ordering.

**Counter-move:** follow the engine's `NativeGameplayTags.h` pattern; keep declarations in a public project header and definitions in one `.cpp`; retrieve the namespace constants inside constructors rather than inventing separate static initialization.

**Fork trigger:** an identical canonical project tag discovered during runtime recon selects reuse; otherwise retain the two exact names.

### Move 4 — Add the project ASC subclass

**Action:** Create `UGameProjectSummer26AbilitySystemComponent` with `GAMEPROJECTSUMMER26_API`. Implement tag input methods using `GetDynamicSpecSourceTags()`, `ABILITYLIST_SCOPE_LOCK`, spec handles, and three duplicate-safe collections: pressed, released, and held. `ProcessAbilityInput` activates inactive matching specs on press, forwards press/release to active specs, and clears per-frame pressed/released collections after processing. Add explicit reset for avatar loss/end play.

**Expected observation:** the class compiles against UE 5.8 without deprecated member access; one press yields one activation attempt; held/released state cannot leak between frames.

**Most likely failure:** iterator invalidation, duplicate activation, a stuck held handle, or use of deprecated `DynamicAbilityTags`.

**Cause it signals:** editing activatable specs without a scope lock, non-unique handle insertion, no per-frame clearing, or tutorial-era API use.

**Counter-move:** store handles, use `AddUnique`, resolve each with `FindAbilitySpecFromHandle`, remove held on release, always clear transient arrays, and use `GetDynamicSpecSourceTags()`.

**Fork trigger:** if the active project already has a runtime-proven input processor, extend it under Fork D; otherwise keep this narrow component-owned bridge.

### Move 5 — Add the replicated AttributeSet

**Action:** Create one exported `UGameProjectSummer26AttributeSet` with `FGameplayAttributeData` for Health, MaxHealth, Oxygen, MaxOxygen, Sanity, and MaxSanity; generate accessors with GAS macros. Replicate all six using `ReplicatedUsing`, `DOREPLIFETIME_CONDITION_NOTIFY(..., COND_None, REPNOTIFY_Always)`, old-value OnRep parameters, and `GAMEPLAYATTRIBUTE_REPNOTIFY`. Clamp maxima to at least zero and currents to `[0, corresponding max]` in both base/current mutation paths needed by UE 5.8 (`PreAttributeBaseChange`, `PreAttributeChange`, and post-effect enforcement). Build.

**Expected observation:** UHT accepts all reflected declarations; server values replicate; local and remote values never leave their bounds.

**Most likely failure:** UHT/RepNotify signature errors, remote values do not update, or current value remains above a reduced maximum.

**Cause it signals:** incorrect macro/signature/lifetime registration, non-replicated property, or clamping only one mutation path.

**Counter-move:** mirror the UE 5.8 `AttributeSet.h` macro examples exactly; use `FGameplayAttributeData` old values; register every property; centralize paired clamp logic and re-clamp current when a maximum changes.

**Fork trigger:** discovery of an authoritative existing project AttributeSet selects Fork A Route A and migration; otherwise retain exactly one new set.

### Move 6 — Add default and cost GameplayEffects

**Action:** Add a default-attributes GameplayEffect that initializes all maxima and currents to 100, and an Oxygen cost GameplayEffect with one additive `-10` modifier. Configure them in C++ constructors with UE 5.8-supported modifier structures. Defaults are applied once by authority; the cost is assigned as the test ability's `CostGameplayEffectClass` and is never applied manually.

**Expected observation:** startup produces exactly 100 for all six values; one successful commit applies exactly one `-10` Oxygen modification.

**Most likely failure:** values remain zero, defaults stack on possession, or Oxygen changes by 20.

**Cause it signals:** effect class not configured/applied, no startup guard, or both manual subtraction and Commit cost are active.

**Counter-move:** inspect the outgoing effect spec and active startup guard; remove all direct Oxygen writes from the ability; apply default effect only on authority and exactly once per persistent PlayerState ASC.

**Fork trigger:** prediction correction or duplicate cost in two-player tests selects Fork E.

### Move 7 — Add the base and test abilities

**Action:** Add an exported project `UGameplayAbility` base. Add `UGameProjectSummer26TestOxygenPulseAbility`, set `InstancingPolicy = InstancedPerActor`, initially set `NetExecutionPolicy = LocalPredicted`, assign the Oxygen cost effect, and call `SetAssetTags` with `Ability.Test.OxygenPulse`. In `ActivateAbility`, call `CommitAbility`; on failure call `EndAbility(..., true, true)` without changing Oxygen; on success emit concise authority/local-role diagnostic evidence and call `EndAbility(..., true, false)` exactly once. Add no cooldown unless explicitly needed to prevent a proven bug.

**Expected observation:** an eligible press starts and ends one ability; successful commit changes Oxygen 100→90; insufficient Oxygen rejects commit and still ends cleanly.

**Most likely failure:** ability remains active, cost bypasses commit, activation occurs twice, or client/server diverge.

**Cause it signals:** missing EndAbility path, direct mutation, duplicated input/grant, or unsafe prediction behavior.

**Counter-move:** instrument activation handle/prediction key/role and all exits; enforce one commit and one end per activation. Use Fork E if prediction is the cause rather than masking it.

**Fork trigger:** clean prediction tests keep Local Predicted; rollback/correction/duplication selects Server Initiated under Fork E.

### Move 8 — Add the PlayerState owner

**Action:** Create exported `AGameProjectSummer26PlayerState : APlayerState, IAbilitySystemInterface`. In its constructor, create exactly one default-subobject project ASC, enable replication, choose `Mixed` replication mode for this player-owned ASC, and create exactly one AttributeSet default subobject. Return the ASC from the interface. Store server-only guards for startup defaults and startup abilities. Configure default effect and startup ability classes without requiring a new Blueprint PlayerState.

**Expected observation:** every player has one persistent ASC whose OwnerActor is its PlayerState; AttributeSet is registered through ownership; no Pawn owns another ASC.

**Most likely failure:** null ASC on client, two ASCs, or Mixed-mode ownership warning.

**Cause it signals:** component not a default subobject/replicated, Character also created an ASC, or PlayerState ownership/controller chain is wrong.

**Counter-move:** inspect actor/component ownership on both peers; delete the duplicate creator; verify the PlayerState owner is ultimately owned by its Controller before retaining Mixed. If that invariant cannot be proved, temporarily use Full replication for correctness and record the reason.

**Fork trigger:** runtime-proven persistent PlayerState selects this route; a project rule that requires Pawn-owned state selects Fork B Route B before further wiring.

### Move 9 — Initialize actor info and duplicate-safe startup state

**Action:** Add one idempotent initialization method called after possession. On server and client call `InitAbilityActorInfo(PlayerState, Character)` when both exist. Only on authority, apply default attributes once and grant startup abilities once. For each granted ability, scan existing specs for the class/source before `GiveAbility`; create the spec with level 1 and add `Input.Ability.Test` via `GetDynamicSpecSourceTags()`. Never clear and re-grant on normal re-possession.

**Expected observation:** owner is PlayerState, avatar is current Character on both peers; one test spec exists per player; attributes start at 100 once.

**Most likely failure:** avatar is null/stale, specs duplicate, or attributes reset to 100 after possession refresh.

**Cause it signals:** initialization before PlayerState replication, startup work running client-side, or guard scoped to the Pawn instead of persistent owner.

**Counter-move:** defer until both actors exist; separate actor-info refresh from authority-only startup; keep guards on PlayerState/ASC; count matching specs/effects in diagnostics.

**Fork trigger:** more than one spec/effect selects Abort 6 until duplicate ownership/grant source is found.

### Move 10 — Wire the active GameMode and Character

**Action:** Set `PlayerStateClass = AGameProjectSummer26PlayerState::StaticClass()` in the runtime-proven GameMode constructor unless an active Blueprint override is proven and intentionally updated. Make the active Character implement `IAbilitySystemInterface` by forwarding to its PlayerState ASC. Override `PossessedBy` for server initialization and `OnRep_PlayerState` for owning-client initialization. Do not create an ASC or AttributeSet on Character.

**Expected observation:** normal ThirdPerson startup creates the custom PlayerState; both callbacks converge on identical actor info; Character interface returns the PlayerState's one ASC.

**Most likely failure:** default APlayerState still spawns, client interface is temporarily null, or Character owns a second component.

**Cause it signals:** Blueprint GameMode override, wrong active class, expected replication timing, or copied Pawn-owned GAS pattern.

**Counter-move:** re-check runtime class names/Class Defaults; tolerate null only before OnRep and guard calls; remove any Character component; update only the proven active PlayerState setting.

**Fork trigger:** active-class mismatch selects Fork C; persistence infeasibility selects Fork B Route B.

### Move 11 — Wire tag input without Character Tick

**Action:** Create `IA_TestOxygenPulse`, map the proven free key in the runtime-active context, expose/assign it on `BP_ThirdPersonCharacter`, and bind Started to `AbilityInputTagPressed(Input.Ability.Test)` and Completed/Canceled to release. Override active PlayerController `PostProcessInput`, call `Super`, obtain the possessed Pawn's ASC through `IAbilitySystemInterface`, and call `ProcessAbilityInput(DeltaTime, bGamePaused)` only for the local controller.

**Expected observation:** each physical press produces one press frame, held state persists only until release, and one activation attempt occurs for the locally controlled Pawn on host and remote client.

**Most likely failure:** host works but remote does not, activation repeats while held, or no action reaches Character.

**Cause it signals:** mapping context absent on remote local player, wrong action assigned in Blueprint, `Triggered` used instead of `Started`, missing release bindings, or processing performed by non-local controllers.

**Counter-move:** use Enhanced Input debugger/log evidence per window; verify context/action assignment; retain Started/Completed/Canceled; log tag, controller role, and spec handle at bridge boundaries.

**Fork trigger:** existing tag router selects Fork D Route A; otherwise retain Route B minimal bridge.

### Move 12 — Compile in controlled increments

**Action:** Close Unreal Editor before each C++ build. Build after dependency/tags, core GAS types, ownership wiring, and input wiring. Use the exact VS Code/UBT command; save each full log. Treat Live Coding as insufficient evidence for reflected layout changes.

**Expected observation:** UBT, UHT, link, and module load succeed with no new warnings attributable to GAS.

**Most likely failure:** generated-code/link errors, stale editor DLL lock, or API mismatch copied from an older tutorial.

**Cause it signals:** malformed UPROPERTY/UFUNCTION signatures, missing API export/dependency, editor still running, or outdated API.

**Counter-move:** fix the smallest failing move using the first causal error, compare installed UE 5.8 headers, rebuild from the same command. Never delete broad project data or user changes.

**Fork trigger:** inability to restore the last known build selects Abort 1/4 depending on cause.

### Move 13 — Validate editor assets and defaults

**Action:** After a successful closed-editor build, open the project once. Confirm tags, classes, PlayerState selection, action assignment, mapping, GE modifiers, and Blueprint compilation. Save only intentional assets. Reopen once to prove serialization.

**Expected observation:** no missing class/tag pins, no dirty unrelated assets, and settings survive reopen.

**Most likely failure:** Blueprint parent missing, action property unassigned, context edit unsaved, or redirect-related Blueprint compile error.

**Cause it signals:** native load failure, wrong active Blueprint, incomplete asset wiring, or stale serialized class path.

**Counter-move:** stop PIE, inspect Output Log and asset references, correct only the intended assets, rebuild native code if class load failed, then reopen and revalidate.

**Fork trigger:** required Blueprint corruption or unrelated mass resave selects Abort 7.

### Move 14 — Run single-player functional proof

**Action:** In ThirdPerson Standalone, record PlayerState/ASC owner/avatar, spec count, all six attributes, selected key, activation start/commit/end, and final values. Press once, release, then press repeatedly until Oxygen reaches zero and attempt once more.

**Expected observation:** initial six values are 100; first accepted press produces exactly 100→90; later accepted presses each subtract 10; at zero the next commit fails, Oxygen remains zero, and the ability ends cancelled.

**Most likely failure:** no activation, double subtraction, negative Oxygen, or stuck active ability.

**Cause it signals:** input/spec tag mismatch, duplicate activation/cost, missing clamp, or lifecycle defect.

**Counter-move:** compare tag/spec/activation/effect evidence in order; fix the first boundary where expected evidence disappears. Do not compensate with arbitrary delays or cooldowns.

**Fork trigger:** single-player failure blocks multiplayer and selects Abort 5 until fixed.

### Move 15 — Run two-player Listen Server proof

**Action:** Configure Play As Listen Server, two players, normal ThirdPerson startup, and independently controlled windows. Record host and remote PlayerState/ASC/spec counts. Activate once from host only, reset by restarting the session, then activate once from remote only. Observe authoritative and owning-client Oxygen after prediction settles; also observe the non-owner where replication visibility permits. Repeat rapidly enough to reveal duplicate/correction faults.

**Expected observation:** each player begins at 100; host input changes only host Oxygen to 90; remote input changes only remote Oxygen to 90; server authority agrees; one press equals one committed cost; no permanent correction, cross-player activation, or duplicate grant occurs.

**Most likely failure:** remote input is ignored, remote shows 90 then snaps incorrectly, both players change, or host has multiple specs.

**Cause it signals:** actor-info timing, mapping context locality, prediction incompatibility, wrong Owner/Avatar, global/shared handles, or duplicate startup.

**Counter-move:** distinguish input, activation RPC, authority commit, and attribute replication with roles/prediction keys. Fix ownership/locality first. If only Local Predicted execution is unstable while Server Initiated is correct, take Fork E and document the latency tradeoff.

**Fork trigger:** prediction-only mismatch selects Fork E Route B; ownership or duplicate-ASC evidence selects Abort 3/6.

### Move 16 — Audit, diff, and hand off evidence

**Action:** Run the final clean editor build, reopen test, complete V1–V18, inspect all changed files/assets against the frozen manifest, and list every intentional delta. Search for duplicate ASCs, direct Oxygen writes, client-side `GiveAbility`, deprecated tag APIs, and missing EndAbility paths. Report pre-existing errors separately.

**Expected observation:** scope matches the planned surface; all mandatory gates pass; no unrelated files changed; evidence supports every claimed result.

**Most likely failure:** untracked asset resaves, undocumented workaround, incomplete multiplayer evidence, or a claim based only on expected behavior.

**Cause it signals:** editor auto-save/resave, scope drift, test omission, or reporting discipline failure.

**Counter-move:** revert/recover unrelated deltas individually, rerun affected tests, and downgrade unsupported claims to `NOT RUN` or `BLOCKED`. Never call the mission complete with a mandatory FAIL/BLOCKED result.

**Fork trigger:** any unresolved mandatory verification selects the corresponding abort or an incomplete handoff, never a success declaration.

## Mandatory forks

### Fork A — Existing GAS

- **Route A, existing GAS found:** stop creation of parallel base types. Inventory ASC count/owner, sets, effects, grants, actor-info calls, input router, and writers; extend the authoritative path and migrate only the Oxygen slice.
- **Route B, no GAS found:** recon selected this route. Create the planned single foundation.
- **Trigger:** any post-recon evidence of reflected/native GAS types or live GAS assets reopens this fork.

### Fork B — ASC ownership

- **Route A, PlayerState-owned:** selected. OwnerActor is project PlayerState; avatar is current Character; initialization in `PossessedBy` and `OnRep_PlayerState`; startup guards persist with PlayerState.
- **Route B, Pawn-owned:** permitted only if runtime project rules prove PlayerState ownership impossible or unwanted and no Pawn replacement persistence is required. Then Character owns exactly one ASC/set and reinitializes on possession; document lost-state behavior.
- **Trigger:** actual respawn/persistence requirements and active class ownership, not tutorial preference.

### Fork C — Active gameplay class mismatch

- **Route A, inferred chain confirmed:** modify the listed GameMode, PlayerState, Character, and PlayerController.
- **Route B, override differs:** trace map World Settings → GameMode → default classes and edit the runtime-active types/Blueprint defaults only.
- **Trigger:** logged runtime class names differ from R4.

### Fork D — Input architecture

- **Route A, existing tag router found:** register `Input.Ability.Test` and feed its established pressed/held/released path.
- **Route B, direct Enhanced Input only:** selected. Add the minimal Character-to-ASC bridge and PlayerController `PostProcessInput` processing.
- **Trigger:** live input architecture discovered during editor recon.

### Fork E — Network execution policy

- **Route A, Local Predicted:** retain only when V14–V17 show one cost, clean reconciliation, and identical settled authority/owner values.
- **Route B, Server Initiated:** use if prediction produces a reproducible correction, double effect, or unsupported behavior while server initiation is correct. Record added perceived latency and the failing Local Predicted evidence.
- **Trigger:** test evidence, never preference.

## Abort conditions

1. Fresh untouched project does not compile or launch, and the failure cannot be cleanly isolated as pre-existing.
2. Untouched two-player normal startup cannot spawn and independently control host and remote client.
3. More than one ASC is found or ownership cannot be determined unambiguously.
4. GameplayAbilities plugin/modules are unavailable or the associated UE 5.8 toolchain cannot build them.
5. Single-player 100→90 exact-cost/lifecycle proof fails after the responsible move is isolated.
6. Startup abilities/effects duplicate, or `GiveAbility`/startup effects execute without server authority.
7. Completion would require repairing unrelated Game Animation Sample assets, mass-resaving content, bypassing normal startup, or overwriting inseparable user changes.
8. Client/server authoritative Oxygen cannot be made to converge under either Local Predicted or Server Initiated policy.
9. A required claim has no obtainable test evidence and is material to multiplayer correctness.

On abort: stop new edits, preserve logs and recovery baseline, identify the first failed invariant, list files changed so far, and state the smallest safe next investigation. Do not claim partial implementation as verified foundation.

## Verification matrix

Each row requires `Result`, `Evidence`, and `Notes`. Mandatory means a `NOT RUN`, `FAIL`, or `BLOCKED` result prevents completion unless the row explicitly permits NOT RUN.

| ID | Verification | Exact pass criterion | Required evidence |
|---|---|---|---|
| V1 | Recovery/change baseline | Intended pre-change files are recoverable and overlapping edits identified | VCS status/commit or snapshot path plus SHA-256 manifest |
| V2 | Fresh pre-change build | `GameProjectSummer26Editor Win64 Development` exits 0 before GAS edits | full timestamped UBT log and exit code |
| V3 | Active chain | Runtime GameMode, PC, PS, Pawn and map are recorded | server/client log or screenshot with exact class paths |
| V4 | Untouched two-player startup | Host and remote spawn and accept independent movement through normal ThirdPerson flow | PIE settings plus two-window observation/video/log |
| V5 | Plugin/module integration | GameplayAbilities, GameplayTags, GameplayTasks, EnhancedInput resolve and target builds | `.uproject`/Build.cs diff and successful staged build |
| V6 | One ASC invariant | Exactly one ASC per player; Owner=PlayerState, Avatar=current Character on server and owner | role-labeled diagnostic output for both players |
| V7 | Attribute construction/replication | Six `FGameplayAttributeData` fields exist, replicate with notify, and use accessor macros | code inspection plus UHT/build evidence |
| V8 | Defaults and clamp | All six start at 100; current values remain `[0,max]`; maxima cannot become negative | runtime dumps and boundary test output |
| V9 | Authority-only duplicate-safe grant | Exactly one test spec and one startup default application per PlayerState | authority/client role logs and spec/effect counts |
| V10 | Re-possession lifecycle | If an existing active-chain re-possession path exists, avatar refreshes without reset/duplicate | runtime before/after values/counts; otherwise `NOT RUN — no existing active-chain re-possession path`, which is permitted |
| V11 | Native tags | Both exact tags resolve; ability uses `SetAssetTags`; spec uses `GetDynamicSpecSourceTags()` | tag browser/runtime lookup plus code inspection |
| V12 | Input bridge | Started/held/released are processed once per frame without Character Tick or stuck handles | Enhanced Input/ASC diagnostics on press-hold-release |
| V13 | Single-player exact test | First accepted action changes Oxygen 100→90 exactly once and ability ends | before/after value plus activation/commit/end IDs |
| V14 | Host multiplayer test | Host-only press changes host authoritative Oxygen 100→90, not remote player's | server and both clients' role/player-labeled values |
| V15 | Remote multiplayer test | Remote-only press changes remote authoritative Oxygen 100→90, not host player's | server and both clients' role/player-labeled values |
| V16 | Prediction convergence | Settled server and owning-client Oxygen agree with one cost and no permanent correction | timestamped/prediction-key logs after settlement |
| V17 | Repeated/insufficient-cost behavior | Each accepted press costs exactly 10; at zero next commit fails, value stays zero, ability ends | sequence 100..0, failed attempt, lifecycle logs |
| V18 | Final scope/build/reopen | Final clean build exits 0; editor reopens; intentional delta only; no new attributable errors | final UBT/runtime logs, changed-file list, asset reopen check |

## Required implementation invariants

- Exactly one ASC exists per player.
- Server grants abilities and applies startup defaults; clients do neither.
- OwnerActor is PlayerState and AvatarActor is the current Character on both relevant peers.
- Attribute initialization comes from a GameplayEffect, not constructor values alone.
- Ability cost comes from `CommitAbility` and its cost GameplayEffect; there is no direct Oxygen subtraction.
- Every ability activation path ends.
- All six attributes replicate and clamp; no plain duplicate stat is writable.
- Input ability identity is a gameplay tag, not a numeric InputID contract.
- No existing input or Game Animation Sample behavior is silently replaced.
- A result is claimed only after the matching verification row has actual evidence.

## Completion and reporting standard

The executor's final report must contain:

1. exact engine/build command and exit result;
2. complete intentional changed-file/asset list;
3. selected route for Forks A–E and the observation that selected each;
4. ASC Owner/Avatar and duplicate counts for host and remote;
5. V1–V18 table filled with `PASS`, `FAIL`, `NOT RUN`, or `BLOCKED`, with evidence paths/excerpts;
6. exact host and remote Oxygen sequences, including 100→90;
7. prediction policy retained and why;
8. pre-existing errors kept separate from new regressions;
9. untested items and residual risks;
10. no statement such as “working,” “replicated,” “verified,” or “complete” unless its mandatory verification evidence is present.

Mission success requires V1–V9 and V11–V18 to pass, with V10 either passing or using its explicitly permitted `NOT RUN` reason. Any other missing mandatory result means the correct status is incomplete or aborted.
