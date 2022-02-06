#pragma once

#include <memory>

#include "LibdragonImage.h"
#include "LibdragonSound.h"
#include "settings/EngineSettings.h"
#include "settings/ProjectSettings.h"
#include "settings/Project.h"

class App;

class Emulator {
   public:
	static void Run(App *app);
};
