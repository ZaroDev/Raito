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

		Mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
		{
			Mat4 to;

			to[0][0] = from->a1; to[0][1] = from->b1;  to[0][2] = from->c1; to[0][3] = from->d1;
			to[1][0] = from->a2; to[1][1] = from->b2;  to[1][2] = from->c2; to[1][3] = from->d2;
			to[2][0] = from->a3; to[2][1] = from->b3;  to[2][2] = from->c3; to[2][3] = from->d3;
			to[3][0] = from->a4; to[3][1] = from->b4;  to[3][2] = from->c4; to[3][3] = from->d4;

			return to;
		}

		Mesh* ProcessMesh(aiNode* node, aiMesh* mesh, const aiScene* scene, const Mat4& transform)
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
			m->Name = node->mName.C_Str();
			m->Transform = transform;


			return m;
		}

		void ProcessNode(std::vector<Mesh*>& meshes, aiNode* node, const aiScene* scene, const Mat4& parent)
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
				meshes.emplace_back(ProcessMesh(node, mesh, scene, transform));
			}
			// then do the same for each of its children
			for (u32 i = 0; i < node->mNumChildren; i++)
			{
				
				ProcessNode(meshes, node->mChildren[i], scene, transform);
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

		ProcessNode(meshes, scene->mRootNode, scene, Mat4(1.0f));

		g_Models.emplace_back(new Model(meshes));

		return (u32)g_Models.size() - 1;
	}
}
