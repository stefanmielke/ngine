#include <fstream>
#include "ProjectSettings.h"

#include "generated/generated.h"
#include "json.hpp"

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

void DisplaySettings::SetResolution(std::string value) {
	if (value == "RESOLUTION_320x240") {
		resolution = RES_320x240;
	} else if (value == "RESOLUTION_320x240") {
		resolution = RES_640x480;
	}
}
void DisplaySettings::SetBitDepth(std::string value) {
	if (value == "DEPTH_16_BPP") {
		bit_depth = DEPTH_16;
	} else if (value == "DEPTH_32_BPP") {
		bit_depth = DEPTH_32;
	}
}
void DisplaySettings::SetBuffers(std::string value) {
	buffers = std::stoi(value);
}
void DisplaySettings::SetGamma(std::string value) {
	if (value == "GAMMA_NONE") {
		gamma = GAMMA_NONE;
	}
}
void DisplaySettings::SetAntialias(std::string value) {
	if (value == "ANTIALIAS_NONE") {
		antialias = ANTIALIAS_NONE;
	} else if (value == "ANTIALIAS_RESAMPLE") {
		antialias = ANTIALIAS_RESAMPLE;
	}
}

void ProjectSettings::LoadFromFile(std::string filepath) {
	std::ifstream project_file(filepath);

	nlohmann::json json;
	project_file >> json;

	display.SetResolution(json["display"]["resolution"]);
	display.SetBitDepth(json["display"]["bit_depth"]);
	display.SetBuffers(json["display"]["buffers"]);
	display.SetGamma(json["display"]["gamma"]);
	display.SetAntialias(json["display"]["antialias"]);

	project_file.close();

	is_open = true;
}

void ProjectSettings::SaveToFile(std::string filepath) {
	FILE *file_project_settings = fopen(filepath.c_str(), "w");

	fprintf(file_project_settings, project_settings_file, display.GetResolution(),
			display.GetBitDepth(), display.buffers, display.GetGamma(), display.GetAntialias());
	fclose(file_project_settings);
}

void ProjectSettings::CloseProject() {
	is_open = false;
	project_directory.clear();
}
