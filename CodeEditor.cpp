#include "CodeEditor.h"

#include "App.h"
#include "ThreadCommand.h"

void CodeEditor::OpenPath(App *app, const std::string& path) {
	char cmd[255];
	snprintf(cmd, 255, "%s %s", app->engine_settings.GetEditorLocation().c_str(), path.c_str());
	ThreadCommand::RunCommandDetached(cmd);
}
