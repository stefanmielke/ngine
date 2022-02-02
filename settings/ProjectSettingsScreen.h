#pragma once

#include "ProjectSettings.h"

struct ProjectSettingsScreen {
	char project_name[100];
	char rom_name[100];

	char display_antialias[100];
	char display_bit_depth[100];
	int display_buffers;
	char display_gamma[100];
	char display_resolution[100];

	void FromProjectSettings(ProjectSettings &project_settings) {
		strcpy(project_name, project_settings.project_name.c_str());
		strcpy(rom_name, project_settings.rom_name.c_str());
		strcpy(display_antialias, project_settings.display.GetAntialias());
		strcpy(display_bit_depth, project_settings.display.GetBitDepth());
		strcpy(display_gamma, project_settings.display.GetGamma());
		strcpy(display_resolution, project_settings.display.GetResolution());
		display_buffers = project_settings.display.buffers;
	}

	void ToProjectSettings(ProjectSettings &project_settings) {
		project_settings.project_name = project_name;
		project_settings.rom_name = rom_name;

		project_settings.display.buffers = display_buffers;
		project_settings.display.SetAntialias(display_antialias);
		project_settings.display.SetBitDepth(display_bit_depth);
		project_settings.display.SetGamma(display_gamma);
		project_settings.display.SetResolution(display_resolution);
	}
};