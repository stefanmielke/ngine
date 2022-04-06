#include "LibdragonScript.h"

#include <fstream>

#include "json.hpp"

#include "App.h"
#include "Libdragon.h"
#include "ScriptBuilder.h"

void replace_all(std::string &str, const std::string &from, const std::string &to) {
	if (from.empty())
		return;

	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

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

void LibdragonScript::SaveToDisk(App *app) {
	const std::string script_json_folder = app->project.project_settings.project_directory +
										   "/.ngine/scripts/";
	std::string filepath = script_json_folder + name + ".script.json";

	nlohmann::json json;
	json["name"] = name;

	std::ofstream file(filepath);
	file << json.dump(4) << std::endl;
	file.close();
}

void LibdragonScript::LoadFromDisk(const std::string &filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
}

void LibdragonScript::DeleteFromDisk(App *app) const {
	const std::string script_json_folder = app->project.project_settings.project_directory +
										   "/.ngine/scripts/";
	std::string filepath = script_json_folder + name;

	std::string json_name = filepath + ".script.json";
	std::filesystem::remove(json_name);

	const std::string script_folder = app->project.project_settings.project_directory +
									  "/src/scripts/";
	filepath = script_folder + name;

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

	if (app->project.project_settings.global_script_name == name) {
		app->project.project_settings.global_script_name = "";
		app->project.project_settings.SaveToDisk();
	}
}

void LibdragonScript::RenameAs(App *app, const std::string &new_name) {
	const std::string script_json_folder = app->project.project_settings.project_directory +
										   "/.ngine/scripts/";
	std::string old_filepath = script_json_folder + name + ".script.json";
	std::string new_filepath = script_json_folder + new_name + ".script.json";

	std::filesystem::copy_file(old_filepath, new_filepath);

	const std::string script_folder = app->project.project_settings.project_directory +
									  "/src/scripts/";
	old_filepath = script_folder + name;
	new_filepath = script_folder + new_name;

	std::string old_header_name = old_filepath + ".script.h";
	std::string new_header_name = new_filepath + ".script.h";
	std::filesystem::copy_file(old_header_name, new_header_name);

	{
		std::ifstream filestream(new_header_name);
		std::ostringstream sstr;
		sstr << filestream.rdbuf();
		std::string content(sstr.str());
		filestream.close();
		replace_all(content, name, new_name);

		std::ofstream out_filestream(new_header_name);
		out_filestream << content;
		out_filestream.close();
	}

	std::string old_c_name = old_filepath + ".script.c";
	std::string new_c_name = new_filepath + ".script.c";
	std::filesystem::copy_file(old_c_name, new_c_name);

	{
		std::ifstream filestream(new_c_name);
		std::ostringstream sstr;
		sstr << filestream.rdbuf();
		std::string content(sstr.str());
		filestream.close();
		replace_all(content, name + ".script.h", new_name + ".script.h");
		replace_all(content, "script_" + name + "_create", "script_" + new_name + "_create");
		replace_all(content, "script_" + name + "_tick", "script_" + new_name + "_tick");
		replace_all(content, "script_" + name + "_display", "script_" + new_name + "_display");
		replace_all(content, "script_" + name + "_destroy", "script_" + new_name + "_destroy");

		std::ofstream out_filestream(new_c_name);
		out_filestream << content;
		out_filestream.close();
	}

	bool updated_scene = false;
	for (auto &scene : app->project.scenes) {
		if (scene.script_name == name) {
			scene.script_name = new_name;
			updated_scene = true;
		}
	}
	if (updated_scene) {
		app->project.SaveToDisk(app->project.project_settings.project_directory);
	}

	bool updated_global = false;
	if (app->project.project_settings.global_script_name == name) {
		app->project.project_settings.global_script_name = new_name;
		app->project.project_settings.SaveToDisk();

		updated_global = true;
	}

	DeleteFromDisk(app);

	name = new_name;
	SaveToDisk(app);

	if (updated_scene || updated_global) {
		Libdragon::Clean(app->project.project_settings.project_directory,
						 app->engine_settings.GetLibdragonExeLocation());
	}
}

bool libdragon_script_comparison(const std::unique_ptr<LibdragonScript> &s1,
								 const std::unique_ptr<LibdragonScript> &s2) {
	return (*s1) < (*s2);
}
