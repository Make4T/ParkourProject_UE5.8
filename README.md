# GASP Parkour Movement Showcase

An in-progress third-person parkour showcase built with **Unreal Engine 5.8**. The project explores responsive traversal, reusable character movement code and GASP-based animation integration in a purpose-built traversal course.

## Current features

- Ledge detection, hang state and climb-up transitions
- Wall-relative left/right shimmy
- Drop, wall-jump and slide movement
- Gameplay Ability System traversal and stamina experiments
- C++ movement, traversal and ability components with Blueprint-exposed tuning
- Parkour showcase maps and supporting UI

## Showcase architecture

```text
Enhanced Input and Blueprint tuning
              │
              ▼
GAS traversal / sprint / dodge abilities
              │
              ▼
Native C++ character + movement + traversal components
              │
              ▼
GASP motion matching, vault and mantle actions
```

The project keeps gameplay rules in C++ components while exposing practical values to Blueprints for iteration. The dedicated `Parkour_Showcase` map demonstrates owned course layout, lighting and route guidance without modifying the original GASP sample maps.

## Portfolio focus

The technical focus is the connection between native traversal code, Gameplay Ability System state and GASP animation actions:

- Slope-aware slide movement with network move serialization
- Endurance/stamina attributes with sprint, slide and dodge costs
- Traversal handoff to GASP vault and mantle actions
- Ledge hang, directional shimmy, climb-up and controlled drop actions
- Purpose-built course and HUD for testing the movement loop

See [the detailed showcase notes](Documentation/Portfolio-Parkour-Showcase.md), [GAS integration notes](Documentation/GAS-Integration.md) and [the parkour foundation](Documentation/Parkour-Foundation.md) for implementation context.

## Technology

- Unreal Engine 5.8
- C++ and Blueprints
- Game Animation Sample Project (GASP)
- Motion Matching, Pose Search, Chooser, Mover and Motion Warping
- Gameplay Ability System

## Open the project

1. Install Unreal Engine **5.8** (the project was developed with 5.8.2).
2. Install the required GASP, MetaHuman and Locomotor content/dependencies through their original Unreal/Fab sources.
3. Open `GameProjectSummer26.uproject`, allow Unreal Engine to regenerate project files, then build the editor target if prompted.

## Repository scope

This repository intentionally tracks project source, configuration, documentation, maps and authored gameplay content. Generated Unreal folders (`Binaries`, `Intermediate`, `Saved` and `DerivedDataCache`) are excluded. The large third-party GASP, MetaHuman and Locomotor content folders are also excluded so the repository stays practical to clone; install them locally before opening the project.

## Validation approach

The repository includes native parkour foundation automation coverage. In the editor, the gameplay showcase should additionally be verified in Play-in-Editor: complete the course, confirm stamina costs and test traversal targets at multiple heights. The detailed showcase document contains the full acceptance checklist.
