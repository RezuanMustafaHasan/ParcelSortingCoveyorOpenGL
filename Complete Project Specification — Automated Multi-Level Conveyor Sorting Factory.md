# Automated Multi-Level Conveyor Sorting Factory  
## Complete OpenGL Project Specification

## 1. Project Goal

Develop a complete interactive **3D automated factory room using modern OpenGL and C++**.

The main focus of the scene must be a **multi-level conveyor sorting system**. Boxes begin on an elevated conveyor, travel forward, fall naturally from the upper conveyor onto a lower conveyor, pass through a scanning/sensor area, and are then automatically routed by a rotating sorting gate toward either a left or right exit conveyor.

The conveyor system must appear to be part of a small automated warehouse/factory room rather than floating objects in an empty OpenGL world.

The scene should demonstrate the following computer graphics concepts clearly:

- 3D object construction from primitive geometry
- Model transformations
- Translation
- Rotation
- Scaling
- Hierarchical transformations
- Continuous animation
- Delta-time-based movement
- Multiple moving objects
- Camera movement
- Perspective projection
- Lighting
- Phong shading
- Material properties
- Point/directional lighting
- State-based object behavior
- Basic user interaction
- Object reuse
- Scene composition
- Simple simulation logic

The project should remain technically manageable. Avoid complicated external 3D models, advanced physics engines, skeletal animation, image processing, or unnecessary dependencies.

Most objects should be created from reusable **cuboids, cylinders, and planes**.

---

# 2. Technology Requirements

Use:

- C++
- Modern OpenGL
- GLFW
- GLAD
- GLM
- GLSL vertex shaders
- GLSL fragment shaders

Use OpenGL Core Profile, preferably OpenGL 3.3 or later.

If an existing project structure already contains helper classes such as:

- Shader / shaderClass
- VAO
- VBO
- EBO

reuse them instead of unnecessarily rewriting everything.

The project should compile as a normal standalone C++ OpenGL application.

No game engine such as Unity or Unreal should be used.

Avoid external model loaders unless absolutely necessary.

The project must primarily demonstrate manually constructed OpenGL geometry and transformations.

---

# 3. Overall Scene Concept

The complete scene represents a small automated parcel-sorting room.

The main processing flow is:

```text
Box Spawn
   ↓
Upper Conveyor
   ↓
Box reaches edge
   ↓
Box falls to lower level
   ↓
Lower Conveyor
   ↓
Sensor / Scanner
   ↓
Sorting Decision
   ↓
Rotating Gate
  ↙       ↘
Left       Right
Exit        Exit
  ↓           ↓
Bin A       Bin B
```

The conveyor is the central element of the entire scene.

The factory environment exists to make the scene believable and visually complete without distracting from the conveyor.

---

# 4. Coordinate System

Use a consistent world coordinate system.

Recommended convention:

```text
X-axis = left/right
Y-axis = up/down
Z-axis = forward/backward
```

Use positive Y as upward.

The main conveyor may run mostly along the Z-axis.

For example:

```text
Box movement:
Z = negative → positive
```

or the reverse.

The exact direction is not important, but it must remain consistent throughout the implementation.

Recommended overall factory room dimensions:

```text
Width:   approximately 20 units
Height:  approximately 8 units
Depth:   approximately 24 units
```

The conveyor should occupy the central region of the room.

---

# 5. Factory Room

Create a simple enclosed or semi-enclosed factory environment.

The room should contain:

- floor
- back wall
- left wall
- right wall
- optionally partial ceiling
- structural columns if useful

The front wall should remain open so the camera can clearly see the machinery.

Recommended style:

- gray concrete floor
- light gray walls
- darker structural beams
- metallic conveyors

Do not make the room overly detailed.

The environment should support the main conveyor visually rather than dominate the scene.

---

# 6. Main Conveyor System

The conveyor system must contain multiple levels.

This is a mandatory feature.

There should be at least:

```text
Upper Conveyor
      ↓
Falling Section
      ↓
Lower Conveyor
      ↓
Sorting Section
      ↓
Two Exit Conveyors
```

---

# 7. Upper Conveyor

The first conveyor is elevated above the lower conveyor.

Recommended height:

```text
Y ≈ 3.5–4.0
```

The conveyor consists of:

- conveyor bed
- belt surface
- side rails
- rollers
- support legs

The upper conveyor should be long enough to visibly show boxes moving before falling.

Example approximate dimensions:

```text
Length: 7–9 units
Width:  2–2.5 units
Height: 0.3–0.5 units
```

The conveyor structure can be created from scaled cubes.

Example conceptual hierarchy:

```text
Upper Conveyor
│
├── Main frame
├── Belt
├── Left rail
├── Right rail
├── Roller 1
├── Roller 2
├── Roller 3
├── ...
└── Support legs
```

---

# 8. Conveyor Belt Appearance

The belt itself does not need a physically simulated continuous texture.

It should visually resemble a dark rubber belt.

