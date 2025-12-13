#pragma once
#include "FieldMetadata.h"
#include "ecs/components/Hierarchy.h"
#include "ecs/components/MeshRenderer.h"
#include "ecs/components/NameTag.h"
#include "ecs/components/Transform.h"

namespace CPURDR
{
	inline void SetupComponentMetadata()
	{
		auto& registry = MetadataRegistry::Instance();

		// =========================
		// NameTag
		// =========================
		{
			ComponentMetadata meta;
			meta.displayName = "NameTag";
			meta.visibleInInspector = true;
			meta.defaultOpen = true;

			meta.fieldMetadata[0] = FieldMetadata{
				.displayName = "Name",
				.editable = true,
				.visible = true,
			};

			registry.RegisterComponent<NameTag>(meta);
		}

		// =========================
		// Transform
		// =========================
		{
			ComponentMetadata meta;
			meta.displayName = "Transform";
			meta.visibleInInspector = true;
			meta.defaultOpen = true;

			meta.fieldMetadata[0] = FieldMetadata{
				.displayName = "Position",
				.editable = true,
				.speed = 0.01f,
			};

			meta.fieldMetadata[1] = FieldMetadata{
				.displayName = "Rotation",
				.editable = true,
				.convertQuaternionToEuler = true
			};

			meta.fieldMetadata[2] = FieldMetadata{
				.displayName = "Scale",
				.editable = true,
				.speed = 0.01f,
				.minValue = -FLT_MAX,
				.maxValue = FLT_MAX
			};

			meta.fieldMetadata[3] = FieldMetadata{
				.displayName = "World Position",
				.editable = false
			};

			meta.fieldMetadata[4] = FieldMetadata{
				.displayName = "World Rotation",
				.editable = false,
				.convertQuaternionToEuler = true
			};

			meta.fieldMetadata[5] = FieldMetadata{
				.displayName = "World Scale",
				.editable = false
			};

			meta.fieldMetadata[6] = FieldMetadata{
				.displayName = "Is Dirty",
				.editable = false
			};

			registry.RegisterComponent<Transform>(meta);
		}

		// =========================
		// MeshRenderer
		// =========================
		{
			ComponentMetadata meta;
			meta.displayName = "MeshRenderer";
			meta.visibleInInspector = true;
			meta.defaultOpen = false;

			meta.fieldMetadata[0] = FieldMetadata{
				.displayName = "Enabled"
			};

			meta.fieldMetadata[1] = FieldMetadata{
				.displayName = "Cast Shadow"
			};

			meta.fieldMetadata[2] = FieldMetadata{
				.displayName = "Receive Shadow"
			};

			meta.fieldMetadata[3] = FieldMetadata{
				.displayName = "Tint",
				.isColor = true
			};

			meta.fieldMetadata[4] = FieldMetadata{
				.displayName = "Alpha",
				.speed = 0.01f,
				.minValue = 0.0f,
				.maxValue = 1.0f
			};

			meta.fieldMetadata[5] = FieldMetadata{
				.displayName = "Backface Culling"
			};

			registry.RegisterComponent<MeshRenderer>(meta);
		}

		// =========================
		// Hierarchy
		// =========================
		{
			ComponentMetadata meta;
			meta.displayName = "Hierarchy";
			meta.visibleInInspector = false;

			registry.RegisterComponent<Hierarchy>(meta);
		}
	}
}
