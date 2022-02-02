#pragma once

#include <string>

class EngineSettings {
   public:
	EngineSettings();

	void SaveToDisk();
	void LoadFromDisk();

	void SetLastOpenedProject(std::string path);
	std::string GetLastOpenedProject() {
		return last_opened_project;
	};

   private:
	std::string last_opened_project;
	std::string emu_cen64_location;
};