Recommended color:

```text
dark gray / almost black
```

The belt can remain geometrically stationary while:

- boxes translate forward
- rollers rotate

This is enough to make the conveyor appear active.

Optionally, several thin belt markings can move repeatedly along the conveyor to improve the illusion of belt motion.

This feature is optional.

---

# 9. Conveyor Rollers

Place multiple cylindrical rollers underneath or partially visible beside the belt.

Rollers should rotate continuously while the conveyor is running.

Use the same cylinder mesh repeatedly.

Do not create a unique mesh for every roller.

Each roller should use translation and rotation to move the same basic cylinder into position.

Example:

```cpp
rollerAngle += rollerSpeed * deltaTime;
```

Then rotate the roller around its local axis.

When the conveyor is paused, rollers must stop rotating.

When conveyor speed increases, roller rotation speed should also increase.

---

# 10. Box Objects

Multiple parcel boxes should continuously travel through the system.

Each box should be a cuboid.

Boxes may have different:

- sizes
- colors
- destinations

Example box colors:

- blue
- orange
- yellow
- green
- brown

Boxes should not all look identical.

However, avoid creating too many different geometries.

Scale the same cube mesh differently.

Example:

```text
Small box  = 0.8 × 0.7 × 0.8
Medium box = 1.0 × 0.8 × 1.0
Large box  = 1.2 × 0.9 × 1.0
```

---

# 11. Box Data Structure

Each moving box should have its own state.

A recommended structure is conceptually:

```cpp
struct Box
{
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 color;

    float rotationAngle;

    BoxState state;
    Destination destination;

    bool scanned;
    bool active;
};
```

Recommended states:

```cpp
enum class BoxState
{
    UPPER_CONVEYOR,
    FALLING,
    LOWER_CONVEYOR,
    SCANNING,
    APPROACHING_GATE,
    LEFT_EXIT,
    RIGHT_EXIT,
    COLLECTED
};
```

Recommended destination:

```cpp
enum class Destination
{
    LEFT,
    RIGHT
};
```

This state-machine approach should be used instead of writing unrelated position checks throughout the rendering code.

---

# 12. Box Spawning

Boxes should appear automatically at the beginning of the upper conveyor.

Do not spawn hundreds of boxes.

A small reusable pool such as:

```text
4–8 boxes
```

is sufficient.

Boxes should spawn one after another with enough spacing so that they do not overlap.

Example:

```text
Spawn interval: 2–4 seconds
```

When a box reaches the final collection area, it may:

- disappear
- reset
- return to the spawn pool

This allows continuous operation without creating unlimited objects.

---

# 13. Upper Conveyor Motion

While a box is in:

```cpp
UPPER_CONVEYOR
```

its position should move toward the end of the upper belt.

Conceptually:

```cpp
box.position.z += conveyorSpeed * deltaTime;
```

The exact axis depends on the final scene orientation.

When the box reaches the upper conveyor edge, change:

```cpp
box.state = FALLING;
```

---

# 14. Multi-Level Falling Animation

This is one of the most important features because it was specifically requested.

A box must visibly leave the upper conveyor and fall onto the lower conveyor.

Do not instantly teleport the box from one level to another.

During:

```cpp
FALLING
```

the box should move downward.

A simple implementation is acceptable.

Example:

```cpp
box.position.y -= fallSpeed * deltaTime;
```

For better appearance, give the box simple vertical acceleration:

```cpp
verticalVelocity += gravity * deltaTime;
box.position.y += verticalVelocity * deltaTime;
```

Use an artificial gravity such as:

```text
gravity = -4 to -9 units/s²
```

Exact physical accuracy is not necessary.

The motion only needs to look believable.

---

# 15. Falling Box Rotation

To make the fall more visually noticeable, optionally rotate the box slightly.

Example:

```cpp
box.rotationAngle += 60.0f * deltaTime;
```

Do not make it spin wildly.

Approximately:

```text
10–30 degrees
```

during the complete fall is enough.

Once the box reaches the lower conveyor:

```cpp
box.position.y = lowerConveyorTopY;
box.state = LOWER_CONVEYOR;
```

The box should then continue normal horizontal movement.

---

# 16. Lower Conveyor

The lower conveyor should receive the falling boxes.

Recommended height:

```text
Y ≈ 1.3–1.7
```

The lower conveyor should begin underneath or slightly ahead of the upper conveyor edge.

It should then carry boxes toward the scanning and sorting system.

Use the same construction style as the upper conveyor:

- bed
- belt
- rails
- rollers
- support legs

Because it is lower, the viewer should immediately recognize the multi-level structure.

---

# 17. Optional Receiving Ramp

A small angled ramp may be placed between the upper and lower levels.

However, boxes should still visibly change height.

The ramp should not completely hide the falling animation.

A short metallic receiving platform is enough.

This can be a rotated cuboid.

---

# 18. Sensor / Scanner Station

