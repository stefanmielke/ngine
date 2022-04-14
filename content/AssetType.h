#pragma once

#include <string>

enum AssetType {
	UNKNOWN,
	FOLDER,
	IMAGE,
	SOUND,
	GENERAL,
};

std::string GetAssetTypeName(AssetType type);
