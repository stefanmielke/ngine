#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "AssetType.h"

class Asset {
   public:
	std::vector<Asset> children;

	Asset(AssetType type, std::string name, Asset *parent = nullptr);

	static Asset *BuildAsset(std::filesystem::path project_assets_folder);

	AssetType GetType() const {
		return type;
	}

	std::string GetName() const {
		return name;
	}

	Asset *const GetParent() const {
		return parent;
	}

	std::filesystem::path GetPath() const {
		return path;
	}

	std::filesystem::path GetFullPath(std::filesystem::path project_folder) const {
		return project_folder / GetPath();
	}

   private:
	AssetType type;
	std::string name;

	Asset *parent;
	std::filesystem::path path;
};
