#include <GLFW/glfw3.h>
#include <iostream>
#include <random>
#include <ctime>
#include "Window.h"
#include "Camera.h"
#include "Stars.h"
#include "SolarSystem.h"
#include "BlackHole.h"
#include "GalacticGas.h"
#include "Input.h"

int WIDTH = 1920;
int HEIGHT = 1080;

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

	std::random_device rd;
	config.seed = rd();

	config.rotationSpeed = 1.0;

	std::cout << "Galaxy seed: " << config.seed << std::endl;

	return config;
}

BlackHoleConfig createDefaultBlackHoleConfig() {
	BlackHoleConfig config;
	config.enableSupermassive = true;

	return config;
}

void render(const std::vector<Star>& stars, const std::vector<BlackHole>& blackHoles,
            const std::vector<GasCloud>& gasClouds, const Camera& camera) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	setupCamera(camera, WIDTH, HEIGHT, solarSystem);

	RenderZone zone = calculateRenderZone(camera);

	renderStars(stars, zone);

	renderGalacticGas(gasClouds, zone);
	renderBlackHoles(blackHoles, zone);

	if (solarSystem.isGenerated) {
		renderSolarSystem(zone);
	}
}

int main() {
	srand(static_cast<unsigned int>(time(nullptr)));

	WindowConfig windowConfig = { WIDTH, HEIGHT, "untitled Galaxy sim" };
	GLFWwindow* window = initWindow(windowConfig);
	if (!window) {
		return -1;
	}

	setupOpenGL();

	Camera camera;
	camera.posY = 200.0;
	camera.pitch = -0.2;
	camera.zoomLevel = 0.001;
	camera.zoom = camera.zoomLevel;

	MouseState mouseState = { WIDTH / 2.0, HEIGHT / 2.0, true };

	initInput(window, camera, mouseState);

	// Generate galaxy
	GalaxyConfig galaxyConfig = createDefaultGalaxyConfig();
	std::vector<Star> stars;
	generateStarField(stars, galaxyConfig);

	BlackHoleConfig blackHoleConfig = createDefaultBlackHoleConfig();
	std::vector<BlackHole> blackHoles;
	generateBlackHoles(blackHoles, blackHoleConfig, galaxyConfig.seed,
	                   galaxyConfig.diskRadius, galaxyConfig.bulgeRadius);

	GasConfig gasConfig = createDefaultGasConfig();
	std::vector<GasCloud> gasClouds;
	generateGalacticGas(gasClouds, gasConfig, galaxyConfig.seed,
	                    galaxyConfig.diskRadius, galaxyConfig.bulgeRadius);

	generateSolarSystem();

	double lastTime = glfwGetTime();

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		updateStarPositions(stars, deltaTime);
		updateBlackHoles(blackHoles, deltaTime);
		updateGalacticGas(gasClouds, deltaTime);
		updatePlanets(deltaTime);

		processInput(window, camera);
		render(stars, blackHoles, gasClouds, camera);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanup(window);
	return 0;
}
