#include "LibdragonSound.h"

#include <fstream>
#include <sstream>

#include "json.hpp"

LibdragonSound::LibdragonSound(LibdragonSoundType type)
	: type(type), wav_loop(false), wav_loop_offset(0), ym_compress(false) {
}

void LibdragonSound::SaveToDisk(std::string &project_directory) {
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

void LibdragonSound::LoadFromDisk(std::string &filepath) {
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
}

void LibdragonSound::DeleteFromDisk(std::string &project_directory) const {
	std::string json_filepath = project_directory + "/.ngine/sounds/" + name + ".sound.json";
	std::filesystem::remove(json_filepath);

	std::string sound_filepath = project_directory + "/" + sound_path;
	std::filesystem::remove(sound_filepath);
}

std::string LibdragonSound::GetTooltip() const {
	std::stringstream tooltip;
	tooltip << name << "\nPath: " << sound_path << "\nDFS_Path: " << dfs_folder << name
			<< ".wav64\n";

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
	return tooltip.str();
}
