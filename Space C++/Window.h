#pragma once

struct WindowConfig {
 int width;
    int height;
    const char* title;
};

extern int WIDTH;
extern int HEIGHT;

struct GLFWwindow* initWindow(const WindowConfig& config);

void setupOpenGL();
void cleanup(struct GLFWwindow* window);

void framebufferSizeCallback(struct GLFWwindow* window, int width, int height);
