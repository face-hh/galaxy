#pragma once
#include "Camera.h"
#include "BlackHole.h"
#include "SolarSystem.h"
#include <vector>

struct MouseState {
    double lastX;
    double lastY;
    bool firstMouse;
};

struct UIState;

void initInput(struct GLFWwindow* window, Camera& camera, MouseState& mouseState);

void mouseMoveCallback(struct GLFWwindow* window, double xpos, double ypos);
void scrollCallback(struct GLFWwindow* window, double xoffset, double yoffset);
void keyCallback(struct GLFWwindow* window, int key, int scancode, int action, int mods);

void setGlobalCamera(Camera* cam);
void setGlobalMouseState(MouseState* ms);
void setGlobalUIState(UIState* ui);
