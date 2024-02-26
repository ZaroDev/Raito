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

#include "pch.h"
#include "AssetImport.h"

#include "Model.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Time/ScopedTimer.h"


#define STB_IMAGE_IMPLEMENTATION
#include "Texture.h"
#include "Renderer/Renderer.h"
#include "stb/stb_image.h"

namespace Raito::Assets
{
	namespace
	{
		std::vector<Model*> g_Models{};
		std::unordered_map<std::filesystem::path, Texture*> g_Textures{};
		std::unordered_map<u32, u32> g_Materials{};

		void LoadTexturesOfType(Mesh* m, const std::filesystem::path& path, aiMaterial* material, aiTextureType type)
		{
			for (u32 i = 0; i < material->GetTextureCount(type); i++)
			{
				aiString str;
				material->GetTexture(type, i, &str);
				std::filesystem::path p = path.parent_path();

				p /= str.C_Str();

				ImportTexture(p);
				const u32 materialId = AddMaterial(Renderer::UNSHADED_MESH);
				m->MaterialId = materialId;

				Renderer::SetMaterialValue(materialId, "u_Texture", reinterpret_cast<ubyte*>(&g_Textures[p]->RenderData), sizeof(g_Textures[p]->RenderData));
			}
		}

		Mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
		{
			Mat4 to;

			to[0][0] = from->a1; to[0][1] = from->b1;  to[0][2] = from->c1; to[0][3] = from->d1;
			to[1][0] = from->a2; to[1][1] = from->b2;  to[1][2] = from->c2; to[1][3] = from->d2;
			to[2][0] = from->a3; to[2][1] = from->b3;  to[2][2] = from->c3; to[2][3] = from->d3;
			to[3][0] = from->a4; to[3][1] = from->b4;  to[3][2] = from->c4; to[3][3] = from->d4;

			return to;
		}

		Mesh* ProcessMesh(const std::filesystem::path& path, aiNode* node, aiMesh* mesh, const aiScene* scene, const Mat4& transform)
		{
			std::vector<Vertex> vertices{};
			std::vector<u32> indices{};

			Mesh* m = new Mesh();

			for(u32 i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				vertex.Position.x = mesh->mVertices[i].x;
				vertex.Position.y = mesh->mVertices[i].y;
				vertex.Position.z = mesh->mVertices[i].z;

				vertex.Normal.x = mesh->mNormals[i].x;
				vertex.Normal.y = mesh->mNormals[i].y;
				vertex.Normal.z = mesh->mNormals[i].z;

				if(mesh->mTextureCoords[0])
				{
					vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
					vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
				}
				else
				{
					vertex.TexCoords = V2(0.0f, 0.0f);
				}
				vertices.emplace_back(vertex);
			}

			for(u32 i = 0; i < mesh->mNumFaces; i++)
			{
				const aiFace face = mesh->mFaces[i];
				for(u32 j = 0; j < face.mNumIndices; j++)
				{
					indices.emplace_back(face.mIndices[j]);
				}
			}

			if(mesh->mMaterialIndex >= 0)
			{
				if(g_Materials.contains(mesh->mMaterialIndex))
				{
					m->MaterialId = g_Materials[mesh->mMaterialIndex];
				}
				else
				{
					aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
					LoadTexturesOfType(m, path, material, aiTextureType_DIFFUSE);
				}
			}

			m->Vertices = vertices;
			m->Indices = indices;
			m->Name = node->mName.C_Str();
			m->Transform = transform;


			return m;
		}

		void ProcessNode(const std::filesystem::path& path, std::vector<Mesh*>& meshes, aiNode* node, const aiScene* scene, const Mat4& parent)
		{
			Mat4 transform = aiMatrix4x4ToGlm(&node->mTransformation);
			if (node->mParent)
			{
				transform = transform * aiMatrix4x4ToGlm(&node->mParent->mTransformation);
			}

			// process all the node's meshes (if any)
			for (u32 i = 0; i < node->mNumMeshes; i++)
			{
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				meshes.emplace_back(ProcessMesh(path, node, mesh, scene, transform));
			}
			// then do the same for each of its children
			for (u32 i = 0; i < node->mNumChildren; i++)
			{
				
				ProcessNode(path,meshes, node->mChildren[i], scene, transform);
			}
		}
	}
	u32 ImportModel(const std::filesystem::path& filePath)
	{
		Assimp::Importer importer;

		ScopedTimer timer("Import asset");

		const aiScene* scene = importer.ReadFile(filePath.string().c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);

		if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			ERR("Model importer", "Assimp error {0}", importer.GetErrorString());
			return 0;
		}

		std::vector<Mesh*> meshes;

		ProcessNode(filePath, meshes, scene->mRootNode, scene, Mat4(1.0f));

		g_Models.emplace_back(new Model(meshes));

		return static_cast<u32>(g_Models.size()) - 1;
	}

	void ImportTexture(const std::filesystem::path& filePath)
	{
		if(g_Textures.contains(filePath) && g_Textures[filePath] != nullptr)
		{
			LOG("Textures", "Texture {0} import skipped", filePath.string());
			return;
		}

		i32 width, height, nChannels;
		ubyte* data = stbi_load(filePath.string().c_str(), &width, &height, &nChannels, 0);

		auto* texture = new Texture(width, height, data);
		g_Textures[filePath] = texture;

		stbi_image_free(data);
	}

	const std::unordered_map<std::filesystem::path, Texture*>& GetAllTextures()
	{
		return g_Textures;
	}
}
