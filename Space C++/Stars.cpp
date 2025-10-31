#include "Stars.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Star type colors (based on stellar classification: O, B, A, F, G, K, M)
struct StarType {
	float r, g, b;
	float probability;
};

const StarType starTypes[] = {
	{0.6f, 0.7f, 1.0f, 0.05f},   // O - Blue (very hot, rare)
	{0.7f, 0.8f, 1.0f, 0.10f},   // B - Blue-white (hot)
	{0.9f, 0.9f, 1.0f, 0.15f},   // A - White (hot)
	{1.0f, 1.0f, 0.9f, 0.20f},   // F - Yellow-white
	{1.0f, 1.0f, 0.7f, 0.25f},   // G - Yellow (like our Sun)
	{1.0f, 0.8f, 0.6f, 0.15f},   // K - Orange
	{1.0f, 0.6f, 0.5f, 0.10f}    // M - Red (cool, common)
};

void generateStarField(std::vector<Star>& stars, const GalaxyConfig& config) {
	std::mt19937 rng(config.seed);
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	std::normal_distribution<float> normalDist(0.0f, 1.0f);

	stars.clear();
	stars.reserve(config.numStars);

	for (int i = 0; i < config.numStars; i++) {
		Star star;

		// Decide if star is in bulge or disk
		// bulge = the spherical central region
		// disk = the flat rotating part with spiral arms
		bool inBulge = dist(rng) < 0.15f; // 15%

		if (inBulge) {
			// spherical distribution
			float theta = dist(rng) * 2.0f * M_PI; // horizontal rotation around the Z axis
			float phi = acos(2.0f * dist(rng) - 1.0f); // vertical angle from the top of sphere
			float radius = pow(dist(rng), 1.0f / 3.0f) * config.bulgeRadius;

			star.x = radius * sin(phi) * cos(theta);
			star.y = radius * sin(phi) * sin(theta);
			star.z = radius * cos(phi);

			// rotation (in union) for bulge stars
			star.radius = sqrt(star.x * star.x + star.z * star.z);
			star.angle = atan2(star.z, star.x);

			// higher velocity since bulge rotates faster
			star.angularVelocity = config.rotationSpeed * 0.5f / (config.bulgeRadius + 1.0f);
		}
		else {
			// disk & arms

			// Exponential disk sampling
			// Radial surface density: Sigma(r) ∝ exp(-r/rd)
			// Radial PDF (per radius) ∝ r * exp(-r/rd)
			// CDF: F(r) = 1 - (1 + r/rd) * exp(-r/rd)
			// Invert F numerically (Newton) to get r for uniform u in [0,1].
			auto sampleExponentialDiskRadius = [&](float diskScale) -> float {
				float u = dist(rng);
				// initial guess: exponential inverse (not exact for r*e^{-r/rd} but a reasonable start)
				float r = -diskScale * std::log(1.0f - u + 1e-8f);
				// Newton iteration to solve F(r) - u = 0
				for (int it = 0; it < 10; ++it) {
					float t = r / diskScale;
					float expNegT = std::exp(-t);
					float F = 1.0f - (1.0f + t) * expNegT;          // F(r)
					float G = F - u;                               // want G == 0
					if (std::fabs(G) < 1e-6f) break;
					// dF/dr = (r / rd^2) * exp(-r/rd)
					float dFdr = (r == 0.0f) ? (1.0f / (diskScale)) * 0.0f : (r / (diskScale * diskScale)) * expNegT;
					// fallback if derivative is too small
					if (dFdr <= 1e-12f) break;
					float delta = G / dFdr;
					r -= delta;
					if (r < 0.0f) { r = 0.0f; break; }
				}
				return r;
				};

			float diskScale = static_cast<float>(config.diskRadius) * 0.25f; // tune to taste
			float radius = sampleExponentialDiskRadius(diskScale);

			// Clip at diskRadius
			if (radius > static_cast<float>(config.diskRadius)) radius = static_cast<float>(config.diskRadius);

			// Base angle
			float theta = dist(rng) * 2.0f * M_PI;

			// Calculate distance to nearest spiral arm
			float minArmDistance = 1e10f;

			for (int arm = 0; arm < config.numSpiralArms; arm++) {
				// Logarithmic spiral: r = a * e^(b * theta)
				// Solving for theta: theta = ln(r/a) / b
				float armOffset = (arm * 2.0f * M_PI) / config.numSpiralArms;

				// Calculate where this radius intersects the spiral arm
				float spiralTheta = log(radius / config.bulgeRadius) / config.spiralTightness + armOffset;

				// Normalize angle difference to [-PI, PI]
				float angleDiff = theta - spiralTheta;
				while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;
				while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;

				// Convert angle difference to distance at this radius
				float armDistance = fabs(angleDiff * radius);
				minArmDistance = fmin(minArmDistance, armDistance);
			}

			// Stars close to arms have high probability, far from arms very low
			float armProximity = exp(-minArmDistance * minArmDistance / (config.armWidth * config.armWidth));

			// Non-linear density weight - creates sharper contrast
			float densityWeight = armProximity * config.armDensityBoost;

			// Reject stars not near arms much more aggressively
			float acceptProbability = (1.0f + densityWeight) / (1.0f + config.armDensityBoost);

			// Add extra rejection for stars far from arms
			if (armProximity < 0.3f) {
				acceptProbability *= 0.2f;// 80% rejection for inter-arm regions
			}

			if (dist(rng) > acceptProbability) {
				i--;  // Retry this star
				continue;
			}

			// Add some noise to position for more natural look (LESS noise for clearer arms)
			float noise = normalDist(rng) * 15.0f;  // Reduced from 30.0f

			star.x = (radius + noise * 0.3f) * cos(theta);
			star.z = (radius + noise * 0.3f) * sin(theta);

			// Y position - disk height with Gaussian distribution
			// use normalized radius fraction for height tapering
			float radiusNorm = radius / static_cast<float>(config.diskRadius);
			float heightScale = config.diskHeight * (1.0f - radiusNorm * 0.5f);  // Thinner at edges
			star.y = normalDist(rng) * heightScale;

			// Rotation for disk stars - differential rotation (Keplerian-like)
			star.radius = radius;
			star.angle = theta;
			// Outer stars rotate slower (like real galaxies)
			star.angularVelocity = config.rotationSpeed * 1.0f / (sqrt(radius / config.bulgeRadius) * (radius + 1.0f));
		}

		// Select star type based on probability
		float typeRoll = dist(rng);
		float cumulative = 0.0f;
		int selectedType = 6;  // Default to M type

		for (int t = 0; t < 7; t++) {
			cumulative += starTypes[t].probability;
			if (typeRoll <= cumulative) {
				selectedType = t;
				break;
			}
		}

		star.r = starTypes[selectedType].r;
		star.g = starTypes[selectedType].g;
		star.b = starTypes[selectedType].b;

		// Random brightness variation
		// Stars in bulge tend to be older/redder/dimmer
		float distFromCenter = sqrt(star.x * star.x + star.y * star.y + star.z * star.z);
		if (distFromCenter < config.bulgeRadius) {
			star.brightness = 0.4f + dist(rng) * 0.4f;  // Dimmer bulge stars
		}
		else {
			star.brightness = 0.3f + dist(rng) * 0.7f;  // Brighter disk stars

			// Stars in spiral arms are brighter (young, hot stars)
			// This will make arms even more visible!
			float minArmDist = 1e10f;
			for (int arm = 0; arm < config.numSpiralArms; arm++) {
				float armOffset = (arm * 2.0f * M_PI) / config.numSpiralArms;
				float spiralTheta = log(star.radius / config.bulgeRadius) / config.spiralTightness + armOffset;
				float angleDiff = star.angle - spiralTheta;
				while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;
				while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;
				float armDist = fabs(angleDiff * star.radius);
				minArmDist = fmin(minArmDist, armDist);
			}
			float armBrightness = exp(-minArmDist * minArmDist / (config.armWidth * config.armWidth * 4.0f));
			star.brightness += armBrightness * 0.3f; // Up to 30% brighter in arms
			if (star.brightness > 1.0f) star.brightness = 1.0f;
		}

		stars.push_back(star);
	}
}

void updateStarPositions(std::vector<Star>& stars, double deltaTime) {
	// Update each star's position based on its angular velocity
	for (auto& star : stars) {
		star.angle += star.angularVelocity * deltaTime;

		// Normalize angle to [0, 2*PI]
		while (star.angle > 2.0f * M_PI) star.angle -= 2.0f * M_PI;
		while (star.angle < 0.0f) star.angle += 2.0f * M_PI;

		// Recalculate X and Z based on new angle (Y stays the same)
		float oldY = star.y;
		star.x = star.radius * cos(star.angle);
		star.z = star.radius * sin(star.angle);
		star.y = oldY;
	}
}

void renderStars(const std::vector<Star>& stars) {
	glPointSize(2.0f);
	glBegin(GL_POINTS);

	for (const auto& star : stars) {
		glColor3f(star.r * star.brightness, star.g * star.brightness, star.b * star.brightness);
		glVertex3f(star.x, star.y, star.z);
	}

	glEnd();
}
