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
	[[nodiscard]] std::string GetEmulatorPath() const {
		return emulator_location;
	};

	void SetLastOpenedProject(std::string path);
	[[nodiscard]] std::string GetLastOpenedProject() const {
		return last_opened_project;
	};

	void SetEditorLocation(std::string path);
	[[nodiscard]] std::string GetEditorLocation() const {
		return editor_location;
	};

	void SetTheme(Theme theme_id);
	[[nodiscard]] Theme GetTheme() const {
		return theme;
	};

	void SetLibdragonExeLocation(std::string path);
	[[nodiscard]] std::string GetLibdragonExeLocation() const {
		return libdragon_exe_location;
	};

   private:
	std::string last_opened_project;
	std::string emulator_location;
	std::string editor_location;
	std::string libdragon_exe_location;
	Theme theme;
};
