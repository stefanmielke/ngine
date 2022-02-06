#include "Project.h"

#include <filesystem>
#include <fstream>

#include "../json.hpp"

void Project::SaveToDisk(std::string &project_directory) {
	std::filesystem::path scenes_folder(project_directory + "/.ngine/scenes");
	if (!std::filesystem::exists(scenes_folder)) {
		std::filesystem::create_directories(scenes_folder);
	}

	for (auto &scene : scenes) {
		nlohmann::json json;
		json["id"] = scene.id;
		json["name"] = scene.name;
		json["fill_color"] = scene.fill_color;
		json["script_name"] = scene.script_name;

		std::ofstream file(project_directory + "/.ngine/scenes/" + std::to_string(scene.id) +
						   ".scene.json");
		file << json.dump(4) << std::endl;
		file.close();
	}
}

void Project::LoadFromDisk(std::string &project_directory) {
	std::filesystem::path scenes_folder(project_directory + "/.ngine/scenes");
	if (!std::filesystem::exists(scenes_folder)) {
		scenes.clear();
		return;
	}

	std::filesystem::recursive_directory_iterator dir_iter(scenes_folder);
	for (auto &file_entry : dir_iter) {
		if (file_entry.is_regular_file()) {
			std::string filepath(file_entry.path());
			if (filepath.ends_with(".scene.json")) {
				nlohmann::json json;

				std::ifstream filestream(file_entry.path());
				filestream >> json;
				filestream.close();

				Scene scene;
				scene.id = json["id"];
				scene.name = json["name"];
				scene.script_name = json["script_name"];

				scene.fill_color[0] = json["fill_color"][0];
				scene.fill_color[1] = json["fill_color"][1];
				scene.fill_color[2] = json["fill_color"][2];

				scenes.push_back(scene);
			}
		}
	}
}
