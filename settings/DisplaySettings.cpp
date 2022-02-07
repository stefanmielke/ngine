#include "DisplaySettings.h"

DisplaySettings::DisplaySettings()
	: resolution(RESOLUTION_320x240),
	  bit_depth(DEPTH_16_BPP),
	  buffers(2),
	  gamma(GAMMA_NONE),
	  antialias(ANTIALIAS_RESAMPLE) {
}

const char *DisplaySettings::GetResolution() const {
	switch (resolution) {
		case RESOLUTION_320x240:
			return "RESOLUTION_320x240";
		case RESOLUTION_640x480:
			return "RESOLUTION_640x480";
		case RESOLUTION_256x240:
			return "RESOLUTION_256x240";
		case RESOLUTION_512x480:
			return "RESOLUTION_512x480";
		case RESOLUTION_512x240:
			return "RESOLUTION_512x240";
		case RESOLUTION_640x240:
			return "RESOLUTION_640x240";
	}
	return "";
}
const char *DisplaySettings::GetBitDepth() const {
	switch (bit_depth) {
		case DEPTH_16_BPP:
			return "DEPTH_16_BPP";
		case DEPTH_32_BPP:
			return "DEPTH_32_BPP";
	}
	return "";
}
const char *DisplaySettings::GetGamma() const {
	switch (gamma) {
		case GAMMA_NONE:
			return "GAMMA_NONE";
		case GAMMA_CORRECT:
			return "GAMMA_CORRECT";
		case GAMMA_CORRECT_DITHER:
			return "GAMMA_CORRECT_DITHER";
	}
	return "";
}
const char *DisplaySettings::GetAntialias() const {
	switch (antialias) {
		case ANTIALIAS_OFF:
			return "ANTIALIAS_OFF";
		case ANTIALIAS_RESAMPLE:
			return "ANTIALIAS_RESAMPLE";
		case ANTIALIAS_RESAMPLE_FETCH_NEEDED:
			return "ANTIALIAS_RESAMPLE_FETCH_NEEDED";
		case ANTIALIAS_RESAMPLE_FETCH_ALWAYS:
			return "ANTIALIAS_RESAMPLE_FETCH_ALWAYS";
	}
	return "";
}

void DisplaySettings::SetResolution(const std::string& value) {
	if (value == "RESOLUTION_320x240") {
		resolution = RESOLUTION_320x240;
	} else if (value == "RESOLUTION_640x480") {
		resolution = RESOLUTION_640x480;
	} else if (value == "RESOLUTION_256x240") {
		resolution = RESOLUTION_256x240;
	} else if (value == "RESOLUTION_512x480") {
		resolution = RESOLUTION_512x480;
	} else if (value == "RESOLUTION_512x240") {
		resolution = RESOLUTION_512x240;
	} else if (value == "RESOLUTION_640x240") {
		resolution = RESOLUTION_640x240;
	}
}
void DisplaySettings::SetBitDepth(const std::string& value) {
	if (value == "DEPTH_16_BPP") {
		bit_depth = DEPTH_16_BPP;
	} else if (value == "DEPTH_32_BPP") {
		bit_depth = DEPTH_32_BPP;
	}
}
void DisplaySettings::SetGamma(const std::string& value) {
	if (value == "GAMMA_NONE") {
		gamma = GAMMA_NONE;
	} else if (value == "GAMMA_CORRECT") {
		gamma = GAMMA_CORRECT;
	} else if (value == "GAMMA_CORRECT_DITHER") {
		gamma = GAMMA_CORRECT_DITHER;
	}
}
void DisplaySettings::SetAntialias(const std::string& value) {
	if (value == "ANTIALIAS_OFF") {
		antialias = ANTIALIAS_OFF;
	} else if (value == "ANTIALIAS_RESAMPLE") {
		antialias = ANTIALIAS_RESAMPLE;
	} else if (value == "ANTIALIAS_RESAMPLE_FETCH_NEEDED") {
		antialias = ANTIALIAS_RESAMPLE_FETCH_NEEDED;
	} else if (value == "ANTIALIAS_RESAMPLE_FETCH_ALWAYS") {
		antialias = ANTIALIAS_RESAMPLE_FETCH_ALWAYS;
	}
}