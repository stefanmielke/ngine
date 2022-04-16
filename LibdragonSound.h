#pragma once

#include <memory>
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

	void DrawTooltip() const;
	[[nodiscard]] std::string GetLibdragonGenFlags() const;
	[[nodiscard]] std::string GetLibdragonExtension() const;
	[[nodiscard]] std::string GetExtension() const;
	[[nodiscard]] std::string GetExtensionName() const;

	bool operator<(const LibdragonSound &other) const {
		return (name < other.name);
	}
};

bool libdragon_sound_comparison(const std::unique_ptr<LibdragonSound> &s1,
								const std::unique_ptr<LibdragonSound> &s2);
