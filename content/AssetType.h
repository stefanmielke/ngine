#pragma once

#include <string>

enum AssetType {
	UNKNOWN,
	FOLDER,
	IMAGE,
	SOUND,
	GENERAL,
	FONT,
};

std::string GetAssetTypeName(AssetType type);
