#pragma once

#include <cstring>
#include <cctype>
#include <cstdlib>
#include <cstdio>

#include <SDL2/SDL.h>

#include "imgui.h"

struct ConsoleApp {
	ImVector<char *> Items;
	bool ScrollToBottom;

	ConsoleApp() {
		ClearLog();

		ScrollToBottom = false;
	}
	~ConsoleApp() {
		ClearLog();
	}

	// Portable helpers
	static char *Strdup(const char *s) {
		IM_ASSERT(s);
		size_t len = strlen(s) + 1;
		void *buf = malloc(len);
		IM_ASSERT(buf);
		return (char *)memcpy(buf, (const void *)s, len);
	}

	void ClearLog() {
		for (int i = 0; i < Items.Size; i++)
			free(Items[i]);
		Items.clear();
	}

	void AddLog(const char *fmt, ...) IM_FMTARGS(2) {
		// FIXME-OPT
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		Items.push_back(Strdup(buf));
	}

	void Draw(const char *title, SDL_Window *window, bool &is_open) {
		is_open = true;

		int window_width, window_height;
		SDL_GetWindowSize(window, &window_width, &window_height);

		ImGui::SetNextWindowSize(ImVec2(window_width, 200));
		if (!ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
			if (ImGui::IsWindowCollapsed()) {
				is_open = false;
				ImGui::SetWindowPos(ImVec2(0, window_height - 19));
			}
			ImGui::End();
			return;
		}
		ImGui::SetWindowPos(ImVec2(0, window_height - 200));

		// TODO: display items starting from the bottom
		if (ImGui::SmallButton("Clear")) {
			ClearLog();
		}
		ImGui::Separator();

		ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false,
						  ImGuiWindowFlags_HorizontalScrollbar);
		if (ImGui::BeginPopupContextWindow()) {
			if (ImGui::Selectable("Clear"))
				ClearLog();
			ImGui::EndPopup();
		}

		// Display every line as a separate entry so we can change their color or add custom
		// widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(),
		// log.end()); NB- if you have thousands of entries this approach may be too inefficient and
		// may require user-side clipping to only process visible items. The clipper will
		// automatically measure the height of your first item and then "seek" to display only items
		// in the visible area. To use the clipper we can replace your standard loop:
		//      for (int i = 0; i < Items.Size; i++)
		//   With:
		//      ImGuiListClipper clipper;
		//      clipper.Begin(Items.Size);
		//      while (clipper.Step())
		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		// - That your items are evenly spaced (same height)
		// - That you have cheap random access to your elements (you can access them given their
		// index,
		//   without processing all the ones before)
		// You cannot this code as-is if a filter is active because it breaks the 'cheap
		// random-access' property. We would need random-access on the post-filtered list. A typical
		// application wanting coarse clipping and filtering may want to pre-compute an array of
		// indices or offsets of items that passed the filtering test, recomputing this array when
		// user changes the filter, and appending newly elements as they are inserted. This is left
		// as a task to the user until we can manage to improve this example code! If your items are
		// of variable height:
		// - Split them into same height items would be simpler and facilitate random-seeking into
		// your list.
		// - Consider using manual call to IsRectVisible() and skipping extraneous decoration from
		// your items.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));  // Tighten spacing

		for (int i = 0; i < Items.Size; i++) {
			const char *item = Items[i];

			// Normally you would store more information in your item than just a string.
			// (e.g. make Items[] an array of structure, store color/type etc.)
			ImVec4 color;
			bool has_color = false;
			if (strstr(item, "[error]")) {
				color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
				has_color = true;
			} else if (strncmp(item, "# ", 2) == 0) {
				color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
				has_color = true;
			}
			if (has_color)
				ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::TextUnformatted(item);
			if (has_color)
				ImGui::PopStyleColor();
		}

		if (ScrollToBottom || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;

		ImGui::PopStyleVar();
		ImGui::EndChild();

		ImGui::End();
	}
};

extern ConsoleApp console;
