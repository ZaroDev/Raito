#include <pch.h>
#include "D3D12ShaderCompilation.h"

#include <Raito/Renderer/GraphicsAPI.h>

#include <d3d12shader.h>
#include <dxcapi.h>

#include <Raito/Renderer/D3D12/D3D12Core.h>
#include <Raito/Renderer/D3D12/D3D12Shaders.h>

using namespace Microsoft::WRL;

namespace Raito::Renderer::D3D12::Shaders
{
	namespace
	{
		struct ShaderFileInfo
		{
			const char* File;
			const char* Function;
			EngineShader::ID ID;
			ShaderType::Type Type;
		};

		constexpr ShaderFileInfo ShaderFiles[]
		{
			{"VertexShader.hlsl", "VSMain", EngineShader::VERTEX_SHADER, ShaderType::VERTEX},
			{"PixelShader.hlsl", "PSMain", EngineShader::PIXEL_SHADER, ShaderType::PIXEL},
		};
		
		static_assert(_countof(ShaderFiles) == EngineShader::MAX);

		constexpr const char* ShadersSourceFiles = "Shaders\\D3D12\\";

		std::wstring ToWString(std::string str)
		{
			std::string s { str};
			return  { s.begin(), s.end()};
		}
		
		class ShaderCompiler
		{
		public:
			ShaderCompiler()
			{
				HRESULT hr = S_OK;
				DXCall(hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_Compiler)))
				if(FAILED(hr))
					return;

