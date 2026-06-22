# TCG6223 Computer Graphics Project

## The Amazing Digital Circus: Blind-Box Battle Arena

This repository contains the completed source code for the TCG6223 Computer Graphics group project. The project is a 3D OpenGL battle game inspired by *The Amazing Digital Circus*, where the player controls Kinger and fights against Caine inside an enclosed circus battle arena.

The game is developed using C++, OpenGL, GLUT/freeglut, Code::Blocks, MinGW, and WinMM audio functions.

---

## Project Overview

The project implements a 3D blind-box battle game with:

* Third-person player control
* Kinger as the playable character
* Caine as the boss character
* Gloinks as supporting enemy/hazard objects
* Animated circus battle arena
* Textured 3D models loaded from text-based model files
* Lighting, shading, texture mapping, animation, collision, hit detection, and game UI
* Background music and sound effects
* Start menu, pause menu, win screen, and death screen

---

## Main Features

### Character System

* Kinger player model with separated body parts
* Caine boss model with separated parts and animation
* Gloinks enemy/hazard models
* Butterfly and Kinger Roll supporting models
* Custom model loading using `.txt` model data
* Texture loading using `stb_image.h`

### Gameplay

* Move Kinger using keyboard controls
* Aim using mouse movement
* Shoot using left mouse button
* Reload weapon
* Jump
* Dodge roll
* Heal using butterfly skill
* Fight Caine through multiple attack patterns
* Avoid Gloinks, sweep attacks, meteors, and other hazards
* Win condition and death condition included

### Environment

* Enclosed circus arena
* Skybox
* Checkerboard ground
* Castle walls
* Cubes, grouped cubes, irregular cubes, pillars, roof, and floating spheres
* Texture mapping for circus theme
* Global and local lighting
* Time-based animation for moving objects
* Collision-based obstacles
* Horizontal glitch sweeping hazard

### Audio

* Background music using WinMM MCI command
* Sound effects using WinMM `PlaySound()`
* Shotgun shooting sound
* Win sound effect
* Required linker option: `-lwinmm`

---

## Controls

### Main Menu

| Key | Action            |
| --- | ----------------- |
| `1` | Start Game        |
| `2` | Debug Environment |
| `3` | Test Arena        |
| `4` | Toggle Difficulty |
| `0` | Exit Game         |

### Gameplay

| Key / Input        | Action                     |
| ------------------ | -------------------------- |
| `W`, `A`, `S`, `D` | Move Kinger                |
| Mouse Movement     | Rotate camera              |
| Left Mouse Button  | Shoot                      |
| `SPACE`            | Jump                       |
| `R`                | Reload                     |
| `C`                | Dodge roll                 |
| `F`                | Heal skill                 |
| `Z` or `ESC`       | Pause / Resume             |
| `B`                | Toggle hitbox display      |
| Arrow Keys         | Camera adjustment          |
| `F1`               | Toggle shading / wireframe |
| `F2`               | Toggle axis rendering      |
| `F3`               | Toggle lighting            |

---

## Project Structure

```text
TCG6223-Project-Source-Code/
│
├── CNAmain.cpp / CNAmain.hpp
│   └── Main program, GLUT window, input handling, camera, UI, and game state.
│
├── CNAWorld.cpp / CNAWorld.hpp
│   └── Main world controller for loading models, textures, lighting, audio, and updates.
│
├── ObjModel.cpp / ObjModel.hpp
│   └── Custom text-based model loader and OpenGL drawing logic.
│
├── TextureLoader.cpp / TextureLoader.hpp
│   └── Texture loading using stb_image.
│
├── Environment.cpp / Environment.hpp
│   └── Skybox, ground, walls, props, lighting, animations, collision, meteors, and glitch hazard.
│
├── Kinger.cpp / Kinger.hpp
├── KingerAnimation.cpp / KingerAnimation.hpp
│   └── Kinger model drawing, movement, shooting, jumping, rolling, reloading, healing, hurt, and death.
│
├── Caine.cpp / Caine.hpp
├── CaineAnimation.cpp / CaineAnimation.hpp
│   └── Caine boss model, AI, attacks, phase changes, projectiles, sweep hazard, clones, and death sequence.
│
├── Gloinks.cpp / Gloinks.hpp
├── GloinksAnimation.cpp / GloinksAnimation.hpp
│   └── Gloinks model drawing, spawning, movement, hit detection, and death behaviour.
│
├── Butterfly.cpp / Butterfly.hpp
├── KingerRoll.cpp / KingerRoll.hpp
│   └── Supporting models for healing and dodge roll.
│
├── AudioManager.cpp / AudioManager.hpp
│   └── Background music and sound effect playback using WinMM.
│
├── Model/
│   └── Text model files and texture images.
│
├── Audio/
│   ├── BGM/
│   └── SFX/
│
├── bin/
│   └── Debug/
│       └── Compiled executable and required runtime files.
│
├── TCG6223-Project.cbp
│   └── Code::Blocks project file.
│
└── state-of-code.md
    └── Technical architecture and code status documentation.
```

---

## How to Compile and Run in Code::Blocks

