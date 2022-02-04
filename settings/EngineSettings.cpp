#include "EngineSettings.h"

#include <fstream>
#include <utility>

#include "../json.hpp"

EngineSettings::EngineSettings() : last_opened_project(""), emulator_location("") {
}

void EngineSettings::SaveToDisk() {
	nlohmann::json json;
	json["engine"]["last_opened_project"] = last_opened_project;
	json["engine"]["emulator_location"] = emulator_location;

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
	if (!json["engine"]["emulator_location"].is_null())
		emulator_location = json["engine"]["emulator_location"];

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
