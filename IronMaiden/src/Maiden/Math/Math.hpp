#pragma once
#include "maidenpch.hpp"
#include "../Core/H_Logger.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#ifdef far
#undef far
#endif
#ifdef near
#undef near
#endif

namespace Madam {
	using namespace glm;
	namespace Math {

		struct Vector3 {
			vec3 value;
			float& x = value.x;
			float& y = value.y;
			float& z = value.z;

			Vector3(glm::vec3 v) : value(v), x(value.x), y(value.y), z(value.z) {}
			Vector3(float x, float y, float z) : value(vec3(x,y,z)), x(value.x), y(value.y), z(value.z) {}
			Vector3() {
				value = vec3(0);
				x = value.x;
				y = value.y;
				z = value.z;
			}
			float& operator[](int i) {
				return value[i];
			}
			operator vec3() const {
				return value;
			}
			Vector3& operator=(const vec3& v) {
				value = v;
				x = value.x;
				y = value.y;
				z = value.z;
				MADAM_CORE_INFO("Vector3: {0}", glm::to_string(value));
				return *this;
			}
			Vector3& operator=(const Vector3& v) {
				value = v.value;
				x = value.x;
				y = value.y;
				z = value.z;
				return *this;
			}
			Vector3& operator+=(const vec3& v) {
				value += v;
				x = value.x;
				y = value.y;
				z = value.z;
				return *this;
			}
			Vector3 operator/(const Vector3& other) const {
				return { x / other.x, y / other.y, z / other.z };
			}
			template <typename T>
			friend Vector3 operator/(const Vector3& lhs, T rhs) {
				return { lhs.value / rhs };
			}
			template <typename T>
			friend Vector3 operator/(T lhs, const Vector3& rhs) {
				return { lhs / rhs.value };
			}
		};

		struct Quaternion {
			quat value;
			float& x = value.x;
			float& y = value.y;
			float& z = value.z;
			float& w = value.w;

			Quaternion(vec3 v) {
				value = quat(v);
				x = value.x;
				y = value.y;
				z = value.z;
				w = value.w;
			}
			Quaternion(Vector3 v) {
				value = quat(v);
				x = value.x;
				y = value.y;
				z = value.z;
				w = value.w;
			}
			Quaternion(quat q) : value(q), x(value.x), y(value.y), z(value.z), w(value.w) {}
			Quaternion(float x, float y, float z, float w) : value(quat(x,y,z,w)), x(value.x), y(value.y), z(value.z), w(value.w) {}
			Quaternion() {
				value = quat();
				x = value.x;
				y = value.y;
				z = value.z;
				w = value.w;
			}
			float& operator[](int i) {
				return value[i];
			}
			Quaternion& operator=(const quat& q) {
				value = q;
				x = value.x;
				y = value.y;
				z = value.z;
				w = value.w;
				return *this;
			}
			Quaternion& operator=(const Quaternion& q) {
				value = q.value;
				x = value.x;
				y = value.y;
				z = value.z;
				w = value.w;
				return *this;
			}
			Quaternion& operator=(const vec3& v) {
				value = quat(v);
				x = value.x;
				y = value.y;
				z = value.z;
				w = value.w;
				return *this;
			}
			Quaternion& operator*=(const Quaternion& q) {
				value = value * q.value;
				x = value.x;
				y = value.y;
				z = value.z;
				w = value.w;
				return *this;
			}
			Quaternion& operator*=(const quat& q) {
				value = value * q;
				x = value.x;
				y = value.y;
				z = value.z;
				w = value.w;
				return *this;
			}
			Quaternion& operator*=(const vec3& v) {
				value = value * quat(v);
				x = value.x;
				y = value.y;
				z = value.z;
				w = value.w;
				return *this;
			}

			Quaternion operator*(const Quaternion& other) const {
				return { x * other.x, y * other.y, z * other.z, w * other.w };
			}

			operator vec3() const {
				return glm::eulerAngles(value);
			}
			operator Vector3() const {
				return glm::eulerAngles(value);
			}
			operator quat() const {
				return value;
			}
			Quaternion operator/(const Quaternion& other) const {
				return { x / other.x, y / other.y, z / other.z, w / other.w };
			}
			vec3 EulerAngles() const {
				return glm::eulerAngles(value);
			}
			static vec3 EulerAngles(const quat& q) {
				return glm::eulerAngles(q);
			}
			static vec3 EulerAngles(const Quaternion& q) {
				return glm::eulerAngles(q.value);
			}
		};

