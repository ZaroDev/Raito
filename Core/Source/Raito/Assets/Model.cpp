#include "pch.h"
#include "Model.h"

#include "Core/Application.h"

#include "Renderer/Renderer.h"

#include "ECS/Entity.h"

namespace Raito::Assets
{
	Model::Model(const std::vector<Mesh*>& meshes)
		
	{
		for(auto& mesh : meshes)
		{
			u32 id = Renderer::AddMesh(mesh);
			m_Meshes.emplace_back(id);


			// TODO: Create materials per mesh
			auto& scene = Core::Application::Get().Scene;

			ECS::Entity ent = scene.CreateEntity(mesh->Name);

			ent.AddOrReplaceComponent<ECS::TransformComponent>(mesh->Transform);
			ent.AddComponent<ECS::MeshComponent>(id);

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
