workspace "IronMaiden"
    architecture "x64"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }
    startproject "Editor"
    startproject "Editor-NoGUI"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "IronMaiden"
    location "IronMaiden"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
    pchheader "maidenpch.hpp"
    pchsource "%{prj.name}/maidenpch.cpp"
    


    files 
    { 
        "%{prj.name}/**.h", 
        "%{prj.name}/**.hpp", 
        "%{prj.name}/**.c", 
        "%{prj.name}/**.cpp" 
    }

    removefiles 
    { 
        "%{prj.name}/vendors/**"
    }
    
    includedirs
    {
        "%{prj.name}",
        "%{prj.name}/vendors/vulkan/include",
        "%{prj.name}/vendors/glfw-3.3.8.bin.WIN64/include",
        "%{prj.name}/vendors/glm",
        "%{prj.name}/vendors/spdlog/include",
        "%{prj.name}/vendors/yaml-cpp/include",
        "%{prj.name}/vendors/stb",
        "%{prj.name}/vendors/entt/single_include",
        "%{prj.name}/vendors/tinyobjloader"
    }

    libdirs 
    { 
        "C:/Program Files/VulkanSDK/1.3.268.0/Lib",
        "%{prj.name}/vendors/glfw-3.3.8.bin.WIN64/lib-vc2022",
        "%{prj.name}/vendors/yaml-cpp/Debug"
    }

    links
    {
        "vulkan-1",
        "glfw3",
        "yaml-cppd",
        "kernel32"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "Off"
        systemversion "latest"

        defines
        {
            "MADAM_PLATFORM_WINDOWS";
            "MADAM_BUILD_DLL";
        }

        postbuildcommands
        {
            "copy /B /Y ..\\bin\\" .. outputdir .. "\\IronMaiden\\IronMaiden.dll ..\\bin\\" .. outputdir .. "\\Editor\\ > nul"
        }

    filter "configurations:Debug"
        defines "MADAM_DEBUG"
        symbols "On"
        runtime "Debug"

    filter "configurations:Release"
        defines "MADAM_RELEASE"
        optimize "On"
        runtime "Release"

    filter "configurations:Dist"
        defines "MADAM_DIST"
        optimize "On"
        runtime "Release"

project "Editor-NoGUI"
    location "Editor-NoGUI"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.hpp",
        "%{prj.name}/**.c",
        "%{prj.name}/**.cpp"
    }

    links
    {
        "kernel32"
    }
    
    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "MADAM_PLATFORM_WINDOWS";
        }

    filter "configurations:Debug"
        defines "MADAM_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "MADAM_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "MADAM_DIST"
        optimize "On"    

project "Editor"
    location "Editor"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    dependson { "Editor-NoGUI" }

    files
    {
        "%{prj.name}/**.h",
        "%{prj.name}/**.hpp",
        "%{prj.name}/**.c",
        "%{prj.name}/**.cpp"
    }

    includedirs
    {
        "IronMaiden/vendors/vulkan/include",
        "IronMaiden/vendors/glfw-3.3.8.bin.WIN64/include",
        "IronMaiden/vendors/glm",
        "IronMaiden/vendors/spdlog/include",
        "IronMaiden/vendors/yaml-cpp/include",
        "IronMaiden/vendors/stb",
        "IronMaiden/vendors/entt/single_include",
        "IronMaiden/vendors/tinyobjloader",
        "IronMaiden/src",
        "IronMaiden"
    }

    libdirs 
    { 
        "C:/Program Files/VulkanSDK/1.3.268.0/Lib",
        "IronMaiden/vendors/glfw-3.3.8.bin.WIN64/lib-vc2022",
        "IronMaiden/vendors/yaml-cpp/Debug"
    }

    links
    {
        "vulkan-1",
        "glfw3",
        "yaml-cppd",
        "kernel32",
        "IronMaiden"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "MADAM_PLATFORM_WINDOWS";
        }

        postbuildcommands
        {
            "call \"compile.bat\""
        }

    filter "configurations:Debug"
        defines "MADAM_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "MADAM_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "MADAM_DIST"
        optimize "On"
