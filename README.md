# GASP Parkour Movement Showcase

An in-progress third-person parkour showcase built with **Unreal Engine 5.8**. The project explores responsive traversal, reusable character movement code and GASP-based animation integration in a purpose-built traversal course.

## Current features

- Ledge detection, hang state and climb-up transitions
- Wall-relative left/right shimmy
- Drop, wall-jump and slide movement
- Gameplay Ability System traversal and stamina experiments
- C++ movement, traversal and ability components with Blueprint-exposed tuning
- Parkour showcase maps and supporting UI

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
