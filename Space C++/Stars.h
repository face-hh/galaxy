#pragma once
#include <vector>

struct RenderZone;

struct Star {
	float x, y, z;
	float r, g, b;
	float brightness;

	// For rotation animation
	float radius;			// Distance from galactic center
	float angle;			// Current angle in XZ plane
	float angularVelocity;  // Rotation speed (radians per second)
};

struct GalaxyConfig {
	int numStars;
	int numSpiralArms;
	double spiralTightness;
	double armWidth;
	double diskRadius;
	double bulgeRadius;
	double diskHeight;
	double bulgeHeight;
	double armDensityBoost;
	unsigned int seed;

	double rotationSpeed;	// Base rotation multiplier
};

void generateStarField(std::vector<Star>& stars, const GalaxyConfig& config);
void updateStarPositions(std::vector<Star>& stars, double deltaTime);
void renderStars(const std::vector<Star>& stars, const RenderZone& zone);
