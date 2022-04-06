#include "LibdragonScript.h"

#include <fstream>

#include "json.hpp"

#include "App.h"
#include "ScriptBuilder.h"

std::string LibdragonScript::GetFilePath(App *app) {
	return app->project.project_settings.project_directory + "/src/scripts/" + GetFileName();
}

void LibdragonScript::LoadText(App *app) {
	std::ifstream filestream(GetFilePath(app));

	std::ostringstream sstr;
	sstr << filestream.rdbuf();
	text = sstr.str();
	text = text.substr(0, std::min(1000, (int)text.length()));
	filestream.close();
}

void LibdragonScript::LoadFromDisk(const std::string &filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
}

void LibdragonScript::DeleteFromDisk(App *app) const {
	const std::string script_folder = app->project.project_settings.project_directory +
									  "/src/scripts/";
	const std::string script_json_folder = app->project.project_settings.project_directory +
										   "/.ngine/scripts/";

	std::string filepath = script_folder + name;

	std::string json_name = filepath + ".script.json";
	std::filesystem::remove(json_name);

	std::string header_name = filepath + ".script.h";
	std::filesystem::remove(header_name);

	std::string c_name = filepath + ".script.c";
	std::filesystem::remove(c_name);

	std::string o_name = filepath + ".script.o";
	std::filesystem::remove(o_name);

	std::string d_name = filepath + ".script.d";
	std::filesystem::remove(d_name);

	for (auto &scene : app->project.scenes) {
		if (scene.script_name == name) {
			scene.script_name = "";
		}
	}
	app->project.SaveToDisk(app->project.project_settings.project_directory);

	bool removed_from_global = (app->project.project_settings.global_script_name == name);
	if (removed_from_global) {
		app->project.project_settings.global_script_name = "";
		app->project.project_settings.SaveToDisk();
	}
}

bool libdragon_script_comparison(const std::unique_ptr<LibdragonScript> &s1,
								 const std::unique_ptr<LibdragonScript> &s2) {
	return (*s1) < (*s2);
}