		static bool DecomposeTransformMatrix(const mat4& transform, Vector3& translation, Quaternion& rotation, Vector3& scale) {

			using T = float;

			mat4 localMatrix(transform);

			if (epsilonEqual(localMatrix[3][3], static_cast<T>(0), epsilon<T>())) {
				return false;
			}
			MADAM_CORE_ASSERT(epsilonEqual(localMatrix[3][3], static_cast<T>(1), static_cast<T>(0.00001)), "");

			MADAM_CORE_ASSERT(
				epsilonEqual(localMatrix[0][3], static_cast<T>(0), epsilon<T>()) &&
				epsilonEqual(localMatrix[1][3], static_cast<T>(0), epsilon<T>()) &&
				epsilonEqual(localMatrix[2][3], static_cast<T>(0), epsilon<T>()), "");

			translation = vec3(localMatrix[3]);
			localMatrix[3] = vec4(0.0f, 0.0f, 0.0f, localMatrix[3].w);

			vec3 row[3], pdum3;

			for (glm::length_t i = 0; i < 3; ++i)
			{
				for (glm::length_t j = 0; j < 3; j++)
				{
					row[i][j] = localMatrix[i][j];
				}
			}

			scale.x = glm::length(row[0]);
			row[0] = glm::detail::scale(row[0], static_cast<T>(1));
			scale.y = glm::length(row[1]);
			row[1] = glm::detail::scale(row[1], static_cast<T>(1));
			scale.z = glm::length(row[2]);
			row[2] = glm::detail::scale(row[2], static_cast<T>(1));

			//Might need this
			/*
			pdum3 = glm::cross(row[1], row[2]);
			if (dot(row[0], pdum3) < 0)
			{
				for (glm::length_t i = 0; i < 3; i++)
				{
					scale[i] *= -1.0f;
					row[i] *= -1.0f;
				}
			}
			*/

			int i, j, k = 0;
			T root, trace = row[0].x + row[1].y + row[2].z;
			if (trace > static_cast<T>(0))
			{
				root = sqrt(trace + static_cast<T>(1));
				rotation.w = static_cast<T>(0.5) * root;
				root = static_cast<T>(0.5) / root;
				rotation.x = root * (row[1].z - row[2].y);
				rotation.y = root * (row[2].x - row[0].z);
				rotation.z = root * (row[0].y - row[1].x);
			} // End if > 0
			else
			{
				static int next[3] = { 1, 2, 0 };
				i = 0;
				if (row[1].y > row[0].x) i = 1;
				if (row[2].z > row[i][i]) i = 2;
				j = next[i];
				k = next[j];

				root = sqrt(row[i][i] - row[j][j] - row[k][k] + static_cast<T>(1.0));

				rotation[i] = static_cast<T>(0.5) * root;
				root = static_cast<T>(0.5) / root;
				rotation[j] = root * (row[i][j] + row[j][i]);
				rotation[k] = root * (row[i][k] + row[k][i]);
				rotation.w = root * (row[j][k] - row[k][j]);
			} // End if <= 0
			return true;
		}

		static bool DecomposeTransformMatrix(const mat4& transform, vec3& translation, quat& rotation, vec3& scale) {

			using T = float;

			mat4 localMatrix(transform);

			if (epsilonEqual(localMatrix[3][3], static_cast<T>(0), epsilon<T>())) {
				return false;
			}
			MADAM_CORE_ASSERT(epsilonEqual(localMatrix[3][3], static_cast<T>(1), static_cast<T>(0.00001)), "");

			MADAM_CORE_ASSERT(
				epsilonEqual(localMatrix[0][3], static_cast<T>(0), epsilon<T>()) &&
				epsilonEqual(localMatrix[1][3], static_cast<T>(0), epsilon<T>()) &&
				epsilonEqual(localMatrix[2][3], static_cast<T>(0), epsilon<T>()), "");

			translation = vec3(localMatrix[3]);
			localMatrix[3] = vec4(0.0f, 0.0f, 0.0f, localMatrix[3].w);

			vec3 row[3], pdum3;

			for (glm::length_t i = 0; i < 3; ++i)
			{
				for (glm::length_t j = 0; j < 3; j++)
				{
					row[i][j] = localMatrix[i][j];
				}
			}

			scale.x = glm::length(row[0]);
			row[0] = glm::detail::scale(row[0], static_cast<T>(1));
			scale.y = glm::length(row[1]);
			row[1] = glm::detail::scale(row[1], static_cast<T>(1));
			scale.z = glm::length(row[2]);
			row[2] = glm::detail::scale(row[2], static_cast<T>(1));

			//Might need this
			/*
			pdum3 = glm::cross(row[1], row[2]);
			if (dot(row[0], pdum3) < 0)
			{
				for (glm::length_t i = 0; i < 3; i++)
				{
					scale[i] *= -1.0f;
					row[i] *= -1.0f;
				}
			}
			*/

			int i, j, k = 0;
			T root, trace = row[0].x + row[1].y + row[2].z;
			if (trace > static_cast<T>(0))
			{
				root = sqrt(trace + static_cast<T>(1));
				rotation.w = static_cast<T>(0.5) * root;
				root = static_cast<T>(0.5) / root;
				rotation.x = root * (row[1].z - row[2].y);
				rotation.y = root * (row[2].x - row[0].z);
				rotation.z = root * (row[0].y - row[1].x);
			} // End if > 0
			else
			{
				static int next[3] = { 1, 2, 0 };
				i = 0;
				if (row[1].y > row[0].x) i = 1;
				if (row[2].z > row[i][i]) i = 2;
				j = next[i];
				k = next[j];

				root = sqrt(row[i][i] - row[j][j] - row[k][k] + static_cast<T>(1.0));

				rotation[i] = static_cast<T>(0.5) * root;
				root = static_cast<T>(0.5) / root;
				rotation[j] = root * (row[i][j] + row[j][i]);
				rotation[k] = root * (row[i][k] + row[k][i]);
				rotation.w = root * (row[j][k] - row[k][j]);
			} // End if <= 0
			return true;
		}
	}
}