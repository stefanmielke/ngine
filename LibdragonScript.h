#pragma once

#include <memory>
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

	bool operator<(const LibdragonScript &other) const {
		return (name < other.name);
	}
};

bool libdragon_script_comparison(const std::unique_ptr<LibdragonScript> &s1,
								 const std::unique_ptr<LibdragonScript> &s2);
