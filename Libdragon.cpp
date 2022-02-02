#include "Libdragon.h"

#include <thread>

#include "ConsoleApp.h"

extern ConsoleApp console;

bool is_running_command;
std::string current_command;

void command_thread(const char *command) {
	int result = system(command);
	is_running_command = false;

	if (result == 0)
		console.AddLog("Finished successfully.");
	else
		console.AddLog("[error] Process returned %d. Check 'build.log' inside the project folder for more info.", result);
}

void run_command(std::string command) {
	if (is_running_command)
		return;

	current_command = command;
	is_running_command = true;
	std::thread(command_thread, current_command.c_str()).detach();
}

void Libdragon::Init(std::string folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon init", folder.c_str());
	run_command(command);
}
void Libdragon::InitSync(std::string folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon init", folder.c_str());
	system(command);
}

void Libdragon::Build(std::string folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon make -j > build.log", folder.c_str());
	run_command(command);
}

void Libdragon::CleanSync(std::string folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon exec make clean > build.log", folder.c_str());
	system(command);
}
