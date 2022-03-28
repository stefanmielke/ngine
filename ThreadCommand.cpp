#include "ThreadCommand.h"

#include <queue>
#include <thread>
#include <utility>

#include "ConsoleApp.h"

#ifdef __WIN32__
char separator[] = " && \0";
#else
char separator[] = "\n\0";
#endif

static bool is_running_command;
static std::string current_command;

static std::queue<std::string> command_queue;

static void run_next_command();

static void command_thread(const char *command) {
	int result = system(command);

	if (command_queue.empty()) {
		is_running_command = false;
	} else {
		run_next_command();
	}

	if (result == 0)
		console.AddLog("Finished successfully.");
	else
		console.AddLog(
			"[error] Process returned %d. Check 'build.log' inside the project folder for more "
			"info.",
			result);
}

static void run_next_command() {
	current_command = command_queue.front();
	std::thread(command_thread, current_command.c_str()).detach();
	command_queue.pop();
}

void ThreadCommand::RunCommand(std::string command) {
	command_queue.push(std::move(command));

	if (is_running_command)
		return;

	is_running_command = true;
	run_next_command();
}

static void run_command_detached(std::string command) {
	console.AddLog("Running %s", command.c_str());

	system(command.c_str());
}

void ThreadCommand::RunCommandDetached(std::string command) {
	std::thread(run_command_detached, command).detach();
}
