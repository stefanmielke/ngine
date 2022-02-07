#pragma once

#include <memory>
#include <vector>

#include "LibdragonImage.h"
#include "LibdragonSound.h"
#include "settings/ProjectSettings.h"

class Content {
   public:
	static void CreateSprites(const ProjectSettings &project_settings,
							  const std::vector<std::unique_ptr<LibdragonImage>> &images);
	static void CreateSounds(const ProjectSettings &project_settings,
							 const std::vector<std::unique_ptr<LibdragonSound>> &sounds);
};
