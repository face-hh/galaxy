#pragma once
#include "Camera.h"

struct MouseState {
    double lastX;
    double lastY;
    bool firstMouse;
};

void initInput(struct GLFWwindow* window, Camera& camera, MouseState& mouseState);

void mouseMoveCallback(struct GLFWwindow* window, double xpos, double ypos);
void scrollCallback(struct GLFWwindow* window, double xoffset, double yoffset);

void setGlobalCamera(Camera* cam);
void setGlobalMouseState(MouseState* ms);
