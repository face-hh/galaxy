# untitled galaxy simulator

A galaxy simulation designed for visual beauty rather than scientific realism

![alt text](ghassets/image.png)
![alt text](ghassets/image2.png)
![alt text](ghassets/image3.png)

Watch YouTube video:

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/P6jMlMi1iTk/0.jpg)](https://www.youtube.com/watch?v=P6jMlMi1iTk)

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

## Build Linux

1. `cmake -S . -B build && cd build`
2. `make`
3. `./SpaceCpp`

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
- **Linux** ✅
- **MacOS** 🫰❌0️⃣0️⃣0️⃣🫵😂 [download](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcSziL_UbSIwkRr35AS4NW_Y6LsV4_Svlp4u_w&s)

## License
no license do whatever you want
