#include "ImGuiPalette.h"

namespace CPURDR::UI
{
	ImGuiPalette MakePalette(ThemeVariant variant)
	{
		if (variant == ThemeVariant::Blender)
		{
			return ImGuiPalette{
				 ImVec4(0.125f, 0.129f, 0.137f, 1.0f),		// bgApp
				 ImVec4(0.157f, 0.161f, 0.173f, 1.0f),		// bgPanel
				 ImVec4(0.176f, 0.180f, 0.192f, 1.0f),		// bgPanelAlt
				 ImVec4(0.176f, 0.184f, 0.200f, 1.0f),		// bgHeader
				 ImVec4(0.220f, 0.227f, 0.243f, 1.0f),		// bgControl
				 ImVec4(0.259f, 0.267f, 0.286f, 1.0f),		// bgControlHover
				 ImVec4(0.306f, 0.314f, 0.337f, 1.0f),		// bgControlAct
				 ImVec4(0.255f, 0.263f, 0.282f, 1.0f),		// border
				 ImVec4(0.208f, 0.216f, 0.235f, 1.0f),		// borderSoft
				 ImVec4(0.910f, 0.918f, 0.937f, 1.0f),		// text
				 ImVec4(0.620f, 0.635f, 0.667f, 1.0f),		// textMuted
				 ImVec4(0.435f, 0.600f, 0.886f, 1.0f),		// accent
				 ImVec4(0.502f, 0.651f, 0.914f, 1.0f),		// accentHover
				 ImVec4(0.545f, 0.694f, 0.941f, 1.0f)		// accentActive
			};
		}

		// Unity
		return ImGuiPalette{
			ImVec4(0.04f, 0.04f, 0.04f, 1.00f),  // bgApp
			ImVec4(0.12f, 0.12f, 0.12f, 1.00f),  // bgPanel
			ImVec4(0.08f, 0.08f, 0.08f, 1.00f),  // bgPanelAlt
			ImVec4(0.04f, 0.04f, 0.04f, 1.00f),  // bgHeader
			ImVec4(0.12f, 0.12f, 0.12f, 1.00f),  // bgControl
			ImVec4(0.18f, 0.18f, 0.18f, 1.00f),  // bgControlHover
			ImVec4(0.28f, 0.28f, 0.28f, 1.00f),  // bgControlActive
			ImVec4(0.08f, 0.08f, 0.08f, 1.00f),  // border
			ImVec4(0.12f, 0.12f, 0.12f, 1.00f),  // borderSoft
			ImVec4(0.95f, 0.95f, 0.95f, 1.00f),  // text
			ImVec4(0.50f, 0.50f, 0.50f, 1.00f),  // textMuted
			ImVec4(1.00f, 0.47f, 0.00f, 1.00f),  // accent
			ImVec4(0.80f, 0.38f, 0.00f, 1.00f),  // accentHover
			ImVec4(1.00f, 0.47f, 0.00f, 1.00f)   // accentActive
		};
	}
}