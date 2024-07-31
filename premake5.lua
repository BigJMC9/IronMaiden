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
    staticruntime "off"

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
        "%{IncludeDir.imguizmo}",
        "%{IncludeDir.shaderc}"
    }

    libdirs 
    { 
        "%{LibDir.VulkanSDK}",
        "%{LibDir.glfw}",
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
        }
        
    filter "configurations:Debug"
        defines "MADAM_DEBUG"
        symbols "on"
        runtime "debug"
        links
        {
            "%{Lib.Vulkan}",
            "%{Lib.glfw}",
            "%{Lib.imgui}"
        }

    filter "configurations:Release"
        defines "MADAM_RELEASE"
        optimize "on"
        runtime "release"
        links
        {
            "%{Lib.Vulkan}",
            "%{Lib.glfw}",
            "%{Lib.imgui}"
        }

    filter "configurations:Dist"
        defines "MADAM_DIST"
        optimize "on"
        runtime "release"
        links
        {
            "%{Lib.Vulkan}",
            "%{Lib.glfw}",
            "%{Lib.imgui}"
        }  

project "Editor"
    location "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

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
        "%{IncludeDir.imguizmo}",
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
        "IronMaiden.lib",
        "%{Lib.yaml_cpp}"
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
            "copy %{wks.location}IronMaiden\\vendors\\yaml-cpp\\bin\\" .. outputdir .. "\\yaml-cpp\\yaml-cpp.dll %{wks.location}bin\\" .. outputdir .. "\\Editor\\",
        }

        postbuildcommands
        {
            "call \"compile.bat\"",
            "C:\\Windows\\System32\\xcopy \"%{wks.location}Editor\\resources\\\" \"%{wks.location}bin\\" .. outputdir .. "\\Editor\\resources\\\" /E /I /Y",
            "C:\\Windows\\System32\\xcopy \"%{wks.location}Editor\\logs\\\" \"%{wks.location}bin\\" .. outputdir .. "\\Editor\\logs\\\" /E /I /Y"
        }

    filter "configurations:Debug"
        symbols "on"
        staticruntime "off"
        defines { 
            "YAML_CPP_DLL";
            "MADAM_DEBUG";
        }

    filter "configurations:Release"
        optimize "on"
        defines {
            "YAML_CPP_DLL";
            "MADAM_RELEASE";
        }

    filter "configurations:Dist"
        optimize "on"
        symbols "off"
        defines {
            "YAML_CPP_DLL";
            "MADAM_DIST";
        }