Before reaching the sorting gate, every box should pass through a scanner.

Build a simple scanner frame above the conveyor.

Recommended shape:

```text
       SENSOR
   ┌────────────┐
   │            │
===│=== BOX ====│===
   │            │
```

Use:

- two vertical cuboids
- one horizontal cuboid
- one small glowing sensor element

The scanner does not need real barcode recognition.

It simply checks the box's predefined destination.

---

# 19. Sorting Decision

Each newly spawned box should receive either:

```cpp
Destination::LEFT
```

or:

```cpp
Destination::RIGHT
```

The destination may be assigned:

- randomly
- alternately
- according to box color

The preferred approach is to relate color to destination so the result is visually understandable.

For example:

```text
Blue / Green box  → LEFT
Orange / Yellow   → RIGHT
```

The exact mapping may be adjusted.

---

# 20. Sensor Activation

When a box enters the sensor area:

```cpp
box.scanned = true;
```

Then the system determines its route.

The scanner should provide visible feedback.

For example:

```text
Blue light flash = scanning
```

or temporarily brighten the scanner element.

The sensor should remain simple.

No complicated visual effect is required.

---

# 21. Sorting Gate

After the scanner there must be a rotating mechanical sorting gate.

The gate should be a long thin cuboid attached to a vertical pivot.

Conceptually:

```text
        Box →
========================

              |
              | gate
              O pivot
```

The pivot must remain fixed.

The gate rotates around that pivot.

This is an important demonstration of transformation order.

---

# 22. Correct Pivot Rotation

Do not rotate the gate around its center if doing so makes the pivot move.

Use hierarchical transformation or translate-rotate-translate logic.

Conceptually:

```cpp
model = glm::mat4(1.0f);

model = glm::translate(model, pivotPosition);

model = glm::rotate(
    model,
    glm::radians(gateAngle),
    glm::vec3(0.0f, 1.0f, 0.0f)
);

model = glm::translate(
    model,
    glm::vec3(gateLength / 2.0f, 0.0f, 0.0f)
);

model = glm::scale(model, gateScale);
```

The gate should visibly swing around one fixed endpoint.

---

# 23. Gate States

Recommended gate logic:

```cpp
enum class GateState
{
    CENTER,
    MOVING_LEFT,
    LEFT,
    MOVING_RIGHT,
    RIGHT,
    RETURNING
};
```

However, this may be simplified if desired.

Recommended angles:

```text
Center = 0°
Left   = approximately -35°
Right  = approximately +35°
```

The gate must animate smoothly.

Do not instantly jump from:

```text
-35° → +35°
```

Use interpolation:

```cpp
gateAngle += gateAngularSpeed * deltaTime;
```

until the desired angle is reached.

---

# 24. Gate Timing

The gate must finish rotating before the box reaches it.

Recommended sequence:

```text
Box enters scanner
      ↓
Destination determined
      ↓
Gate starts moving
      ↓
Box continues slowly toward gate
      ↓
Gate reaches target angle
      ↓
Box enters sorting area
      ↓
Box redirected
```

Avoid situations where a box crosses the gate before the gate reaches its position.

---

# 25. Sorting Movement

When the box reaches the gate, change its movement direction.

For example, after sorting:

```cpp
LEFT_EXIT
```

may move using both:

```text
X negative
Z positive
```

while:

```cpp
RIGHT_EXIT
```

may move:

```text
X positive
Z positive
```

This creates two diagonal exit paths.

Alternatively, the main conveyor can split into a clear Y-shape.

The visual result should make it obvious that boxes are being routed to different destinations.

---

# 26. Exit Conveyors

Create two separate conveyor branches:

```text
              MAIN
               |
               |
             GATE
            /    \
           /      \
       LEFT      RIGHT
       EXIT       EXIT
```

Each should contain:

- conveyor bed
- dark belt
- rollers
- supports
- side rails

They can be shorter than the main conveyors.

---

# 27. Collection Bins

At the end of each exit conveyor, place a large collection container.

Use simple cuboid geometry.

Recommended:

```text
LEFT  → Blue collection bin
RIGHT → Orange/red collection bin
```

Boxes should continue until they reach the bin.

Then:

```cpp
box.state = COLLECTED;
```

The box may remain briefly visible before being reset.

---

# 28. Factory Control Panel

Place a small industrial control panel somewhere near the conveyor.

Build it from:

- one vertical cuboid
- small colored buttons
- indicator circles/cylinders
- optional display rectangle

Suggested visual layout:

```text
┌────────────────────┐
│ FACTORY CONTROL    │
│                    │
│ ● RUN              │
│ ● SORTING          │
│ ● STOP             │
│                    │
│ [START] [STOP]     │
└────────────────────┘
```

The panel does not require mouse interaction.

Keyboard actions should change the visible indicator states.

---

# 29. Stack Indicator Lamp

Place a vertical industrial signal tower near the sorting area.

Use three short cylinders or spheres.

