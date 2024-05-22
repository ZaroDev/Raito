IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Core/Vendor/GLFW/include"
IncludeDir["DX12TK"] = "%{wks.location}/packages/directxtk12_uwp.2024.1.1.1/include"
IncludeDir["spdlog"] = "%{wks.location}/Core/Vendor/spdlog/include"
IncludeDir["glad"] = "%{wks.location}/Core/Vendor/glad/include"
IncludeDir["glm"] = "%{wks.location}/Core/Vendor/glm"
IncludeDir["ImGui"] = "%{wks.location}/Editor/Vendor/ImGui"
IncludeDir["assimp"] = "%{wks.location}/Core/Vendor/assimp/include"
IncludeDir["nvrhi"] = "%{wks.location}/Core/Vendor/nvrhi/include"
IncludeDir["DXC"] = "%{wks.location}/Core/Vendor/DXC/inc"


LibraryDir = {}
LibraryDir["DXC"] = "%{wks.location}/Core/Vendor/DXC/lib/"
Library = {}
Library["DXC"] = "%{LibraryDir.DXC}/x64/dxcompiler.lib"
