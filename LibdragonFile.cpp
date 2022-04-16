#include "LibdragonFile.h"

#include <fstream>

#include "json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_custom.h"

LibdragonFile::LibdragonFile() : dfs_folder("/"), copy_to_filesystem(true) {
}

void LibdragonFile::SaveToDisk(const std::string &project_directory) {
	nlohmann::json json = {
		{"name", name},
		{"file_type", file_type},
		{"file_path", file_path},
		{"dfs_folder", dfs_folder},
		{"copy_to_filesystem", copy_to_filesystem},
	};

	std::string directory = project_directory + "/.ngine/general/";
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directories(directory);

	std::string filepath = directory + GetFilename() + ".general.json";

	std::ofstream filestream(filepath);
	filestream << json.dump(4) << std::endl;
	filestream.close();
}

void LibdragonFile::LoadFromDisk(const std::string &filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
	file_type = json["file_type"];
	file_path = json["file_path"];
	dfs_folder = json["dfs_folder"];
	copy_to_filesystem = json["copy_to_filesystem"];
}

void LibdragonFile::DeleteFromDisk(const std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/general/" + GetFilename() +
								".general.json";
	std::filesystem::remove(json_filepath);

	std::string filepath = project_directory + "/" + file_path;
	std::filesystem::remove(filepath);
}

std::string LibdragonFile::GetFilename() const {
	return name + file_type;
}

void LibdragonFile::DrawTooltip() const {
	std::stringstream tooltip;
	tooltip << "Path: " << file_path << "\nDFS_Path: " << dfs_folder << GetFilename()
			<< "\nCopy to Filesystem: " << (copy_to_filesystem ? "yes" : "no") << "\n";

	ImGui::BeginTooltip();
	render_badge("file", ImVec4(1.f, .4f, .4f, 0.7f));
	ImGui::SameLine();
	ImGui::Text("%s", GetFilename().c_str());
	ImGui::Separator();

	ImGui::Text("%s", tooltip.str().c_str());
	ImGui::EndTooltip();
}

bool libdragon_file_comparison(const std::unique_ptr<LibdragonFile> &f1,
							   const std::unique_ptr<LibdragonFile> &f2) {
	return (*f1) < (*f2);
}
