#pragma once

#include <string>

enum AssetType {
	UNKNOWN,
	FOLDER,
	IMAGE,
	SOUND,
	GENERAL,
	FONT,
	TILED_MAP,
	LDTK_MAP,
};

std::string GetAssetTypeName(AssetType type);
