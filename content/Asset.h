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
	~Asset() {
	}

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

	std::string GetPath() const {
		return path;
	}

	std::string GetFullPath(std::string project_folder) const {
		return project_folder + "/" + GetPath();
	}

	bool operator<(const Asset &other) const {
		return (name < other.name);
	}

   private:
	AssetType type;
	std::string name;

	AssetReferenceUnion asset_ref;

	Asset *parent;
	std::string path;
};

bool libdragon_asset_comparison(const Asset &s1, const Asset &s2);
