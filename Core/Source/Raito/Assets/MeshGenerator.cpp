#include <pch.h>
#include "MeshGenerator.h"

namespace Raito::Assets::MeshGenerator
{
    std::vector<Vertex> GenerateSphereVertices(float radius, u32 segments)
	{
        std::vector<Vertex> vertices;
        const float PI = 3.14159265359f;


        // Generate vertices on the sphere
        for (int x = 0; x <= segments; ++x) {
            for (int y = 0; y <= segments; ++y) {
                float xSegment = (float)x / (float)segments;
                float ySegment = (float)y / (float)segments;
                float xPos = (std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI)) * radius;
                float yPos = (std::cos(ySegment * PI)) * radius;
                float zPos = (std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI)) * radius;

                vertices.push_back(Vertex{ { xPos, yPos, zPos }, { xPos, yPos, zPos }, { xSegment, ySegment } });
            }
        }

        return vertices;
    }

    std::vector<u32> GenerateSphereIndices(u32 segments)
    {
        std::vector<u32> indices;
        bool oddRow = false;
        // Generate indices for triangles
        for (u32 y = 0; y < segments; y++) {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= segments; ++x)
                {
                    indices.push_back(y * (segments + 1) + x);
                    indices.push_back((y + 1) * (segments + 1) + x);
                }
            }
            else
            {
                for (int x = segments; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (segments + 1) + x);
                    indices.push_back(y * (segments + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        return indices;
    }

    Mesh GenerateSphere(float radius, u32 segments)
    {
        return Mesh(GenerateSphereVertices(radius, segments), GenerateSphereIndices(segments), RenderMode::TRIANGLE_STRIP);
    }
}
