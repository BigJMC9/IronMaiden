--Dependencies

VULKAN_SDK = os.getenv("VK_SDK_PATH")

CoreBuildDependencies = {}
CoreBuildDependencies["yaml_cpp"] = "%{wks.location}/IronMaiden/vendors/yaml-cpp"

IncludeDir = {}
IncludeDir["stb"] = "%{wks.location}/IronMaiden/vendors/stb"
IncludeDir["yaml_cpp"] = "%{wks.location}/IronMaiden/vendors/yaml-cpp/include"
IncludeDir["glfw"] = "%{wks.location}/IronMaiden/vendors/glfw-3.3.8.bin.WIN64/include"
IncludeDir["glm"] = "%{wks.location}/IronMaiden/vendors/glm"
IncludeDir["imgui"] = "%{wks.location}/IronMaiden/vendors/imgui"
IncludeDir["imguizmo"] = "%{wks.location}/IronMaiden/vendors/ImGuizmo"
IncludeDir["entt"] = "%{wks.location}/IronMaiden/vendors/entt/single_include"
IncludeDir["tinyobj"] = "%{wks.location}/IronMaiden/vendors/tinyobjloader"
IncludeDir["openfbx"] = "%{wks.location}/IronMaiden/vendors/OpenFBX/src"
IncludeDir["spdlog"] = "%{wks.location}/IronMaiden/vendors/spdlog/include"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/include"

LibDir = {}
LibDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibDir["glfw"] = "%{wks.location}/IronMaiden/vendors/glfw-3.3.8.bin.WIN64/lib-vc2022"
LibDir["yaml_cpp"] = "%{wks.location}/IronMaiden/vendors/yaml-cpp/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/yaml-cpp"
LibDir["imgui"] = "%{wks.location}/IronMaiden/vendors/imgui/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/ImGui"

StaticLib = {}
StaticLib["Vulkan"] = "vulkan-1.lib"
StaticLib["glfw"] = "glfw3_mt.lib"
StaticLib["yaml_cpp"] = "yaml-cpp.lib"
StaticLib["imgui"] = "ImGui.lib"

Lib = {}
Lib["Vulkan"] = "vulkan-1.lib"
Lib["glfw"] = "glfw3.lib"
Lib["yaml_cpp"] = "yaml-cpp.lib"
Lib["imgui"] = "ImGui.lib"