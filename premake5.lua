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
    include("IronMaiden/vendors/spdlog")

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
    
    dependson{"ImGui", "yaml-cpp", "spdlog"}


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
        "%{LibDir.spdlog}"
    }

    links
    {
        "%{Lib.Vulkan}",
        "%{Lib.glfw}",
        "%{Lib.imgui}",
        "%{Lib.spdlog}"
    }

    filter "action:vs*"
        buildoptions {"/utf-8"}

    filter "files:IronMaiden/vendors/ImGuizmo/**.cpp"
    flags{ "NoPCH" }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "_CRT_SECURE_NO_WARNINGS";
            "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS";
            "MADAM_PLATFORM_WINDOWS";
            "SPDLOG_COMPILED_LIB";
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

project "Editor"
    location "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    dependson {"IronMaiden"}

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
        "%{LibDir.spdlog}",
		"%{LibDir.VulkanSDK}",
    }

    links
    {
        "IronMaiden.lib",
        "%{Lib.yaml_cpp}",
        "%{Lib.spdlog}"
    }

    filter "action:vs*"
        buildoptions {"/utf-8"}

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "_CRT_SECURE_NO_WARNINGS";
            "_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS";
            "MADAM_PLATFORM_WINDOWS";
            "SPDLOG_COMPILED_LIB";
        }

        postbuildcommands
        {
            "call \"compile.bat\"",
			"copy /Y \"%{VULKAN_SDK}\\Bin\\shaderc_shared.dll\" \"%{cfg.targetdir}\\\"",
			"copy /Y \"%{VULKAN_SDK}\\Bin\\shaderc_sharedd.dll\" \"%{cfg.targetdir}\\\"",
            "C:\\Windows\\System32\\xcopy \"resources\\*\" \"%{cfg.targetdir}\\resources\" /E /I /Y",
			"dotnet build \"Managed\\Maiden.ScriptHost\\Maiden.ScriptHost.csproj\" --configuration Release --output \"%{cfg.targetdir}\\Managed\" --nologo --verbosity quiet"
        }

    filter "configurations:Debug"
        symbols "on"
        staticruntime "off"
        links { "shaderc_sharedd.lib" }
        defines { 
            "YAML_CPP_DLL";
            "MADAM_DEBUG";
        }

    filter "configurations:Release"
        optimize "on"
        links { "shaderc_shared.lib" }
        defines {
            "YAML_CPP_DLL";
            "MADAM_RELEASE";
        }

    filter "configurations:Dist"
        optimize "on"
        symbols "off"
        links { "shaderc_shared.lib" }
        defines {
            "YAML_CPP_DLL";
            "MADAM_DIST";
        }

group("Tests")
project "AssetTests"
    location "Tests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    files { "Tests/**.cpp", "Editor/src/Scripting/ScriptEngine.cpp" }
    includedirs {
        "IronMaiden/src",
        "IronMaiden",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.imgui}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
		"%{IncludeDir.imguizmo}"
    }
    libdirs {
        "bin/" .. outputdir .. "/IronMaiden",
        "%{LibDir.yaml_cpp}",
        "%{LibDir.spdlog}",
        "%{LibDir.VulkanSDK}",
        "%{LibDir.glfw}",
        "%{LibDir.imgui}"
    }
    links {
        "IronMaiden.lib",
        "%{Lib.yaml_cpp}",
        "%{Lib.spdlog}",
        "%{Lib.Vulkan}",
        "%{Lib.glfw}",
        "%{Lib.imgui}"
    }
    postbuildcommands {
        "dotnet build \"../Editor/Managed/Maiden.ScriptHost/Maiden.ScriptHost.csproj\" --configuration Release --output \"%{cfg.targetdir}\\Managed\" --nologo --verbosity quiet"
    }
    filter "action:vs*"
        buildoptions { "/utf-8" }
    filter "system:windows"
        systemversion "latest"
        defines { "MADAM_PLATFORM_WINDOWS", "SPDLOG_COMPILED_LIB", "_CRT_SECURE_NO_WARNINGS" }
    filter "configurations:Debug"
        symbols "on"
        defines { "MADAM_DEBUG", "YAML_CPP_DLL" }
		links { "shaderc_sharedd.lib" }
    filter "configurations:Release"
        optimize "on"
        defines { "MADAM_RELEASE", "YAML_CPP_DLL" }
		links { "shaderc_shared.lib" }
    filter "configurations:Dist"
        optimize "on"
        defines { "MADAM_DIST", "YAML_CPP_DLL" }
		links { "shaderc_shared.lib" }
