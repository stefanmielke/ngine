#pragma once

#include <string>

enum LibdragonSoundType {
	SOUND_UNKNOWN,
	SOUND_WAV,
	SOUND_XM,
	SOUND_YM,
};

class LibdragonSound {
   public:
	std::string name;
	std::string sound_path;
	std::string dfs_folder;

	LibdragonSoundType type;

	bool wav_loop;
	int wav_loop_offset;

	bool ym_compress;

	explicit LibdragonSound(LibdragonSoundType type);

	void SaveToDisk(const std::string &project_directory);
	void LoadFromDisk(const std::string &filepath);
	void DeleteFromDisk(const std::string &project_directory) const;

	[[nodiscard]] std::string GetTooltip() const;
	[[nodiscard]] std::string GetLibdragonGenFlags() const;
	[[nodiscard]] std::string GetLibdragonExtension() const;
};
