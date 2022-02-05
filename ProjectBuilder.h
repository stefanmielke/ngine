#pragma once

#include <memory>

#include "LibdragonImage.h"
#include "settings/Project.h"
#include "settings/ProjectSettings.h"

class ProjectBuilder {
   public:
	static void Create(std::string project_folder);
	static void Build(ProjectSettings &project_settings, Project &project, std::vector<std::unique_ptr<LibdragonImage>> &images);
	static void Rebuild(ProjectSettings &project_settings, Project &project, std::vector<std::unique_ptr<LibdragonImage>> &images);

	static void GenerateStaticFiles(std::string project_folder);
};
