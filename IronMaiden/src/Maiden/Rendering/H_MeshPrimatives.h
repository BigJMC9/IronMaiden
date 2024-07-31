#pragma once
#include "H_Mesh.h"

namespace Madam
{
	struct Primative
	{
		struct Quad
		{
			static inline std::vector<glm::vec3> GetVertices()
			{
				return
				{
					glm::vec3(-0.5, -0.5, 0),
					glm::vec3(-0.5, 0.5, 0),
					glm::vec3(0.5, 0.5, 0),
					glm::vec3(0.5, -0.5, 0)
				};
			}

			static inline std::vector<uint32_t> GetIndices()
			{
				return
				{
					0, 1, 2,
					0, 2, 3
				};
			}

			static inline std::vector<uint32_t> GetNormals()
			{
				return
				{
					0, 1, 0,
				};
			}
		};

		struct Cube
		{
			static inline std::vector<glm::vec3> GetVertices()
			{
				return
				{
					glm::vec3(-0.5, -0.5, -0.5), //Bottom Left
					glm::vec3(-0.5, 0.5, -0.5), //Top Left
					glm::vec3(0.5, 0.5, -0.5), //Top Right
					glm::vec3(0.5, -0.5, -0.5), // Bottom Right
					glm::vec3(-0.5, -0.5, 0.5), //Bottom Left
					glm::vec3(-0.5, 0.5, 0.5), //Top Left
					glm::vec3(0.5, 0.5, 0.5), //Top Right
					glm::vec3(0.5, -0.5, 0.5) //Bottom Right		
				};
			}

			static inline std::vector<uint32_t> GetIndices()
			{
				return
				{
					//Front
					0, 1, 2,
					0, 2, 3,
					//Back
					4, 6, 5,
					4, 7, 6,
					//Top
					1, 5, 6,
					1, 6, 2,
					//Bottom
					0, 7, 4,
					0, 3, 7,
					//Left
					4, 5, 1,
					4, 1, 0,
					//Right
					7, 2, 6,
					7, 3, 2
				};
			}

			static inline std::vector<uint32_t> GetNormals()
			{
				return
				{
					//Front
					0, 1, 2,
					0, 2, 3,
					//Back
					4, 6, 5,
					4, 7, 6,
					//Top
					1, 5, 6,
					1, 6, 2,
					//Bottom
					0, 7, 4,
					0, 3, 7,
					//Left
					4, 5, 1,
					4, 1, 0,
					//Right
					7, 2, 6,
					7, 3, 2
				};
			}
		};
	};
}