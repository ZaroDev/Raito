#include "pch.h"
#include "AssetImport.h"

#include "Model.h"
#include "Mesh.h"
#include "Material.h"

namespace Raito::Assets
{
	namespace
	{
		std::vector<std::shared_ptr<Model>> Models{};
	}
	u32 ImportModel(const std::filesystem::path& filePath)
	{
		std::shared_ptr<Model> model = std::make_shared<Model>();
		
		Models.push_back(model);

		return u32();
	}
}