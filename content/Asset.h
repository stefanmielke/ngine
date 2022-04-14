#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "AssetReference.h"
#include "AssetType.h"

class LibdragonImage;
class LibdragonSound;
class LibdragonFile;

class Asset {
   public:
	std::vector<Asset> children;

	Asset(AssetType type, std::string name, Asset *parent = nullptr,
		  AssetReferenceUnion asset_reference = AssetReferenceUnion());

	static Asset *BuildAsset(std::filesystem::path project_assets_folder);

	AssetType GetType() const {
		return type;
	}

	std::string GetName() const {
		return name;
	}

	AssetReferenceUnion GetAssetReference() const {
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

	AssetReferenceUnion asset_ref;

	Asset *parent;
	std::filesystem::path path;
};
