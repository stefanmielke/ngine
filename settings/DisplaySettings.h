#pragma once

#include <string>

enum Resolution {
	RESOLUTION_320x240,
	RESOLUTION_640x480,
	RESOLUTION_256x240,
	RESOLUTION_512x480,
	RESOLUTION_512x240,
	RESOLUTION_640x240,
};

enum BitDepth {
	DEPTH_16_BPP,
	DEPTH_32_BPP,
};

enum Gamma {
	GAMMA_NONE,
	GAMMA_CORRECT,
	GAMMA_CORRECT_DITHER,
};

enum Antialias {
	ANTIALIAS_OFF,
	ANTIALIAS_RESAMPLE,
	ANTIALIAS_RESAMPLE_FETCH_NEEDED,
	ANTIALIAS_RESAMPLE_FETCH_ALWAYS,
};

class DisplaySettings {
   public:
	Resolution resolution;
	BitDepth bit_depth;
	int buffers;
	Gamma gamma;
	Antialias antialias;

	DisplaySettings();

	[[nodiscard]] const char *GetResolution() const;
	[[nodiscard]] const char *GetBitDepth() const;
	[[nodiscard]] const char *GetGamma() const;
	[[nodiscard]] const char *GetAntialias() const;

	void SetResolution(const std::string& value);
	void SetBitDepth(const std::string& value);
	void SetGamma(const std::string& value);
	void SetAntialias(const std::string& value);
};
