#pragma once

#include "maidenpch.hpp"
#include "../Scene/H_Entity.hpp"

namespace Madam {

    class SceneSerializer;

    //Human interface device (see pg 53)

    enum class KeyCode {
        Q = GLFW_KEY_Q,
        W = GLFW_KEY_W,
        E = GLFW_KEY_E,
        R = GLFW_KEY_R,
        T = GLFW_KEY_T,
        Y = GLFW_KEY_Y,
        U = GLFW_KEY_U,
        I = GLFW_KEY_I,
        O = GLFW_KEY_O,
        P = GLFW_KEY_P,
        A = GLFW_KEY_A,
        S = GLFW_KEY_S,
        D = GLFW_KEY_D,
        F = GLFW_KEY_F,
        G = GLFW_KEY_G,
        H = GLFW_KEY_H,
        J = GLFW_KEY_J,
        K = GLFW_KEY_K,
        L = GLFW_KEY_L,
        Z = GLFW_KEY_Z,
        X = GLFW_KEY_X,
        C = GLFW_KEY_C,
        V = GLFW_KEY_V,
        B = GLFW_KEY_B,
        N = GLFW_KEY_N,
        M = GLFW_KEY_M,
        SPACE = GLFW_KEY_SPACE,
        LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
        LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
        LEFT_ALT = GLFW_KEY_LEFT_ALT,
        RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
        RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
        RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
        ESCAPE = GLFW_KEY_ESCAPE,
        ENTER = GLFW_KEY_ENTER,
        TAB = GLFW_KEY_TAB,
        BACKSPACE = GLFW_KEY_BACKSPACE,
        DEL = GLFW_KEY_DELETE,
        INSERT = GLFW_KEY_INSERT,
        HOME = GLFW_KEY_HOME,
		END = GLFW_KEY_END,
        UP = GLFW_KEY_UP,
        DOWN = GLFW_KEY_DOWN,
        LEFT = GLFW_KEY_LEFT,
        RIGHT = GLFW_KEY_RIGHT,
        F1 = GLFW_KEY_F1,
        F2 = GLFW_KEY_F2,
        F3 = GLFW_KEY_F3,
        F4 = GLFW_KEY_F4,
        F5 = GLFW_KEY_F5,
        F6 = GLFW_KEY_F6,
        F7 = GLFW_KEY_F7,
        F8 = GLFW_KEY_F8,
        F9 = GLFW_KEY_F9,
        F10 = GLFW_KEY_F10,
        F11 = GLFW_KEY_F11,
        F12 = GLFW_KEY_F12,
    };

	class MADAM_API Input{

	public:
        
        bool IsKeyDown(KeyCode key);
        bool IsKeyUp(KeyCode key);
        
        Input(const Input&) = delete;
        Input& operator=(const Input&) = delete;

        static Input& Get() {
			static Input instance;
			return instance;
		}

    private:
        Input() {}
	};
}