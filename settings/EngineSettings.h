#pragma once

#include <string>

class EngineSettings {
   public:
	EngineSettings();

	void SaveToDisk();
	void LoadFromDisk();

	void SetMupen64Path(std::string path);
	std::string GetMupen64Path() {
		return emu_mupen64_location;
	};

	void SetLastOpenedProject(std::string path);
	std::string GetLastOpenedProject() {
		return last_opened_project;
	};

   private:
	std::string last_opened_project;
	std::string emu_mupen64_location;
};
