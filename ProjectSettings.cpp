#include "ProjectSettings.h"

#include "generated/generated.h"

DisplaySettings::DisplaySettings()
	: resolution(RES_320x240),
	  bit_depth(DEPTH_16),
	  buffers(2),
	  gamma(GAMMA_NONE),
	  antialias(ANTIALIAS_RESAMPLE) {
}

const char *DisplaySettings::GetResolution() {
	switch (resolution) {
		case RES_320x240:
			return "RESOLUTION_320x240";
		case RES_640x480:
			return "RESOLUTION_640x480";
	}
	return "";
}

const char *DisplaySettings::GetBitDepth() {
	switch (bit_depth) {
		case DEPTH_16:
			return "DEPTH_16_BPP";
		case DEPTH_32:
			return "DEPTH_32_BPP";
	}
	return "";
}

const char *DisplaySettings::GetGamma() {
	switch (gamma) {
		case GAMMA_NONE:
			return "GAMMA_NONE";
	}
	return "";
}

const char *DisplaySettings::GetAntialias() {
	switch (antialias) {
		case ANTIALIAS_NONE:
			return "ANTIALIAS_NONE";
		case ANTIALIAS_RESAMPLE:
			return "ANTIALIAS_RESAMPLE";
	}
	return "";
}

void ProjectSettings::LoadFromFile(std::string filepath) {
}

void ProjectSettings::SaveToFile(std::string filepath) {
	FILE *file_project_settings = fopen(filepath.c_str(), "w");

	fprintf(file_project_settings, project_settings_file, display.GetResolution(),
			display.GetBitDepth(), display.buffers, display.GetGamma(), display.GetAntialias());
	fclose(file_project_settings);
}
