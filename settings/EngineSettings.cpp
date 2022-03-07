#include "EngineSettings.h"

#include <fstream>
#include <utility>

#include "../json.hpp"

EngineSettings::EngineSettings() : editor_location("code"), libdragon_exe_location("libdragon"), theme(THEME_DARK) {
}

void EngineSettings::SaveToDisk() {
	nlohmann::json json = {
		{
			"engine",
			{
				{"editor_location", editor_location},
				{"emulator_location", emulator_location},
				{"last_opened_project", last_opened_project},
				{"libdragon_exe_location", libdragon_exe_location},
				{"theme", theme},
			},
		},
	};

	std::ofstream filestream("ngine.engine.json");
	filestream << json.dump(4);
	filestream.close();
}

void EngineSettings::LoadFromDisk() {
	if (!std::filesystem::exists("ngine.engine.json"))
		return;

	std::ifstream filestream("ngine.engine.json");

	nlohmann::json json;
	filestream >> json;

	last_opened_project = json["engine"]["last_opened_project"];
	emulator_location = json["engine"]["emulator_location"];
	editor_location = json["engine"]["editor_location"];
	libdragon_exe_location = json["engine"]["libdragon_exe_location"];
	theme = json["engine"]["theme"];

	filestream.close();
}

void EngineSettings::SetLastOpenedProject(std::string path) {
	last_opened_project = std::move(path);

	SaveToDisk();
}

void EngineSettings::SetEmulatorPath(std::string path) {
	emulator_location = std::move(path);

	SaveToDisk();
}

void EngineSettings::SetEditorLocation(std::string path) {
	editor_location = std::move(path);

	SaveToDisk();
}

void EngineSettings::SetTheme(Theme theme_id) {
	theme = theme_id;

	SaveToDisk();
}

void EngineSettings::SetLibdragonExeLocation(std::string path) {
	libdragon_exe_location = std::move(path);

	SaveToDisk();
}
