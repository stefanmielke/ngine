#pragma once

#include <string>
#include "settings/DisplaySettings.h"

class App;

class Libdragon {
   public:
	static bool InitSync(const App *app);
	static void Build(const App *app);
	static void Clean(const App *app);
	static void CleanSync(const App *app);
	static void Update(const App *app);
	static void Install(const App *app);
	static void Disasm(const App *app);
	static void Exec(const App *app, const std::string &command);
	static std::string GetVersion(const App *app);

	static void GitCheckout(const App *app, const std::string &checkout_folder_relative,
							const std::string &branch);
	static bool GitSubmoduleAddSync(const App *app, const std::string &submodule_uri,
									const std::string &submodule_folder);

	static unsigned int GetColor3(const float color[3], BitDepth bit_depth);
	static unsigned int GetColor4(const float color[4], BitDepth bit_depth);
};
