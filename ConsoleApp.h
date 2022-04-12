#pragma once

#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <sstream>

#include <SDL2/SDL.h>

#include "imgui.h"

struct ConsoleApp {
	ImVector<char *> Items;
	bool ScrollToBottom;

	ConsoleApp();
	~ConsoleApp();

	void ClearLog();
	void CopyLog();
	void AddLog(const char *fmt, ...) IM_FMTARGS(2);
	void Draw(const char *title, SDL_Window *window, bool &is_open);
};

extern ConsoleApp console;
