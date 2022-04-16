#pragma once

#include <memory>

#include "AssetType.h"

class LibdragonFile;
class LibdragonFont;
class LibdragonImage;
class LibdragonSound;

union AssetReferenceUnion {
	std::unique_ptr<LibdragonImage> *image;
	std::unique_ptr<LibdragonSound> *sound;
	std::unique_ptr<LibdragonFile> *file;
	std::unique_ptr<LibdragonFont> *font;
};

struct AssetReference {
   public:
	AssetReference() : asset_type(UNKNOWN), asset_ref(AssetReferenceUnion()) {
	}

	void Ref(AssetType type, AssetReferenceUnion ref) {
		asset_type = type;
		asset_ref = ref;
	}

	AssetType Type() const {
		return asset_type;
	}

	AssetReferenceUnion Ref() const {
		return asset_ref;
	}

	void Reset() {
		asset_type = UNKNOWN;
		asset_ref = AssetReferenceUnion();
	}

   private:
	AssetType asset_type;
	AssetReferenceUnion asset_ref;
};