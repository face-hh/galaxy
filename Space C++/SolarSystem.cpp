#include "SolarSystem.h"
#include "UI.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <random>

const PlanetData PLANET_DATA[NUM_PLANETS] = {
    {"Mercury", 0.39, 0.383, 0.7f, 0.7f, 0.7f},
    {"Venus", 0.72, 0.949, 0.9f, 0.8f, 0.6f},
    {"Earth", 1.00, 1.000, 0.3f, 0.5f, 0.8f},
    {"Mars", 1.52, 0.532, 0.8f, 0.4f, 0.3f},
    {"Jupiter", 5.20, 11.21, 0.9f, 0.8f, 0.6f},
    {"Saturn", 9.54, 9.45, 0.9f, 0.9f, 0.7f},
    {"Uranus", 19.2, 4.01, 0.6f, 0.8f, 0.9f},
    {"Neptune", 30.1, 3.88, 0.4f, 0.5f, 0.9f}};

SolarSystem solarSystem = {0.0, 0.0, 0.0, false};
Sun sun = {0.0, 0.0, 0.0, 2.0};
std::vector<Planet> planets;

RenderZone calculateRenderZone(const Camera &camera)
{
    RenderZone zone;
    zone.zoomLevel = camera.zoomLevel;
    zone.distanceFromSystem = 0.0;

    const double GALAXY_ZOOM_MAX = 0.1;
    const double SYSTEM_ZOOM_MIN = 100.0;

    if (camera.zoomLevel < GALAXY_ZOOM_MAX)
    {
        zone.solarSystemScaleMultiplier = 1.0;
        zone.starBrightnessFade = 1.0;
        zone.renderOrbits = false;
    }
    else if (camera.zoomLevel < SYSTEM_ZOOM_MIN)
    {
        double t = (camera.zoomLevel - GALAXY_ZOOM_MAX) / (SYSTEM_ZOOM_MIN - GALAXY_ZOOM_MAX);
        t = t * t * t; // Cubic easing

        zone.solarSystemScaleMultiplier = 1.0 + (g_currentSolarSystemScale - 1.0) * t;
        zone.starBrightnessFade = 1.0;
        zone.renderOrbits = false;
    }
    else
    {
        zone.solarSystemScaleMultiplier = g_currentSolarSystemScale;
        zone.starBrightnessFade = 1.0;
        zone.renderOrbits = true;
    }

    return zone;
}

void generateSolarSystem()
{
    std::cout << "Generating solar system..." << std::endl;

    // radius 200-600 to avoid bulge and edge
    double radius = 200.0 + (rand() / (double)RAND_MAX) * 400.0;
    double angle = (rand() / (double)RAND_MAX) * 2.0 * M_PI;
    double verticalOffset = ((rand() / (double)RAND_MAX) - 0.5) * 20.0;

    solarSystem.centerX = radius * cos(angle);
    solarSystem.centerY = verticalOffset;
    solarSystem.centerZ = radius * sin(angle);
    solarSystem.isGenerated = true;

    sun.x = solarSystem.centerX;
    sun.y = solarSystem.centerY;
    sun.z = solarSystem.centerZ;
    sun.radius = 2.0;

    planets.clear();
    for (int i = 0; i < NUM_PLANETS; i++)
    {
        Planet planet;
        planet.orbitRadius = PLANET_DATA[i].orbitRadius * 0.15;
        planet.radius = PLANET_DATA[i].radius * 0.01;
        planet.r = PLANET_DATA[i].r;
        planet.g = PLANET_DATA[i].g;
        planet.b = PLANET_DATA[i].b;
        planet.angle = ((double)rand() / RAND_MAX) * 2.0 * M_PI;
        planet.orbitalSpeed = 0.0005 / sqrt(planet.orbitRadius);

        planet.x = sun.x + planet.orbitRadius * cos(planet.angle);
        planet.y = sun.y;
        planet.z = sun.z + planet.orbitRadius * sin(planet.angle);

        planets.push_back(planet);
    }

    std::cout << "Solar system at (" << solarSystem.centerX << ", "
              << solarSystem.centerY << ", " << solarSystem.centerZ << ")" << std::endl;
}

