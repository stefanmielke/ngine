#include "Asset.h"

#include "../App.h"
#include "AssetReference.h"

extern App *g_app;

Asset::Asset(AssetType type, std::string name, Asset *parent, AssetReferenceUnion asset_reference)
	: type(type),
	  name(name),
	  asset_ref(asset_reference),
	  parent(parent),
	  path(parent ? (parent->GetPath() + "/" + name) : name) {
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

void sort_asset_children(std::vector<Asset> &children) {
	if (children.empty())
		return;

	std::sort(children.begin(), children.end(), libdragon_asset_comparison);
	for (auto &child : children) {
		sort_asset_children(child.children);
	}
}

void sort_asset(Asset *asset) {
	sort_asset_children(asset->children);
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
	for (auto &asset : g_app->project.fonts) {
		Asset *cur_asset = CreateAndReturnAssetFolder(root_asset, asset->dfs_folder);

		AssetReferenceUnion ref;
		ref.font = &asset;

		cur_asset->children.emplace_back(FONT, asset->name, cur_asset, ref);
	}
	for (auto &asset : g_app->project.tiled_maps) {
		Asset *cur_asset = CreateAndReturnAssetFolder(root_asset, asset->dfs_folder);

		AssetReferenceUnion ref;
		ref.tiled = &asset;

		cur_asset->children.emplace_back(TILED_MAP, asset->name, cur_asset, ref);
	}

	sort_asset(root_asset);

	return root_asset;
}

bool libdragon_asset_comparison(const Asset &s1, const Asset &s2) {
	if (s1.GetType() == FOLDER && s2.GetType() != FOLDER)
		return false;
	else if (s1.GetType() != FOLDER && s2.GetType() == FOLDER)
		return true;

	return s1 < s2;
}
