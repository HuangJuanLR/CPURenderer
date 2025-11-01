#include <iostream>
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "SDL3/SDL_messagebox.h"
#include "ext/matrix_transform.hpp"

#include "Model.h"
#include "App.h"
#include "Graphics.h"
#include "plog/Log.h"

namespace CPURDR
{
	Model::Model(const std::string& file):
		m_Rng(std::random_device{}()),
		m_Dist(0, 255),
		m_DepthBuffer(1920, 1080, 1.0f),
		m_ColorBuffer(1920, 1080, 0xFF000000)
	{
		Assimp::Importer imp;

		const aiScene* pScene = imp.ReadFile(file.c_str(),
			aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals);

		if (pScene == nullptr)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
				"Error",
				"Error loading model",
				nullptr);

			return;
		}

		for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
		{
			const aiMesh* mesh = pScene->mMeshes[i];

			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			glm::vec3 vector;

			glm::mat4 matRot = glm::mat4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, -1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);

			for (unsigned int j = 0; j < mesh->mNumVertices; j++)
			{
				Vertex vertex;
				vector.x = mesh->mVertices[j].x;
				vector.y = mesh->mVertices[j].y;
				vector.z = mesh->mVertices[j].z;
				glm::vec4 transformedPos = matRot * glm::vec4(vector, 1.0f);
				vertex.position = glm::vec3(transformedPos);

				if (mesh->HasNormals())
				{
					vector.x = mesh->mNormals[j].x;
					vector.y = mesh->mNormals[j].y;
					vector.z = mesh->mNormals[j].z;
					vertex.normal = vector;
				}

				vertices.push_back(vertex);

			}

			for (unsigned int j = 0; j < mesh->mNumFaces; j++)
			{
				const aiFace face = mesh->mFaces[j];

				for (unsigned int k = 0; k < face.mNumIndices; k++)
				{
					indices.push_back(face.mIndices[k]);
				}
			}

			std::vector<SDL_Color> colors;
			colors.reserve(indices.size() / 3);
			for (unsigned int i = 0; i < indices.size(); i += 3)
			{
				SDL_Color c = {
					static_cast<uint8_t>(m_Dist(m_Rng)),
					static_cast<uint8_t>(m_Dist(m_Rng)),
					static_cast<uint8_t>(m_Dist(m_Rng)),
					255
				};
				colors.push_back(c);
			}

			m_Meshes.push_back(Mesh(vertices, indices, colors));
		}

	}

	Model::~Model() noexcept
	{
	}

	void Model::Draw(Context* context)
	{
		if (!context)
		{
			PLOG_ERROR << "Model::Draw() called with null context";
			return;
		}

		int width = context->GetFramebufferWidth();
		int height = context->GetFramebufferHeight();

		Texture2D_RGBA* colorBuffer = context->GetColorBuffer();
		Texture2D_RFloat* depthBuffer = context->GetDepthBuffer();

		if (!colorBuffer || !depthBuffer)
		{
			PLOG_ERROR << "Model::Draw() - Context has null framebuffers";
			return;
		}

		float centerX = width / 2.0f;
		float centerY = height / 2.0f;

		for (int i = 0; i < m_Meshes.size(); i++)
		{
			auto vertices = m_Meshes[i].vertices;
			auto indices = m_Meshes[i].indices;

			for (int j = 0, t = 0; j < m_Meshes[i].indices.size(); j+=3, t++)
			{
				glm::vec3 p0 = vertices[indices[j]].position;
				glm::vec3 p1 = vertices[indices[j + 1]].position;
				glm::vec3 p2 = vertices[indices[j + 2]].position;

				// Capybara model's z is -1 ~ 1 in Houdini
				p0 += glm::vec3(0, 0, 1);
				p0 *= glm::vec3(1, 1, 0.5);
				p1 += glm::vec3(0, 0, 1);
				p1 *= glm::vec3(1, 1, 0.5);
				p2 += glm::vec3(0, 0, 1);
				p2 *= glm::vec3(1, 1, 0.5);
				p0 = p0 * glm::vec3(500, 500, 255) + glm::vec3(centerX, centerY, 0);
				p1 = p1 * glm::vec3(500, 500, 255) + glm::vec3(centerX, centerY, 0);
				p2 = p2 * glm::vec3(500, 500, 255) + glm::vec3(centerX, centerY, 0);

				// Normalize Z
				p0.z /= 255.0f;
				p1.z /= 255.0f;
				p2.z /= 255.0f;

				const SDL_Color col = (t < m_Meshes[i].colors.size())? m_Meshes[i].colors[t] : SDL_Color{255, 255, 255, 255};
				uint32_t color = (col.r << 24) | (col.g << 16) | (col.b << 8) | 255;

				Graphics::Triangle(p0, p1, p2, width, height, *depthBuffer, *colorBuffer, color);
			}
		}
	}

	void Model::Draw(Context* context, const Camera& camera)
	{
		if (!context)
		{
			PLOG_ERROR << "Model::Draw() called with null context";
			return;
		}

		int width = context->GetFramebufferWidth();
		int height = context->GetFramebufferHeight();
		float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

		Texture2D_RGBA* colorBuffer = context->GetColorBuffer();
		Texture2D_RFloat* depthBuffer = context->GetDepthBuffer();

		if (!colorBuffer || !depthBuffer)
		{
			PLOG_ERROR << "Model::Draw() - Context has null framebuffers";
			return;
		}

		glm::mat4 modelMatrix = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		float time = SDL_GetTicks() / 1000.0f;
		float rotationSpeed = 45.0f;
		float rotationAngle = time * rotationSpeed;
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 viewMatrix = camera.GetViewMatrix();
		glm::mat4 projectionMatrix = camera.GetProjectionMatrix(aspectRatio);
		glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

		for (int meshIdx = 0; meshIdx < m_Meshes.size(); meshIdx++)
		{
			const auto& mesh = m_Meshes[meshIdx];
			const auto& vertices = mesh.vertices;
			const auto& indices = mesh.indices;

			for (int j = 0, t = 0; j < indices.size(); j += 3, t++)
			{
				glm::vec3 p0 = vertices[indices[j]].position;
				glm::vec3 p1 = vertices[indices[j + 1]].position;
				glm::vec3 p2 = vertices[indices[j + 2]].position;

				// Transform vertices to CS
				glm::vec4 clip0 = mvpMatrix * glm::vec4(p0, 1.0f);
				glm::vec4 clip1 = mvpMatrix * glm::vec4(p1, 1.0f);
				glm::vec4 clip2 = mvpMatrix * glm::vec4(p2, 1.0f);

				if (clip0.w != 0.0f) clip0 /= clip0.w;
				if (clip1.w != 0.0f) clip1 /= clip1.w;
				if (clip2.w != 0.0f) clip2 /= clip2.w;

				// Frustum culling
				if (clip0.x < -1.0f || clip0.x > 1.0f || clip0.y < -1.0f || clip0.y > 1.0f ||
					clip1.x < -1.0f || clip1.x > 1.0f || clip1.y < -1.0f || clip1.y > 1.0f ||
					clip2.x < -1.0f || clip2.x > 1.0f || clip2.y < -1.0f || clip2.y > 1.0f)
				{
					continue;
				}

				// Depth Culling
				if (clip0.z < -1.0f || clip0.z > 1.0f ||
					clip1.z < -1.0f || clip1.z > 1.0f ||
					clip2.z < -1.0f || clip2.z > 1.0f)
				{
					continue;
				}

				// NDC to ScreenSpace -1~1 to 0~width
				glm::vec3 screen0, screen1, screen2;
				screen0.x = (clip0.x + 1.0f) * 0.5f * width;
				screen1.x = (clip1.x + 1.0f) * 0.5f * width;
				screen2.x = (clip2.x + 1.0f) * 0.5f * width;
				screen0.y = (clip0.y + 1.0f) * 0.5f * height;
				screen1.y = (clip1.y + 1.0f) * 0.5f * height;
				screen2.y = (clip2.y + 1.0f) * 0.5f * height;
				screen0.z = (clip0.z + 1.0f) * 0.5f;
				screen1.z = (clip1.z + 1.0f) * 0.5f;
				screen2.z = (clip2.z + 1.0f) * 0.5f;

				const SDL_Color col = (t < mesh.colors.size())? mesh.colors[t] : SDL_Color{255, 255, 255, 255};
				uint32_t color = (col.r << 24) | (col.g << 16) | (col.b << 8) | 255;
				Graphics::Triangle(screen0, screen1, screen2, width, height, *depthBuffer, *colorBuffer, color);

			}
		}

	}

	void Model::DrawTriangle(SDL_Renderer* renderer)
	{
		App& app = App::GetInstance();
		int width = app.GetLogicWidth();
		int height = app.GetLogicHeight();

		// Resize when window changed
		if (m_DepthBuffer.GetWidth() != width || m_DepthBuffer.GetHeight() != height)
		{
			m_DepthBuffer.Resize(width, height, 1.0f);
			m_ColorBuffer.Resize(width, height, 1.0f);
		}

		// Clear every frame
		m_DepthBuffer.Clear(1.0f);
		m_ColorBuffer.Clear(0x141414FF);

		float centerX = width / 2.0f;
		float centerY = height / 2.0f;

		for (int i = 0; i < m_Meshes.size(); i++)
		{
			auto vertices = m_Meshes[i].vertices;
			auto indices = m_Meshes[i].indices;
			for (int j = 0, t = 0; j < m_Meshes[i].indices.size(); j += 3, t++)
			{
				glm::vec3 p0 = vertices[indices[j]].position;
				glm::vec3 p1 = vertices[indices[j + 1]].position;
				glm::vec3 p2 = vertices[indices[j + 2]].position;

				// Capybara model's z is -1 ~ 1 in Houdini
				p0 += glm::vec3(0, 0, 1);
				p0 *= glm::vec3(1, 1, 0.5);
				p1 += glm::vec3(0, 0, 1);
				p1 *= glm::vec3(1, 1, 0.5);
				p2 += glm::vec3(0, 0, 1);
				p2 *= glm::vec3(1, 1, 0.5);
				p0 = p0 * glm::vec3(500, 500, 255) + glm::vec3(centerX, centerY, 0);
				p1 = p1 * glm::vec3(500, 500, 255) + glm::vec3(centerX, centerY, 0);
				p2 = p2 * glm::vec3(500, 500, 255) + glm::vec3(centerX, centerY, 0);

				// Normalize Z
				p0.z /= 255.0f;
				p1.z /= 255.0f;
				p2.z /= 255.0f;

				const SDL_Color col = (t < m_Meshes[i].colors.size())?
					m_Meshes[i].colors[t] : SDL_Color{255, 255, 255, 255};
				uint32_t color = (col.r << 24) | (col.g << 16) | (col.b << 8) | 255;
				Graphics::Triangle(p0, p1, p2, width, height, m_DepthBuffer, m_ColorBuffer, color);
			}
		}

		// m_ColorBuffer.Fill(0x70707070);
	}
}
