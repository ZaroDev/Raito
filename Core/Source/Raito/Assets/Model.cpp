#include "pch.h"
#include "Model.h"


#include "Renderer/Renderer.h"

namespace Raito::Assets
{
	Model::Model(const std::vector<Mesh*>& meshes)
		
	{
		for(auto& mesh : meshes)
		{
			u32 id = Renderer::AddMesh(mesh);
			m_Meshes.emplace_back(id);

			// Since we don't need the mesh data anymore we just delete the whole mesh
			delete mesh;
		}

	}

	Model::~Model()
	{
		for (const u32 mesh : m_Meshes)
		{
			Renderer::RemoveMesh(mesh);
		}

		m_Meshes.clear();
	}
}
