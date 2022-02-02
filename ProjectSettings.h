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

	void LoadFromFile(std::string &filepath);
	void SaveToFile(std::string &filepath);

	void CloseProject();

	[[nodiscard]] bool IsOpen() const {
		return is_open;
	};
};
