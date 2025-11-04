#include "Window.h"
#include <GLFW/glfw3.h>
#include <iostream>

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	WIDTH = width;
	HEIGHT = height;

	glViewport(0, 0, width, height);
}

GLFWwindow* initWindow(const WindowConfig& config) {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glViewport(0, 0, config.width, config.height);

	return window;
}

void setupOpenGL() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.02f, 1.0f);
}

void cleanup(GLFWwindow* window) {
	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();
}
