if exist "bin" (
    rmdir /s /q "bin"
)
if exist "bin-int" (
    rmdir /s /q "bin-int"
)
if exist ".vs" (
    rmdir /s /q ".vs"
)
if exist "IronMaiden/vendors/imgui/bin" (
    rmdir /s /q "IronMaiden/vendors/imgui/bin"
)
if exist "IronMaiden/vendors/imgui/bin-int" (
    rmdir /s /q "IronMaiden/vendors/imgui/bin-int"
)
if exist "IronMaiden/vendors/yaml-cpp/bin" (
    rmdir /s /q "IronMaiden/vendors/yaml-cpp/bin"
)
if exist "IronMaiden/vendors/yaml-cpp/bin-int" (
    rmdir /s /q "IronMaiden/vendors/yaml-cpp/bin-int"
)
call vendors\bin\premake\premake5.exe vs2022
PAUSE