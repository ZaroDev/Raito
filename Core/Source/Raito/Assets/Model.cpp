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
			ent.AddComponent<ECS::MeshComponent>(id, mesh->MaterialId);

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
