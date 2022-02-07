#pragma once

#include "settings/ProjectSettings.h"
#include "settings/Project.h"

class ScriptBuilder {
   public:
	static bool CreateScriptFile(const ProjectSettings &project_settings, const char *script_name);
	static void DeleteScriptFile(Project &project, const char *script_name);
};
