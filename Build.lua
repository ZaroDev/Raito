-- premake5.lua
workspace "Raito"
architecture "x64"
configurations {"Debug", "Release", "Dist"}
startproject "Editor"

-- Workspace-wide build options for MSVC
filter "system:windows"
buildoptions {"/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus"}

outputdir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

include "Dependencies.lua"

group "Core"
include "Core/Build-Core.lua"
group ""

group "Core/Dependencies"
include "Core/Vendor/GLFW"
include "Core/Vendor/glad"
include "Core/Vendor/assimp"
include "Core/Vendor/D3D12MemoryAllocator"
group ""

group "Editor/Dependencies"
include "Editor/Vendor/ImGui"
group ""

group "Editor"
include "Editor/Build-App.lua"
group ""