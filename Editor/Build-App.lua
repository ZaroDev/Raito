project "Editor"
language "C++"
cppdialect "C++20"
targetdir "Binaries/%{cfg.buildcfg}"
staticruntime "off"

files {"Source/**.h", "Source/**.cpp", "Vendor/ImGuizmo/ImGuizmo.h", "Vendor/ImGuizmo/ImGuizmo.cpp"
 }


includedirs {"Source", -- Include Core
"../Core/Source", "%{IncludeDir.spdlog}", "%{IncludeDir.glm}", "%{IncludeDir.ImGui}", "%{IncludeDir.GLFW}",
             "%{IncludeDir.assimp}", "%{IncludeDir.ImGuizmo}"
            }

links {"Core", "ImGui"}

targetdir("%{wks.location}/Binaries/" .. outputdir .. "/%{prj.name}")
objdir("%{wks.location}/Binaries/Intermediates/" .. outputdir .. "/%{prj.name}")
debugdir("%{wks.location}/Assets")

filter "system:windows"
systemversion "latest"
defines {"WINDOWS"}
postbuildcommands {("{COPY} \"%{cfg.buildtarget.relpath}\" \"%{wks.location}Assets\"")}

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
