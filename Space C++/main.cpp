#include <GLFW/glfw3.h>
#include <iostream>
#include "Window.h"
#include "Camera.h"
#include "Stars.h"
#include "Input.h"

// Configuration
const int WIDTH = 1920;
const int HEIGHT = 1080;

GalaxyConfig createDefaultGalaxyConfig() {
	GalaxyConfig config;
	config.numStars = 1000000;
	config.numSpiralArms = 2;
	config.spiralTightness = 0.3;
	config.armWidth = 60.0;
	config.diskRadius = 800.0;
	config.bulgeRadius = 150.0;
	config.diskHeight = 50.0;
	config.bulgeHeight = 100.0;
	config.armDensityBoost = 10.0;
	config.seed = 42;
	config.rotationSpeed = 1.0;
	return config;
}

void render(const std::vector<Star>& stars, const Camera& camera) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setupCamera(camera, WIDTH, HEIGHT);
	renderStars(stars);
}

int main() {
	WindowConfig windowConfig = { WIDTH, HEIGHT, "untitled Galaxy sim" };
	GLFWwindow* window = initWindow(windowConfig);
	if (!window) {
		return -1;
	}

	setupOpenGL();

	Camera camera;
	MouseState mouseState = { WIDTH / 2.0, HEIGHT / 2.0, true };

	initInput(window, camera, mouseState);

	// Generate galaxy
	GalaxyConfig galaxyConfig = createDefaultGalaxyConfig();
	std::vector<Star> stars;
	generateStarField(stars, galaxyConfig);

	double lastTime = glfwGetTime();

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		updateStarPositions(stars, deltaTime);

		processInput(window, camera);
		render(stars, camera);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanup(window);
	return 0;
}
