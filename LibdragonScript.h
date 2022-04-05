#pragma once

#include <string>

#include "settings/ProjectSettings.h"

class App;

class LibdragonScript {
   public:
	std::string name;
	std::string text;

	std::string GetFilePath(App *app);

	std::string GetFileName() {
		return name + ".script.c";
	}

	void LoadText(App *app);

	void LoadFromDisk(const std::string &filepath);
	void DeleteFromDisk(App *app) const;
};
