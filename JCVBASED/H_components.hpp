#pragma once

#include "H_JCVB_model.hpp"
//#include "H_JCVB_material.hpp"
#include "H_JCVB_texture.hpp"

//libs
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef far
#undef far
#endif
#ifdef near
#undef near
#endif


namespace Digestion {

	//Update to be more serialized friendly, Fix after getting Serialization working
	struct Camera {
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 inverseViewMatrix{ 1.f };

		const glm::mat4& getProjection() const {
			return projectionMatrix;
		}

		const glm::mat4& getView() const {
			return viewMatrix;
		}

		const glm::mat4& getInverseView() const {
			return inverseViewMatrix;
		}

		const glm::vec3 getPosition() const {
			return glm::vec3(inverseViewMatrix[3]);
		}

		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
			projectionMatrix = glm::mat4{ 1.0f };
			projectionMatrix[0][0] = 2.f / (right - left);
			projectionMatrix[1][1] = 2.f / (bottom - top);
			projectionMatrix[2][2] = 1.f / (far - near);
			projectionMatrix[3][0] = -(right + left) / (right - left);
			projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
			projectionMatrix[3][2] = -near / (far - near);
		}

		void setPerspectiveProjection(float fovy, float aspect, float near, float far) {
			assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
			const float tanHalfFovy = tan(fovy / 2.f);
			projectionMatrix = glm::mat4{ 0.0f };
			projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
			projectionMatrix[1][1] = 1.f / (tanHalfFovy);
			projectionMatrix[2][2] = far / (far - near);
			projectionMatrix[2][3] = 1.f;
			projectionMatrix[3][2] = -(far * near) / (far - near);

		}

		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }) {
			const glm::vec3 w{ glm::normalize(direction) };
			const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
			const glm::vec3 v{ glm::cross(w, u) };

			viewMatrix = glm::mat4{ 1.f };
			viewMatrix[0][0] = u.x;
			viewMatrix[1][0] = u.y;
			viewMatrix[2][0] = u.z;
			viewMatrix[0][1] = v.x;
			viewMatrix[1][1] = v.y;
			viewMatrix[2][1] = v.z;
			viewMatrix[0][2] = w.x;
			viewMatrix[1][2] = w.y;
			viewMatrix[2][2] = w.z;
			viewMatrix[3][0] = -glm::dot(u, position);
			viewMatrix[3][1] = -glm::dot(v, position);
			viewMatrix[3][2] = -glm::dot(w, position);

			inverseViewMatrix = glm::mat4{ 1.f };
			inverseViewMatrix[0][0] = u.x;
			inverseViewMatrix[0][1] = u.y;
			inverseViewMatrix[0][2] = u.z;
			inverseViewMatrix[1][0] = v.x;
			inverseViewMatrix[1][1] = v.y;
			inverseViewMatrix[1][2] = v.z;
			inverseViewMatrix[2][0] = w.x;
			inverseViewMatrix[2][1] = w.y;
			inverseViewMatrix[2][2] = w.z;
			inverseViewMatrix[3][0] = position.x;
			inverseViewMatrix[3][1] = position.y;
			inverseViewMatrix[3][2] = position.z;
		}

		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f }) {
			assert(((target - position).x != .0f && (target - position).y != .0f && (target - position).z != .0f) && "Direction provided is zero!");
			setViewDirection(position, target - position, up);
		}

		void setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
			const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
			const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
			viewMatrix = glm::mat4{ 1.f };
			viewMatrix[0][0] = u.x;
			viewMatrix[1][0] = u.y;
			viewMatrix[2][0] = u.z;
			viewMatrix[0][1] = v.x;
			viewMatrix[1][1] = v.y;
			viewMatrix[2][1] = v.z;
			viewMatrix[0][2] = w.x;
			viewMatrix[1][2] = w.y;
			viewMatrix[2][2] = w.z;
			viewMatrix[3][0] = -glm::dot(u, position);
			viewMatrix[3][1] = -glm::dot(v, position);
			viewMatrix[3][2] = -glm::dot(w, position);

			inverseViewMatrix = glm::mat4{ 1.f };
			inverseViewMatrix[0][0] = u.x;
			inverseViewMatrix[0][1] = u.y;
			inverseViewMatrix[0][2] = u.z;
			inverseViewMatrix[1][0] = v.x;
			inverseViewMatrix[1][1] = v.y;
			inverseViewMatrix[1][2] = v.z;
			inverseViewMatrix[2][0] = w.x;
			inverseViewMatrix[2][1] = w.y;
			inverseViewMatrix[2][2] = w.z;
			inverseViewMatrix[3][0] = position.x;
			inverseViewMatrix[3][1] = position.y;
			inverseViewMatrix[3][2] = position.z;
		}
	};

	struct MeshFilter {
		std::shared_ptr<Model> model;

		MeshFilter() = default;
		MeshFilter(MeshFilter&) = default;
	};

	//Not Component, needs to be moved
	struct Shader {
		std::string vertShaderPath;
		std::string fragShaderPath;

		Shader() = default;
		Shader(Shader&) = default;
	};

	struct Material {
		std::shared_ptr<Shader> shader = nullptr;

		std::shared_ptr<Texture> diffuseMap = nullptr;
		std::shared_ptr<Texture> normalMap = nullptr;
		std::shared_ptr<Texture> ambientOcclusionMap = nullptr;
		std::shared_ptr<Texture> glossMap = nullptr;

		Material() = default;
		Material(Material&) = default;
	};

	struct MeshRenderer {

		MeshFilter mesh;
		std::shared_ptr<Material> material = nullptr;

		MeshRenderer() = default;
		MeshRenderer(MeshRenderer&) = default;

		std::shared_ptr<Model> getModel() {
			return mesh.model;
		}

		std::shared_ptr<Material> getMaterial() {
			return material;
		}
	};

	struct PointLight {
		glm::vec3 color;
		float radius = 5.0f;
		float intensity = 1.0f;

		PointLight() = default;
		PointLight(PointLight&) = default;
	};

	struct Transform {
		glm::vec3 translation{}; // (position offset)
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 m_transform() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f}
			};
		}

		glm::mat3 normalMatrix() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			const glm::vec3 invScale = 1.0f / scale;

			return glm::mat3{
				{
					invScale.x * (c1 * c3 + s1 * s2 * s3),
					invScale.x * (c2 * s3),
					invScale.x * (c1 * s2 * s3 - c3 * s1),
				},
				{
					invScale.y * (c3 * s1 * s2 - c1 * s3),
					invScale.y * (c2 * c3),
					invScale.y * (c1 * c3 * s2 + s1 * s3),
				},
				{
					invScale.z * (c2 * s1),
					invScale.z * (-s2),
					invScale.z * (c1 * c2),
				},
			};
		}


		Transform() = default;
		Transform(const Transform&) = default;
		//Transform(const glm::mat4& transform) : m_transform(transform) {}

		operator glm::mat4() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f}
			};
		}
		//operator const glm::mat4& () const { return m_transform; }
	};

	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup<Transform, MeshRenderer,
		MeshFilter, Material, Camera, PointLight>;
}