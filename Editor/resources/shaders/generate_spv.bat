setlocal
set "current_dir=%~dp0"
cd "C:\Users\xbox\Desktop\Development\glslang\bin"
glslangValidator.exe -V -x -o "%current_dir%texture_shader.vert.u32" "%current_dir%texture_shader.vert"
glslangValidator.exe -V -x -o "%current_dir%texture_shader.frag.u32" "%current_dir%texture_shader.frag"

glslangValidator.exe -V -x -o "%current_dir%simple_shader.vert.u32" "%current_dir%simple_shader.vert"
glslangValidator.exe -V -x -o "%current_dir%simple_shader.frag.u32" "%current_dir%simple_shader.frag"

glslangValidator.exe -V -x -o "%current_dir%point_light.vert.u32" "%current_dir%point_light.vert"
glslangValidator.exe -V -x -o "%current_dir%point_light.frag.u32" "%current_dir%point_light.frag"

pause