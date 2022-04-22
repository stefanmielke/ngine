#include "ConsoleApp.h"

// Portable helpers
static char *Strdup(const char *s) {
	IM_ASSERT(s);
	size_t len = strlen(s) + 1;
	void *buf = malloc(len);
	IM_ASSERT(buf);
	return (char *)memcpy(buf, (const void *)s, len);
}

ConsoleApp::ConsoleApp() {
	ClearLog();

	ScrollToBottom = false;
}
ConsoleApp::~ConsoleApp() {
	ClearLog();
}

void ConsoleApp::ClearLog() {
	for (int i = 0; i < Items.Size; i++)
		free(Items[i]);
	Items.clear();
}

void ConsoleApp::CopyLog() {
	std::stringstream text_stream;
	for (int i = 0; i < Items.Size; i++)
		text_stream << Items[i] << std::endl;

	ImGui::SetClipboardText(text_stream.str().c_str());
}

void ConsoleApp::AddLog(const char *fmt, ...) {
	// FIXME-OPT
	char buf[1024 * 1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
	buf[IM_ARRAYSIZE(buf) - 1] = 0;
	va_end(args);
	Items.push_back(Strdup(buf));
}

void ConsoleApp::Draw(const char *title, SDL_Window *window, bool &is_open) {
	int window_width, window_height;
	SDL_GetWindowSize(window, &window_width, &window_height);

	ImGui::SetNextWindowSize(ImVec2((float)window_width, 200.f));
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);

	bool title_has_color = false;
	std::string title_text(title);
	if (!is_open && Items.Size > 0) {
		title_text = Items[Items.Size - 1];

		if (title_text.starts_with("[error]")) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
			title_has_color = true;
		} else if (title_text.starts_with("! ")) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 1.0f, 0.6f, 1.0f));
			title_has_color = true;
		} else if (title_text.starts_with("# ")) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f));
			title_has_color = true;
		}

		if (title_text.starts_with("! ") || title_text.starts_with("# ")) {
			title_text = title_text.substr(2, title_text.length() - 2);
		}
	}
	title_text.append("###CONSOLE_OUTPUT");

	is_open = true;
	if (!ImGui::Begin(title_text.c_str(), nullptr,
					  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
		if (ImGui::IsWindowCollapsed()) {
			is_open = false;
			ImGui::SetWindowPos(ImVec2(0, (float)window_height - 19.f));
		}
		if (title_has_color) {
			ImGui::PopStyleColor();
		}
		ImGui::End();
		return;
	}
	if (title_has_color) {
		ImGui::PopStyleColor();
	}
	ImGui::SetWindowPos(ImVec2(0, (float)window_height - 200.f));

	if (ImGui::SmallButton("Clear")) {
		ClearLog();
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("Copy Log")) {
		CopyLog();
	}
	ImGui::Separator();

	ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::Selectable("Clear"))
			ClearLog();
		ImGui::EndPopup();
	}

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
		} else if (strncmp(item, "! ", 2) == 0) {
			color = ImVec4(0.6f, 1.0f, 0.6f, 1.0f);
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