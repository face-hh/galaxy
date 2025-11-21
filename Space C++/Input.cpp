#include "Input.h"
#include "UI.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <random>

static Camera* g_camera = nullptr;
static MouseState* g_mouseState = nullptr;
static UIState* g_uiState = nullptr;

void setGlobalCamera(Camera* cam) {
	g_camera = cam;
}

void setGlobalMouseState(MouseState* ms) {
	g_mouseState = ms;
}

void setGlobalUIState(UIState* ui) {
	g_uiState = ui;
}

void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
	if (!g_camera || !g_mouseState) return;

	if (g_uiState && g_uiState->isVisible) return;

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

	if (g_uiState && g_uiState->isVisible) return;

	bool ctrlHeld = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
	g_camera->freeZoomMode = ctrlHeld;

	double zoomFactor = 1.15;

	if (yoffset > 0) {
		g_camera->zoomLevel *= zoomFactor;
	}
	else {
		g_camera->zoomLevel /= zoomFactor;
	}

	if (g_camera->zoomLevel < 0.0001) g_camera->zoomLevel = 0.0001;
	if (g_camera->zoomLevel > 10000.0) g_camera->zoomLevel = 10000.0;

	g_camera->zoom = g_camera->zoomLevel;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (!g_camera) return;
	if (action != GLFW_PRESS) return;
}

void initInput(GLFWwindow* window, Camera& camera, MouseState& mouseState) {
	setGlobalCamera(&camera);
	setGlobalMouseState(&mouseState);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetKeyCallback(window, keyCallback);
}
