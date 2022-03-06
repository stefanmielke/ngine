#pragma once

#include <memory>
#include <vector>
#include <SDL2/SDL.h>

#include "ProjectSettings.h"
#include "Scene.h"
#include "../LibdragonImage.h"
#include "../LibdragonFile.h"
#include "../LibdragonSound.h"

class App;

class Project {
   public:
	std::vector<Scene> scenes;

	std::vector<std::string> script_files;
	std::vector<std::unique_ptr<LibdragonSound>> sounds;
	std::vector<std::unique_ptr<LibdragonImage>> images;
	std::vector<std::unique_ptr<LibdragonFile>> general_files;

	ProjectSettings project_settings;

	void SaveToDisk(const std::string &project_directory);
	void LoadFromDisk(const std::string &project_directory);

	bool Open(const char *path, App *app);
	void Close();

	void ReloadImages(SDL_Renderer *renderer);
	void ReloadScripts();
	void ReloadSounds();
	void ReloadGeneralFiles();

	Project() = default;
	Project(Project const &) = delete;
	Project &operator=(Project const &) = delete;

	~Project();
};
