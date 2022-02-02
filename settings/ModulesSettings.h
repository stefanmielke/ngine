#pragma once

class ModulesSettings {
   public:
	bool display;
	bool dfs;
	bool rdp;
	bool timer;
	bool controller;
	bool console;

	bool debug_is_viewer;
	bool debug_usb;

	ModulesSettings();
};
