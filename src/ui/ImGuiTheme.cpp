#include "ImGuiTheme.h"

namespace CPURDR::UI
{
	static void ApplyColors(const ImGuiPalette& palette)
	{
		ImVec4* color = ImGui::GetStyle().Colors;

		color[ImGuiCol_Text] = palette.text;
		color[ImGuiCol_TextDisabled] = palette.textMuted;

		color[ImGuiCol_WindowBg] = palette.bgPanel;
		color[ImGuiCol_ChildBg] = palette.bgPanelAlt;
		color[ImGuiCol_PopupBg] = palette.bgPanel;

		color[ImGuiCol_Border] = palette.border;
		color[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

		color[ImGuiCol_FrameBg] = palette.bgControl;
		color[ImGuiCol_FrameBgHovered] = palette.bgControlHover;
		color[ImGuiCol_FrameBgActive] = palette.bgControlActive;

		color[ImGuiCol_TitleBg] = palette.bgHeader;
		color[ImGuiCol_TitleBgActive] = palette.bgHeader;
		color[ImGuiCol_TitleBgCollapsed] = palette.bgHeader;
		color[ImGuiCol_MenuBarBg] = palette.bgHeader;

		color[ImGuiCol_Header] = palette.bgControl;
		color[ImGuiCol_HeaderHovered] = palette.bgControlHover;
		color[ImGuiCol_HeaderActive] = palette.bgControlHover;

		color[ImGuiCol_Button] = palette.bgControl;
		color[ImGuiCol_ButtonHovered] = palette.bgControlHover;
		color[ImGuiCol_ButtonActive] = palette.bgControlActive;

		color[ImGuiCol_CheckMark] = palette.accent;
		color[ImGuiCol_SliderGrab] = palette.accent;
		color[ImGuiCol_SliderGrabActive] = palette.accentActive;

		color[ImGuiCol_Separator] = palette.borderSoft;
		color[ImGuiCol_SeparatorHovered] = palette.border;
		color[ImGuiCol_SeparatorActive] = palette.accent;

		color[ImGuiCol_ResizeGrip] = palette.bgControl;
		color[ImGuiCol_ResizeGripHovered] = palette.bgControlHover;
		color[ImGuiCol_ResizeGripActive] = palette.accent;

		color[ImGuiCol_TabHovered] = palette.bgControlHover;
		color[ImGuiCol_Tab] = palette.bgPanelAlt;
		color[ImGuiCol_TabSelected] = palette.bgControl;
		color[ImGuiCol_TabSelectedOverline] = ImVec4(0, 0, 0, 0);
		color[ImGuiCol_TabDimmed] = palette.bgPanelAlt;
		color[ImGuiCol_TabDimmedSelected] = palette.bgControl;
		color[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0, 0, 0, 0);

		color[ImGuiCol_DockingPreview] = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.45f);
		color[ImGuiCol_DockingEmptyBg] = palette.bgApp;

		color[ImGuiCol_ScrollbarBg] = palette.bgPanelAlt;
		color[ImGuiCol_ScrollbarGrab] = palette.bgControl;
		color[ImGuiCol_ScrollbarGrabHovered] = palette.bgControlHover;
		color[ImGuiCol_ScrollbarGrabActive] = palette.bgControlActive;

		color[ImGuiCol_TextSelectedBg] = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.28f);
		color[ImGuiCol_DragDropTarget] = palette.accentHover;
		color[ImGuiCol_NavHighlight] = ImVec4(0, 0, 0, 0);
		color[ImGuiCol_NavWindowingHighlight] = ImVec4(0, 0, 0, 0);
	}

	void ApplyTheme(float mainScale, ThemeVariant variant)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		// Default dark
		ImGui::StyleColorsDark();

		// Rounded Island
		style.WindowRounding = 4.0f;
		style.ChildRounding = 4.0f;
		style.PopupRounding = 2.0f;
		style.FrameRounding = 1.0f;
		style.GrabRounding = 4.0f;
		style.TabRounding = 1.0f;
		style.ScrollbarRounding = 2.0f;

		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.TabBorderSize = 0.0f;

		style.WindowPadding = ImVec2(10.0f, 10.0f);
		style.FramePadding = ImVec2(8.0f, 5.0f);
		style.ItemSpacing = ImVec2(8.0f, 6.0f);
		style.ItemInnerSpacing = ImVec2(6.0f, 5.0f);
		style.CellPadding = ImVec2(7.0f, 5.0f);

		style.IndentSpacing = 22.0f;
		style.ScrollbarSize = 14.0f;
		style.GrabMinSize = 9.0f;

		// Removes the small ImGui window menu button
		style.WindowMenuButtonPosition = ImGuiDir_None;
		style.WindowTitleAlign = ImVec2(0.03f, 0.5f);

		style.DockingSeparatorSize = 2.0f;

		style.ScaleAllSizes(mainScale);

		ApplyColors(MakePalette(variant));
	}

	ImGuiWindowFlags GetEditorPanelFlags(bool withMenuBar)
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
		if (withMenuBar)
		{
			flags |= ImGuiWindowFlags_MenuBar;
		}
		return flags;
	}

}