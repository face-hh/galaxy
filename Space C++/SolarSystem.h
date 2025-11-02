#pragma once
#include <vector>
#include "Camera.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const double GALAXY_TO_SYSTEM_TRANSITION_DIST = 50.0;
const double SYSTEM_SCALE_MULTIPLIER = 500.0;
const int NUM_PLANETS = 8;

struct PlanetData {
    const char* name;
    double orbitRadius;
    double radius;
    float r, g, b;
};

struct Planet {
    double x, y, z;
    double orbitRadius;
    double radius;
    double angle;
    double orbitalSpeed;
    float r, g, b;
};

struct Sun {
    double x, y, z;
    double radius;
};

struct SolarSystem {
    double centerX, centerY, centerZ;
    bool isGenerated;
};

struct RenderZone {
    double distanceFromSystem;
    double zoomLevel;
    double solarSystemScaleMultiplier;
    double starBrightnessFade;
    bool renderOrbits;
};

extern const PlanetData PLANET_DATA[NUM_PLANETS];

extern SolarSystem solarSystem;
extern Sun sun;
extern std::vector<Planet> planets;

RenderZone calculateRenderZone(const Camera& camera);
void generateSolarSystem();
void updatePlanets(double deltaTime);
void renderSolarSystem(const RenderZone& zone);
