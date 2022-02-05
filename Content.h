#pragma once

#include <memory>
#include <vector>

#include "LibdragonImage.h"
#include "settings/ProjectSettings.h"

class Content {
   public:
	static void CreateSprites(ProjectSettings &project_settings, std::vector<std::unique_ptr<LibdragonImage>> &images);
};
