#pragma once

#include <string>

class EngineSettings {
   public:
	EngineSettings();

	void SaveToDisk();
	void LoadFromDisk();

	void SetEmulatorPath(std::string path);
	std::string GetMupen64Path() {
		return emulator_location;
	};

	void SetLastOpenedProject(std::string path);
	std::string GetLastOpenedProject() {
		return last_opened_project;
	};

   private:
	std::string last_opened_project;
	std::string emulator_location;
};