Colors:

```text
RED
YELLOW
GREEN
```

Recommended behavior:

### Green

Conveyor operating normally.

### Yellow

A box is being scanned or sorted.

### Red

System paused.

The active lamp should appear brighter than inactive lamps.

You can achieve this using:

- brighter material color
- emissive component
- simplified lighting logic

Actual bloom is not required.

---

# 30. Storage Rack

Create a simple warehouse rack against one wall.

Use thin cuboids for:

- vertical supports
- horizontal shelves

Place several spare parcel boxes on the shelves.

The rack should be static.

Its purpose is to make the room look like a real warehouse.

Reuse the same box geometry used by moving parcels.

---

# 31. Safety Fence

Create a simple safety barrier around part of the sorting machinery.

Use:

- thin vertical cuboids
- thin horizontal cuboids
- yellow or metallic frame

Do not model real wire mesh.

A simplified fence is sufficient.

The fence helps demonstrate repeated object placement and improves the industrial appearance.

---

# 32. Wall Ventilation Fan

Place a ventilation fan on one wall.

The fan should contain:

- central hub
- 3–4 blades
- outer frame if desired

The blades may be thin cuboids.

The fan must rotate continuously while the application is running.

Example:

```cpp
fanAngle += fanSpeed * deltaTime;
```

This provides another obvious rotational animation.

It should rotate around its center.

---

# 33. Factory Pipes

Add several simple pipes along one wall or near the ceiling.

Reuse cylinder geometry.

Recommended appearance:

- gray metallic cylinders
- one or two vertical pipes
- one horizontal pipe

Do not create a complicated plumbing network.

These are environmental objects only.

---

# 34. Floor Markings

Optionally place simple colored floor strips around the conveyor.

For example:

```text
yellow safety lines
```

These can be created from extremely thin scaled cubes placed slightly above the floor.

No textures are required.

---

# 35. Lighting

Use at least one meaningful lighting setup.

The project must implement **Phong lighting**.

The fragment shader should support:

```text
Ambient
Diffuse
Specular
```

Basic formula conceptually:

```text
FinalColor =
Ambient +
Diffuse +
Specular
```

Each object should use its material color.

---

# 36. Main Point Light

Place an overhead point light above the conveyor sorting area.

Approximate position:

```text
x = 0
y = 6–7
z = sorting area
```

This light should make surfaces facing it brighter and surfaces facing away darker.

Include distance attenuation if convenient.

Example coefficients:

```text
constant  = 1.0
linear    = 0.09
quadratic = 0.032
```

Exact values may be adjusted for the scale of the scene.

---

# 37. Ambient Light

Use moderate ambient lighting so dark surfaces remain visible.

Do not make ambient lighting so strong that the Phong effect disappears.

A good balance should clearly show:

- bright upper surfaces
- darker side surfaces
- specular highlights on metallic structures

---

# 38. Optional Secondary Light

A second weak light may be added if some portions of the factory become too dark.

However, avoid excessive lighting complexity.

The main goal is to demonstrate understandable Phong shading.

---

# 39. Materials

Assign different material appearances.

Suggested materials:

```text
Conveyor frame → metallic gray
Belt           → dark gray / black
Boxes          → colorful matte surfaces
Gate           → yellow/orange industrial color
Rails          → dark metallic gray
Floor          → gray
Walls          → light gray
Bins           → blue / orange
Rack           → gray
Safety fence   → yellow
```

Metal objects can use higher specular strength.

Boxes should have lower specular strength.

---

# 40. Camera System

Provide an interactive camera.

At minimum, support:

### Perspective angled view

This should be the default view.

The default camera must clearly show:

- upper conveyor
- falling area
- lower conveyor
- sorting gate
- exit conveyors

### Top view

The top view should make the sorting paths easy to understand.

### Optional side view

A side view is useful because it clearly shows the height difference between upper and lower conveyors.

---

# 41. Camera Controls

Recommended controls:

```text
W / S → Move forward/backward
A / D → Move left/right

Arrow keys or mouse → Rotate view

1 → Main perspective camera
2 → Top camera
3 → Side camera
```

If mouse-look implementation already exists, reuse it.

Do not make camera controls unnecessarily complicated.

---

# 42. Simulation Controls

Use keyboard controls such as:

```text
SPACE → Pause/resume conveyor system

+ / = → Increase conveyor speed

-     → Decrease conveyor speed

G     → Manually test sorting gate

R     → Reset simulation

1     → Perspective camera

2     → Top camera

3     → Side camera

L     → Toggle main factory light

F     → Toggle ventilation fan if desired

ESC   → Close application
```

Avoid conflicting keys between camera and simulation controls.

If number keys are used for camera selection, do not reuse them elsewhere.

---

# 43. Conveyor Speed

Maintain one primary variable:

```cpp
float conveyorSpeed;
```

Use it for:

- upper box movement
- lower box movement
- exit conveyor movement

