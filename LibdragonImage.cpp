#include "LibdragonImage.h"

#include <fstream>

#include "json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_custom.h"

std::string get_libdragon_image_type_name(LibdragonImageType type) {
	switch (type) {
		case IMAGE_UNKNOWN:
			return "unknown";
		case IMAGE_PNG:
			return "png";
		case IMAGE_BMP:
			return "bmp";
		case IMAGE_JPG:
			return "jpg";
		case IMAGE_JPEG:
			return "jpeg";
		case IMAGE_PCX:
			return "pcx";
		case IMAGE_TGA:
			return "tga";
	}

	return "none";
}

std::string get_libdragon_image_type_extension(LibdragonImageType type) {
	switch (type) {
		case IMAGE_UNKNOWN:
			return ".unknown";
		case IMAGE_PNG:
			return ".png";
		case IMAGE_BMP:
			return ".bmp";
		case IMAGE_JPG:
			return ".jpg";
		case IMAGE_JPEG:
			return ".jpeg";
		case IMAGE_PCX:
			return ".pcx";
		case IMAGE_TGA:
			return ".tga";
	}

	return ".none";
}

LibdragonImage::LibdragonImage()
	: dfs_folder("/"),
	  h_slices(1),
	  v_slices(1),
	  width(0),
	  height(0),
	  display_width(0),
	  display_height(0),
	  type(IMAGE_PNG),
	  loaded_image(nullptr) {
}

LibdragonImage::~LibdragonImage() {
	if (loaded_image) {
		SDL_DestroyTexture(loaded_image);
		loaded_image = nullptr;
	}
	if (loaded_image_overlay) {
		SDL_DestroyTexture(loaded_image_overlay);
		loaded_image_overlay = nullptr;
	}
}

void LibdragonImage::SaveToDisk(const std::string &project_directory) {
	nlohmann::json json = {
		{"name", name},			{"image_path", image_path}, {"dfs_folder", dfs_folder},
		{"h_slices", h_slices}, {"v_slices", v_slices},		{"type", type},
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
	if (!json["type"].is_null())
		type = json["type"];

	std::replace(dfs_folder.begin(), dfs_folder.end(), '\\', '/');
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

	loaded_image_overlay = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
											 SDL_TEXTUREACCESS_TARGET, w * 2, h * 2);
	SDL_SetTextureBlendMode(loaded_image_overlay, SDL_BLENDMODE_BLEND);

	const float max_size = 130.f;
	if (w > h) {
		h = (int)(((float)h / (float)w) * max_size);
		w = (int)max_size;
	} else {
		w = (int)(((float)w / (float)h) * max_size);
		h = (int)max_size;
	}

	display_width = w;
	display_height = h;

	RecreateOverlay(renderer, h_slices, v_slices);
}

void LibdragonImage::RecreateOverlay(SDL_Renderer *renderer, int overlay_h_slices, int overlay_v_slices) {
	SDL_SetRenderTarget(renderer, loaded_image_overlay);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

	int total_h_slices = overlay_h_slices - 1;
	if (total_h_slices > 0) {
		float steps = ((float)width * 2) / (float)overlay_h_slices;
		for (int h = 1; h <= total_h_slices; ++h) {
			SDL_RenderDrawLine(renderer, steps * h, 0, steps * h, (height * 2));
		}
	}
	int total_v_slices = overlay_v_slices - 1;
	if (total_v_slices > 0) {
		float steps = ((float)height * 2) / (float)overlay_v_slices;
		for (int v = 1; v <= total_v_slices; ++v) {
			SDL_RenderDrawLine(renderer, 0, steps * v, (width * 2), steps * v);
		}
	}

	SDL_Rect rect = {0, 0, width * 2, height * 2};
	SDL_RenderDrawRect(renderer, &rect);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	SDL_SetRenderTarget(renderer, nullptr);
}

void LibdragonImage::DrawTooltip() const {
	std::stringstream tooltip;
	tooltip << "Path: " << image_path << "\nDFS_Path: " << dfs_folder << name
			<< ".sprite\nSize: " << width << "x" << height << "\nSlices: " << h_slices << "x"
			<< v_slices << "\n";

	ImGui::BeginTooltip();
	render_badge("sprite", ImVec4(.4f, .8f, .4f, 0.7f));
	ImGui::SameLine();
	render_badge(get_libdragon_image_type_name(type).c_str(), ImVec4(.4f, .8f, .4f, 0.7f));
	ImGui::SameLine();
	ImGui::Text("%s", name.c_str());
	ImGui::Separator();

	ImGui::Text("%s", tooltip.str().c_str());
	ImGui::Separator();

	ImGui::Image((ImTextureID)(intptr_t)loaded_image, ImVec2((float)width, (float)height));
	ImGui::EndTooltip();
}
