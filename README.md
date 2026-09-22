# Automated Multi-Level Conveyor Sorting Factory

A complete modern OpenGL 3.3 demonstration of an automated parcel-sorting room. Reusable cube and cylinder meshes build the entire environment; no models or textures are required.

## What is implemented

- Elevated input conveyor and lower processing conveyor
- Gravity-driven, visibly rotating box drop between levels
- Pool of eight reusable parcels with varied colors and sizes
- Explicit box state machine: upper, falling, lower, scanning, gate approach, left/right exit, collected
- Scanner pause and blue scan feedback
- Automatically aligned sorting gate with a fixed hierarchical pivot
- Left and right exit conveyors and color-coded collection bins
- Rotating rollers and wall ventilation fan
- Operating/scanning/paused stack lights
- Phong ambient, diffuse, and specular lighting with inverse-transpose normal handling
- Toggleable attenuated point light and a weak directional fill light
- Factory room, rack, stored parcels, safety barrier, pipes, panel, supports, rails, and floor markings
- Delta-time movement, speed adjustment, reset, and continuous parcel reuse
- Native fullscreen launch, runtime fullscreen toggle, free-look mouse camera, and three demonstration presets

## Dependencies

- A C++17 compiler
- CMake 3.20+
- Git (used by CMake during the first configure)
- OpenGL-capable graphics driver

CMake downloads pinned versions of GLFW 3.3.9, GLM 0.9.9.8, and GLAD 2.0.6. They are placed in the build directory and do not modify the source tree.

## Build

### Windows (Visual Studio)

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\conveyor_factory.exe
```

### Windows (MinGW)

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build -j
.\build\conveyor_factory.exe
```

### Linux

Install the normal GLFW build prerequisites (X11/Wayland development packages), then:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/conveyor_factory
```

The shader directory is copied beside the executable automatically. The program also supports being run directly from the project root.

The application launches in native fullscreen. Pass `--windowed` when troubleshooting or when a windowed start is preferred; `F11` can switch modes at any time.

## Controls

| Input | Action |
|---|---|
| `Space` | Pause/resume the conveyor simulation |
| `+` / `-` | Increase/decrease conveyor speed |
| `R` | Reset boxes, gate, timers, and speed |
| `G` | Alternate the gate manually for demonstration |
| `L` | Toggle the main point light |
| `1` | Perspective overview |
| `2` | Top view |
| `3` | Side view of the level change |
| `W A S D` | Move camera |
| `Q / E` | Move camera down/up |
| Mouse | Look around |
| Left click | Recapture the mouse after releasing it |
| Wheel | Zoom |
| `Tab` | Capture/release the mouse cursor |
| `F11` | Toggle fullscreen/windowed mode |
| `Esc` | Exit |

## Sorting logic

Parcels receive alternating destinations when they are taken from the fixed-size pool. Blue, green, and brown parcels begin with a left destination; orange, yellow, and red parcels begin with a right destination. A parcel stops briefly at the scanner, which requests the correct gate angle. On approach it waits if necessary until the gate is within four degrees of its target. It then changes to the matching diagonal-exit state. Once it reaches a bin it remains briefly visible, becomes inactive, and is reused by a later spawn.

The gate model is built as `translate(pivot) * rotate(angle) * translate(child) * scale`, so its pivot never moves. Simulation updates and rendering are kept in separate methods.

## Source layout

```text
include/            Shader, camera, geometry, and scene declarations
src/                Application, rendering, and simulation implementation
shaders/            GLSL 330 Core Phong shaders
CMakeLists.txt      Reproducible build and dependency setup
```

## Graphics concepts demonstrated

Translation moves parcels and positions equipment; scaling reuses primitives for every factory object; rotation animates rollers, the fan, falling boxes, and the gate; hierarchical transforms preserve mechanical pivots; the camera supplies view transforms; perspective supplies projection; and the shaders implement Phong lighting for non-uniformly scaled objects.
