/*
MIT License

Copyright (c) 2023 Víctor Falcón Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
// Includes for Raito apps
#include <Raito/Core/Log.h>
#include <Raito/Core/BasicTypes.h>
#include <Raito/Core/Assert.h>
#include <Raito/Core/Common.h>
#include <Raito/Core/UUID.h>
#include <Raito/Core/Application.h>
#include <Raito/Time/Time.h>
#include <Raito/Time/ScopedTimer.h>
#include <Raito/Input/Input.h>
#include <Raito/Input/KeyCodes.h>
// ECS
#include <Raito/ECS/Scene.h>
#include <Raito/ECS/Entity.h>
#include <Raito/ECS/Components.h>

// Math
#include <Raito/Math/Math.h>

#include <Windows.h>

// Renderer
#include <Raito/Renderer/GraphicsAPI.h>
#include <Raito/Renderer/RHI.h>
#include <Raito/Renderer/Renderer.h>
#include <Raito/Renderer/Shader.h>
#include <Raito/Renderer/Camera.h>

// Window
#include <Raito/Window/Window.h>

// Assets
#include <Raito/Assets/Mesh.h>
#include <Raito/Assets/Model.h>
#include <Raito/Assets/AssetImport.h>
#include <Raito/Assets/Assets.h>

// Random
#include <Raito/Random/Random.h>

constexpr u32 NUM_POINT = 512;
constexpr u32 NUM_DIRECTIONAL = 1;
