#pragma once
#include "FieldMetadata.h"
#include "imgui.h"
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include "pfr.hpp"

namespace CPURDR
{
	class PFRInspector
	{
	public:
		template<typename T>
		static bool DrawField(const char* name, T& value, const FieldMetadata& metadata)
		{
			bool changed = false;

			if (!metadata.visible)
			{
				return false;
			}

			if (!metadata.editable)
			{
				ImGui::BeginDisabled();
			}

			if constexpr (std::is_same_v<T, float>)
			{
				changed = ImGui::DragFloat(name, &value, metadata.speed, metadata.minValue, metadata.maxValue);
			}
			else if constexpr(std::is_same_v<T, int>)
			{
				int min = static_cast<int>(metadata.minValue);
				int max = static_cast<int>(metadata.maxValue);
				changed = ImGui::DragInt(name, &value, metadata.speed, min, max);
			}
			else if constexpr(std::is_same_v<T, bool>)
			{
				changed = ImGui::Checkbox(name, &value);
			}
			else if constexpr(std::is_same_v<T, std::string>)
			{
				char buffer[256];
				strncpy_s(buffer, value.c_str(), sizeof(buffer) - 1);
				buffer[sizeof(buffer) - 1] = '\0';

				if (ImGui::InputText(name, buffer, sizeof(buffer)))
				{
					value = buffer;
					changed = true;
				}
			}
			else if constexpr(std::is_same_v<T, glm::vec2>)
			{
				changed = ImGui::DragFloat2(name, glm::value_ptr(value), metadata.speed);
			}
			else if constexpr(std::is_same_v<T, glm::vec3>)
			{
				changed = ImGui::DragFloat3(name, glm::value_ptr(value), metadata.speed);
			}
			else if constexpr(std::is_same_v<T, glm::vec4>)
			{
				if (metadata.isColor)
				{
					changed = ImGui::ColorEdit4(name, glm::value_ptr(value));
				}
				else
				{
					changed = ImGui::DragFloat4(name, glm::value_ptr(value), metadata.speed);
				}
			}
			else if constexpr(std::is_same_v<T, glm::quat>)
			{
				if (metadata.convertQuaternionToEuler)
				{
					glm::vec3 euler = glm::degrees(glm::eulerAngles(value));
					if (ImGui::DragFloat3(name, glm::value_ptr(euler), 0.5f))
					{
						value = glm::quat(glm::radians(euler));
						changed = true;
					}
				}
				else
				{
					// As non-intuitive Quaternion
					float quat[4] = {value.w, value.x, value.y, value.z};
					if (ImGui::DragFloat4(name, quat, 0.01, -1.0f, 1.0f))
					{
						value = glm::quat(quat[0], quat[1], quat[2], quat[3]);
						changed = true;
					}
				}
			}
			else if constexpr(std::is_same_v<T, uint32_t>)
			{
				if (metadata.isColor)
				{
					ImVec4 color;
					color.x = ((value >> 24) & 0xFF) / 255.0f;
					color.y = ((value >> 16) & 0xFF) / 255.0f;
					color.z = ((value >> 8) & 0xFF) / 255.0f;
					color.w = (value & 0xFF) / 255.0f;
					if (ImGui::ColorEdit4(name, &color.x))
					{
						value = (uint32_t)(color.x * 255) << 24 |
								(uint32_t)(color.y * 255) << 16 |
								(uint32_t)(color.z * 255) << 8 |
								(uint32_t)(color.w * 255);
						changed = true;
					}
				}
				else
				{
					int tempValue = static_cast<int>(value);
					if (ImGui::DragInt(name, &tempValue, metadata.speed, (int)metadata.minValue, (int)metadata.maxValue))
					{
						value = static_cast<uint32_t>(tempValue);
						changed = true;
					}
				}
			}
			else
			{
				// unknown type
				ImGui::TextDisabled("%s: <unsupported type>", name);
			}

			if (!metadata.editable)
			{
				ImGui::EndDisabled();
			}
			return changed;
		}

		template<typename Component>
		static void DrawComponent(Component& component, ComponentMetadata* metadata)
		{
			auto fieldCount = pfr::tuple_size_v<Component>;
			auto fieldNames = pfr::names_as_array<Component>();

			size_t fieldIndex = 0;
			pfr::for_each_field(component, [&](auto& field, size_t idx)
			{
				FieldMetadata fieldMeta;
				if (metadata && metadata->fieldMetadata.contains(idx))
				{
					fieldMeta = metadata->fieldMetadata[idx];
				}
				else
				{
					fieldMeta.displayName = std::string(fieldNames[idx]);
				}

				const char* displayName = fieldMeta.displayName.empty() ?
					fieldNames[idx].data() : fieldMeta.displayName.c_str();

				DrawField(displayName, field, fieldMeta);
			});
		}

		template<typename Component>
		static bool DrawComponentInspector(Component& component)
		{
			auto* metadata = MetadataRegistry::Instance().GetComponentMetadata<Component>();

			if (metadata && !metadata->visibleInInspector)
			{
				return false;
			}

			std::string componentName = metadata? metadata->displayName : typeid(Component).name();

			ImGuiTreeNodeFlags flags = (metadata && metadata->defaultOpen) ? ImGuiTreeNodeFlags_DefaultOpen : 0;

			bool changed = false;
			if (ImGui::CollapsingHeader(componentName.c_str(), flags))
			{
				ImGui::PushID(componentName.c_str());
				DrawComponent(component, metadata);
				ImGui::PopID();
				ImGui::Spacing();
				changed = true;
			}

			return changed;
		}
	};
}
