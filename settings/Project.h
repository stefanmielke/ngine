#pragma once

#include <memory>
#include <vector>

#include "ProjectSettings.h"
#include "Scene.h"
#include "../LibdragonSound.h"
#include "../LibdragonImage.h"

class Project {
   public:
	std::vector<Scene> scenes;

	std::vector<std::string> script_files;
	std::vector<std::unique_ptr<LibdragonSound>> sounds;
	std::vector<std::unique_ptr<LibdragonImage>> images;

	ProjectSettings project_settings;

	void SaveToDisk(std::string &project_directory);
	void LoadFromDisk(std::string &project_directory);
};
