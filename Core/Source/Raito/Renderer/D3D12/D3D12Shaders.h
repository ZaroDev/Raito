#pragma once

#include "D3D12Common.h"
#undef DOMAIN

namespace Raito::Renderer::D3D12::Shaders
{
    struct ShaderType
    {
        enum Type : uint32_t
        {
            VERTEX = 0,
            HULL,
            DOMAIN,
            GEOMETRY,
            PIXEL,
            COMPUTE,
            AMPLIFICATION,
            MESH,
            
            MAX
        };
    };

    struct EngineShader {
        enum ID : uint32_t
        {
            FULLSCREEN_TRIANGLE = 0,
            FILL_COLOR_PS,
            MAX
        };
    };
    bool Initialize();
    void ShutDown();

    D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id);
}