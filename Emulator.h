#pragma once

#include "settings/EngineSettings.h"
#include "settings/ProjectSettings.h"
#include "settings/Project.h"

class Emulator {
   public:
	static void Run(EngineSettings &engine_settings, ProjectSettings &project_settings,
					Project &project);
};
