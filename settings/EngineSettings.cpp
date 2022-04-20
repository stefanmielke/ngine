#include "EngineSettings.h"

#include <fstream>
#include <utility>

#include "../json.hpp"
#include "../Libdragon.h"

#ifdef __linux__
#include <pwd.h>
#include <unistd.h>
#endif

#include "../ThreadCommand.h"

EngineSettings::EngineSettings()
	: editor_location("code"),
	  libdragon_exe_location(),
	  libdragon_use_bundled(true),
	  theme(THEME_DARK),
	  engine_settings_folder(),
	  engine_settings_filepath() {
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
				{"libdragon_use_bundled", libdragon_use_bundled},
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

void EngineSettings::LoadFromDisk(const App *app) {
	// TODO: old location, remove on 2.0.0
	if (std::filesystem::exists("ngine.engine.json")) {
		LoadFromDisk(app, "ngine.engine.json");
		SaveToDisk();  // save on new location

		std::filesystem::remove("ngine.engine.json");
		return;
	}

	if (!std::filesystem::exists(engine_settings_filepath)) {
		return;
	}

	LoadFromDisk(app, engine_settings_filepath);
}

void EngineSettings::LoadFromDisk(const App *app, const std::string &path) {
	std::ifstream filestream(path);

	nlohmann::json json;
	filestream >> json;
	filestream.close();

	last_opened_project = json["engine"]["last_opened_project"];
	emulator_location = json["engine"]["emulator_location"];
	editor_location = json["engine"]["editor_location"];
	libdragon_exe_location = json["engine"]["libdragon_exe_location"];
	theme = json["engine"]["theme"];

	if (!json["engine"]["libdragon_use_bundled"].is_null())
		libdragon_use_bundled = json["engine"]["libdragon_use_bundled"];

	if (last_opened_project.empty()) {
		last_opened_project = ".";
	}

	if (libdragon_use_bundled || !libdragon_exe_location.empty()) {
		libdragon_version = Libdragon::GetVersion(app);
	}

	ReloadDockerVersion();
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

void EngineSettings::SetLibdragonExeLocation(const App *app, bool use_bundled, std::string path) {
	libdragon_exe_location = std::move(path);
	libdragon_use_bundled = use_bundled;

	SaveToDisk();

	if (libdragon_use_bundled || !libdragon_exe_location.empty()) {
		libdragon_version = Libdragon::GetVersion(app);
	}
}

void EngineSettings::ReloadDockerVersion() {
	std::string command("docker version --format 'docker {{.Server.Version}}'");

	std::string result;
	if (ThreadCommand::RunCommand(command, result) != EXIT_SUCCESS) {
		docker_version = "Docker is not installed.";
	} else {
		docker_version = result;
	}
}
