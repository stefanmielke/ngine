#include "ThreadCommand.h"

#include <thread>
#include <utility>

#include "ConsoleApp.h"

extern ConsoleApp console;

static bool is_running_command;
static std::string current_command;

static void command_thread(const char *command) {
	int result = system(command);
	is_running_command = false;

	if (result == 0)
		console.AddLog("Finished successfully.");
	else
		console.AddLog(
			"[error] Process returned %d. Check 'build.log' inside the project folder for more "
			"info.",
			result);
}

void ThreadCommand::RunCommand(std::string command) {
	if (is_running_command)
		return;

	current_command = std::move(command);
	is_running_command = true;
	std::thread(command_thread, current_command.c_str()).detach();
}
