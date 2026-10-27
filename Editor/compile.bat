setlocal
set "current_dir=%~dp0"
if not defined VK_SDK_PATH exit /b 1
cd /d "%VK_SDK_PATH%\Bin" || exit /b 1
glslc.exe "%current_dir%\resources\shaders\simple_shader.vert" -o "%current_dir%\resources\shaders\simple_shader.vert.spv"
glslc.exe "%current_dir%\resources\shaders\simple_shader.frag" -o "%current_dir%\resources\shaders\simple_shader.frag.spv"
glslc.exe "%current_dir%\resources\shaders\shadow_map.vert" -o "%current_dir%\resources\shaders\shadow_map.vert.spv"
glslc.exe "%current_dir%\resources\shaders\point_light.vert" -o "%current_dir%\resources\shaders\point_light.vert.spv"
glslc.exe "%current_dir%\resources\shaders\point_light.frag" -o "%current_dir%\resources\shaders\point_light.frag.spv"
glslc.exe "%current_dir%\resources\shaders\texture_shader.vert" -o "%current_dir%\resources\shaders\texture_shader.vert.spv"
glslc.exe "%current_dir%\resources\shaders\texture_shader.frag" -o "%current_dir%\resources\shaders\texture_shader.frag.spv"
glslc.exe "%current_dir%\resources\shaders\grid_shader.vert" -o "%current_dir%\resources\shaders\grid_shader.vert.spv"
glslc.exe "%current_dir%\resources\shaders\grid_shader.frag" -o "%current_dir%\resources\shaders\grid_shader.frag.spv"
glslc.exe "%current_dir%\resources\shaders\skybox_shader.vert" -o "%current_dir%\resources\shaders\skybox_shader.vert.spv"
glslc.exe "%current_dir%\resources\shaders\skybox_shader.frag" -o "%current_dir%\resources\shaders\skybox_shader.frag.spv"
glslc.exe "%current_dir%\resources\shaders\skybox_shader_1.1.vert" -o "%current_dir%\resources\shaders\skybox_shader_1.1.vert.spv"
glslc.exe "%current_dir%\resources\shaders\skybox_shader_1.1.frag" -o "%current_dir%\resources\shaders\skybox_shader_1.1.frag.spv"
glslc.exe "%current_dir%\resources\shaders\skybox_shader_1.2.vert" -o "%current_dir%\resources\shaders\skybox_shader_1.2.vert.spv"
glslc.exe "%current_dir%\resources\shaders\skybox_shader_1.2.frag" -o "%current_dir%\resources\shaders\skybox_shader_1.2.frag.spv"
