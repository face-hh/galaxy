#include "Input.h"
#include <GLFW/glfw3.h>
#include <iostream>

static Camera* g_camera = nullptr;
static MouseState* g_mouseState = nullptr;

void setGlobalCamera(Camera* cam) {
	g_camera = cam;
}

void setGlobalMouseState(MouseState* ms) {
	g_mouseState = ms;
}

void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
	if (!g_camera || !g_mouseState) return;

	if (g_mouseState->firstMouse) {
		g_mouseState->lastX = xpos;
		g_mouseState->lastY = ypos;
		g_mouseState->firstMouse = false;
		return;
	}

	double xoffset = xpos - g_mouseState->lastX;
	double yoffset = g_mouseState->lastY - ypos;
	g_mouseState->lastX = xpos;
	g_mouseState->lastY = ypos;

	g_camera->yaw -= xoffset * g_camera->lookSpeed;
	g_camera->pitch += yoffset * g_camera->lookSpeed;

	if (g_camera->pitch > 1.5) g_camera->pitch = 1.5;
	if (g_camera->pitch < -1.5) g_camera->pitch = -1.5;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	if (!g_camera) return;

	g_camera->zoom *= (1.0 - yoffset * 0.1);
	if (g_camera->zoom < 0.001) g_camera->zoom = 0.001;
	if (g_camera->zoom > 1000.0) g_camera->zoom = 1000.0;
}

void initInput(GLFWwindow* window, Camera& camera, MouseState& mouseState) {
	setGlobalCamera(&camera);
	setGlobalMouseState(&mouseState);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetScrollCallback(window, scrollCallback);
}
