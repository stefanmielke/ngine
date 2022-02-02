#pragma once

#include "ProjectSettings.h"

class ProjectBuilder {
   public:
	static void Create(std::string project_folder);
	static void Build(ProjectSettings project_settings);
};