Roller speed should be proportional to conveyor speed.

For example:

```cpp
rollerAngularSpeed = conveyorSpeed * multiplier;
```

Provide:

```text
minimum speed
normal speed
maximum speed
```

Do not allow negative speed unless reverse operation is intentionally implemented.

---

# 44. Pause Behavior

When paused:

- boxes stop moving
- rollers stop rotating
- sorting gate animation pauses
- box falling should ideally pause
- indicator light becomes red

Camera movement should remain usable.

The user should still be able to inspect the scene while the factory is paused.

---

# 45. Delta Time

All continuous animation must use frame-independent movement.

Use:

```cpp
deltaTime = currentFrame - lastFrame;
```

Examples:

```cpp
position += velocity * deltaTime;
angle += angularSpeed * deltaTime;
```

Do not make animation speed dependent on frame rate.

---

# 46. Hierarchical Modeling

Use hierarchical modeling wherever appropriate.

Important examples:

### Sorting gate

Pivot transforms parent the gate body.

### Fan

Fan center determines rotation of all blades.

### Conveyor

Conveyor base position can act as the reference for:

- rails
- rollers
- supports
- belt

### Control panel

Buttons and lights should be positioned relative to the control panel.

Avoid manually using unrelated absolute coordinates for every child object when a parent transformation would make the design cleaner.

---

# 47. Primitive Geometry

Create reusable primitive rendering functions.

Recommended conceptual functions:

```cpp
drawCube(...)
drawCylinder(...)
drawPlane(...)
```

or object classes if the existing architecture already supports them.

A single cube should be reusable for:

- conveyors
- walls
- floor
- rails
- boxes
- gate
- shelves
- panel
- supports
- fence
- fan blades

A single cylinder should be reusable for:

- rollers
- indicator lamps
- pipes
- fan hub

Do not create redundant geometry buffers for identical basic meshes unless the current architecture requires it.

---

# 48. Recommended Object Rendering Functions

Organize the drawing code into logical functions such as:

```cpp
drawFactoryRoom();
drawUpperConveyor();
drawLowerConveyor();
drawExitConveyors();
drawConveyorRollers();

drawBoxes();
drawScanner();
drawSortingGate();

drawControlPanel();
drawIndicatorTower();

drawStorageRack();
drawSafetyFence();
drawWallFan();
drawFactoryPipes();

drawCollectionBins();
```

The rendering loop should not contain hundreds of unrelated transformation commands directly.

---

# 49. Recommended Program Architecture

A reasonable project structure would be:

```text
project/
│
├── main.cpp
│
├── shaderClass.h
├── shaderClass.cpp
│
├── VAO.h / VAO.cpp
├── VBO.h / VBO.cpp
├── EBO.h / EBO.cpp
│
├── Camera.h
├── Camera.cpp
│
├── Geometry.h
├── Geometry.cpp
│
├── FactoryScene.h
├── FactoryScene.cpp
│
├── Box.h
├── Box.cpp
│
├── shaders/
│   ├── default.vert
│   └── default.frag
│
└── README.md
```

This is only a recommendation.

If an existing starter project already has its own architecture, preserve that architecture instead of forcing unnecessary restructuring.

---

# 50. Separation of Simulation and Rendering

Avoid mixing simulation logic directly into drawing functions.

Recommended structure:

```cpp
processInput();

updateSimulation(deltaTime);

renderScene();

swapBuffers();
```

Inside:

```cpp
updateSimulation()
```

handle:

- box motion
- box state transitions
- gravity
- gate movement
- scanner states
- roller angle
- fan rotation
- box spawning

Inside rendering functions, mostly handle:

- model matrices
- shader uniforms
- mesh rendering

---

# 51. Box State Machine

Implement the complete path approximately as:

```text
SPAWN
 ↓
UPPER_CONVEYOR
 ↓
FALLING
 ↓
LOWER_CONVEYOR
 ↓
SCANNING
 ↓
APPROACHING_GATE
 ↓
LEFT_EXIT / RIGHT_EXIT
 ↓
COLLECTED
 ↓
RESET / RESPAWN
```

This logic should be clearly visible in the source code.

---

# 52. Recommended Simulation Logic

Conceptually:

```cpp
switch (box.state)
{
    case UPPER_CONVEYOR:
        moveAlongUpperConveyor();
        if (reachedUpperEdge)
            startFalling();
        break;

    case FALLING:
        applyGravity();
        if (reachedLowerBelt)
            enterLowerConveyor();
        break;

    case LOWER_CONVEYOR:
        moveTowardScanner();
        if (insideScanner)
            scanBox();
        break;

    case SCANNING:
        determineDestination();
        rotateGate();
        box.state = APPROACHING_GATE;
        break;

    case APPROACHING_GATE:
        moveTowardGate();
        if (reachedGate)
            chooseExit();
        break;

    case LEFT_EXIT:
        moveAlongLeftExit();
        break;

    case RIGHT_EXIT:
        moveAlongRightExit();
        break;

    case COLLECTED:
        resetBoxLater();
        break;
}
```

