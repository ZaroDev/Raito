project "Core"
kind "StaticLib"
language "C++"
cppdialect "C++20"
targetdir "Binaries/%{cfg.buildcfg}"
staticruntime "off"

pchheader "pch.h"
pchsource "Source/Raito/pch.cpp"

files {"Source/**.h", "Source/**.hpp", "Source/**.c", "Source/**.cpp"}

includedirs {"Source", "Source/Raito",  "Vendor/GLFW/include"}
links{"d3d12.lib","dxgi.lib", "d3dcompiler.lib", "GLFW"}

targetdir("../Binaries/" .. OutputDir .. "/%{prj.name}")
objdir("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

filter "system:windows"
systemversion "latest"
defines {}

filter "configurations:Debug"
defines {"DEBUG"}
runtime "Debug"
symbols "On"

filter "configurations:Release"
defines {"RELEASE"}
runtime "Release"
optimize "On"
symbols "On"

filter "configurations:Dist"
defines {"DIST"}
runtime "Release"
optimize "On"
symbols "Off"
