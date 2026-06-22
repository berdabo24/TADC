# State of Code — TCG6223 Fixed-Function OpenGL Game Project

## Technical Onboarding Guide & Architecture Documentation

This document summarizes the final state of the TCG6223 Computer Graphics project source code. The project is a completed 3D third-person OpenGL battle game set inside a circus-themed arena inspired by *The Amazing Digital Circus*.

---

## 1. High-Level Project Overview

The project is a 3D battle game developed using:

* C++
* OpenGL fixed-function pipeline
* GLUT/freeglut
* Code::Blocks
* MinGW
* WinMM audio functions

The player controls Kinger in a third-person view and fights against Caine inside an enclosed circus battle arena. The game includes player movement, shooting, rolling, healing, enemy hazards, boss attacks, UI states, audio, collision, and win/lose conditions.

---

## 2. Main Gameplay Loop

The main gameplay loop is handled through GLUT callbacks in `CNAmain.cpp`.

### Main Flow

1. `main()` starts GLUT.
2. `myInit()` creates the OpenGL window and initializes rendering settings.
3. `myvirtualworld.init()` loads models, textures, lighting, and audio.
4. `myDisplayFunc()` runs repeatedly to:

   * Clear the frame.
   * Update camera position.
   * Check game state.
   * Draw the world.
   * Draw HUD and UI.
   * Update input states.
   * Call `tickTime()` for gameplay updates.
5. `glutMainLoop()` keeps the program running.

---

## 3. Main Source Files

```text
CNAmain.cpp / CNAmain.hpp
```

Handles the main program, GLUT callbacks, window creation, input, camera, HUD, menus, pause screen, death screen, win screen, and state transitions.

```text
CNAWorld.cpp / CNAWorld.hpp
```

Owns the main game objects, including Kinger, Caine, Gloinks, Butterfly, Kinger Roll, Environment, and AudioManager. It handles model loading, texture loading, lighting setup, world drawing, and update calls.

```text
ObjModel.cpp / ObjModel.hpp
```

Loads custom text-based model files and renders the models using OpenGL.

```text
TextureLoader.cpp / TextureLoader.hpp
stb_image.h
```

Loads PNG/JPG texture images into OpenGL texture IDs.

```text
Environment.cpp / Environment.hpp
```

Handles the skybox, ground, castle wall, cubes, grouped cubes, irregular cube props, pillars, roof, floating spheres, meteor hazard, collision, lighting zones, environment animation, and horizontal glitch sweeping visual effect.

```text
Kinger.cpp / Kinger.hpp
KingerAnimation.cpp / KingerAnimation.hpp
```

Handles Kinger model rendering, texture binding, player movement, jump, shooting, reload, dodge roll, heal, bullet update, hit detection, damage, and death animation.

```text
Caine.cpp / Caine.hpp
CaineAnimation.cpp / CaineAnimation.hpp
```

Handles Caine model rendering, boss animation, phase transition, projectiles, teleport/clone effects, sweep attack, attack patterns, health, hurt effect, and death sequence.

```text
Gloinks.cpp / Gloinks.hpp
GloinksAnimation.cpp / GloinksAnimation.hpp
```

Handles Gloinks model drawing, spawning, movement, collision/hit response, damage state, and death behaviour.

```text
Butterfly.cpp / Butterfly.hpp
KingerRoll.cpp / KingerRoll.hpp
```

Handles supporting models used for healing and dodge roll effects.

```text
AudioManager.cpp / AudioManager.hpp
```

Handles background music and sound effects using WinMM.

---

## 4. Game State System

The game uses the following UI states:

```cpp
enum GameUIState
{
    START_MENU,
    GAMEPLAY,
    PAUSE_MENU,
    DEATH_SCREEN,
    WIN_SCREEN
};
```

### START_MENU

Displays the title menu and allows users to start the game, enter debug environment, enter test arena, toggle difficulty, or exit.

### GAMEPLAY

Runs the main battle. Kinger can move, shoot, jump, roll, reload, and heal. Caine and Gloinks update during this state.

### PAUSE_MENU

Allows the player to restart, return to the main menu, or resume.

### DEATH_SCREEN

Shown when Kinger dies.

### WIN_SCREEN

Shown after Caine is defeated and the win sequence completes. A win sound effect plays when this state is triggered.

---

## 5. Player System — Kinger

Kinger is the playable character. The player controls Kinger using keyboard and mouse input.

### Kinger Features

* Third-person movement
* Camera-relative movement
* Jumping
* Shooting
* Reloading
* Dodge roll
* Healing with butterfly skill
* Hurt reaction
* Death animation
* Health and ammo display
* Bullet and hit detection
* Shotgun sound effect when firing

### Main Controls

