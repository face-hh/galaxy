# untitled galaxy simulator

A galaxy simulation designed for visual beauty rather than scientific realism

![alt text](ghassets/image.png)
![alt text](ghassets/image2.png)
![alt text](ghassets/image3.png)

Watch YouTube video:
<iframe width="560" height="315" src="https://www.youtube.com/embed/ROMLBio1iCI?si=PQCR76pHwkfXbuL9" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>

## Requirements

### Software
- **Visual Studio 2022** (v143 platform toolset)
- **Windows SDK 10.0** or later
- **C++17** compiler support

### Dependencies
You'll need to download and set up the following libraries:

1. **GLFW 3.4** (Windows 64-bit binaries)
   - Download from [glfw.org](https://www.glfw.org/download.html)
   - Use the pre-compiled binaries for VC2022

2. **GLAD** (OpenGL loader)
   - Generate from [glad.dav1d.de](https://glad.dav1d.de/)
   - Core profile, OpenGL 3.3+

3. **OpenGL 3.3+**
   - Included with Windows (opengl32.lib)

## Build

1. Open `Space C++.sln` in Visual Studio 2022
2. Configure library paths:
   - Right-click the project → Properties
   - **C/C++** → **Additional Include Directories**: Add paths to:
     - `glfw-3.4.bin.WIN64\include`
     - `glad\include`
   - **Linker** → **Additional Library Directories**: Add path to:
     - `glfw-3.4.bin.WIN64\lib-vc2022`
   - **Linker** → **Input** → **Additional Dependencies**: Ensure these are listed:
     - `glfw3.lib`
     - `opengl32.lib`
3. Select **x64** platform (Win32 is also supported)
4. Choose **Debug** or **Release** configuration
5. Build and run (F5)

## Configuration

The simulation can be configured through the UI (press TAB to toggle) or by modifying `createDefaultGalaxyConfig()` in `main.cpp`:

```cpp
config.numStars = 1000000;
config.spiralTightness = 0.3;
config.diskRadius = 800.0;
config.bulgeRadius = 150.0;
config.rotationSpeed = 1.0;
```

## Controls

- **WASD** - Move camera
- **Mouse** - Look around
- **Scroll** - Zoom in/out
- **Ctrl** (hold) - Move zoom anchor to solar system instead of (0,0,0)
- **Tab** - simulation config

## Platform Support
- **Windows** ✅
- **Linux** 🖕
- **MacOS** 🫰❌0️⃣0️⃣0️⃣🫵😂

## License
no license do whatever you want
