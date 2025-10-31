#pragma once

struct WindowConfig {
 int width;
    int height;
    const char* title;
};

struct GLFWwindow* initWindow(const WindowConfig& config);

void setupOpenGL();
void cleanup(struct GLFWwindow* window);
