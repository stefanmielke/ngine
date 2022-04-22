#include "imgui_custom.h"

#include <string>

#include "../App.h"
#include "../ThreadCommand.h"

extern App *g_app;

ImVec4 color_invalid_input = ImVec4(1, .3f, .3f, 1);

void render_badge(const char *label, ImVec4 color) {
	ImGui::BeginDisabled();
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	ImGuiStyle &style = ImGui::GetStyle();
	float prev_alpha = style.Alpha;
	style.Alpha = 1.f;
	ImGui::SmallButton(label);
	style.Alpha = prev_alpha;
	ImGui::PopStyleColor();
	ImGui::EndDisabled();
}

bool link_button(const char *label, const char *url) {
	if (g_app->engine_settings.GetTheme() == THEME_LIGHT) {
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, .1f));
	} else {
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, .1f));
	}

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 1, 1));

	bool result = ImGui::Button(label);
	if (result && url) {
		App::OpenUrl(url);
	}

	ImGui::PopStyleColor(6);

	return result;
}

bool input_text_dfs_folder(char *buf, int buf_size) {
	std::string dfs_folder_string(buf);
	bool dfs_invalid = !dfs_folder_string.ends_with("/");

	if (dfs_invalid)
		ImGui::PushStyleColor(ImGuiCol_Text, color_invalid_input);
	ImGui::InputText("DFS Folder", buf, buf_size, ImGuiInputTextFlags_CharsFilePathDFS);
	if (dfs_invalid)
		ImGui::PopStyleColor();

	return !dfs_invalid;
}
