#pragma once
#include "entt.hpp"
#include "imgui.h"
#include "MetaReflection.h"

namespace CPURDR
{
	class MetaInspector
	{
	public:
		static bool DrawMetaField(entt::meta_data data, entt::meta_any& instance)
		{
			FieldMetadata meta = GetFieldMetadataOrDefault(data);

			if (!meta.visible)
			{
				return false;
			}

			entt::meta_any fieldValue = data.get(instance);
			if (!fieldValue)
			{
				return false;
			}

			bool changed = false;
			const char* displayName = meta.displayName.empty()? "Unknown" : meta.displayName.c_str();

			if (!meta.editable)
			{
				ImGui::BeginDisabled();
			}

			entt::meta_type fieldType = data.type();

			if (fieldType == entt::resolve<float>())
			{
				float value = fieldValue.cast<float>();
				if (ImGui::DragFloat(displayName, &value, meta.speed, meta.minValue, meta.maxValue))
				{
					data.set(instance, value);
					changed = true;
				}
			}
			else if (fieldType == entt::resolve<int>())
			{
				int value = fieldValue.cast<int>();
				if (ImGui::DragInt(displayName, &value, meta.speed, (int)meta.minValue, (int)meta.maxValue))
				{
					data.set(instance, value);
					changed = true;
				}
			}
			else if (fieldType == entt::resolve<bool>())
            {
                bool value = fieldValue.cast<bool>();
                if (ImGui::Checkbox(displayName, &value))
                {
                    data.set(instance, value);
                    changed = true;
                }
            }
            else if (fieldType == entt::resolve<std::string>())
            {
                std::string value = fieldValue.cast<std::string>();
                char buffer[256];
                strncpy_s(buffer, value.c_str(), sizeof(buffer) - 1);
                buffer[sizeof(buffer) - 1] = '\0';

                if (ImGui::InputText(displayName, buffer, sizeof(buffer)))
                {
                    data.set(instance, std::string(buffer));
                    changed = true;
                }
            }
            else if (fieldType == entt::resolve<glm::vec2>())
            {
                glm::vec2 value = fieldValue.cast<glm::vec2>();
                if (ImGui::DragFloat2(displayName, glm::value_ptr(value), meta.speed))
                {
                    data.set(instance, value);
                    changed = true;
                }
            }
            else if (fieldType == entt::resolve<glm::vec3>())
            {
                glm::vec3 value = fieldValue.cast<glm::vec3>();
                if (ImGui::DragFloat3(displayName, glm::value_ptr(value), meta.speed))
                {
                    data.set(instance, value);
                    changed = true;
                }
            }
            else if (fieldType == entt::resolve<glm::vec4>())
            {
                glm::vec4 value = fieldValue.cast<glm::vec4>();
                if (meta.isColor)
                {
                    if (ImGui::ColorEdit4(displayName, glm::value_ptr(value)))
                    {
                        data.set(instance, value);
                        changed = true;
                    }
                }
                else
                {
                    if (ImGui::DragFloat4(displayName, glm::value_ptr(value), meta.speed))
                    {
                        data.set(instance, value);
                        changed = true;
                    }
                }
            }
            else if (fieldType == entt::resolve<glm::quat>())
            {
                glm::quat value = fieldValue.cast<glm::quat>();
                if (meta.convertQuaternionToEuler)
                {
                    glm::vec3 euler = glm::degrees(glm::eulerAngles(value));
                    if (ImGui::DragFloat3(displayName, glm::value_ptr(euler), 0.5f))
                    {
                        data.set(instance, glm::quat(glm::radians(euler)));
                        changed = true;
                    }
                }
                else
                {
                    float quat[4] = { value.w, value.x, value.y, value.z };
                    if (ImGui::DragFloat4(displayName, quat, 0.01f, -1.0f, 1.0f))
                    {
                        data.set(instance, glm::quat(quat[0], quat[1], quat[2], quat[3]));
                        changed = true;
                    }
                }
            }
            else if (fieldType == entt::resolve<uint32_t>())
            {
                uint32_t value = fieldValue.cast<uint32_t>();
                if (meta.isColor)
                {
                    // Convert uint32 RGBA to float color
                    ImVec4 color;
                    color.x = ((value >> 24) & 0xFF) / 255.0f;
                    color.y = ((value >> 16) & 0xFF) / 255.0f;
                    color.z = ((value >> 8) & 0xFF) / 255.0f;
                    color.w = (value & 0xFF) / 255.0f;

                    if (ImGui::ColorEdit4(displayName, &color.x))
                    {
                        value = (uint32_t)(color.x * 255) << 24 |
                                (uint32_t)(color.y * 255) << 16 |
                                (uint32_t)(color.z * 255) << 8 |
                                (uint32_t)(color.w * 255);
                        data.set(instance, value);
                        changed = true;
                    }
                }
                else
                {
                    int temp = static_cast<int>(value);
                    if (ImGui::DragInt(displayName, &temp, meta.speed, (int)meta.minValue, (int)meta.maxValue))
                    {
                        data.set(instance, static_cast<uint32_t>(temp));
                        changed = true;
                    }
                }
            }
            else
            {
                ImGui::TextDisabled("%s: <unsupported type>", displayName);
            }

			if (!meta.editable)
			{
				ImGui::EndDisabled();
			}

			return changed;
		}

		template<typename Component>
		static bool DrawComponentInspector(Component& component)
		{
			entt::meta_type type = entt::resolve<Component>();
			if (!type)
			{
				ImGui::TextDisabled("Component not registered with entt::meta");
				return false;
			}

			ComponentMetadata meta = GetComponentMetadataOrDefault(type);

			if (!meta.visibleInInspector)
			{
				return false;
			}

			const char* displayName = meta.displayName.empty()? typeid(Component).name() : meta.displayName.c_str();

			ImGuiTreeNodeFlags flags = meta.defaultOpen? ImGuiTreeNodeFlags_DefaultOpen : 0;

			bool changed = false;
			if (ImGui::CollapsingHeader(displayName, flags))
			{
				ImGui::PushID(displayName);

				entt::meta_any instance = entt::forward_as_meta(component);

				for (auto&& [id, data] : type.data())
				{
					if (DrawMetaField(data, instance))
					{
						changed = true;
					}
				}

				ImGui::PopID();
				ImGui::Spacing();
			}

			return changed;
		}
	};
}
