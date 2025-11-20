#include "Camera.h"
#include "SolarSystem.h"
#include "UI.h"
#include <GLFW/glfw3.h>
#include <cmath>

void setupCamera(const Camera& camera, int width, int height, const SolarSystem& solarSystem) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double fov = 45.0;
	double aspect = (double)width / (double)height;
	double nearPlane = 0.1;
	double farPlane = 10000.0;

	double f = 1.0 / tan(fov * 0.5 * M_PI / 180.0);
	double projection[16] = {
		f / aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (farPlane + nearPlane) / (nearPlane - farPlane), -1,
		0, 0, (2 * farPlane * nearPlane) / (nearPlane - farPlane), 0
	};
	glLoadMatrixd(projection);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotated(-camera.pitch * 180.0 / M_PI, 1, 0, 0);
	glRotated(-camera.yaw * 180.0 / M_PI, 0, 1, 0);

	glTranslated(-camera.posX, -camera.posY, -camera.posZ);

	if (camera.freeZoomMode) {
		glTranslated(solarSystem.centerX, solarSystem.centerY, solarSystem.centerZ);
		glScaled(camera.zoom, camera.zoom, camera.zoom);
		glTranslated(-solarSystem.centerX, -solarSystem.centerY, -solarSystem.centerZ);
	} else {
		glScaled(camera.zoom, camera.zoom, camera.zoom);
	}
}

void processInput(GLFWwindow* window, Camera& camera, const UIState* uiState) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	if (uiState && uiState->isVisible) return;

	double speedMultiplier = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 2.0 : 1.0;
	double currentSpeed = camera.moveSpeed * speedMultiplier;

	double forward[3] = {
		-sin(camera.yaw),
		0,
		-cos(camera.yaw)
	};

	double right[3] = {
		-sin(camera.yaw - M_PI / 2),
		0,
		-cos(camera.yaw - M_PI / 2)
	};

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.posX += forward[0] * currentSpeed;
		camera.posZ += forward[2] * currentSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.posX -= forward[0] * currentSpeed;
		camera.posZ -= forward[2] * currentSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.posX -= right[0] * currentSpeed;
		camera.posZ -= right[2] * currentSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.posX += right[0] * currentSpeed;
		camera.posZ += right[2] * currentSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.posY += currentSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camera.posY += currentSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		camera.posY -= currentSpeed;
	}
}
