#include "LibdragonTiledMap.h"

#include <fstream>

#include "json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_custom.h"

LibdragonTiledMap::LibdragonTiledMap() : dfs_folder("/") {
}

void LibdragonTiledMap::SaveToDisk(const std::string &project_directory) {
	nlohmann::json json = {
		{"name", name},
		{"file_path", file_path},
		{"dfs_folder", dfs_folder},
	};

	std::string directory = project_directory + "/.ngine/tiled_maps/";
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directories(directory);

	std::string filepath = directory + name + ".tiled_maps.json";

	std::ofstream filestream(filepath);
	filestream << json.dump(4) << std::endl;
	filestream.close();
}

void LibdragonTiledMap::LoadFromDisk(const std::string &filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
	file_path = json["file_path"];
	dfs_folder = json["dfs_folder"];
}

void LibdragonTiledMap::DeleteFromDisk(const std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/tiled_maps/" + name +
								".tiled_maps.json";
	std::filesystem::remove(json_filepath);

	std::string filepath = project_directory + "/" + file_path;
	std::filesystem::remove(filepath);
}

void LibdragonTiledMap::DrawTooltip() const {
	std::stringstream tooltip;
	tooltip << "Path: " << file_path << ".tmx"
			<< "\nDFS Path: " << dfs_folder << name << ".map";

	ImGui::BeginTooltip();
	render_badge("map", ImVec4(.4f, .1f, .1f, 0.7f));
	ImGui::SameLine();
	render_badge("tiled", ImVec4(.4f, .1f, .1f, 0.7f));
	ImGui::SameLine();
	ImGui::Text("%s", name.c_str());
	ImGui::Separator();

	ImGui::Text("%s", tooltip.str().c_str());
	ImGui::EndTooltip();
}
