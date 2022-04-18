#include "AssetType.h"

std::string GetAssetTypeName(AssetType type) {
	switch (type) {
		case UNKNOWN:
			return "unknown";
		case FOLDER:
			return "folder";
		case IMAGE:
			return "sprite";
		case SOUND:
			return "sound";
		case GENERAL:
			return "file";
		case FONT:
			return "font";
		case TILED_MAP:
		case LDTK_MAP:
			return "map";
	}

	return "";
}
