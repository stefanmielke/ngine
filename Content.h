#pragma once

#include <memory>
#include <vector>

#include "LibdragonImage.h"
#include "LibdragonSound.h"
#include "settings/ProjectSettings.h"

class Content {
   public:
	static void CreateSprites(ProjectSettings &project_settings, std::vector<std::unique_ptr<LibdragonImage>> &images);
	static void CreateSounds(ProjectSettings &project_settings, std::vector<std::unique_ptr<LibdragonSound>> &sounds);
};
