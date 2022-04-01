#include "EngineSettings.h"

#include <fstream>
#include <utility>

#include "../json.hpp"

#ifdef WIN32
const char *ngine_settings_folder = "%AppData%/ngine/";
const char *ngine_settings_path = "%AppData%/ngine/ngine.engine.json";
#else
#include <pwd.h>
const char *ngine_settings_folder = "~/.ngine/";
const char *ngine_settings_path = "~/.ngine/ngine.engine.json";
#endif

EngineSettings::EngineSettings()
	: editor_location("code"),
	  libdragon_exe_location("libdragon"),
	  theme(THEME_DARK),
	  engine_settings_folder(ngine_settings_folder),
	  engine_settings_filepath(ngine_settings_path) {
#ifdef WIN32
	engine_settings_folder = getenv("APPDATA") + std::string("\\ngine\\");
	engine_settings_filepath = getenv("APPDATA") + std::string("\\ngine\\ngine.engine.json");
#else
	const char *homedir = getenv("HOME");
	if (!homedir) {
		homedir = getpwuid(getuid())->pw_dir;
	}

	engine_settings_folder = homedir + std::string("/ngine/");
	engine_settings_filepath = homedir + std::string("/ngine/ngine.engine.json");
#endif
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

	if (!std::filesystem::exists(engine_settings_folder)) {
		std::filesystem::create_directories(engine_settings_folder);
		return;
	}

	std::ofstream filestream(engine_settings_filepath);
	filestream << json.dump(4);
	filestream.close();
}

void EngineSettings::LoadFromDisk() {
	// TODO: old location, remove on 2.0.0
	if (std::filesystem::exists("ngine.engine.json")) {
		LoadFromDisk("ngine.engine.json");
		SaveToDisk();  // save on new location

		std::filesystem::remove("ngine.engine.json");
		return;
	}

	if (!std::filesystem::exists(engine_settings_filepath)) {
		return;
	}

	LoadFromDisk(ngine_settings_path);
}

void EngineSettings::LoadFromDisk(const std::string &path) {
	std::ifstream filestream(path);

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
