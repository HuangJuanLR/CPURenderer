#pragma once
#include "gtc/type_ptr.hpp"
#include "entt.hpp"
#include "imgui.h"
#include "ecs/components/MeshRenderer.h"
#include "render/Material.h"
#include "render/MaterialManager.h"

namespace CPURDR
{
	class MaterialPropertyInspector
	{
	public:
		static bool Draw(MeshRenderer& renderer)
		{
			bool changed = false;

			Material* baseMaterial = MaterialManager::GetInstance().GetMaterial(renderer.materialId);
			if (!baseMaterial)
			{
				ImGui::TextDisabled("No material assigned");
				return false;
			}

			changed |= DrawShaderSelector(renderer);

			ImGui::Separator();

			IShader* shader = ShaderManager::GetInstance().GetShader(baseMaterial->shaderId);
			if (!shader)
			{
				ImGui::TextDisabled("Invalid shader");
				return false;
			}

			ImGui::Text("Material: %s", baseMaterial->name.c_str());
			ImGui::Text("Shader: %s", shader->GetName().c_str());

			if (!renderer.propertyOverrides.empty())
			{
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "(%zu override)",
					renderer.propertyOverrides.size());
			}

			ImGui::Separator();

			if (!renderer.propertyOverrides.empty())
			{
				if (ImGui::Button("Clear All Overrides"))
				{
					renderer.ClearAllOverrides();
					changed = true;
				}
				ImGui::Separator();
			}

			const auto& properties = shader->GetProperties();
			for (const auto& prop: properties)
			{
				changed |= DrawProperty(prop, *baseMaterial, renderer);
			}

			return changed;
		}

		static bool DrawShaderSelector(MeshRenderer& renderer)
		{
			bool changed = false;

			Material* baseMaterial = MaterialManager::GetInstance().GetMaterial(renderer.materialId);
			if (!baseMaterial) return false;

			IShader* currentShader = ShaderManager::GetInstance().GetShader(baseMaterial->shaderId);
			const char* currentShaderName = currentShader? currentShader->GetName().c_str() : "Invalid shader";

			ImGui::Text("Shader: ");
			ImGui::SameLine();

			if (ImGui::BeginCombo("##ShaderSelect", currentShaderName))
			{
				const auto& shaders = ShaderManager::GetInstance().GetAllShaders();

				if (shaders.empty())
				{
					ImGui::TextDisabled("No shaders available");
					ImGui::EndCombo();
					return false;
				}

				for (IShader* shader : shaders)
				{
					if (!shader) continue;

					bool isSelected = (currentShader != nullptr && currentShader->GetId() == shader->GetId());

					if (ImGui::Selectable(shader->GetName().c_str(), isSelected))
					{
						if (!isSelected)
						{
							baseMaterial->shaderId = shader->GetId();

							renderer.ClearAllOverrides();

							baseMaterial->properties.clear();
							for (const auto& prop: shader->GetProperties())
							{
								std::visit([&](auto&& val)
								{
									baseMaterial->properties[prop.name] = val;
								}, prop.defaultValue);
							}

							changed = true;
						}
					}

					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			return changed;
		}

	private:
		static bool DrawProperty(const ShaderPropertyDefinition& prop, Material& baseMaterial,
			MeshRenderer& renderer)
		{
			bool changed = false;
			bool hasOverride = renderer.HasOverride(prop.name);

			ImGui::PushID(prop.name.c_str());

			if (hasOverride)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
				ImGui::Text("*");
				ImGui::PopStyleColor();
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("This property is overridden for this instance");
				}
			}
			else
			{
				ImGui::Text(" ");
			}
			ImGui::SameLine();

			switch (prop.type)
			{
				case ShaderPropertyType::Float:
					changed = DrawFloatProperty(prop, baseMaterial, renderer, hasOverride);
					break;

				case ShaderPropertyType::Int:
					changed = DrawIntProperty(prop, baseMaterial, renderer, hasOverride);
					break;

				case ShaderPropertyType::Vec2:
					changed = DrawVec2Property(prop, baseMaterial, renderer, hasOverride);
					break;

				case ShaderPropertyType::Vec3:
					changed = DrawVec3Property(prop, baseMaterial, renderer, hasOverride);
					break;

				case ShaderPropertyType::Vec4:
					changed = DrawVec4Property(prop, baseMaterial, renderer, hasOverride);
					break;

				case ShaderPropertyType::Color:
					changed = DrawColorProperty(prop, baseMaterial, renderer, hasOverride);
					break;

				default:
					ImGui::Text("%s: (unsupported type)", prop.displayName.c_str());
					break;
			}

			// Clear override button
			if (hasOverride)
			{
				ImGui::SameLine();
				if (ImGui::SmallButton("X"))
				{
					renderer.ClearOverride(prop.name);
					changed = true;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Clear override (revert to material default)");
				}
			}

			ImGui::PopID();

			return changed;
		}

