#pragma once

#include <memory>
#include <vector>
#include <SDL2/SDL.h>

#include "ProjectSettings.h"
#include "Scene.h"
#include "../LibdragonSound.h"
#include "../LibdragonImage.h"

class App;

class Project {
   public:
	std::vector<Scene> scenes;

	std::vector<std::string> script_files;
	std::vector<std::unique_ptr<LibdragonSound>> sounds;
	std::vector<std::unique_ptr<LibdragonImage>> images;

	ProjectSettings project_settings;

	void SaveToDisk(std::string &project_directory);
	void LoadFromDisk(std::string &project_directory);

	bool Open(const char *path, App *app);

	void ReloadImages(SDL_Renderer *renderer);
	void ReloadScripts();
	void ReloadSounds();
};