				DXCall(hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_Utils)))
				if(FAILED(hr))
					return;

				DXCall(hr = m_Utils->CreateDefaultIncludeHandler(&m_IncludeHandler));
				if(FAILED(hr))
					return;
			}
			DISABLE_COPY(ShaderCompiler)
			DISABLE_MOVE(ShaderCompiler)
			
			IDxcBlob* Compile(ShaderFileInfo info, const std::filesystem::path& fullPath)
			{
				ASSERT(m_Compiler && m_Utils && m_IncludeHandler);
				HRESULT hr = S_OK;

				// Load the source file using utils interface
				ComPtr<IDxcBlobEncoding> sourceBlob = nullptr;
				DXCall(hr = m_Utils->LoadFile(fullPath.c_str(), nullptr, &sourceBlob));
				if(FAILED(hr))
					return nullptr;

				ASSERT(sourceBlob && sourceBlob->GetBufferSize());

				std::wstring file = ToWString(info.File);
				std::wstring func = ToWString(info.Function);
				std::wstring prof = ToWString(m_ProfileStrings[static_cast<uint32_t>(info.Type)]);
				std::wstring incl = ToWString(ShadersSourceFiles);

				LPCWSTR args[]
				{
					file.c_str(),			// Optional shader source file name for error reporting
					L"-E", func.c_str(),	// Entry function
					L"-T", prof.c_str(),	// Target profile
					L"-I", incl.c_str(),	// Include path
					DXC_ARG_ALL_RESOURCES_BOUND,
#if DEBUG
					DXC_ARG_DEBUG,
					DXC_ARG_SKIP_OPTIMIZATIONS,
#else
					DXC_ARG_OPTIMIZATION_LEVEL3,
#endif
					DXC_ARG_WARNINGS_ARE_ERRORS,
					L"-Qstrip_reflect",		// Strip reflections into a separate blob
					L"-Qstrip_debug",		// Strip debug information into a separate blob
				};
				
				return Compile(sourceBlob.Get(), args, _countof(args));
			}
			IDxcBlob* Compile(IDxcBlobEncoding* sourceBlob, LPCWSTR* args, uint32_t numArgs)
			{
				DxcBuffer buffer{};
				buffer.Encoding = DXC_CP_ACP; // Auto-detect text format, I guess?????
				buffer.Ptr = sourceBlob->GetBufferPointer();
				buffer.Size = sourceBlob->GetBufferSize();

				HRESULT hr = S_OK;
				ComPtr<IDxcResult> results = nullptr;
				DXCall(hr = m_Compiler->Compile(&buffer, args, numArgs, m_IncludeHandler.Get(), IID_PPV_ARGS(&results)));
				if(FAILED(hr))
					return nullptr;

				ComPtr<IDxcBlobUtf8> errors = nullptr;
				DXCall(hr = results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
				if(FAILED(hr))
					return nullptr;

				if(errors && errors->GetStringLength())
				{
					D_ERROR("Shader compilation error {}", errors->GetStringPointer());
				}
				else
				{
					D_LOG("Compilation succeeded!");
				}

				HRESULT status = S_OK;
				DXCall(hr = results->GetStatus(&status))
				if(FAILED(hr) || FAILED(status))
					return nullptr;

				ComPtr<IDxcBlob> shader = nullptr;
				DXCall(hr = results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader), nullptr));
				if(FAILED(hr))
					return nullptr;

				return shader.Detach();
			}
		private:
			// NOTE: Shader Model
			constexpr static const char*		m_ProfileStrings[]{ "vs_6_5", "hs_6_5", "ds_6_5", "gs_6_5", "ps_6_5", "cs_6_5", "as_6_5", "ms_6_5"};
			static_assert(_countof(m_ProfileStrings) == ShaderType::MAX);

			ComPtr<IDxcCompiler3>				m_Compiler = nullptr;
			ComPtr<IDxcUtils>					m_Utils = nullptr;
			ComPtr<IDxcIncludeHandler>			m_IncludeHandler = nullptr;
			
		};
		
		decltype(auto) GetEngineShadersPath()
		{
			return std::filesystem::absolute(Renderer::GetEngineShadersPath(API::D3D12));
		}
		bool CompiledShadersAreUpToDate()
		{
			// Get the path to the compiled shaders binary.
			auto engineShadersPath = GetEngineShadersPath();
			if (!std::filesystem::exists(engineShadersPath))
				return false;

			auto shadersCompilationTime = std::filesystem::last_write_time(engineShadersPath);

			std::filesystem::path path{};
			std::filesystem::path fullPath{};

			// Check if either of engine shader source files is newer than the compiled shader file.
			// In that case, we need to recompile.

			for (u32 i = 0; i < EngineShader::MAX; i++)
			{
				auto& info = ShaderFiles[i];

				path = ShadersSourceFiles;
				path += info.File;

				fullPath = std::filesystem::absolute(path);
				if (!std::filesystem::exists(fullPath))
					return false;

				auto shaderFileTime = std::filesystem::last_write_time(fullPath);
				if (shaderFileTime > shadersCompilationTime)
				{
					return false;
				}
			}
			return true;
		}
		bool SaveCompiledShaders(std::vector<ComPtr<IDxcBlob>> & shaders)
		{
			auto engineShadersPath = GetEngineShadersPath();
			std::filesystem::create_directories(engineShadersPath.parent_path());
			std::ofstream file(engineShadersPath, std::ios::out | std::ios::binary);
			if (!file || !std::filesystem::exists(engineShadersPath))
			{
				file.close();
				D_ERROR("Failed to save shader, file doesn't exists");
				return false;
			}

			for (auto& shader : shaders)
			{
				const D3D12_SHADER_BYTECODE byteCode{ shader->GetBufferPointer(), shader->GetBufferSize() };
				file.write((char*)&byteCode.BytecodeLength, sizeof(byteCode.BytecodeLength));
				file.write((char*)byteCode.pShaderBytecode, byteCode.BytecodeLength);
			}

			file.close();
			return true;
		}
	}
	bool CompileShaders()
	{
		if (CompiledShadersAreUpToDate())
			return true;

		std::vector<ComPtr<IDxcBlob>> shaders;

		std::filesystem::path path{};
		std::filesystem::path fullPath{};

		ShaderCompiler compiler;
		
		//Compile shaders and put them together in a buffer in the same order of compilation.
		for (uint32_t i = 0; i < EngineShader::MAX; i++)
		{
			auto& info = ShaderFiles[i];

			path = ShadersSourceFiles;
			path += info.File;
			fullPath = std::filesystem::absolute(path);
			if (!std::filesystem::exists(fullPath))
			{
				D_ERROR("Shader file at {} doesn't exist", fullPath.string().c_str());
				return false;
			}
			ComPtr<IDxcBlob> compiledShader = compiler.Compile(info, fullPath);
			if (compiledShader && compiledShader->GetBufferPointer() && compiledShader->GetBufferSize())
			{
				shaders.emplace_back(std::move(compiledShader));
			}
			else
			{
				return false;
			}
		}

		return SaveCompiledShaders(shaders);
	}
}