Exact implementation may vary, but the behavior should follow this conceptual structure.

---

# 53. Collision Requirements

A full physics collision engine is NOT required.

Use predetermined trigger coordinates.

Examples:

```cpp
if (box.position.z >= upperEndZ)
```

or:

```cpp
if (distance(box.position, scannerPosition) < threshold)
```

This is enough for the project.

Avoid complicated rigid-body collision detection.

---

# 54. Box Spacing

The system must avoid boxes visually intersecting.

Simplest solution:

- use a fixed spawn interval
- allow only a limited number of active boxes
- ensure conveyor speed is consistent

If needed, check distance from the newest box before spawning another.

---

# 55. Sorting Gate Conflict Prevention

Two boxes should not enter the gate simultaneously.

Maintain adequate spacing between boxes.

The gate should have enough time to:

```text
sort one box
↓
return/change orientation
↓
receive next box
```

A spawn interval of several seconds should make this manageable.

---

# 56. Continuous Operation

The factory should operate continuously without user intervention.

Typical sequence:

```text
Box 1 → left
Box 2 → right
Box 3 → right
Box 4 → left
...
```

After boxes reach their collection bins, they can be reused.

The system should continue indefinitely.

---

# 57. Visual Animation Requirements

At least the following animations must be visible simultaneously during normal operation:

- boxes moving along conveyor
- conveyor rollers rotating
- boxes falling between conveyor levels
- sorting gate rotating
- ventilation fan rotating
- indicator lamp changing state

These animations together should make the factory look active.

---

# 58. Static Environmental Objects

Use several simple static objects to complete the room:

- warehouse rack
- spare boxes
- safety fence
- pipes
- control panel
- collection bins
- walls
- floor
- conveyor supports

Do not make all objects animated.

Static environment objects are acceptable and desirable.

---

# 59. Visual Priority

The visual hierarchy should be:

```text
1. Conveyor system
2. Moving boxes
3. Falling box transition
4. Scanner
5. Sorting gate
6. Exit conveyors
7. Industrial room decoration
```

The viewer should immediately understand what the main machine does.

Avoid placing large decorations that hide the conveyors.

---

# 60. Suggested Scene Layout

A possible top/angled layout:

```text
 ------------------------------------------------------
|                                                      |
|  STORAGE RACK                         WALL FAN        |
|  ┌──────────┐                            O           |
|  │ □ □ □ □  │                                        |
|  │ □ □ □    │                                        |
|  └──────────┘                                        |
|                                                      |
|             UPPER CONVEYOR                           |
|             ======================>                  |
|                                  |                   |
|                                  | DROP              |
|                                  ↓                   |
|                           ===================        |
|                           LOWER CONVEYOR ====>       |
|                                   |                  |
|                               [SCANNER]              |
|                                   |                  |
|                                  BOX                 |
|                                   |                  |
|                              SORTING GATE            |
|                                /       \             |
|                               /         \            |
|                        LEFT EXIT      RIGHT EXIT      |
|                         =======        =======        |
|                            ↓              ↓           |
|                         [BIN A]        [BIN B]        |
|                                                      |
|   CONTROL PANEL          SAFETY FENCE                |
|   ┌───────────┐         | | | | | |                  |
|   │ ● ● ●     │         | | | | | |                  |
|   └───────────┘                                      |
|                                                      |
 ------------------------------------------------------
```

This does not need to be followed pixel-perfectly.

Maintain the same functional organization.

---

# 61. Recommended Dimensions

Use approximate values and adjust visually.

Example:

```text
Room:
20 × 8 × 24

Upper conveyor:
2.2 wide
8 long
Y ≈ 4

Lower conveyor:
2.2 wide
7 long
Y ≈ 1.5

Exit conveyors:
1.7 wide
5 long

Boxes:
0.8–1.2 units

Gate:
2–3 units long
0.15–0.25 units thick

Safety fence:
1.5–2 units tall

Storage rack:
3–4 units wide
3 units tall
```

Exact dimensions can be modified to prevent intersections and improve composition.

---

# 62. Color Scheme

Use a consistent industrial palette.

Recommended:

```text
Floor:
medium gray

Walls:
light gray

Conveyor frame:
dark metallic gray

Belt:
almost black

Rails:
steel gray

Gate:
yellow/orange

Safety fence:
industrial yellow

Boxes:
blue
orange
green
brown
yellow

Collection Bin A:
blue

Collection Bin B:
orange/red

Control panel:
dark gray

Rack:
gray

Pipes:
metal gray
```

Avoid extremely saturated colors everywhere.

---

# 63. Shader Requirements

Vertex shader should receive at minimum:

```text
model
view
projection
```

It should output:

- fragment position
- transformed normal
- object color or required material information

Fragment shader should implement Phong lighting.

Required calculations:

```text
ambient
diffuse
specular
```

