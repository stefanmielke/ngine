#include "LibdragonSound.h"

#include <fstream>
#include <sstream>

#include "json.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_custom.h"

LibdragonSound::LibdragonSound(LibdragonSoundType type)
	: type(type), wav_loop(false), wav_loop_offset(0), ym_compress(false) {
}

void LibdragonSound::SaveToDisk(const std::string &project_directory) {
	nlohmann::json json = {
		{"name", name},
		{"sound_path", sound_path},
		{"dfs_folder", dfs_folder},
		{"type", type},
		{"wav_loop", wav_loop},
		{"wav_loop_offset", wav_loop_offset},
		{"ym_compress", ym_compress},
	};

	std::string directory = project_directory + "/.ngine/sounds/";
	if (!std::filesystem::exists(directory))
		std::filesystem::create_directories(directory);

	std::string filepath = directory + name + ".sound.json";

	std::ofstream filestream(filepath);
	filestream << json.dump(4) << std::endl;
	filestream.close();
}

void LibdragonSound::LoadFromDisk(const std::string &filepath) {
	nlohmann::json json;

	std::ifstream filestream(filepath);
	filestream >> json;
	filestream.close();

	name = json["name"];
	sound_path = json["sound_path"];
	dfs_folder = json["dfs_folder"];

	type = json["type"];
	wav_loop = json["wav_loop"];
	wav_loop_offset = json["wav_loop_offset"];
	ym_compress = json["ym_compress"];

	std::replace(dfs_folder.begin(), dfs_folder.end(), '\\', '/');
}

void LibdragonSound::DeleteFromDisk(const std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/sounds/" + name + ".sound.json";
	std::filesystem::remove(json_filepath);

	std::string sound_filepath = project_directory + "/" + sound_path;
	std::filesystem::remove(sound_filepath);
}

void LibdragonSound::DrawTooltip() const {
	std::string dfs_prefix;
	if (type == SOUND_XM || type == SOUND_YM) {
		dfs_prefix = "rom:";
	}

	std::stringstream tooltip;
	tooltip << "Path: " << sound_path << "\nDFS Path: " << dfs_prefix << dfs_folder
			<< name << GetLibdragonExtension() << "\n";

	switch (type) {
		case SOUND_UNKNOWN: {
			tooltip << "\nType: UNKNOWN";
		} break;
		case SOUND_WAV: {
			tooltip << "\nType: WAV (Waveforms)\nLoop: " << (wav_loop ? "Yes" : "No");
			if (wav_loop)
				tooltip << "\nLoop Offset: " << wav_loop_offset;
		} break;
		case SOUND_XM: {
			tooltip << "\nType: XM (MilkyTracker, OpenMPT)";
		} break;
		case SOUND_YM: {
			tooltip << "\nType: YM (Arkos Tracker II)\nCompress: " << (ym_compress ? "Yes" : "No");
		} break;
		default: {
			tooltip << "\nType: NOT MAPPED";
		} break;
	}

	ImGui::BeginTooltip();
	render_badge("sound", ImVec4(.4f, .4f, 1.f, 0.7f));
	ImGui::SameLine();
	render_badge(GetExtensionName().c_str(), ImVec4(.4f, .4f, 1.f, 0.7f));
	ImGui::SameLine();
	ImGui::Text("%s", name.c_str());
	ImGui::Separator();

	ImGui::Text("%s", tooltip.str().c_str());
	ImGui::EndTooltip();
}
std::string LibdragonSound::GetLibdragonExtension() const {
	switch (type) {
		case SOUND_UNKNOWN:
			return ".no_ext";
		case SOUND_WAV:
			return ".wav64";
		case SOUND_XM:
			return ".xm64";
		case SOUND_YM:
			return ".ym64";
		default:
			return ".not_mapped";
	}
}
std::string LibdragonSound::GetLibdragonGenFlags() const {
	switch (type) {
		case SOUND_WAV: {
			if (wav_loop)
				return "--wav-loop true --wav-loop-offset " + std::to_string(wav_loop_offset);
			else
				return "";
		}
		case SOUND_YM:
			return std::string("--ym-compress ") + (ym_compress ? "true" : "false");
		case SOUND_UNKNOWN:
		case SOUND_XM:
		default:
			return "";
	}
}

std::string LibdragonSound::GetExtension() const {
	switch (type) {
		case SOUND_UNKNOWN:
			return ".no_ext";
		case SOUND_WAV:
			return ".wav";
		case SOUND_XM:
			return ".xm";
		case SOUND_YM:
			return ".ym";
		default:
			return ".not_mapped";
	}
}

std::string LibdragonSound::GetExtensionName() const {
	switch (type) {
		case SOUND_UNKNOWN:
			return "no_ext";
		case SOUND_WAV:
			return "wav";
		case SOUND_XM:
			return "xm";
		case SOUND_YM:
			return "ym";
		default:
			return "not_mapped";
	}
}
