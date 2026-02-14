#pragma once

#include "imgui.h"

namespace CPURDR::UI
{
	enum class ThemeVariant
	{
		Unity,
		Blender
	};

	struct ImGuiPalette
	{
		ImVec4 bgApp;
		ImVec4 bgPanel;
		ImVec4 bgPanelAlt;
		ImVec4 bgHeader;
		ImVec4 bgControl;
		ImVec4 bgControlHover;
		ImVec4 bgControlActive;
		ImVec4 border;
		ImVec4 borderSoft;
		ImVec4 text;
		ImVec4 textMuted;
		ImVec4 accent;
		ImVec4 accentHover;
		ImVec4 accentActive;
	};

	ImGuiPalette MakePalette(ThemeVariant variant);
}