# AI Coding Assistant Instructions for Wind Engine (C++ 3D Rendering)

## Project Overview
This is a cross-platform C++ 3D rendering engine using OpenGL/EGL, supporting desktop (GLFW) and Android platforms. The engine focuses on model rendering with instancing, camera interaction, and shader-based effects.

## Architecture
- **Entry Points**: `main.cpp` (desktop GLFW), `jni_main.cpp` (Android JNI)
- **Core Library**: `EGL_Component` - modular component system
- **Key Components**:
  - `Component_3DModels/ModelRenderer`: Main renderer with threading and picking
  - `Component_Camera/Camera`: Camera management with GLM matrices
  - `Component_Shader_Blinn_Phong`: GLSL shaders converted to C++ headers
  - `Component_Mouse/CameraInteractor`: Touch/mouse interaction handling
  - `Component_FBO`: Framebuffer objects for offscreen rendering
- **Data Flow**: ModelLoader → ShaderProgram → Renderer → Camera → FBO output

## Build Workflows
- **Windows EXE**: Run `Compile_exe.bat` - uses MinGW, Ninja, CMake
- **Android SO**: Run `compile_so.bat` - uses Android NDK, validates shaders first
- **Shader Conversion**: Python scripts in `Component_Shader_Blinn_Phong/` convert GLSL to `.h` headers
- **CMake Structure**: Root `CMakeLists.txt` + `EGL_Component/CMakelists.txt` for library

## Coding Conventions
- **Cross-Platform**: Use `#ifdef __ANDROID__` for platform-specific code (EGL/GLES3 vs GLFW/GLAD)
- **Instancing**: Enabled via `ENABLE_INSTANCING` macro, affects `FlexableTouchPad` includes
- **Memory**: Use GLM for math (`glm::mat4`, `glm::vec3`), smart pointers where appropriate
- **Components**: Each in separate directory with `.hpp`/`.cpp`, follow `Component_Name/` pattern
- **Shaders**: Write GLSL in `.glsl` files, auto-convert to headers via Python

## Common Patterns
- **Renderer Initialization**: Create `ModelRenderer` instance, set camera/interactor, start render thread
- **Model Loading**: Use `ModelLoader_Universal_Instancing` for Assimp-based loading
- **Picking**: `requestPick()` triggers offscreen render for object selection
- **Threading**: `std::atomic<bool>` for render loop control, `std::thread` for async operations
- **UBO Updates**: Use `Component_UBO` for uniform buffer objects with `Globals` struct

## Examples
- **Basic Render Loop**: See `main.cpp` lines 50-100 for GLFW window setup and render thread
- **Android JNI**: `jni_main.cpp` shows surface handling and Java interop
- **Component Usage**: `ModelRenderer.hpp` demonstrates composition of Camera, Shader, FBO components
- **Shader Integration**: `WindShader.hpp` shows GLSL header inclusion pattern

## Dependencies
- **3rdparty**: Assimp (model loading), GLM (math), GLFW/GLAD (desktop), SOIL2 (textures)
- **Build Tools**: CMake 3.16+, Ninja, Python 3 (for shader conversion)
- **Android**: NDK 23+, JNI for Kotlin integration

## Debugging Tips
- **Shader Validation**: Run `validate_shaders.py` before building Android
- **Build Logs**: Check `build_win/` or `build_android/` for CMake output
- **Platform Issues**: Desktop uses GLFW callbacks, Android uses JNI touch events