		static bool DrawFloatProperty(const ShaderPropertyDefinition& prop, Material& baseMaterial,
                                       MeshRenderer& renderer, bool hasOverride)
        {
            float value;
            if (hasOverride)
            {
                value = renderer.GetOverride<float>(prop.name, 0.0f);
            }
            else
            {
                value = baseMaterial.GetFloat(prop.name, std::get<float>(prop.defaultValue));
            }

            float speed = (prop.maxValue - prop.minValue) / 100.0f;
            if (speed < 0.01f) speed = 0.01f;

            if (ImGui::DragFloat(prop.displayName.c_str(), &value, speed, prop.minValue, prop.maxValue))
            {
                renderer.SetOverride(prop.name, value);
                return true;
            }
            return false;
        }

        static bool DrawIntProperty(const ShaderPropertyDefinition& prop, Material& baseMaterial,
                                     MeshRenderer& renderer, bool hasOverride)
        {
            int value;
            if (hasOverride)
            {
                value = renderer.GetOverride<int>(prop.name, 0);
            }
            else
            {
                value = baseMaterial.Get<int>(prop.name, std::get<int>(prop.defaultValue));
            }

            if (ImGui::DragInt(prop.displayName.c_str(), &value, 1.0f,
                               static_cast<int>(prop.minValue), static_cast<int>(prop.maxValue)))
            {
                renderer.SetOverride(prop.name, value);
                return true;
            }
            return false;
        }

        static bool DrawVec2Property(const ShaderPropertyDefinition& prop, Material& baseMaterial,
                                      MeshRenderer& renderer, bool hasOverride)
        {
            glm::vec2 value;
            if (hasOverride)
            {
                value = renderer.GetOverride<glm::vec2>(prop.name, glm::vec2(0.0f));
            }
            else
            {
                value = baseMaterial.Get<glm::vec2>(prop.name, std::get<glm::vec2>(prop.defaultValue));
            }

            if (ImGui::DragFloat2(prop.displayName.c_str(), glm::value_ptr(value), 0.01f))
            {
                renderer.SetOverride(prop.name, value);
                return true;
            }
            return false;
        }

        static bool DrawVec3Property(const ShaderPropertyDefinition& prop, Material& baseMaterial,
                                      MeshRenderer& renderer, bool hasOverride)
        {
            glm::vec3 value;
            if (hasOverride)
            {
                value = renderer.GetOverride<glm::vec3>(prop.name, glm::vec3(0.0f));
            }
            else
            {
                value = baseMaterial.GetVec3(prop.name, std::get<glm::vec3>(prop.defaultValue));
            }

            if (ImGui::DragFloat3(prop.displayName.c_str(), glm::value_ptr(value), 0.01f))
            {
                renderer.SetOverride(prop.name, value);
                return true;
            }
            return false;
        }

        static bool DrawVec4Property(const ShaderPropertyDefinition& prop, Material& baseMaterial,
                                      MeshRenderer& renderer, bool hasOverride)
        {
            glm::vec4 value;
            if (hasOverride)
            {
                value = renderer.GetOverride<glm::vec4>(prop.name, glm::vec4(0.0f));
            }
            else
            {
                value = baseMaterial.GetVec4(prop.name, std::get<glm::vec4>(prop.defaultValue));
            }

            if (ImGui::DragFloat4(prop.displayName.c_str(), glm::value_ptr(value), 0.01f))
            {
                renderer.SetOverride(prop.name, value);
                return true;
            }
            return false;
        }

        static bool DrawColorProperty(const ShaderPropertyDefinition& prop, Material& baseMaterial,
                                       MeshRenderer& renderer, bool hasOverride)
        {
            glm::vec3 value;
            if (hasOverride)
            {
                value = renderer.GetOverride<glm::vec3>(prop.name, glm::vec3(1.0f));
            }
            else
            {
                value = baseMaterial.GetVec3(prop.name, std::get<glm::vec3>(prop.defaultValue));
            }

            if (ImGui::ColorEdit3(prop.displayName.c_str(), glm::value_ptr(value)))
            {
                renderer.SetOverride(prop.name, value);
                return true;
            }
            return false;
        }
	};
}
