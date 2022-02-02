#include "ProjectSettings.h"

#include <fstream>

#include "ConsoleApp.h"
#include "json.hpp"

extern ConsoleApp console;

ProjectSettings::ProjectSettings()
	: is_open(false), project_name("Hello NGine"), rom_name("hello_ngine") {
}

bool ProjectSettings::LoadFromFile(std::string &folder) {
	project_directory = folder;

	if (!std::filesystem::exists(project_directory + "/ngine.project.json")) {
		console.AddLog("Project file not found (is the project still being created?).");
		return false;
	}

	std::ifstream project_file(project_directory + "/ngine.project.json");

	nlohmann::json json;
	project_file >> json;

	display.SetResolution(json["display"]["resolution"]);
	display.SetBitDepth(json["display"]["bit_depth"]);
	display.buffers = json["display"]["buffers"];
	display.SetGamma(json["display"]["gamma"]);
	display.SetAntialias(json["display"]["antialias"]);

	project_file.close();

	is_open = true;
	return true;
}

void ProjectSettings::SaveToFile(std::string &filepath) {
	nlohmann::json json;
	json["project"]["name"] = project_name;
	json["project"]["rom"] = rom_name;

	json["display"]["resolution"] = display.GetResolution();
	json["display"]["bit_depth"] = display.GetBitDepth();
	json["display"]["buffers"] = display.buffers;
	json["display"]["gamma"] = display.GetGamma();
	json["display"]["antialias"] = display.GetAntialias();

	std::ofstream project_file(filepath);

	project_file << json.dump(4);

	project_file.close();
}

void ProjectSettings::CloseProject() {
	is_open = false;
	project_directory.clear();
}
