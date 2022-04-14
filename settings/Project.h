#pragma once

#include <memory>
#include <vector>
#include <SDL2/SDL.h>

#include "ProjectSettings.h"
#include "Scene.h"
#include "../LibdragonImage.h"
#include "../LibdragonFile.h"
#include "../LibdragonScript.h"
#include "../LibdragonSound.h"
#include "../content/Asset.h"

class App;

class Project {
   public:
	std::vector<Scene> scenes;

	std::vector<std::unique_ptr<LibdragonScript>> script_files;
	std::vector<std::unique_ptr<LibdragonSound>> sounds;
	std::vector<std::unique_ptr<LibdragonImage>> images;
	std::vector<std::unique_ptr<LibdragonFile>> general_files;

	Asset *assets;

	ProjectSettings project_settings;

	void SaveToDisk(const std::string &project_directory);
	void LoadFromDisk(const std::string &project_directory);

	bool Open(const char *path, App *app);
	void Close(App *app);

	void ReloadImages(SDL_Renderer *renderer);
	void ReloadScripts(App *app);
	void ReloadSounds();
	void ReloadGeneralFiles();

	void ReloadAssets();

	explicit Project(App *app);
	Project(Project const &) = delete;
	Project &operator=(Project const &) = delete;

	~Project();
};
