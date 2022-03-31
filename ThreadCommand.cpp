#include "ThreadCommand.h"

#include <queue>
#include <thread>
#include <unistd.h>
#include <utility>

#include "App.h"
#include "ConsoleApp.h"

extern App *g_app;

#ifdef __WIN32__
char separator[] = " && \0";
#else
char separator[] = "\n\0";
#endif

int exec(std::string cmd) {
	char buffer[128];
	std::string output;

	cmd.append(" 2>&1");

	chdir(g_app->project.project_settings.project_directory.c_str());

	auto pipe = popen(cmd.c_str(), "r");
	if (!pipe)
		throw std::runtime_error("popen() failed!");

	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != nullptr) {
			output += buffer;
		}
	}

	if (!output.empty()) {
		console.AddLog("%s", output.c_str());
	}

	int result = pclose(pipe);
	chdir(g_app->GetEngineDirectory().c_str());

	return result;
}

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
		console.AddLog("[error] Process returned %d.", result);
}

static void run_next_command() {
	current_command = command_queue.front();
	std::thread(command_thread, current_command.c_str()).detach();
	command_queue.pop();
}

void ThreadCommand::QueueCommand(std::string command) {
	command_queue.push(std::move(command));

	if (is_running_command)
		return;

	is_running_command = true;
	run_next_command();
}

static void run_command_detached(std::string command) {
	console.AddLog("Running %s", command.c_str());

	exec(command);
}

void ThreadCommand::RunCommand(std::string command) {
	run_command_detached(std::move(command));
}
void ThreadCommand::RunCommandDetached(std::string command) {
	std::thread(run_command_detached, command).detach();
}
