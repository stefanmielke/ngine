#pragma once

#include <string>

#include "settings/EngineSettings.h"

class App;

class AppGui {
   public:
	static void Update(App &app);

	static void ChangeTheme(App &app, Theme theme);

	static void ProcessImportFile(App &app, std::string file_path);

   private:
	static void RenderMenuBar(App &app);
	static void RenderNewProjectWindow(App &app);
	static void RenderOpenProjectWindow(App &app);
	static void RenderContentBrowser(App &app);
	static void RenderSceneWindow(App &app);
	static void RenderSettingsWindow(App &app);

	static void RenderContentBrowserNew(App &app);
};
