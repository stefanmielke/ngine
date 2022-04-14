#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "AssetType.h"

class LibdragonImage;
class LibdragonSound;
class LibdragonFile;

union AssetReference {
	std::unique_ptr<LibdragonImage> *image;
	std::unique_ptr<LibdragonSound> *sound;
	std::unique_ptr<LibdragonFile> *file;

	AssetReference() = default;
};

class Asset {
   public:
	std::vector<Asset> children;

	Asset(AssetType type, std::string name, Asset *parent = nullptr,
		  AssetReference asset_reference = AssetReference());

	static Asset *BuildAsset(std::filesystem::path project_assets_folder);

	AssetType GetType() const {
		return type;
	}

	std::string GetName() const {
		return name;
	}

	AssetReference GetAssetReference() const {
		return asset_ref;
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

	AssetReference asset_ref;

	Asset *parent;
	std::filesystem::path path;
};
