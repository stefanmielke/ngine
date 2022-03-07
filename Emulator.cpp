#include "Emulator.h"

#include <filesystem>

#include "App.h"
#include "ConsoleApp.h"
#include "ThreadCommand.h"
#include "ProjectBuilder.h"

void Emulator::Run(App *app) {
	std::string rom_filename = app->project.project_settings.rom_name + ".z64";

	console.AddLog("Opening rom in emulator as '%s %s'...",
				   app->engine_settings.GetEmulatorPath().c_str(), rom_filename.c_str());

	if (!std::filesystem::exists(app->project.project_settings.project_directory + "/" + rom_filename)) {
		console.AddLog("Rom file was not created. Triggering build before running...");
		ProjectBuilder::Build(app);
	}

	char cmd[255];
	snprintf(cmd, 255, "cd %s\n%s %s", app->project.project_settings.project_directory.c_str(),
			 app->engine_settings.GetEmulatorPath().c_str(), rom_filename.c_str());
	ThreadCommand::RunCommand(cmd);
}
