#pragma once

#include <vector>

#include "Scene.h"

class Project {
   public:
	std::vector<Scene> scenes;

	void SaveToDisk(std::string &project_directory);
	void LoadFromDisk(std::string &project_directory);
};
