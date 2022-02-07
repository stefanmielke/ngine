#pragma once

#include <string>

enum Theme {
	THEME_DARK,
	THEME_LIGHT,
	THEME_CLASSIC,
};

class EngineSettings {
   public:
	EngineSettings();

	void SaveToDisk();
	void LoadFromDisk();

	void SetEmulatorPath(std::string path);
	std::string GetEmulatorPath() {
		return emulator_location;
	};

	void SetLastOpenedProject(std::string path);
	std::string GetLastOpenedProject() {
		return last_opened_project;
	};

	void SetTheme(Theme theme_id);
	Theme GetTheme() {
		return theme;
	};

   private:
	std::string last_opened_project;
	std::string emulator_location;
	Theme theme;
};
