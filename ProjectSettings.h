#pragma once

#include <string>

#include "DisplaySettings.h"

class ProjectSettings {
   private:
	bool is_open;

   public:
	std::string project_name;
	std::string rom_name;
	std::string project_directory;

	DisplaySettings display;

	ProjectSettings();

	bool LoadFromFile(std::string &folder);
	void SaveToFile();

	void CloseProject();

	[[nodiscard]] bool IsOpen() const {
		return is_open;
	};
};
