#include "ScriptBuilder.h"

#include <filesystem>
#include <fstream>

#include "App.h"
#include "ConsoleApp.h"
#include "json.hpp"
#include "Libdragon.h"
#include "generated/generated.h"

bool ScriptBuilder::CreateScriptFile(const ProjectSettings &project_settings, const char *script_name) {
	const std::string script_folder = project_settings.project_directory + "/src/scripts/";
	const std::string script_json_folder = project_settings.project_directory + "/.ngine/scripts/";

	std::filesystem::create_directories(script_folder);
	std::filesystem::create_directories(script_json_folder);

	{
		std::string filepath = script_json_folder + script_name + ".script.json";

		if (std::filesystem::exists(filepath)) {
			console.AddLog("Script with the same name already exists.");
			return false;
		}

		nlohmann::json json;
		json["name"] = script_name;

		std::ofstream file(filepath);
		file << json.dump(4) << std::endl;
		file.close();
	}
	{
		std::string filepath = script_folder + script_name;

		std::string header_name = filepath + ".script.h";
		FILE *filestream = fopen(header_name.c_str(), "w");
		fprintf(filestream, script_blank_gen_h, script_name, script_name, script_name, script_name);
		fclose(filestream);

		std::string c_name = filepath + ".script.c";
		filestream = fopen(c_name.c_str(), "w");
		fprintf(filestream, script_blank_gen_c, script_name, script_name, script_name, script_name,
				script_name);
		fclose(filestream);
	}

	return true;
}

void ScriptBuilder::DeleteScriptFile(App *app, const char *script_name) {
	const std::string script_folder = app->project.project_settings.project_directory + "/src/scripts/";
	const std::string script_json_folder = app->project.project_settings.project_directory + "/.ngine/scripts/";

	{
		std::string filepath = script_json_folder + script_name + ".script.json";
		std::filesystem::remove(filepath);
	}
	{
		std::string filepath = script_folder + script_name;

		std::string header_name = filepath + ".script.h";
		std::filesystem::remove(header_name);

		std::string c_name = filepath + ".script.c";
		std::filesystem::remove(c_name);

		std::string o_name = filepath + ".script.o";
		std::filesystem::remove(o_name);

		std::string d_name = filepath + ".script.d";
		std::filesystem::remove(d_name);
	}
	bool removed_from_scene = false;
	{
		std::string script_name_str(script_name);
		for (auto &scene : app->project.scenes) {
			if (scene.script_name == script_name_str) {
				scene.script_name = "";
				removed_from_scene = true;
			}
		}
	}
	app->project.SaveToDisk(app->project.project_settings.project_directory);

	// force clean if the script was referenced on a scene to stop build errors
	if (removed_from_scene) {
		console.AddLog("Script was referenced by a Scene. Cleaning the project...");

		Libdragon::Clean(app->project.project_settings.project_directory, app->engine_settings.GetLibdragonExeLocation());
	}
}
