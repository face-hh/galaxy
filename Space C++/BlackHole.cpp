#include "BlackHole.h"
#include "SolarSystem.h"
#include "UI.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Color3 {
	float r, g, b;
};

const float KM_TO_SIM_UNITS = 1.0e-8f;
const float VISUAL_SCALE_FACTOR = 3.0f; // its much smaller in reality but we scale it up for visibility

void generateBlackHoles(std::vector<BlackHole>& blackHoles, const BlackHoleConfig& config,
	unsigned int seed, double diskRadius, double bulgeRadius) {
	blackHoles.clear();

	if (config.enableSupermassive) {
		BlackHole smbh;
		smbh.x = 0.0f;
		smbh.y = 0.0f;
		smbh.z = 0.0f;
		smbh.mass = g_currentBlackHoleMass * 1e6f;

		float rsKm = calculateSchwarzschildRadius(smbh.mass);
		smbh.eventHorizonRadius = rsKm * KM_TO_SIM_UNITS * VISUAL_SCALE_FACTOR;
		smbh.accretionDiskInnerRadius = smbh.eventHorizonRadius * 3.0f;
		smbh.accretionDiskOuterRadius = smbh.eventHorizonRadius * 20.0f;
		smbh.diskRotationAngle = 0.0f;
		smbh.diskRotationSpeed = 0.5f;

		blackHoles.push_back(smbh);
	}
}

void updateBlackHoles(std::vector<BlackHole>& blackHoles, double deltaTime) {
	for (auto& bh : blackHoles) {
		bh.diskRotationAngle += bh.diskRotationSpeed * deltaTime;
		while (bh.diskRotationAngle > 2.0f * M_PI) {
			bh.diskRotationAngle -= 2.0f * M_PI;
		}
	}
}

