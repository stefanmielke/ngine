#include "ThreadCommand.h"

#include <queue>
#include <thread>
#include <utility>

#include <iostream>
#include <sstream>

#include "ConsoleApp.h"

#ifdef __WIN32__
char separator[] = " && \0";

#define exec system
#else
#include <unistd.h>

char separator[] = "\n\0";

int exec(std::string cmd) {
	std::array<char, 128> buffer{};
	std::string result;

	cmd.append(" 2>&1 &");

	auto pipe = popen(cmd.c_str(), "r");
	if (!pipe)
		throw std::runtime_error("popen() failed!");

	while (!feof(pipe)) {
		if (fgets(buffer.data(), 128, pipe) != nullptr)
			result += buffer.data();
	}

	if (!result.empty()) {
		console.AddLog("%s", result.c_str());
	}

	return pclose(pipe);
}
#endif

static bool is_running_command;
static std::string current_command;

static std::queue<std::string> command_queue;

static void run_next_command();

static void command_thread(const char *command) {
	int result = exec(command);

	if (command_queue.empty()) {
		is_running_command = false;
	} else {
		run_next_command();
	}

	if (result == EXIT_SUCCESS)
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
