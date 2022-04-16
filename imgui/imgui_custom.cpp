#include "imgui_custom.h"

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
