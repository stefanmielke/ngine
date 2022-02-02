#pragma once

#include "settings/Project.h"
#include "settings/ProjectSettings.h"

class ProjectBuilder {
   public:
	static void Create(std::string project_folder);
	static void Build(ProjectSettings project_settings, Project &project);
	static void Rebuild(ProjectSettings project_settings, Project &project);

	static void GenerateStaticFiles(std::string project_folder);
};
