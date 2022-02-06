#pragma once

class App;

class AppGui {
   public:
	static void Update(App &app);

   private:
	static void RenderMenuBar(App &app);
	static void RenderNewProjectWindow(App &app);
	static void RenderOpenProjectWindow(App &app);
	static void RenderContentBrowser(App &app);
	static void RenderSceneWindow(App &app);
	static void RenderSettingsWindow(App &app);
};
