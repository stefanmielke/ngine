#include "ScriptBuilder.h"

#include <filesystem>
#include <fstream>

#include "ConsoleApp.h"
#include "json.hpp"
#include "generated/generated.h"

extern ConsoleApp console;

void ScriptBuilder::CreateScriptFile(ProjectSettings &project_settings, const char *script_name) {
	const std::string script_folder = project_settings.project_directory + "/src/scripts/";
	const std::string script_json_folder = project_settings.project_directory + "/.ngine/scripts/";

	std::filesystem::create_directories(script_folder);
	std::filesystem::create_directories(script_json_folder);

	{
		std::string filepath = script_json_folder + script_name + ".script.json";

		if (std::filesystem::exists(filepath)) {
			console.AddLog("Script with the same name already exists.");
			return;
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
}
