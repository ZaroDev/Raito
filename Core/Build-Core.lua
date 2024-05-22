project "Core"
kind "StaticLib"
language "C++"
cppdialect "C++20"
targetdir "Binaries/%{cfg.buildcfg}"
staticruntime "off"

pchheader "pch.h"
pchsource "Source/Raito/pch.cpp"

files {"Source/**.h", "Source/**.hpp", "Source/**.c", "Source/**.cpp", "Source/**.cc"}

includedirs {"Source", "Source/Raito", "%{IncludeDir.GLFW}", "%{IncludeDir.DX12TK}", "%{IncludeDir.spdlog}",
             "%{IncludeDir.glad}", "%{IncludeDir.glm}", "%{IncludeDir.assimp}", "%{IncludeDir.nvrhi}", "%{IncludeDir.DXC}"}

links {"GLFW", "glad", "assimp", "nvrhi"}

targetdir("../Binaries/" .. outputdir .. "/%{prj.name}")
objdir("../Binaries/Intermediates/" .. outputdir .. "/%{prj.name}")

filter "system:windows"
systemversion "latest"
links {"%{Library.DXC}"}
postbuildcommands {"{COPY} \"%{wks.location}/Core/Vendor/DXC/bin/x64/dxil.dll\" \"%{wks.location}Assets\"",
                   "{COPY} \"%{wks.location}/Core/Vendor/DXC/bin/x64/dxcompiler.dll\" \"%{wks.location}Assets\""}

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