Use normalized surface normals.

Use:

```text
normal matrix
```

when appropriate:

```cpp
transpose(inverse(mat3(model)))
```

or equivalent.

---

# 64. Normal Handling

Lighting must look correct when cubes are non-uniformly scaled.

Therefore normals must be transformed correctly.

Do not simply multiply normals by the model matrix if non-uniform scaling is used.

Use the inverse-transpose normal matrix.

---

# 65. Depth Testing

Enable depth testing:

```cpp
glEnable(GL_DEPTH_TEST);
```

The scene must render correctly in 3D.

---

# 66. Face Culling

Face culling is optional.

If enabled, ensure geometry winding is correct.

Do not allow objects to disappear because of incorrect face orientation.

---

# 67. Background Color

Use a neutral industrial background.

If the factory room fills most of the view, a slightly dark blue-gray clear color is appropriate.

Avoid pure white or extremely bright backgrounds.

---

# 68. Window

Recommended:

```text
1280 × 720
```

or:

```text
1000 × 800
```

The exact resolution is flexible.

Enable viewport updates when the window is resized.

---

# 69. Controls Display

The project should print control instructions to the console when it starts.

Example:

```text
=== Automated Conveyor Factory ===

SPACE : Pause / Resume
+     : Increase conveyor speed
-     : Decrease conveyor speed
R     : Reset simulation
G     : Test sorting gate
L     : Toggle main light

1     : Perspective camera
2     : Top camera
3     : Side camera

ESC   : Exit
```

This makes the project easier to demonstrate.

---

# 70. Reset Function

Pressing:

```text
R
```

should restore the factory simulation to its initial condition.

Reset:

- box positions
- box states
- box velocities
- gate angle
- gate state
- spawn timer
- conveyor speed if desired
- scanner status

Camera may remain unchanged.

---

# 71. Manual Gate Demonstration

The user should be able to manually test the sorting gate.

For example:

```text
G → alternate gate left/right
```

This is useful during demonstration even when no box is nearby.

Automatic sorting must still work normally.

---

# 72. Main Light Toggle

Pressing:

```text
L
```

should enable/disable or greatly reduce the main point light.

This allows the lighting implementation to be demonstrated.

Do not make the entire scene completely invisible when light is disabled.

Ambient lighting may remain.

---

# 73. Camera Presets

Recommended preset coordinates should be chosen after the scene dimensions are finalized.

Conceptually:

### Perspective

Located diagonally above and outside the conveyor.

### Top

Camera directly above the sorting system.

### Side

Camera facing the upper/lower conveyor transition.

Switching should happen immediately when the appropriate key is pressed.

Smooth interpolation is optional.

---

# 74. Code Quality

The final project must be readable.

Avoid:

- one enormous `main.cpp`
- duplicated cube vertices
- duplicated rendering code
- unexplained magic numbers everywhere
- frame-dependent movement
- unnecessary global variables
- complex physics
- external assets that make setup difficult

Use descriptive names.

Example:

```cpp
upperConveyorY
lowerConveyorY
sortingGateAngle
conveyorSpeed
boxSpawnTimer
```

instead of meaningless names such as:

```cpp
a
x1
temp2
obj7
```

---

# 75. Constants

Store important layout values centrally.

For example:

```cpp
constexpr float UPPER_CONVEYOR_HEIGHT = 4.0f;
constexpr float LOWER_CONVEYOR_HEIGHT = 1.5f;

constexpr float NORMAL_CONVEYOR_SPEED = 1.5f;

constexpr float GATE_LEFT_ANGLE  = -35.0f;
constexpr float GATE_RIGHT_ANGLE = 35.0f;
```

This makes it easier to adjust the scene.

---

# 76. Reusable Transformation Function

It may be useful to create helper functions like:

```cpp
glm::mat4 createModelMatrix(
    glm::vec3 position,
    glm::vec3 rotation,
    glm::vec3 scale
);
```

However, hierarchical objects such as the gate and fan should still use appropriate parent transformations.

---

# 77. Important Demonstrable Graphics Concepts

The final implementation should intentionally make the following concepts easy to explain during viva/demo:

### Translation

Used for moving boxes and positioning objects.

### Scaling

Used to turn a cube into:

- conveyor
- wall
- shelf
- box
- rail
- gate

### Rotation

Used for:

- rollers
- gate
- ventilation fan
- optional falling boxes

### Hierarchical transformation

Used for:

- gate pivot
- fan blades
- conveyor components

### View transformation

Used by the camera.

### Projection transformation

Use perspective projection.

### Lighting

Phong model.

### Animation

Delta-time-based motion.

### State machine

Controls box progress through the factory.

---

# 78. Things NOT Required

Do not spend time implementing:

- advanced collision physics
- physics engines
- realistic rigid-body simulation
- shadows
- ray tracing
- skeletal animation
- imported Blender models
- complicated texture mapping
- particle systems
- bloom
- post-processing
- realistic barcode recognition
- networking
- machine learning

