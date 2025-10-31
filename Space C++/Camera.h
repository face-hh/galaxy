#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Camera {
    double posX = 0.0, posY = 0.0, posZ = 10.0;
    double pitch = 0.0, yaw = 0.0;
    double zoom = 1.0;
    double moveSpeed = 0.1;
    double lookSpeed = 0.002;
};

void setupCamera(const Camera& camera, int width, int height);
void processInput(struct GLFWwindow* window, Camera& camera);
