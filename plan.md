# MentalEngine MVP Plan

## Goal

- Editor-only MVP for building a simple 3D scene.
- User can create primitive entities (cube, plane, sphere), view them in a scene list, select them from the list or by clicking in the viewport, and edit transform with a gizmo.
- Viewport includes a grid and a basic Unity-style fly camera.

## MVP Boundaries

- No save/load.
- No gameplay systems.
- No materials/shader graph/lighting pipeline beyond what is required for readable editor rendering.
- No general-purpose rendering abstractions unless they are needed by the editor scene path.

## Execution Order

### 1. Core Editor Scene Model

- Add an entity system sufficient for the editor MVP:
  - entity id
  - display name
  - transform (position, rotation, scale)
  - primitive type (cube, plane, sphere)
- Add editor state:
  - selected entity id
  - hover entity id
  - current gizmo mode (translate / rotate / scale)
- Keep this minimal and in-memory only.

### 2. Main Loop and Editor Layer Separation

- Split the current single render loop into:
  - platform/window update
  - input collection
  - editor update
  - render update
- Introduce an editor application layer instead of keeping all behavior in `editor/src/main.cpp`.
- Make room for per-frame scene update and viewport interaction logic.

### 3. Input System for Editor Interaction

- Add unified keyboard/mouse input state:
  - mouse position and delta
  - mouse buttons pressed/held/released
  - keyboard keys pressed/held/released
  - mouse wheel
- Required interactions:
  - RMB + mouse move to look around
  - WASD + QE to move camera
  - mouse click for selection
  - hotkeys for gizmo mode (`W`, `E`, `R`)

### 4. Scene Camera

- Implement an editor fly camera:
  - perspective projection
  - position + yaw/pitch rotation
  - movement relative to camera orientation
  - configurable speed and boost
- Camera must drive viewport rendering and mouse picking rays.

### 5. Runtime Scene Rendering Foundation

- Stop treating rendering as "clear swapchain only".
- Add the minimum scene render path:
  - depth texture
  - depth testing
  - per-frame camera data
  - mesh draw submission for multiple scene objects
- Dynamic rendering is fine, but it is only a means to render the viewport scene.

### 6. Primitive Mesh Generation

- Add built-in mesh data generation/loading for:
  - cube
  - plane
  - sphere
- Upload vertex/index buffers once and reuse them across entities.
- Keep one simple vertex format for MVP:
  - position
  - normal
  - optional color

### 7. Minimal Shader and Pipeline Set

- Implement only the shader modules and graphics pipelines required for MVP:
  - primitive mesh pipeline
  - grid pipeline
  - selection/picking pipeline if needed by chosen picking approach
- Descriptor management should be introduced only around actual MVP data binding needs:
  - camera uniform data
  - object transform data
  - optional ID buffer/picking resources

### 8. Viewport Scene Rendering

- Render all primitive entities using their world transforms.
- Render a grid aligned to the world origin.
- Ensure the scene is readable without a full lighting system:
  - either simple directional lighting
  - or vertex/normal-based shading with clear contrast
- Selected entity should have visible feedback.

### 9. Editor UI Framework

- Add the minimal editor UI needed for MVP:
  - viewport panel
  - entity list panel
  - simple toolbar or controls for primitive creation and gizmo mode
- Keep UI architecture simple and focused on editor workflows.

### 10. Entity Creation Workflow

- User can create:
  - cube
  - plane
  - sphere
- New entities should:
  - appear in the entity list
  - render in the viewport
  - become selectable/editable

### 11. Selection from Entity List

- Clicking an entity in the list selects it.
- Selection updates:
  - editor state
  - viewport highlight
  - gizmo target

### 12. Viewport Picking

- Implement object picking from mouse click in the viewport.
- Prefer a robust MVP approach:
  - render entity IDs to an offscreen target and read back clicked pixel
  - or use reliable ray/primitive intersection if render-ID path is not ready
- Picking must work with overlapping objects and perspective view.

### 13. Transform Gizmo

- Add translate / rotate / scale gizmo for selected entity.
- Required behavior:
  - gizmo renders on top of scene clearly enough for editing
  - drag axis/plane handles with mouse
  - update selected entity transform live
- Start with translation first, then scale, then rotation if implementation risk is high.

### 14. Selection and Transform Feedback

- Selected entity should be obvious in both UI and viewport.
- Hover feedback is desirable if cheap, but selection feedback is required.
- Keep transform editing stable across frame updates and camera movement.

### 15. MVP Polish and Stability

- Handle resize correctly for all viewport render resources.
- Ensure camera controls do not conflict with UI focus.
- Validate behavior with multiple entities of each primitive type.
- Remove temporary debug-only assumptions from the editor path.

## Definition of Done

- App opens into an editor window.
- Viewport renders a grid and basic 3D primitives.
- User can create cubes, planes, and spheres.
- User can see all entities in a list.
- User can select an entity from the list or by clicking it in the viewport.
- User can move around the scene with a fly camera.
- User can translate, rotate, and scale the selected entity with a gizmo.

## Notes

- "Implement Graphics Pipeline", "Implement Descriptors", and "Implement Shader modules" are not top-level MVP goals by themselves.
- They should be implemented only as part of the scene viewport/editor feature path above.