1. Download or clone this repository.
2. Open Code::Blocks.
3. Open the project file:

```text
TCG6223-Project.cbp
```

4. Make sure the compiler is MinGW.

5. Check the linker settings:

```text
Project → Build options...
```

Then click the **project file name** on the left side, for example:

```text
TCG6223-Project
```

Do not select only `Debug` or `Release`, because the linker settings should apply to the whole project.

After selecting the project name, click:

```text
Linker settings
```

Under **Other linker options**, make sure these options are included:

```text
-lopengl32
-lglu32
-lglut32
-lwinmm
```

Depending on the local GLUT/freeglut setup, the GLUT option may also be:

```text
-lfreeglut
```

6. Rebuild the project:

```text
Build → Rebuild
```

7. Run the program:

```text
Run
```

---

## How to Run by Double-Clicking the EXE

The game can also be run directly by double-clicking the compiled `.exe` file.

Make sure the `.exe`, required DLL files, `Model` folder, and `Audio` folder are placed together.

Example:

```text
TCG6223_Game/
│
├── TCG6223-Project.exe
├── libfreeglut.dll
├── glut32.dll
├── libgcc_s_seh-1.dll
├── libstdc++-6.dll
├── libwinpthread-1.dll
│
├── Model/
│   ├── Kinger/
│   ├── Caine/
│   ├── Gloinks/
│   ├── Environment/
│   └── ...
│
└── Audio/
    ├── BGM/
    └── SFX/
```

Important:
The `Model` and `Audio` folders must be in the same folder level as the `.exe`, because the program uses relative file paths.

---

## Windows SmartScreen Warning

When running the `.exe`, Windows may show this warning:

```text
Windows protected your PC
Microsoft Defender SmartScreen prevented an unrecognized app from starting.
```

This can happen because the executable is newly built and not recognized by Windows yet.

### Option 1: Run Anyway

If you trust the project files:

1. Click `More info`.
2. Click `Run anyway`.

### Option 2: Unblock the EXE File

If Windows keeps warning you:

1. Right-click the `.exe` file.
2. Select `Properties`.
3. Under the `General` tab, look for:

```text
Security: This file came from another computer and might be blocked to help protect this computer.
```

4. Check `Unblock`.
5. Click `Apply`.
6. Click `OK`.
7. Run the program again.

---

## Common Runtime Issues

### Missing DLL Files

If the game runs inside Code::Blocks but does not run by double-clicking the `.exe`, it usually means some runtime DLL files are missing beside the executable.

Common required DLL files include:

```text
libfreeglut.dll
glut32.dll
libgcc_s_seh-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```

The DLL files should be placed in the same folder as the `.exe`.

Example:

```text
TCG6223_Game/
│
├── TCG6223-Project.exe
├── libfreeglut.dll
├── glut32.dll
├── libgcc_s_seh-1.dll
├── libstdc++-6.dll
├── libwinpthread-1.dll
├── Model/
└── Audio/
```

#### Where to Find the DLL Files

For MinGW runtime DLLs, check the Code::Blocks MinGW folder:

```text
C:\Program Files (x86)\CodeBlocks\MinGW\bin
```

Look for:

```text
libgcc_s_seh-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```

Some MinGW versions may use a different GCC runtime DLL, such as:

```text
libgcc_s_dw2-1.dll
```

Use the DLL files that match the MinGW compiler used to build the project.

#### Where to Find `libfreeglut.dll`

If `libfreeglut.dll` is missing, download the FreeGLUT MinGW package from:

```text
https://www.songho.ca/opengl/gl_freeglut.html
```

Basic steps:

1. Download the FreeGLUT package for MinGW.
2. Extract the downloaded file.
3. Open the extracted `bin` folder.
4. Copy:

```text
libfreeglut.dll
```

5. Paste it beside the game `.exe`.

Do not use random DLL download websites if possible. It is safer to use the FreeGLUT package or the same DLL file from the computer that successfully built the project.

---

### Model Files Cannot Load

If the console shows:

```text
Failed to open model file
```

then the `Model` folder is not beside the `.exe`.

Fix the folder structure:

```text
TCG6223-Project.exe
Model/
Audio/
```

The program uses relative paths, so the `Model` and `Audio` folders must follow the `.exe`.

---

### Audio Does Not Play

Make sure the `Audio` folder is beside the `.exe`.

Example:

```text
TCG6223-Project.exe
Audio/
├── BGM/
└── SFX/
```

Also make sure the linker contains:

```text
-lwinmm
```

The project uses:

* MCI command for background music
* `PlaySound()` for sound effects

---

## Notes for Developers

* Do not move the `Model` folder unless the file paths in the code are updated.
* Do not remove `stb_image.h`.
* Avoid using absolute paths such as `C:\Users\...` in source code.
* Keep all file paths relative so the project can run on other computers.
* Rebuild the project after modifying source files.
* Keep the `Audio` and `Model` folders with the executable when testing outside Code::Blocks.

---

## Final Status

The project is complete and ready for final demonstration and submission. It includes character models, environment models, texture mapping, lighting, animation, game controls, collision, battle logic, user interface, audio, win/death screens, and executable runtime support.
