#pragma once

#include <memory>

#include "LibdragonImage.h"
#include "LibdragonSound.h"
#include "settings/Project.h"
#include "settings/ProjectSettings.h"

class ProjectBuilder {
   public:
	static void Create(App *app, std::string project_folder);
	static void Build(App *app);
	static void Rebuild(App *app);

	static void GenerateStaticFiles(const std::string& project_folder);
};
