#include "LibdragonImage.h"

#include <fstream>

#include "json.hpp"

LibdragonImage::LibdragonImage() : dfs_folder("/"), h_slices(1), v_slices(1) {
}

void LibdragonImage::SaveToDisk(std::string project_directory) {
	nlohmann::json json = {
		{"name", name},			{"image_path", image_path}, {"dfs_folder", dfs_folder},
		{"h_slices", h_slices}, {"v_slices", v_slices},
	};

	std::string directory = project_directory + "/.ngine/sprites/";
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directories(directory);

	std::string filepath = directory + name + ".sprite.json";

	std::ofstream filestream(filepath);
	filestream << json.dump(4) << std::endl;
	filestream.close();
}

void LibdragonImage::LoadFromDisk(std::string filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
	image_path = json["image_path"];
	dfs_folder = json["dfs_folder"];
	h_slices = json["h_slices"];
	v_slices = json["v_slices"];
}
