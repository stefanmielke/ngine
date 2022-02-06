#include "LibdragonImage.h"

#include <fstream>

#include "json.hpp"

LibdragonImage::LibdragonImage()
	: dfs_folder("/"),
	  h_slices(1),
	  v_slices(1),
	  width(0),
	  height(0),
	  display_width(0),
	  display_height(0),
	  loaded_image(nullptr) {
}

LibdragonImage::~LibdragonImage() {
	if (loaded_image) {
		SDL_DestroyTexture(loaded_image);
		loaded_image = nullptr;
	}
}

void LibdragonImage::SaveToDisk(std::string &project_directory) {
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

void LibdragonImage::LoadFromDisk(std::string &filepath) {
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

void LibdragonImage::DeleteFromDisk(std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/sprites/" + name + ".sprite.json";
	std::filesystem::remove(json_filepath);

	std::string image_filepath = project_directory + "/" + image_path;
	std::filesystem::remove(image_filepath);
}
