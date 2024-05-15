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
            std::vector<ubyte> pixels(width * height * 3); // RGBA

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
                }
            }

            return pixels;
        }

        Texture* g_ChessTexture = nullptr;
        u32 g_DefaultSphereModel;
        u32 g_ChessMaterial;
	}

    bool Initialize()
    {
        const auto sphere = new Mesh(MeshGenerator::GenerateSphere(0.1f, 16));
        u32 g_DefaultSphereModel = Renderer::AddMesh(sphere);

        g_ChessMaterial = Renderer::AddMaterial(Renderer::G_BUFFER);


        auto textureData = GenerateChessPixels(CHESS_TEXTURE_SIZE, CHESS_TEXTURE_SIZE);
        g_ChessTexture = new Texture(CHESS_TEXTURE_SIZE, CHESS_TEXTURE_SIZE, 3, textureData.data(), DIFFUSE);
        Renderer::SetMaterialValue(g_ChessMaterial, "u_Albedo", reinterpret_cast<ubyte*>(&g_ChessTexture->RenderData), sizeof(g_ChessTexture->RenderData));

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

    u32 GetChessMaterial()
    {
        return g_ChessMaterial;
    }
}
