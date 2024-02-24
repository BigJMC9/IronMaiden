#pragma once

#include "maidenpch.hpp"
#include "H_Entity.hpp"
#include "H_Entity.hpp"

namespace Madam {

    class SceneSerializer;

	class KeyboardMovementController{

	public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
            int leftControl = GLFW_KEY_LEFT_CONTROL;
        };

        void moveInPlaneXZ(GLFWwindow* window, float dt, Entity& gameObject);
        void handleCommands(GLFWwindow* window, SceneSerializer* scene);

        KeyMappings keys{};
        float moveSpeed{ 3.f };
        float lookSpeed{ 1.5f };
        bool isHandling = false;
	};
}