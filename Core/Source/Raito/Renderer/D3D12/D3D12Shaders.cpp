#include <pch.h>
#include "D3D12Shaders.h"

#include "FileSystem/FileSystem.h"

namespace Raito::Renderer::D3D12::Shaders
{
    namespace
    {
        typedef struct CompiledShader
        {
            uint64_t Size;
            const uint8_t ByteCode;
        } const * CompiledShaderPtr;

        // Each element in this array point to an offset withing the shaders blob.
        CompiledShaderPtr EngineShaders[EngineShader::MAX]{};

        // This is a chunk of memory that contains all compiled engine shaders.
        // The blob is an array of shader byte code consisting of a uint64_t size and
        // an array of bytes.
        std::shared_ptr<uint8_t[]> ShadersBlob{};

        bool LoadEngineShaders()
        {
        	ASSERT(!ShadersBlob);
            uint64_t size = 0;
            
            bool result = true;
            if (!FileSystem::LoadEngineShaders(ShadersBlob, size))
            {
                return false;
            }
            ASSERT(ShadersBlob && size);
            uint64_t offset = 0;
            uint32_t index = 0;

            while (offset < size && result)
            {
            	ASSERT(index < EngineShader::MAX);
                CompiledShaderPtr& shader = EngineShaders[index];
                
                ASSERT(!shader);
                result &= index < EngineShader::MAX && !shader;
                
                if (!result)
                {
                    break;
                }
                shader = reinterpret_cast<const CompiledShaderPtr>(&ShadersBlob[offset]);
                offset += sizeof(uint64_t) + shader->Size;
                ++index;
            }

            ASSERT(offset == size && index == EngineShader::MAX);

            return true;
        }
    }

    bool Initialize()
    {
        return LoadEngineShaders();
    }

    void ShutDown()
    {
        for (uint32_t i = 0; i < EngineShader::MAX; i++)
        {
            EngineShaders[i] = {};
        }
        ShadersBlob.reset();
    }
    D3D12_SHADER_BYTECODE GetEngineShader(EngineShader::ID id)
    {
		ASSERT(id < EngineShader::MAX);

        const CompiledShaderPtr shader = EngineShaders[id];
        ASSERT(shader && shader->Size);

        return { &shader->ByteCode, shader->Size };
    }
}