```text
W / A / S / D       Move
Mouse movement     Rotate camera
Left mouse button  Shoot
SPACE              Jump
R                  Reload
C                  Dodge roll
F                  Heal
Z / ESC            Pause
```

---

## 6. Boss System — Caine

Caine is the main boss character.

### Caine Features

* Idle hover animation
* Jaw movement
* Shooting attack
* Phase-based behaviour
* Sweep attack
* Projectile attacks
* Teleport/clone effects
* Meteor-related attack mode
* Hurt reaction
* Death sequence
* Win trigger after defeat

Caine is updated using time-based logic and state variables, allowing different attacks and animations to activate depending on the game state and phase.

---

## 7. Gloinks System

Gloinks act as supporting enemy/hazard objects.

### Gloinks Features

* Multiple geometric body shapes
* Animated movement
* Spawning logic
* Hit response
* Damage state
* Death/falling behaviour
* Interaction with Kinger during gameplay

---

## 8. Environment System

The battle environment is an enclosed circus arena.

### Environment Components

* Skybox
* Checkerboard ground
* Castle walls
* Cubes
* Grouped cubes
* Irregular cube props
* Pillars
* Roof structure
* Floating spheres
* Meteor hazard
* Horizontal glitch sweeping hazard
* Collision-based obstacles

### Environment Rendering

The environment uses OpenGL transformations such as:

* `glTranslatef()`
* `glScalef()`
* `glRotatef()`
* `glPushMatrix()`
* `glPopMatrix()`

These transformations position, scale, and animate each object in the world.

### Environment Animation

Time-based animation is used so that movement remains consistent across different frame rates. Animated elements include:

* Moving cubes
* Floating spheres
* Rotating roof
* Meteor effects
* Horizontal glitch sweeping hazard

### Collision

Collision logic prevents Kinger from walking through selected environment objects and helps define the playable area.

---

## 9. Camera and Input System

The game uses a third-person over-the-shoulder camera.

### Camera Features

* Camera follows Kinger
* Mouse controls yaw and pitch
* Camera uses `gluLookAt()`
* Camera is clamped within the arena boundary
* Camera height follows Kinger smoothly
* Screen shake is applied during selected effects

Keyboard states are stored using the `keyStates` array. Mouse movement is handled through GLUT passive motion callbacks.

---

## 10. UI and HUD

The game includes several UI elements.

### HUD

* Kinger health bar
* Ammo display
* Heal charge display
* Caine boss health bar
* Crosshair
* Control tips
* Sweep attack warning indicator
* Test arena control guide

### Menus and Screens

* Start menu
* Pause menu
* Death screen
* Win screen

The UI is drawn using 2D orthographic projection after the 3D scene is rendered.

---

## 11. Audio System

Audio is handled by `AudioManager`.

### Background Music

Background music uses WinMM MCI command through `mciSendString()`.

The working setup uses an MCI alias for BGM playback.

### Sound Effects

Sound effects use `PlaySound()` with asynchronous playback.

Current sound effects include:

* Shotgun shooting sound
* Win sound effect

### Linker Requirement

The project must link with:

```text
-lwinmm
```

This is required because both `mciSendString()` and `PlaySound()` are part of the Windows Multimedia library.

---

## 12. Build and Runtime Requirements

### Required Environment

* Windows 10 or later
* Code::Blocks
* MinGW compiler
* OpenGL
* GLU
* GLUT/freeglut
* WinMM linker support

### Required Runtime Files

The executable must be placed together with:

```text
Model/
Audio/
libfreeglut.dll
glut32.dll
libgcc_s_seh-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```

The exact DLLs may depend on the local MinGW/freeglut setup.

---

## 13. Current Completion Status

### Completed

* Kinger player model and animation
* Caine boss model and animation
* Gloinks model and animation
* Butterfly and Kinger Roll supporting models
* Environment model loading
* Texture mapping
* Global and local lighting
* Time-based animation
* Player controls
* Camera system
* Shooting, reload, jump, roll, and heal
* Caine boss battle logic
* Gloinks interaction
* Environment collision
* Horizontal glitch sweeping hazard
* Meteor hazard
* UI and HUD
* Start menu
* Pause menu
* Death screen
* Win screen
* Background music
* Shotgun sound effect
* Win sound effect
* Direct executable runtime support

### Final Status

The project is complete and ready for demonstration and submission.

---

## 14. Important Notes for Future Developers

* Keep `Model` and `Audio` beside the `.exe`.
* Do not remove `stb_image.h`.
* Do not use absolute file paths.
* Keep file paths relative for portability.
* Always rebuild after changing source files.
* Keep `-lwinmm` in linker settings for audio.
* Avoid placing audio triggers inside drawing functions because drawing functions run every frame.
* Trigger one-time sounds during state changes, such as when switching to `WIN_SCREEN`.
* Use the same MinGW/freeglut DLL files that match the build environment.
