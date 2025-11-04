#pragma once
#include <vector>

struct RenderZone;

enum class BlackHoleType {
    SUPERMASSIVE,
    STELLAR
};

struct BlackHole {
    float x, y, z;
    BlackHoleType type;

    float mass;
    float eventHorizonRadius;
    float accretionDiskInnerRadius;
    float accretionDiskOuterRadius;
    bool hasAccretionDisk;

    float diskRotationAngle;
    float diskRotationSpeed;

    float radius;
    float angle;
    float angularVelocity;
};

struct BlackHoleConfig {
    bool enableSupermassive;
    int numStellarBlackHoles;
    float stellarBlackHoleFraction;
};

void generateBlackHoles(std::vector<BlackHole>& blackHoles, const BlackHoleConfig& config, unsigned int seed, double diskRadius, double bulgeRadius);
void updateBlackHoles(std::vector<BlackHole>& blackHoles, double deltaTime);
void renderBlackHoles(const std::vector<BlackHole>& blackHoles, const RenderZone& zone);

const double SOLAR_MASS_KG = 1.989e30;
const double SPEED_OF_LIGHT = 2.998e8;
const double GRAVITATIONAL_CONSTANT = 6.674e-11;

inline float calculateSchwarzschildRadius(float solarMasses) {
    return 2.95f * solarMasses;
}
