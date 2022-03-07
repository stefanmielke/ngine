#include "CodeEditor.h"

#include "App.h"
#include "ConsoleApp.h"
#include "ThreadCommand.h"

bool CodeEditor::OpenPath(App *app, const std::string& path) {
	std::string editor_location(app->engine_settings.GetEditorLocation());
	if (editor_location.empty()) {
		console.AddLog("Editor path not set. Please set it under 'Engine' settings.");
		return false;
	}

	char cmd[255];
	snprintf(cmd, 255, "%s %s", editor_location.c_str(), path.c_str());
	ThreadCommand::RunCommandDetached(cmd);

	return true;
}