These are outside the project scope.

The goal is a polished OpenGL graphics project, not a factory simulator.

---

# 79. Optional Enhancements

Only implement these after all primary requirements work correctly:

- simple moving belt stripes
- small digital display on control panel
- box counter
- simple transparent scanner beam
- ceiling lamps
- sliding factory door
- emergency stop button
- speed indicator
- simple text overlay
- slightly different box sizes
- optional night/dim lighting mode

These are secondary.

Do not implement them at the expense of core features.

---

# 80. Priority Order

Development should follow approximately this order:

1. Get primitive cube rendering working.
2. Get cylinder rendering working.
3. Create factory floor and walls.
4. Create upper conveyor.
5. Create lower conveyor.
6. Create exit conveyors.
7. Create one moving box.
8. Implement box falling.
9. Implement multiple boxes.
10. Implement scanner.
11. Implement automatic destination assignment.
12. Implement sorting gate pivot rotation.
13. Implement left/right sorting.
14. Add collection bins.
15. Add conveyor rollers.
16. Add Phong lighting.
17. Add camera controls.
18. Add control panel.
19. Add indicator tower.
20. Add rack, fence, fan, and pipes.
21. Add keyboard controls.
22. Polish object placement and colors.
23. Test entire simulation.
24. Clean code and documentation.

Core simulation must be completed before adding decorative objects.

---

# 81. Acceptance Criteria

The project is complete only when all of the following work:

- A visible factory room exists.
- There are at least two conveyor height levels.
- Boxes move along an elevated conveyor.
- Boxes visibly fall from the upper level.
- Boxes land correctly on the lower conveyor.
- Lower conveyor carries them to a scanner.
- The scanner determines a left/right destination.
- The sorting gate rotates around a fixed pivot.
- The gate visibly changes direction depending on the box.
- Boxes enter different exit conveyors.
- Boxes reach separate collection bins.
- Multiple boxes continuously cycle through the system.
- Conveyor rollers rotate.
- A wall fan rotates.
- Indicator lights change according to simulation state.
- Conveyor can be paused.
- Conveyor speed can be adjusted.
- Camera has multiple useful views.
- Phong lighting is clearly visible.
- Scene uses depth testing.
- Movement uses delta time.
- The application can run continuously without boxes accumulating infinitely.
- Objects do not visibly pass through major structural parts.
- The code is organized and understandable.

---

# 82. Required Final Deliverables

Provide the complete working project including:

```text
1. All C++ source files
2. Header files
3. GLSL shaders
4. Required OpenGL helper classes
5. Build instructions
6. Dependency instructions
7. README
8. Keyboard control documentation
```

The README should explain:

- project overview
- implemented features
- controls
- required libraries
- how to compile
- how the sorting logic works
- major OpenGL concepts demonstrated

---

# 83. Expected Final Appearance

The final scene should visually communicate:

> “This is a small automated parcel-sorting factory where packages arrive from an elevated conveyor, fall onto a lower processing conveyor, are scanned, and are automatically routed into different collection areas.”

A viewer who sees the project for the first time should be able to understand this process without needing a long explanation.

The scene should feel like one integrated factory machine instead of unrelated OpenGL objects.

---

# 84. Most Important Requirement

Do not turn the project into an unnecessarily complicated industrial simulation.

The strongest parts should be:

```text
Multi-Level Conveyor
        +
Falling Boxes
        +
Automated Sensor
        +
Rotating Sorting Gate
        +
Two Routing Paths
        +
Multiple Coordinated Animations
        +
Phong-Lit Factory Environment
```

The project should achieve visual complexity primarily through **reuse of simple geometric objects and transformations**, not through complicated external assets.

The implementation should prioritize:

```text
Correctness
→ Clear animations
→ Good scene composition
→ Demonstration of OpenGL concepts
→ Code readability
→ Visual polish
```

over unnecessary complexity.

---

# 85. Instruction to the Coding Agent

Build the complete application rather than providing isolated code snippets.

When implementing:

- inspect and reuse any starter OpenGL code that is provided;
- preserve existing Shader, VAO, VBO, EBO, camera, or utility classes when they already work;
- modify existing code instead of rebuilding the project unnecessarily;
- keep dependencies minimal;
- compile and test the project after major changes;
- fix compilation/runtime errors;
- make reasonable layout adjustments when required;
- ensure all transformations are visually correct;
- ensure the gate rotates around its actual pivot;
- ensure falling boxes land on the lower conveyor;
- ensure boxes remain properly spaced;
- ensure automatic sorting continues indefinitely;
- ensure camera presets show the important parts of the scene;
- ensure all animations are frame-rate independent.

If a minor visual parameter is unspecified, choose a sensible value rather than stopping implementation to ask for confirmation.

The final result should be a **fully working, demonstration-ready OpenGL graphics laboratory project**, not merely a partially implemented framework.