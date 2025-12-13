#pragma once
#include <cstdint>

namespace CPURDR
{
	struct MeshRenderer
	{
		bool enabled = true;
		bool castShadow = true;
		bool receiveShadows = true;

		uint32_t tint = 0xFFFFFFFF;
		float alpha = 1.0f;

		bool backfaceCulling = true;


		void SetTintRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
		{
			tint = (r << 24) | (g << 16) | (b << 8) | a;
		}
	};
}
