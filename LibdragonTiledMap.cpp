#include "LibdragonTiledMap.h"

#include <fstream>

#include "App.h"
#include "ConsoleApp.h"
#include "json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_custom.h"
#include "pugixml/pugixml.hpp"

extern App *g_app;

LibdragonTiledMap::LibdragonTiledMap() : dfs_folder("/") {
}

void LibdragonTiledMap::SaveToDisk(const std::string &project_directory) {
	nlohmann::json json = {
		{"name", name},
		{"file_path", file_path},
		{"dfs_folder", dfs_folder},
	};

	for (auto &layer : layers) {
		nlohmann::json json_layer = {{"name", layer.name}};

		json["layers"].push_back(json_layer);
	}

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

	LoadLayers();
}

void LibdragonTiledMap::DeleteFromDisk(const std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/tiled_maps/" + name +
								".tiled_maps.json";
	std::filesystem::remove(json_filepath);

	std::string filepath = project_directory + "/" + file_path;
	std::filesystem::remove(filepath);
}

void LibdragonTiledMap::LoadLayers() {
	std::string full_file_path = g_app->project.project_settings.project_directory + "/" +
								 file_path;
	layers = LoadLayers(full_file_path);
}

std::vector<LibdragonMapLayer> LibdragonTiledMap::LoadLayers(const std::string &file_path) {
	pugi::xml_document tmx_file;

	auto result = tmx_file.load_file(file_path.c_str());
	if (result.status != pugi::status_ok) {
		console.AddLog("Error loading tiled map: %s", result.description());
	}

	std::vector<LibdragonMapLayer> layers;

	auto xml_layers = tmx_file.child("map").children("layer");
	for (auto &xml_layer : xml_layers) {
		LibdragonMapLayer layer;
		layer.name = xml_layer.attribute("name").value();

		layers.push_back(layer);
	}

	return layers;
}

void LibdragonTiledMap::DrawTooltip() const {
	std::stringstream tooltip;
	tooltip << "Path: " << file_path << "\n\nLayers:";
	for (auto &layer : layers) {
		tooltip << "\n- " << layer.name << " (DFS Path: " << dfs_folder << name << "_" << layer.name
				<< ".map)";
	}

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
