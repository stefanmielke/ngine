#include "LibdragonSound.h"

#include <fstream>

#include "json.hpp"

void LibdragonSound::SaveToDisk(std::string &project_directory) {
	nlohmann::json json = {
		{"name", name},
		{"sound_path", sound_path},
		{"dfs_folder", dfs_folder},
	};

	std::string directory = project_directory + "/.ngine/sounds/";
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directories(directory);

	std::string filepath = directory + name + ".sound.json";

	std::ofstream filestream(filepath);
	filestream << json.dump(4) << std::endl;
	filestream.close();
}

void LibdragonSound::LoadFromDisk(std::string &filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
	sound_path = json["sound_path"];
	dfs_folder = json["dfs_folder"];
}

void LibdragonSound::DeleteFromDisk(std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/sounds/" + name + ".sound.json";
	std::filesystem::remove(json_filepath);

	std::string sound_filepath = project_directory + "/" + sound_path;
	std::filesystem::remove(sound_filepath);
}
