# Iron Maiden Engine

The Iron Maiden Engine is a lightweight, Vulkan-based game engine designed for developing both 2D and 3D games. Built for speed and flexibility, it combines essential features with a streamlined user experience to support rapid, efficient game development.

## Purpose

The Iron Maiden Engine is crafted to minimize the bloat found in other engines, offering a streamlined experience for developers who prioritize speed and adaptability. Its user-friendly interface allows for easy onboarding of new team members, while its robust backend is optimized for professional use.

We have drawn inspiration from a range of popular engines, integrating the best elements of each to create a balanced and powerful tool.

## Inspiration

Our goal is to provide an advanced UI/UX experience inspired by Unity, yet tailored to experienced developers. The Iron Maiden Engine includes a custom compilation tool, which optimizes code for each platform, ensuring your game performs at its best without unnecessary overhead.

## Supported Platforms

At present, the Iron Maiden Engine supports Windows 10. We are actively working to expand compatibility to additional platforms in future releases, including other PC operating systems and mobile platforms, to provide greater accessibility and flexibility for developers.

## Current Features

- **Customizable Render Order:** Configure the order in which the renderer processes objects, giving developers control over scene appearance.
- **Scene Management:** Create and organize scenes, add objects, and import models with ease.
- **Modular Engine Library:** Develop custom applications using the engine's core features, making it adaptable to a wide range of projects.
- **Scene Saving and Loading:** Seamlessly save and load scenes to streamline workflow and improve efficiency.

## Development Roadmap

Based on feedback we have decided to add a UI-system to the Engine a lot earlier than anticipated, mainly to help with ease of development and debugging.
Because this engine use to be a non-UI based engine and use to use a DLL rather than a static library in early stages of development a lot of features that were tested and used early in development like the early C++ Script build tool are not working correctly and have issues, thus we have choosen to leave them out for stability of early development releases.
To address this we as a team are currently doing major restructuring of the Engine's backend to be a lot more accessible by the application rather than having general engine operations being internal and to adhere to being more readable to developers.
We are aiming for a major release in the first half of 2025 which will include all the neccessary features to make a game.

### Upcoming Features for 2025 Release
**Developed by Core Team**
- **Asset System**
- **Shader Compilation System** (supporting SPV, GLSL, and HLSL)
- **Custom C++ Script Build Tool**
- **Custom Render Passes**
- **Project Launcher Application**
- **Vulkan Descriptor Pool Management System**
- **Garbage Collection System**
- **Scene Overhaul:**
    - Scene Object Hierarchy
    - Scene Objects can be saved as prefabs

### Backend Restructuring
**Developed by QA Team**
 - **Renderer and Pipeline Integration**
 - **VKImages and VK Descriptor Pool Management**
 - **Header Structure Overhaul** (eliminating dependencies and dependency issues)
 - **Custom Pointer Data Structures for Garbage Collection**
 - **Adhering to Updated Code Style**

## Fixed Issues

- **Rotation Issues:** Object rotations now rely on Quanternions, Object rotations will also respect scene hierachy.

## Known Issues

The engine is currently in an early development phase, and we are aware of several existing issues that will be addressed as we progress through the restructuring process.

### Known Issues
- **Render Pass Shutdown:** Render passes and renderer are not always shutting down correctly, causing potential stability issues.

## Building
### Step-by-Step Guide
1. Install [Github Desktop for Windows](https://desktop.github.com/) then [fork and clone our repository](https://docs.github.com/en/get-started/exploring-projects-on-github/contributing-to-a-project)
    When Github Desktop is finished creating a fork, it will ask you, **How are you planning to use this fork?**. Select **For my own purpose** to ensure any changes made stay in your local repository.

    **Alternative Options**
    - To use Git from the command line, refer to the [Setting up Git](https://docs.github.com/en/get-started/getting-started-with-git/set-up-git) and [Fork a Repo](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/fork-a-repo) guides.
    - Alternatively, download the source code using the **Download ZIP** button.
2. Install Visual Studio 2022
    Any Desktop edition version 17.5 or higher is supported; we recommend 17.9.3 as this is the version the development team currently uses.
3. Install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) then setup an environment path for VK_SDK_PATH
    We recommend version 1.3.268.0 to ensure compatibility with the development environment.
4. Run Setup.bat to download all additional dependencies
    Setup.bat uses **git** to pull in required submodules. Ensure that **git** is correctly configured in your environment path.

    **Alternative Options**
    - Download dependencies using Git: [see more](#download-dependencies-using-git)
    - Download dependencies manually: [see more](#download-dependencies-manually)

5. Run CompileProject.bat to build the project

### Note Very Important
To ensure the engine functions correctly, please complete the following setup steps:

1. Launch the Engine.
2. Create a Folder for your projects.
3. Open **pref.conf** in the **IronMaiden/Editor** directory.
4. Update the **ProjectsDirectory** field to your newly created projects directory:
    ```
    ProjectsDirectory: [Enter directory path here]
    ```
5. Restart the Engine

The engine should now run correctly with your configured project directory.

### Alternative Options

#### Download dependencies using **git**
1. Navigate to the project root directory: **IronMaiden/**
2. Run the following commands: 
    ```
    git init
    git submodule update --init --recursive
    ```
3. Navigate to **IronMaiden/IronMaiden/vendors/imgui** directory.
4. Check out the imgui docking branch: 
    ```
    git checkout docking
    ```
#### Download dependencies manually
If you prefer to download dependencies manually, download and unzip the following repositories, placing each in the **IronMaiden/IronMaiden/vendors** directory:
- [entt](https://github.com/skypjack/entt)
- [glm](https://github.com/g-truc/glm)
- [ImGuizmo](https://github.com/BigJMC9/ImGuizmo/tree/master)
- [imgui](https://github.com/BigJMC9/imgui/tree/docking)
- [spdlog](https://github.com/BigJMC9/spdlog)
- [stb](https://github.com/nothings/stb)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
- [yaml-cpp](https://github.com/BigJMC9/yaml-cpp)
- [OpenFBX](https://github.com/nem0/OpenFBX)

## Development Team

As of October 21st, we are excited to announce the formation of the Iron Maiden Engine development team. This project began as a solo endeavor, and seeing it evolve with such significant progress has been incredibly rewarding. I am honored to take on the roles of **Project Leader** and **Lead Engineer** as nominated by the team.
BigJMC9 - Jacob Crosbie

### Core Team
**Lead Engineer** - BigJMC9 - Jacob Crosbie<br />
**Software Engineer** - luisadrias111 - Luis Adrias<br />
**Software Engineer** - What-Kins - Josh Watkins<br />

### Quality Assurance Team
**Lead Engineer** - BigJMC9 - Jacob Crosbie<br />
**Software Engineer** - What-Kins - Josh Watkins<br />
**Quality Assurance Tester** - Jack Absalom<br />

## License

This project is licensed under the [Apache-2.0 License](LICENSE.txt).
