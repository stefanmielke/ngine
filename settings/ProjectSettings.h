#pragma once

#include <string>

#include "DisplaySettings.h"
#include "ModulesSettings.h"

class ProjectSettings {
   private:
	bool is_open;

   public:
	int next_scene_id;
	std::string project_name;
	std::string rom_name;
	std::string project_directory;

	DisplaySettings display;
	ModulesSettings modules;

	ProjectSettings();

	bool LoadFromFile(std::string &folder);
	void SaveToDisk();

	void CloseProject();

	[[nodiscard]] bool IsOpen() const {
		return is_open;
	};
};