void renderBlackHoles(const std::vector<BlackHole>& blackHoles, const RenderZone& zone) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (const auto& bh : blackHoles) {
		float visualScale = 1.5f;

		bool highQuality = false;
		bool mediumQuality = false;
		bool lowQuality = true;

		if (zone.zoomLevel > 2000.0) {
			highQuality = true;
			mediumQuality = false;
			lowQuality = false;
		}
		else if (zone.zoomLevel > 100.0) {
			highQuality = false;
			mediumQuality = true;
			lowQuality = false;
		}

		glPushMatrix();
		glTranslatef(bh.x, bh.y, bh.z);

		// accretion disk
		int numRings, numSegments, numLayers;
		if (highQuality) {
			numRings = 40;
			numSegments = 128;
			numLayers = 4;
		}
		else if (mediumQuality) {
			numRings = 20;
			numSegments = 64;
			numLayers = 2;
		}
		else {
			numRings = 10;
			numSegments = 32;
			numLayers = 1;
		}

		for (int layer = 0; layer < numLayers; layer++) {
			float layerAlpha = (layer == 0) ? 0.9f : (layer == 1) ? 0.5f : (layer == 2) ? 0.25f : 0.12f;
			float layerScale = 1.0f + (float)layer * 0.2f;

			for (int side = 0; side < 2; side++) {
				float sideAlpha = (side == 0) ? 1.0f : 0.6f;

				for (int ring = 0; ring < numRings - 1; ring++) {
					float t1 = ring / (float)numRings;
					float t2 = (ring + 1) / (float)numRings;

					float innerRadius1 = (bh.accretionDiskInnerRadius +
						t1 * (bh.accretionDiskOuterRadius - bh.accretionDiskInnerRadius))
						* visualScale * layerScale;
					float innerRadius2 = (bh.accretionDiskInnerRadius +
						t2 * (bh.accretionDiskOuterRadius - bh.accretionDiskInnerRadius))
						* visualScale * layerScale;

					auto getColor = [](float t) -> Color3 {
						Color3 color;
						if (t < 0.12f) {
							color.r = 0.4f + t * 2.0f;
							color.g = 0.5f + t * 2.5f;
							color.b = 1.0f;
						}
						else if (t < 0.25f) {
							float s = (t - 0.12f) / 0.13f;
							color.r = 0.65f + s * 0.35f;
							color.g = 0.8f + s * 0.2f;
							color.b = 1.0f;
						}
						else if (t < 0.4f) {
							float s = (t - 0.25f) / 0.15f;
							color.r = 1.0f;
							color.g = 1.0f;
							color.b = 1.0f;
						}
						else if (t < 0.6f) {
							float s = (t - 0.4f) / 0.2f;
							color.r = 1.0f;
							color.g = 1.0f - s * 0.2f;
							color.b = 1.0f - s * 0.6f;
						}
						else if (t < 0.8f) {
							float s = (t - 0.6f) / 0.2f;
							color.r = 1.0f;
							color.g = 0.8f - s * 0.4f;
							color.b = 0.4f - s * 0.3f;
						}
						else {
							float s = (t - 0.8f) / 0.2f;
							color.r = 1.0f - s * 0.2f;
							color.g = 0.4f - s * 0.25f;
							color.b = 0.1f;
						}
						return color;
						};

					Color3 color1 = getColor(t1);
					Color3 color2 = getColor(t2);

					float brightness1 = (1.0f - t1 * 0.65f) * layerAlpha * sideAlpha;
					float brightness2 = (1.0f - t2 * 0.65f) * layerAlpha * sideAlpha;

					glBegin(GL_QUAD_STRIP);
					for (int i = 0; i <= numSegments; i++) {
						float angle = (i / (float)numSegments) * 2.0f * (float)M_PI + bh.diskRotationAngle;
						float cosA = cos(angle);
						float sinA = sin(angle);

						float yOffset1, yOffset2;
						if (side == 0) {
							yOffset1 = -t1 * t1 * innerRadius1 * 0.05f;
							yOffset2 = -t2 * t2 * innerRadius2 * 0.05f;
						}
						else {
							float warp1 = (1.0f - t1) * (1.0f - t1);
							float warp2 = (1.0f - t2) * (1.0f - t2);
							float puff1 = (t1 > 0.6f) ? pow((t1 - 0.6f) / 0.4f, 1.5f) * 2.0f : 0.0f;
							float puff2 = (t2 > 0.6f) ? pow((t2 - 0.6f) / 0.4f, 1.5f) * 2.0f : 0.0f;
							yOffset1 = warp1 * innerRadius1 * 0.3f + puff1 * innerRadius1 * 0.15f;
							yOffset2 = warp2 * innerRadius2 * 0.3f + puff2 * innerRadius2 * 0.15f;
						}

						float dopplerFactor = 1.0f + 0.5f * cosA;
						if (side == 1) dopplerFactor = 1.0f + 0.2f * cosA;

						glColor4f(color1.r * brightness1 * dopplerFactor,
							color1.g * brightness1 * dopplerFactor,
							color1.b * brightness1 * dopplerFactor,
							brightness1);
						glVertex3f(innerRadius1 * cosA, yOffset1, innerRadius1 * sinA);

						glColor4f(color2.r * brightness2 * dopplerFactor,
							color2.g * brightness2 * dopplerFactor,
							color2.b * brightness2 * dopplerFactor,
							brightness2);
						glVertex3f(innerRadius2 * cosA, yOffset2, innerRadius2 * sinA);
					}
					glEnd();
				}
			}
		}

		// relativistic jets
		float jetLength = bh.accretionDiskOuterRadius * visualScale * 2.0f;
		float jetWidth = bh.accretionDiskInnerRadius * visualScale * 0.25f;

		int jetLayers, jetSegments;
		if (highQuality) {
			jetLayers = 4;
			jetSegments = 24;
		}
		else if (mediumQuality) {
			jetLayers = 3;
			jetSegments = 16;
		}
		else {
			jetLayers = 2;
			jetSegments = 12;
		}

		for (int jetLayer = 0; jetLayer < jetLayers; jetLayer++) {
			float jetAlpha = (jetLayer == 0) ? 0.9f : (jetLayer == 1) ? 0.6f : (jetLayer == 2) ? 0.3f : 0.15f;
			float jetScale = 1.0f + (float)jetLayer * 0.2f;

			float greenR = (jetLayer == 0) ? 0.2f : 0.3f;
			float greenG = (jetLayer == 0) ? 1.0f : 0.9f;
			float greenB = (jetLayer == 0) ? 0.4f : 0.5f;

			glBegin(GL_TRIANGLE_FAN);
			glColor4f(greenR, greenG, greenB, jetAlpha);
			glVertex3f(0.0f, jetLength * jetScale, 0.0f);
			glColor4f(greenR * 0.5f, greenG * 0.5f, greenB * 0.5f, 0.0f);
			for (int i = 0; i <= jetSegments; i++) {
				float angle = (i / (float)jetSegments) * 2.0f * (float)M_PI;
				glVertex3f(jetWidth * jetScale * cos(angle),
					jetLength * 0.15f,
					jetWidth * jetScale * sin(angle));
			}
			glEnd();

			glBegin(GL_TRIANGLE_FAN);
			glColor4f(greenR, greenG, greenB, jetAlpha);
			glVertex3f(0.0f, -jetLength * jetScale, 0.0f);
			glColor4f(greenR * 0.5f, greenG * 0.5f, greenB * 0.5f, 0.0f);
			for (int i = 0; i <= jetSegments; i++) {
				float angle = (i / (float)jetSegments) * 2.0f * (float)M_PI;
				glVertex3f(jetWidth * jetScale * cos(angle),
					-jetLength * 0.15f,
					jetWidth * jetScale * sin(angle));
			}
			glEnd();
		}

		// photon sphere / gravitational lensing
		float photonSphereRadius = bh.eventHorizonRadius * visualScale * 1.5f;

		int numLensRings, lensSegments;
		if (highQuality) {
			numLensRings = 8;
			lensSegments = 64;
		}
		else if (mediumQuality) {
			numLensRings = 4;
			lensSegments = 32;
		}
		else {
			numLensRings = 2;
			lensSegments = 24;
		}

		for (int lensLayer = 0; lensLayer < numLensRings; lensLayer++) {
			float lensRadius = photonSphereRadius * (1.0f + (float)lensLayer * 0.15f);
			float lensAlpha = 0.6f / (1.0f + (float)lensLayer * 0.6f);
			float lensWidth = 3.0f + (float)lensLayer * 0.8f;

			glLineWidth(lensWidth);
			glBegin(GL_LINE_LOOP);
			glColor4f(1.0f, 0.95f, 0.7f, lensAlpha);
			for (int i = 0; i < lensSegments; i++) {
				float angle = (i / (float)lensSegments) * 2.0f * (float)M_PI;
				glVertex3f(lensRadius * cos(angle), 0.0f, lensRadius * sin(angle));
			}
			glEnd();
		}
		glLineWidth(1.0f); 

		// event horizon shadow
		float shadowRadius = bh.eventHorizonRadius * visualScale * 2.5f;

		int latSegments, lonSegments;
		if (highQuality) {
			latSegments = 24;
			lonSegments = 32;
		}
		else if (mediumQuality) {
			latSegments = 16;
			lonSegments = 24;
		}
		else {
			latSegments = 12;
			lonSegments = 16;
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

		for (int lat = 0; lat < latSegments; lat++) {
			float theta1 = lat * M_PI / latSegments;
			float theta2 = (lat + 1) * M_PI / latSegments;

			glBegin(GL_QUAD_STRIP);
			for (int lon = 0; lon <= lonSegments; lon++) {
				float phi = lon * 2.0f * M_PI / lonSegments;

				float x1 = shadowRadius * sin(theta1) * cos(phi);
				float y1 = shadowRadius * cos(theta1);
				float z1 = shadowRadius * sin(theta1) * sin(phi);

				float x2 = shadowRadius * sin(theta2) * cos(phi);
				float y2 = shadowRadius * cos(theta2);
				float z2 = shadowRadius * sin(theta2) * sin(phi);

				glVertex3f(x1, y1, z1);
				glVertex3f(x2, y2, z2);
			}
			glEnd();
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		// glow around event horizon
		int numGlowLayers;
		if (highQuality) {
			numGlowLayers = 12;
		}
		else if (mediumQuality) {
			numGlowLayers = 6;
		}
		else {
			numGlowLayers = 3;
		}

		for (int i = 0; i < numGlowLayers; i++) {
			float glowSize = shadowRadius * (1.0f + (float)i * 0.3f);
			float glowAlpha = 0.25f / (1.0f + (float)i * 0.5f);

			glPointSize(glowSize);
			glBegin(GL_POINTS);
			glColor4f(1.0f, 0.85f, 0.5f, glowAlpha);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glEnd();
		}

		glPopMatrix();
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
