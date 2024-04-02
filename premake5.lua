--Config

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

include("dpc.lua")

group("Dependencies")
    include("IronMaiden/vendors/yaml-cpp")

group("")

project "IronMaiden"
    location "IronMaiden"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

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
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.tinyobj}",
        "%{IncludeDir.openfbx}",
    }

    libdirs 
    { 
        "%{LibDir.VulkanSDK}",
        "%{LibDir.glfw}",
        "%{LibDir.yaml_cpp}"
    }

    links
    {
        "%{Lib.Vulkan}",
        "%{Lib.glfw}",
        "%{Lib.yaml_cpp}"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "_CRT_SECURE_NO_WARNINGS";
            "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS";
            "MADAM_PLATFORM_WINDOWS";
            "MADAM_BUILD_DLL";
        }

    filter "configurations:Debug"
        defines "MADAM_DEBUG"
        symbols "on"
        runtime "debug"

    filter "configurations:Release"
        defines "MADAM_RELEASE"
        optimize "on"
        runtime "release"

    filter "configurations:Dist"
        defines "MADAM_DIST"
        optimize "on"
        runtime "release"

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
        staticruntime "on"
        systemversion "latest"

        defines
        {
            "_CRT_SECURE_NO_WARNINGS";
            "MADAM_PLATFORM_WINDOWS";
        }

    filter "configurations:Debug"
        defines "MADAM_DEBUG"
        symbols "on"

    filter "configurations:Release"
        defines "MADAM_RELEASE"
        optimize "on"

    filter "configurations:Dist"
        defines "MADAM_DIST"
        optimize "on"    

project "Editor"
    location "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

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
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.spdlog}",
        --"IronMaiden/vendors/yaml-cpp/include",
        --"IronMaiden/vendors/stb",
        "%{IncludeDir.entt}",
        --"IronMaiden/vendors/tinyobjloader",
        --"IronMaiden/vendors/OpenFBX/src",
        "IronMaiden/src",
        "IronMaiden"
    }

    links
    {
        "IronMaiden"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "_CRT_SECURE_NO_WARNINGS";
            "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS";
            "MADAM_PLATFORM_WINDOWS";
        }

        postbuildcommands
        {
            "call \"compile.bat\""
        }

    filter "configurations:Debug"
        defines "MADAM_DEBUG"
        symbols "on"

    filter "configurations:Release"
        defines "MADAM_RELEASE"
        optimize "on"

    filter "configurations:Dist"
        defines "MADAM_DIST"
        optimize "on"
