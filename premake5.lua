--Config

workspace "IronMaiden"
    architecture "x64"

    configurations
    {
        "Debug",
        "DebugDLL",
        "Release",
        "ReleaseDLL",
        "Dist"
    }
    startproject "Editor"
    startproject "Editor-NoGUI"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include("dpc.lua")

group("Dependencies")
    include("IronMaiden/vendors/yaml-cpp")
    include("IronMaiden/vendors/imgui")

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
        "%{prj.name}/**.cpp",
    }

    removefiles 
    { 
        "%{prj.name}/vendors/**"
    }
    
    files 
    {
        "%{prj.name}/vendors/ImGuizmo/ImGuizmo.h",
        "%{prj.name}/vendors/ImGuizmo/ImGuizmo.cpp"
    }

    includedirs
    {
        "%{prj.name}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.tinyobj}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.openfbx}",
        "%{IncludeDir.imguizmo}"
    }

    libdirs 
    { 
        "%{LibDir.VulkanSDK}",
        "%{LibDir.glfw}",
        "%{LibDir.yaml_cpp}",
        "%{LibDir.imgui}",
    }

    filter "files:IronMaiden/vendors/ImGuizmo/**.cpp"
    flags{ "NoPCH" }

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
        links
        {
            "%{StaticLib.Vulkan}",
            "%{StaticLib.glfw}",
            "%{StaticLib.yaml_cpp}",
            "%{StaticLib.imgui}",
        }
    
    filter "configurations:DebugDLL"
        defines "MADAM_DEBUG"
        symbols "on"
        staticruntime "off"
        runtime "debug"
        kind "SharedLib"
        links
        {
            "%{Lib.Vulkan}",
            "%{Lib.glfw}",
            "%{Lib.yaml_cpp}",
            "%{Lib.imgui}",
        }
        defines "MADAM_DYNAMIC_LINK"
        defines "IMGUI_API"

        postbuildcommands {
            "copy %{wks.location}bin\\" .. outputdir .. "\\IronMaiden\\IronMaiden.dll %{wks.location}bin\\" .. outputdir .. "\\Editor\\",
        }

    filter "configurations:Release"
        defines "MADAM_RELEASE"
        optimize "on"
        runtime "release"
        links
        {
            "%{StaticLib.Vulkan}",
            "%{StaticLib.glfw}",
            "%{StaticLib.yaml_cpp}",
            "%{StaticLib.imgui}",
        }

    filter "configurations:ReleaseDLL"
        optimize "on"
        staticruntime "off"
        runtime "debug"
        kind "SharedLib"
        links
        {
            "%{Lib.Vulkan}",
            "%{Lib.glfw}",
            "%{Lib.yaml_cpp}",
            "%{Lib.imgui}",
            "dwmapi.lib",
        }

        postbuildcommands {
            "copy %{wks.location}bin\\" .. outputdir .. "\\IronMaiden\\IronMaiden.dll %{wks.location}bin\\" .. outputdir .. "\\Editor\\",
        }
        defines "MADAM_DYNAMIC_LINK"
        defines "IMGUI_API"


    filter "configurations:Dist"
        defines "MADAM_DIST"
        optimize "on"
        runtime "release"
        links
        {
            "%{StaticLib.Vulkan}",
            "%{StaticLib.glfw}",
            "%{StaticLib.yaml_cpp}"
        }

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
        "%{IncludeDir.imgui}",
        "IronMaiden/src",
        "IronMaiden"
    }

    libdirs 
    { 
        "bin/" .. outputdir .. "/IronMaiden",
        "%{LibDir.yaml_cpp}",
    }

    links
    {
        "IronMaiden.lib"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "_CRT_SECURE_NO_WARNINGS";
            "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS";
            "MADAM_PLATFORM_WINDOWS"; 
        }

        prebuildcommands
        {
            "copy %{wks.location}bin\\" .. outputdir .. "\\IronMaiden\\IronMaiden.dll %{wks.location}bin\\" .. outputdir .. "\\Editor\\",
            "copy %{wks.location}IronMaiden\\vendors\\yaml-cpp\\bin\\" .. outputdir .. "\\yaml-cpp\\yaml-cpp.dll %{wks.location}bin\\" .. outputdir .. "\\Editor\\",
        }

        postbuildcommands
        {
            "call \"compile.bat\""
        }

    filter "configurations:Debug"
        defines "MADAM_DEBUG"
        symbols "on"
        defines "YAML_CPP_STATIC_DEFINE"
    
    filter "configurations:DebugDLL"
        defines "MADAM_DEBUG"
        symbols "on"
        staticruntime "off"
        defines { 
            "YAML_CPP_DLL";
            "MADAM_DYNAMIC_LINK";
        }

    filter "configurations:ReleaseDLL"
        optimize "on"
        staticruntime "off"
        defines { 
            "YAML_CPP_DLL";
            "MADAM_DYNAMIC_LINK";
        }

    filter "configurations:Release"
        defines "MADAM_RELEASE"
        optimize "on"
        defines "YAML_CPP_STATIC_DEFINE"

    filter "configurations:Dist"
        defines "MADAM_DIST"
        optimize "on"
        defines "YAML_CPP_STATIC_DEFINE"

