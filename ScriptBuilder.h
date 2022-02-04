#pragma once

#include "settings/ProjectSettings.h"

class ScriptBuilder {
   public:
	static void CreateScriptFile(ProjectSettings &project_settings, const char *script_name);
};
