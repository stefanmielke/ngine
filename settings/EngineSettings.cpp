#include "EngineSettings.h"

#include <fstream>
#include <utility>

#include "../json.hpp"

EngineSettings::EngineSettings() : last_opened_project(""), emu_mupen64_location("") {
}

void EngineSettings::SaveToDisk() {
	nlohmann::json json;
	json["engine"]["last_opened_project"] = last_opened_project;
	json["engine"]["emu_mupen64_location"] = emu_mupen64_location;

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

	if (!json["engine"]["last_opened_project"].is_null())
		last_opened_project = json["engine"]["last_opened_project"];
	if (!json["engine"]["emu_mupen64_location"].is_null())
		emu_mupen64_location = json["engine"]["emu_mupen64_location"];

	filestream.close();
}

void EngineSettings::SetLastOpenedProject(std::string path) {
	last_opened_project = std::move(path);

	SaveToDisk();
}

void EngineSettings::SetMupen64Path(std::string path) {
	emu_mupen64_location = std::move(path);

	SaveToDisk();
}
