#pragma once

#include <vector>

#include "Scene.h"

class Project {
   public:
	std::vector<Scene> scenes;

	void SaveToDisk(std::string &folder);
	void LoadFromDisk(std::string &folder);
};