void updatePlanets(double deltaTime)
{
    for (auto &planet : planets)
    {
        planet.angle += planet.orbitalSpeed * deltaTime;
        while (planet.angle > 2.0 * M_PI)
            planet.angle -= 2.0 * M_PI;
        while (planet.angle < 0.0)
            planet.angle += 2.0 * M_PI;

        planet.x = sun.x + planet.orbitRadius * cos(planet.angle);
        planet.z = sun.z + planet.orbitRadius * sin(planet.angle);
    }
}

void drawSphere(float radius, int segments)
{
    for (int lat = 0; lat < segments; lat++)
    {
        float theta1 = lat * M_PI / segments;
        float theta2 = (lat + 1) * M_PI / segments;

        glBegin(GL_QUAD_STRIP);
        for (int lon = 0; lon <= segments; lon++)
        {
            float phi = lon * 2 * M_PI / segments;

            float x1 = radius * sin(theta1) * cos(phi);
            float y1 = radius * cos(theta1);
            float z1 = radius * sin(theta1) * sin(phi);

            float x2 = radius * sin(theta2) * cos(phi);
            float y2 = radius * cos(theta2);
            float z2 = radius * sin(theta2) * sin(phi);

            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
        }
        glEnd();
    }
}

void renderSolarSystem(const RenderZone &zone)
{
    double scale = zone.solarSystemScaleMultiplier;

    glPushMatrix();
    glTranslated(sun.x, sun.y, sun.z);
    glScaled(scale, scale, scale);

    float sunRadius = 0.01f;
    if (zone.zoomLevel > 1000.0)
        sunRadius = 0.05f;
    else if (zone.zoomLevel > 500.0)
        sunRadius = 0.04f;
    else if (zone.zoomLevel > 100.0)
        sunRadius = 0.03f;
    else if (zone.zoomLevel > 10.0)
        sunRadius = 0.02f;
    else if (zone.zoomLevel > 1.0)
        sunRadius = 0.015f;

    glColor3f(1.0f, 1.0f, 0.3f);
    drawSphere(sunRadius / scale, 16);
    glPopMatrix();

    for (const auto &planet : planets)
    {
        glPushMatrix();
        glTranslated(sun.x, sun.y, sun.z);
        glScaled(scale, scale, scale);

        double relX = (planet.x - sun.x) / scale;
        double relY = (planet.y - sun.y) / scale;
        double relZ = (planet.z - sun.z) / scale;
        glTranslated(relX, relY, relZ);

        float planetRadius = 0.002f;
        if (zone.zoomLevel > 1000.0)
            planetRadius = 0.003f;
        else if (zone.zoomLevel > 500.0)
            planetRadius = 0.003f;
        else if (zone.zoomLevel > 100.0)
            planetRadius = 0.0025f;
        else if (zone.zoomLevel > 50.0)
            planetRadius = 0.002f;
        else if (zone.zoomLevel > 10.0)
            planetRadius = 0.002f;

        glColor3f(planet.r, planet.g, planet.b);
        drawSphere(planetRadius / scale, 12);
        glPopMatrix();

        if (zone.renderOrbits)
        {
            glPushMatrix();
            glTranslated(sun.x, sun.y, sun.z);
            glScaled(scale, scale, scale);

            glBegin(GL_LINE_LOOP);
            glColor3f(0.3f, 0.3f, 0.3f);
            for (int i = 0; i < 64; i++)
            {
                double angle = (i / 64.0) * 2.0 * M_PI;
                double x = planet.orbitRadius * cos(angle) / scale;
                double z = planet.orbitRadius * sin(angle) / scale;
                glVertex3f(x, 0, z);
            }
            glEnd();
            glPopMatrix();
        }
    }
}
