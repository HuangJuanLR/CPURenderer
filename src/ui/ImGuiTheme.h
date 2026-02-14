#pragma once
#include "ImGuiPalette.h"

namespace CPURDR::UI
{
	void ApplyTheme(float mainScale, ThemeVariant variant = ThemeVariant::Unity);
	ImGuiWindowFlags GetEditorPanelFlags(bool withMenuBar = false);
}

