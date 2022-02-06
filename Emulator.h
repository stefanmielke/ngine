#pragma once

#include <memory>

#include "LibdragonImage.h"
#include "LibdragonSound.h"
#include "settings/EngineSettings.h"
#include "settings/ProjectSettings.h"
#include "settings/Project.h"

class Emulator {
   public:
	static void Run(EngineSettings &engine_settings, ProjectSettings &project_settings,
					Project &project, std::vector<std::unique_ptr<LibdragonImage>> &images,
					std::vector<std::unique_ptr<LibdragonSound>> &sounds);
};
