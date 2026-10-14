# GASP Parkour & Gameplay Ability System

## Portfolio scope

This Unreal Engine 5.8 prototype extends the Game Animation Sample (GASP) with
project-specific native C++ movement and Gameplay Ability System (GAS) code.
The portfolio demonstration focuses on sprinting, sliding, dodging, stamina,
and the bridge between native traversal code and GASP vault/mantle actions.

The showcase map is intentionally owned by this project:

- Showcase map: `/Game/Portfolio/Parkour/Maps/Parkour_Showcase`
- Active pawn foundation: `BP_PlayerCharacter_Parkour`
- Native parent: `AGameProjectSummer26ParkourCharacter`
- Active GameMode: `GameMode_Parkour`

The original GASP map and sample assets remain unchanged. The showcase map is
a new blank level where presentation, course layout, lighting, signage, and
owned Blueprint overrides belong.

## Demonstrated systems

- Native C++ parkour movement component with a replicated slide movement mode.
- Slope-aware slide acceleration and custom network move serialization.
- GAS-backed endurance/stamina attributes, sprint cost, dodge cost, and
  regeneration effects.
- Traversal component that hands off to GASP traversal actions for vaulting
  and mantling.
- Automated `GameProjectSummer26.Parkour.Foundation` test covering the native
  parkour foundation and traversal bridge contract.

## Input ownership

The existing `BP_PlayerCharacter_Parkour` already owns Enhanced Input action
graphs for sprint, crouch, and jump. New native runtime mappings must not be
added in parallel, because that would cause duplicate input handling. Any
future input change is made one action at a time in an owned child Blueprint,
then tested in PIE.

## Showcase acceptance checklist

Before publishing this work to the portfolio, verify each item in a playable
build or PIE session:

- Sprint consumes endurance and stops when endurance is depleted.
- Slide starts from the intended input path and behaves correctly on a slope.
- Dodge consumes endurance and respects its recovery/cooldown behavior.
- Low, medium, and high traversal targets select appropriate GASP actions.
- The course can be completed from spawn to finish without a soft lock.
- The showcase map has clear lighting, readable route markers, and an
  on-screen explanation of the controls.
- Capture a short gameplay clip and at least three screenshots from the owned
  showcase map.

## Public repository boundary

The future public repository contains only project-owned material:

- Selected `Source/GameProjectSummer26` C++ files and tests.
- This documentation and architecture notes.
- Screenshots and a short gameplay video link.
- Setup instructions that require the official Game Animation Sample.

It must not redistribute GASP, Epic sample content, Marketplace content,
derived binaries, or the full project `Content` directory.
