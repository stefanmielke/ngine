#pragma once

#include <memory>

#include "LibdragonImage.h"
#include "LibdragonSound.h"
#include "settings/Project.h"
#include "settings/ProjectSettings.h"

class ProjectBuilder {
   public:
	static void Create(App *app, std::string project_folder);
	static void Build(Project &project);
	static void Rebuild(Project &project);

	static void GenerateStaticFiles(std::string project_folder);
};
