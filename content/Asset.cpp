#include "Asset.h"

#include "../App.h"
#include "AssetReference.h"

extern App *g_app;

Asset::Asset(AssetType type, std::string name, Asset *parent, AssetReferenceUnion asset_reference)
	: type(type),
	  name(name),
	  asset_ref(asset_reference),
	  parent(parent),
	  path(parent ? parent->GetPath() / name : std::filesystem::path(name)) {
}

Asset *CreateAndReturnAssetFolder(Asset *root, const std::string &dfs_folder) {
	std::vector<std::string> folders;
	std::istringstream f(dfs_folder);
	std::string s;
	while (std::getline(f, s, '/')) {
		folders.push_back(s);
	}

	Asset *cur_asset = root;
	for (auto &folder : folders) {
		if (folder.empty()) {
			continue;
		}

		Asset *child = nullptr;
		for (auto &child_folder : cur_asset->children) {
			if (child_folder.GetType() == FOLDER && child_folder.GetName() == folder) {
				child = &child_folder;
				break;
			}
		}
		if (!child) {
			cur_asset->children.emplace_back(FOLDER, folder, cur_asset);
			child = &(*(cur_asset->children.end() - 1));
		}
		cur_asset = child;
	}

	return cur_asset;
}

Asset *Asset::BuildAsset(std::filesystem::path root_folder) {
	Asset *root_asset = new Asset(FOLDER, "Assets");

	for (auto &asset : g_app->project.images) {
		Asset *cur_asset = CreateAndReturnAssetFolder(root_asset, asset->dfs_folder);

		AssetReferenceUnion ref;
		ref.image = &asset;

		cur_asset->children.emplace_back(IMAGE, asset->name, cur_asset, ref);
	}
	for (auto &asset : g_app->project.sounds) {
		Asset *cur_asset = CreateAndReturnAssetFolder(root_asset, asset->dfs_folder);

		AssetReferenceUnion ref;
		ref.sound = &asset;

		cur_asset->children.emplace_back(SOUND, asset->name, cur_asset, ref);
	}
	for (auto &asset : g_app->project.general_files) {
		Asset *cur_asset = CreateAndReturnAssetFolder(root_asset, asset->dfs_folder);

		AssetReferenceUnion ref;
		ref.file = &asset;

		cur_asset->children.emplace_back(GENERAL, asset->name, cur_asset, ref);
	}

	return root_asset;
}
