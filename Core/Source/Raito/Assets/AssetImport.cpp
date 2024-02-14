#include "pch.h"
#include "AssetImport.h"

#include "Model.h"
#include "Mesh.h"
#include "Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Time/ScopedTimer.h"

namespace Raito::Assets
{
	namespace
	{
		std::vector<Model*> g_Models{};

		Mesh* ProcessMesh(aiNode* node, aiMesh* mesh, const aiScene* scene)
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

				// TODO: Import material data

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

			m->Vertices = vertices;
			m->Indices = indices;

			auto transform = node->mTransformation * node->mParent->mTransformation;
			m->Transform = Mat4(transform.a1, transform.a2, transform.a3, transform.a4,
								transform.b1, transform.b2, transform.b3, transform.b4,
								transform.c1, transform.c2, transform.c3, transform.c4,
								transform.d1, transform.d2, transform.d3, transform.d4);


			return m;
		}

		void ProcessNode(std::vector<Mesh*>& meshes, aiNode* node, const aiScene* scene)
		{
			// process all the node's meshes (if any)
			for (u32 i = 0; i < node->mNumMeshes; i++)
			{
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				meshes.emplace_back(ProcessMesh(node, mesh, scene));
			}
			// then do the same for each of its children
			for (u32 i = 0; i < node->mNumChildren; i++)
			{
				ProcessNode(meshes, node->mChildren[i], scene);
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

		ProcessNode(meshes, scene->mRootNode, scene);

		g_Models.emplace_back(new Model(meshes));

		return (u32)g_Models.size() - 1;
	}
}
