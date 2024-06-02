#include "App.h"
#include "EditorCommon.h"

#include "ImGui/ImGuiEditor.h"

#include "Panels/Panel.h"

#include "Panels/Performance.h"
#include "Panels/Shaders.h"
#include "Panels/Hierarchy.h"

#include "optick/src/optick.h"
#include "Panels/Assets.h"
#include "Panels/Framebuffers.h"

namespace Editor
{

	bool App::OnInit()
	{
		E_LOG("Application Initialized!");
		ImGuiEditor::Init();

		m_Panels.emplace_back(std::make_unique<Performance>());
		m_Panels.emplace_back(std::make_unique<Shaders>());
		m_Panels.emplace_back(std::make_unique<Hierarchy>());
		m_Panels.emplace_back(std::make_unique<Assets>());
		m_Panels.emplace_back(std::make_unique<Framebuffers>());

		Raito::Assets::ImportModel("Meshes/Bricks/bricks.gltf");
		Raito::Assets::ImportModel("Meshes/Sponza/Sponza.gltf");
		Raito::Assets::ImportModel("Meshes/DamagedHelmet/DamagedHelmet.gltf");
		
		// Manually import the fking texture: pavlito
		Raito::Assets::ImportTexture("Meshes/Bricks/bump.jpg", Raito::Assets::TextureType::HEIGHT);

		Raito::Assets::Texture::TextureData tex = Raito::Assets::GetTexture("Meshes/Bricks/bump.jpg")->RenderData;

		Raito::Renderer::SetMaterialValue(0, "u_HeightMap", reinterpret_cast<ubyte*>(&tex), sizeof(tex));

		for (u32 i = 0; i < NUM_POINT; i++)
		{
			Raito::V3 position = Raito::Random::Vec3(-1.f, 1.f);
			Raito::V3 color = Raito::Random::Vec3(0.5f, 1.0f);
			position.y = Raito::Random::Float(0.f, 5.f);

			Raito::ECS::Entity entity = Scene.CreateEntity("Point Light" + std::to_string(i));
			entity.AddComponent<Raito::ECS::LightComponent>(
				Raito::ECS::LightComponent::Type::POINT_LIGHT,
				color,
				2
			);
			
			entity.AddOrReplaceComponent<Raito::ECS::TransformComponent>(
				position,
				Raito::Quaternion{ Raito::V3(0.0f) },
				Raito::V3(0.1f)
			);
		}
		for(u32 i = 0; i< NUM_DIRECTIONAL; i++)
		{
			Raito::ECS::Entity entity = Scene.CreateEntity("Directional light" + std::to_string(i));
			Raito::V3 color = Raito::Random::Vec3(1.0f, 1.0f);
			Raito::V3 direction = Raito::Random::Vec3(1.0, 10.0);

			entity.AddComponent<Raito::ECS::LightComponent>(
				Raito::ECS::LightComponent::Type::DIRECTIONAL,
				color,
				Raito::V3{20, 200, 20}
			);
			const float posX = Raito::Random::Float(-40.f, -20.f);
			const float posZ = Raito::Random::Float(-40.f, -20.f);
			entity.AddOrReplaceComponent<Raito::ECS::TransformComponent>(
				Raito::V3{ posX, 100.0f, posZ },
				Raito::Quaternion{ Raito::V3(0.0f) },
				Raito::V3(1.f)
			);



		}

		return true;
	}

	bool App::OnUpdate()
	{
		OPTICK_CATEGORY("Update App", Optick::Category::Scene);
		for (const auto& p : m_Panels)
		{
			p->Update();
		}

		return m_Running;
	}

	bool App::OnRenderGUI()
	{
		OPTICK_CATEGORY("Update ImGui", Optick::Category::Debug);
		ImGuiEditor::Begin();

		for (const auto& p : m_Panels)
		{
			p->Render();
		}

		ImGuiEditor::End();

		return m_Running;
	}
	void App::OnShutdown()
	{
		ImGuiEditor::Shutdown();
	}
}

CREATE_AND_RUN(Editor::App, Raito::Core::ApplicationInfo("Editor", 1280, 720, Raito::Renderer::API::OPENGL, false))