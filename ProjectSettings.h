#pragma once

#include <string>
enum Resolution {
	RES_320x240,
	RES_640x480,
};

enum BitDepth {
	DEPTH_16,
	DEPTH_32,
};

enum Gamma {
	GAMMA_NONE,
};

enum Antialias {
	ANTIALIAS_NONE,
	ANTIALIAS_RESAMPLE,
};

class DisplaySettings {
   public:
	Resolution resolution;
	BitDepth bit_depth;
	int buffers;
	Gamma gamma;
	Antialias antialias;

	DisplaySettings();

	const char *GetResolution();
	const char *GetBitDepth();
	const char *GetGamma();
	const char *GetAntialias();
};

class ProjectSettings {
   public:
	DisplaySettings display;

	void LoadFromFile(std::string filepath);
	void SaveToFile(std::string filepath);
};
