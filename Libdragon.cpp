#include "Libdragon.h"

#include <thread>

bool is_running_command;
std::string current_command;

void command_thread(const char *command) {
	system(command);
	is_running_command = false;
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
	snprintf(command, 500, "cd %s\nlibdragon make -j", folder.c_str());
	run_command(command);
}

void Libdragon::Rebuild(std::string folder) {
	char command[500];
	snprintf(command, 500, "cd %s\nlibdragon make clean\nlibdragon make -j", folder.c_str());
	run_command(command);
}
