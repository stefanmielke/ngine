#include "Emulator.h"

#include <filesystem>

#include "ConsoleApp.h"
#include "ThreadCommand.h"
#include "ProjectBuilder.h"

extern ConsoleApp console;

void Emulator::Run(EngineSettings &engine_settings, ProjectSettings &project_settings,
				   Project &project) {
	std::string rom_filename = project_settings.rom_name + ".z64";

	console.AddLog("Opening rom in emulator as '%s %s'...",
				   engine_settings.GetMupen64Path().c_str(), rom_filename.c_str());

	if (!std::filesystem::exists(project_settings.project_directory + "/" + rom_filename)) {
		console.AddLog("Rom file was not created. Triggering build before running...");
		ProjectBuilder::Build(project_settings, project);
	}

	char cmd[255];
	snprintf(cmd, 255, "cd %s\n%s %s", project_settings.project_directory.c_str(),
			 engine_settings.GetMupen64Path().c_str(), rom_filename.c_str());
	ThreadCommand::RunCommand(cmd);
}
