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

void LibdragonImage::SaveToDisk(const std::string &project_directory) {
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

void LibdragonImage::LoadFromDisk(const std::string &filepath) {
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

void LibdragonImage::DeleteFromDisk(const std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/sprites/" + name + ".sprite.json";
	std::filesystem::remove(json_filepath);

	std::string image_filepath = project_directory + "/" + image_path;
	std::filesystem::remove(image_filepath);
}

void LibdragonImage::LoadImage(const std::string &project_directory, SDL_Renderer *renderer) {
	std::string path(project_directory + "/" + image_path);

	loaded_image = IMG_LoadTexture(renderer, path.c_str());

	int w, h;
	SDL_QueryTexture(loaded_image, nullptr, nullptr, &w, &h);

	width = w;
	height = h;

	const float max_size = 100.f;
	if (w > h) {
		h = (int)(((float)h / (float)w) * max_size);
		w = (int)max_size;
	} else {
		w = (int)(((float)w / (float)h) * max_size);
		h = (int)max_size;
	}

	display_width = w;
	display_height = h;
}

std::string LibdragonImage::GetTooltip() const {
	std::stringstream tooltip;
	tooltip << name << "\nPath: " << image_path << "\nDFS_Path: " << dfs_folder << name
			<< ".sprite\nSize: " << width << "x" << height << "\nSlices: " << h_slices << "x"
			<< v_slices << "\n";

	return tooltip.str();
}

bool libdragon_image_comparison(const std::unique_ptr<LibdragonImage> &i1,
								const std::unique_ptr<LibdragonImage> &i2) {
	return (*i1) < (*i2);
}
