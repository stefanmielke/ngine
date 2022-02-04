#pragma once

#include "settings/ProjectSettings.h"
#include "settings/Project.h"

class ScriptBuilder {
   public:
	static void CreateScriptFile(ProjectSettings &project_settings, const char *script_name);
	static void DeleteScriptFile(ProjectSettings &project_settings, Project &project,
								 const char *script_name);
};
