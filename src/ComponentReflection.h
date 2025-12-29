#pragma once
#include <entt.hpp>
#include "MetaReflection.h"
#include "ecs/components/Hierarchy.h"
#include "ecs/components/MeshFilter.h"
#include "ecs/components/MeshRenderer.h"
#include "ecs/components/NameTag.h"
#include "ecs/components/Transform.h"

namespace CPURDR
{
	using namespace entt::literals;

	inline void RegisterComponentReflection()
	{
		// ==================================
		// Transform
		// ==================================
		entt::meta_factory<Transform>{}
		.type("Transform"_hs)
		.custom<ComponentMetadata>(ComponentMetadata{
			.displayName = "Transform",
			.visibleInInspector = true,
			.defaultOpen = true
		})
		.data<&Transform::position>("position"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Position",
				.editable = true,
				.visible = true,
				.speed = 0.01f,
			})
		.data<&Transform::rotation>("rotation"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Rotation",
				.editable = true,
				.visible = true,
				.speed = 0.01f,
				.convertQuaternionToEuler = true
			})
		.data<&Transform::eulerAngles>("eulerAngles"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Euler Angles",
				.editable = false,
				.visible = false,
				.speed = 0.01f,
			})
		.data<&Transform::scale>("scale"_hs)
				.custom<FieldMetadata>(FieldMetadata{
					.displayName = "Scale",
					.editable = true,
					.visible = true,
					.speed = 0.01f
				})
		.data<&Transform::worldPosition>("worldPosition"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "World Position",
				.editable = false,
				.visible = true
			})
		.data<&Transform::worldRotation>("worldRotation"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "World Rotation",
				.editable = false,
				.visible = true,
				.convertQuaternionToEuler = true
			})
		.data<&Transform::worldScale>("worldScale"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "World Scale",
				.editable = false,
				.visible = true
			})
		.data<&Transform::isDirty>("isDirty"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Is Dirty",
				.editable = false,
				.visible = true
			});

		// ==================================
		// NameTag
		// ==================================
		entt::meta_factory<NameTag>{}
		.type("NameTag"_hs)
		.custom<ComponentMetadata>(ComponentMetadata{
			.displayName = "NameTag",
			.visibleInInspector = true,
			.defaultOpen = true
		})
		.data<&NameTag::name>("name"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Name",
				.editable = true,
				.visible = true
			});

		// ==================================
		// MeshRenderer
		// ==================================
		entt::meta_factory<MeshRenderer>{}
		.type("MeshRenderer"_hs)
		.custom<ComponentMetadata>(ComponentMetadata{
			.displayName = "Mesh Renderer",
			.visibleInInspector = true,
			.defaultOpen = false
		})
		.data<&MeshRenderer::enabled>("enabled"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Enabled"
			})
		.data<&MeshRenderer::castShadow>("castShadow"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Cast Shadow"
			})
		.data<&MeshRenderer::receiveShadows>("receiveShadows"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Receive Shadows"
			})
		.data<&MeshRenderer::tint>("tint"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Tint",
				.isColor = true
			})
		.data<&MeshRenderer::alpha>("alpha"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Alpha",
				.speed = 0.01f,
				.minValue = 0.0f,
				.maxValue = 1.0f
			})
		.data<&MeshRenderer::backfaceCulling>("backfaceCulling"_hs)
			.custom<FieldMetadata>(FieldMetadata{
				.displayName = "Backface Culling"
			});

		// ==================================
		// Hierarchy
		// ==================================
		entt::meta_factory<Hierarchy>{}
		.type("Hierarchy"_hs)
		.custom<ComponentMetadata>(ComponentMetadata{
			.displayName = "Hierarchy",
			.visibleInInspector = false,
			.defaultOpen = false
		});
	}
}
