project "Editor"
language "C++"
cppdialect "C++20"
targetdir "Binaries/%{cfg.buildcfg}"
staticruntime "off"

files {"Source/**.h", "Source/**.cpp"}

includedirs {"Source", -- Include Core
"../Core/Source"}

links {"Core"}

targetdir("../Binaries/" .. outputdir .. "/%{prj.name}")
objdir("../Binaries/Intermediates/" .. outputdir .. "/%{prj.name}")

filter "system:windows"
systemversion "latest"
defines {"WINDOWS"}

filter "configurations:Debug"
kind "ConsoleApp"
defines {"DEBUG"}
runtime "Debug"
symbols "On"

filter "configurations:Release"
kind "ConsoleApp"
defines {"RELEASE"}
runtime "Release"
optimize "On"
symbols "On"

filter "configurations:Dist"
kind "WindowedApp"
defines {"DIST"}
runtime "Release"
optimize "On"
symbols "Off"
