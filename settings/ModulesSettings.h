#pragma once

class ModulesSettings {
   public:
	bool display;
	bool dfs;
	bool rdp;
	bool timer;
	bool controller;
	bool console;
	bool audio;
	bool audio_mixer;
	bool scene_manager;
	bool memory_pool;

	bool debug_is_viewer;
	bool debug_usb;

	ModulesSettings();
};
