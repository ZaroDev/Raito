#include <pch.h>
#include "Assets.h"

#include "Raito/Renderer/Renderer.h"

#include "MeshGenerator.h"

#define CHESS_TEXTURE_SIZE 1024

namespace Raito::Assets
{
	namespace
	{
		std::vector<ubyte> GenerateChessPixels(int width, int height)
		{
			std::vector<ubyte> pixels(width * height * 4); // RGBA

			for (int y = 0; y < height; ++y)
			{
				for (int x = 0; x < width; ++x)
				{
					// Calculate the color based on the position
					if ((x + y) % 2 == 0)
					{
						// Black square
						pixels[(y * width + x) * 3] = 0; // R
						pixels[(y * width + x) * 3 + 1] = 0; // G
						pixels[(y * width + x) * 3 + 2] = 0; // B
					}
					else
					{
						// White square
						pixels[(y * width + x) * 3] = 255; // R
						pixels[(y * width + x) * 3 + 1] = 255; // G
						pixels[(y * width + x) * 3 + 2] = 255; // B
					}
					pixels[(y * width + x) * 3 + 3] = 255;
				}
			}

			return pixels;
		}

		std::vector<ubyte> GenerateTexture(i32 width, i32 height, V3 color)
		{
			std::vector<ubyte> pixels(width * height * 4); // RGBA
			for (int y = 0; y < height; ++y)
			{
				for (int x = 0; x < width; ++x)
				{

					pixels[(y * width + x) * 3] = color.r; // R
					pixels[(y * width + x) * 3 + 1] = color.g; // G
					pixels[(y * width + x) * 3 + 2] = color.b; // B
					pixels[(y * width + x) * 3 + 3] = 255;
				}
			}
			return pixels;
		}

		Texture* g_ChessTexture = nullptr;
		u32 g_DefaultSphereModel;
		u32 g_ChessMaterial;
		Texture* g_WhiteTexture = nullptr;
		Texture* g_BlackTexture = nullptr;
	}

	bool Initialize()
	{
		//const auto sphere = new Mesh(MeshGenerator::GenerateSphere(20.0f, 16));
		//u32 g_DefaultSphereModel = Renderer::AddMesh(sphere);

		////g_ChessMaterial = Renderer::AddMaterial(Renderer::G_BUFFER);

		{
			auto textureData = GenerateChessPixels(CHESS_TEXTURE_SIZE, CHESS_TEXTURE_SIZE);
			g_ChessTexture = new Texture(CHESS_TEXTURE_SIZE, CHESS_TEXTURE_SIZE, 4, textureData.data(), DIFFUSE);
			//Renderer::SetMaterialValue(g_ChessMaterial, "u_Albedo", reinterpret_cast<ubyte*>(&g_ChessTexture->RenderData), sizeof(g_ChessTexture->RenderData));
		}
		{
			auto textureData = GenerateTexture(1, 1, { 255, 255, 255 });
			g_WhiteTexture = new Texture(1, 1, 4, textureData.data(), DIFFUSE);
		}
		{
			auto textureData = GenerateTexture(1, 1, { 0, 0, 0 });
			g_BlackTexture = new Texture(1, 1, 4, textureData.data(), DIFFUSE);
		}


		return true;
	}

	u32 GetDefaultSphere()
	{
		return g_DefaultSphereModel;
	}

	const Texture& GetChessTexture()
	{
		return *g_ChessTexture;
	}

	const Texture& GetWhiteTexture()
	{
		return *g_WhiteTexture;
	}

	const Texture& GetBlackTexture()
	{
		return *g_BlackTexture;
	}

	u32 GetChessMaterial()
	{
		return g_ChessMaterial;
	}
}
