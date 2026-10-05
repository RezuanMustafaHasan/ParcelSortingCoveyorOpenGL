# Automated Multi-Level Conveyor Sorting Factory

A complete modern OpenGL 3.3 demonstration of an automated parcel-sorting room. Reusable cube and cylinder meshes build the entire environment; no models or textures are required.

## What is implemented

- Elevated input conveyor and lower processing conveyor
- Gravity-driven, visibly rotating box drop between levels
- Pool of twelve reusable parcels in route-matched blue and orange, with varied sizes
- Explicit box state machine: upper, falling, lower, scanning, gate approach, left/right exit, dropping, collected
- Scanner pause and blue scan feedback
- Interlocked sorting gate with articulated guides, a fixed drive pivot, and smooth circular parcel turns
- Left and right exit conveyors and color-coded collection bins
- Rotating rollers and wall ventilation fan
- Operating/scanning/paused stack lights
- Hemisphere ambient lighting, warm overhead fixtures, Phong materials, and filtered directional shadows
- Toggleable attenuated overhead lights and directional daylight
- Warm ivory and teal factory interior with three loaded racks, labelled cartons, packing bench, tool chest, forklift, pallet truck, staged pallet loads, compressor, drums, utility cabinets, fire equipment, windows, shutter, and safety markings
- Automatic near-wall cutaway and unobstructed overhead inspection view
- Delta-time movement, speed adjustment, reset, and continuous parcel reuse
- Windowed launch on macOS, runtime fullscreen toggle, free-look mouse camera, and three demonstration presets

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

On macOS the application opens in a window so it appears in the current desktop Space. Use `F11` for fullscreen. On other platforms it starts fullscreen; `--windowed` selects a window instead.

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

Blue parcels go to lane A; orange parcels go to lane B. The scanner holds each parcel for 0.65 seconds. The leading parcel reserves the gate and waits for alignment within 0.1 degrees. The gate stays locked until the entire parcel clears the guides, including when `G` is pressed repeatedly. Upstream parcels queue with size-aware spacing, and the upper conveyor waits for a clear landing area.

Shared layout dimensions drive the visible conveyor geometry and parcel routes. Parcels follow a constant-radius turn, rotate to face the exit belt, clear its end, then fall into the matching bin. Packing tape and labels rotate with their carton. Short simulation substeps preserve stable motion at low frame rates. Completed parcels are counted and recycled after collection.

Rendering lives in `FactoryScene`; `FactorySimulation` runs independently of OpenGL so the sorting behaviour can be checked without a display.

## Verification and preview captures

```bash
ctest --test-dir build --output-on-failure
./build/conveyor_factory --capture /tmp/factory.ppm --time 24 --view 1
```

The simulation checks cover both destinations, parcel spacing, gate ownership, conveyor clearance, bin landings, pause/reset, speed changes, and repeated manual gate requests. They run at three conveyor speeds and frame intervals from 1/144 second to 1/10 second.

`--capture` renders one deterministic PPM image in a hidden OpenGL window and exits. Use `--view 1`, `2`, or `3` for the overview, top, or side camera. An available graphics context is still required.

## Source layout

```text
include/            Shader, camera, geometry, and scene declarations
src/                Application, rendering, and simulation implementation
shaders/            GLSL 330 Core Phong shaders
CMakeLists.txt      Reproducible build and dependency setup
```

## Graphics concepts demonstrated

Translation moves parcels and positions equipment; scaling reuses primitives for every factory object; rotation animates rollers, the fan, falling boxes, and the gate; hierarchical transforms preserve mechanical pivots; the camera supplies view transforms; perspective supplies projection; and the shaders implement Phong lighting for non-uniformly scaled objects.
