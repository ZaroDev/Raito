project "Core"
kind "StaticLib"
language "C++"
cppdialect "C++20"
targetdir "Binaries/%{cfg.buildcfg}"
staticruntime "off"

pchheader "pch.h"
pchsource "Source/Raito/pch.cpp"

files {"Source/**.h", "Source/**.hpp", "Source/**.c", "Source/**.cpp", "Source/**.cc"}

includedirs {"Source", "Source/Raito", "%{IncludeDir.GLFW}", "%{IncludeDir.DX12TK}", "%{IncludeDir.spdlog}"}
links {"GLFW"}

targetdir("../Binaries/" .. outputdir .. "/%{prj.name}")
objdir("../Binaries/Intermediates/" .. outputdir .. "/%{prj.name}")

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
