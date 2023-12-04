-- premake5.lua
workspace "Raito"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "Editor"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Core"
	include "Core/Build-Core.lua"
group ""

group "Core/Dependencies"
   include "Core/Vendor/GLFW"
group ""

include "Editor/Build-App.lua"