#pragma once

#include "settings/ProjectSettings.h"

class ProjectBuilder {
   public:
	static void Create(std::string project_folder);
	static void Build(ProjectSettings project_settings);
	static void Rebuild(ProjectSettings project_settings);
};
