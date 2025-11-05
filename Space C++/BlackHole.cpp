#include "BlackHole.h"
#include "SolarSystem.h"
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
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::normal_distribution<float> normalDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> massDist(5.0f, 100.0f);

    blackHoles.clear();

    if (config.enableSupermassive) {
        BlackHole smbh;
        smbh.type = BlackHoleType::SUPERMASSIVE;
        smbh.x = 0.0f;
        smbh.y = 0.0f;
        smbh.z = 0.0f;
        smbh.mass = 4.3e6f;

        float rsKm = calculateSchwarzschildRadius(smbh.mass);
        smbh.eventHorizonRadius = rsKm * KM_TO_SIM_UNITS * VISUAL_SCALE_FACTOR;
        smbh.accretionDiskInnerRadius = smbh.eventHorizonRadius * 3.0f;
        smbh.accretionDiskOuterRadius = smbh.eventHorizonRadius * 20.0f;
        smbh.hasAccretionDisk = true;
        smbh.diskRotationAngle = 0.0f;
        smbh.diskRotationSpeed = 0.5f;
        smbh.radius = 0.0f;
        smbh.angle = 0.0f;
        smbh.angularVelocity = 0.0f;

        blackHoles.push_back(smbh);
    }

    for (int i = 0; i < config.numStellarBlackHoles; i++) {
        BlackHole bh;
        bh.type = BlackHoleType::STELLAR;

        float massRoll = dist(rng);
        bh.mass = 5.0f + pow(massRoll, 2.0f) * 95.0f;

        float rsKm = calculateSchwarzschildRadius(bh.mass);
        bh.eventHorizonRadius = rsKm * KM_TO_SIM_UNITS * VISUAL_SCALE_FACTOR;

        bh.hasAccretionDisk = (dist(rng) < config.stellarBlackHoleFraction);

        if (bh.hasAccretionDisk) {
            bh.accretionDiskInnerRadius = bh.eventHorizonRadius * 3.0f;
            bh.accretionDiskOuterRadius = bh.eventHorizonRadius * 15.0f;
            bh.diskRotationSpeed = 2.0f + dist(rng) * 3.0f;
        } else {
            bh.accretionDiskInnerRadius = 0.0f;
            bh.accretionDiskOuterRadius = 0.0f;
            bh.diskRotationSpeed = 0.0f;
        }

        bh.diskRotationAngle = dist(rng) * 2.0f * M_PI;

        bool inBulge = dist(rng) < 0.2f;

        if (inBulge) {
            float theta = dist(rng) * 2.0f * M_PI;
            float phi = acos(2.0f * dist(rng) - 1.0f);
            float radius = pow(dist(rng), 1.0f / 3.0f) * bulgeRadius;

            bh.x = radius * sin(phi) * cos(theta);
            bh.y = radius * sin(phi) * sin(theta);
            bh.z = radius * cos(phi);

            bh.radius = sqrt(bh.x * bh.x + bh.z * bh.z);
            bh.angle = atan2(bh.z, bh.x);
            bh.angularVelocity = 0.3f / (bulgeRadius + 1.0f);
        } else {
            float diskScale = diskRadius * 0.25f;
            float u = dist(rng);
            float radius = -diskScale * log(1.0f - u + 1e-8f);

            if (radius > diskRadius * 1.5f) {
                radius = diskRadius * 1.5f;
            }

            float theta = dist(rng) * 2.0f * M_PI;

            bh.x = radius * cos(theta);
            bh.z = radius * sin(theta);
            bh.y = normalDist(rng) * 30.0f;

            bh.radius = radius;
            bh.angle = theta;
            bh.angularVelocity = 0.5f / (sqrt(radius / bulgeRadius) * (radius + 1.0f));
        }

        blackHoles.push_back(bh);
    }
}

void updateBlackHoles(std::vector<BlackHole>& blackHoles, double deltaTime) {
    for (auto& bh : blackHoles) {
        if (bh.hasAccretionDisk) {
            bh.diskRotationAngle += bh.diskRotationSpeed * deltaTime;
            while (bh.diskRotationAngle > 2.0f * M_PI) {
                bh.diskRotationAngle -= 2.0f * M_PI;
            }
        }

        if (bh.type == BlackHoleType::STELLAR) {
            bh.angle += bh.angularVelocity * deltaTime;

            while (bh.angle > 2.0f * M_PI) bh.angle -= 2.0f * M_PI;
            while (bh.angle < 0.0f) bh.angle += 2.0f * M_PI;

            float oldY = bh.y;
            bh.x = bh.radius * cos(bh.angle);
            bh.z = bh.radius * sin(bh.angle);
            bh.y = oldY;
        }
    }
}

void renderBlackHoles(const std::vector<BlackHole>& blackHoles, const RenderZone& zone) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (const auto& bh : blackHoles) {
        if (zone.zoomLevel < 0.001 && bh.type == BlackHoleType::STELLAR) {
            continue;
        }

        float visualScale;

        if (bh.type == BlackHoleType::SUPERMASSIVE) {
            visualScale = 1.5f;
        } else {
            visualScale = 80.0f;
        }

        bool highQuality = false;
        bool mediumQuality = false;
        bool lowQuality = true;

        if (bh.type == BlackHoleType::SUPERMASSIVE && zone.zoomLevel > 2000.0) {
            highQuality = true;
            mediumQuality = false;
            lowQuality = false;
        } else if (bh.type == BlackHoleType::SUPERMASSIVE && zone.zoomLevel > 100.0) {
            highQuality = false;
            mediumQuality = true;
            lowQuality = false;
        } else if (zone.zoomLevel > 5000.0) {
            highQuality = true;
            mediumQuality = false;
            lowQuality = false;
        } else if (zone.zoomLevel > 500.0) {
            highQuality = false;
            mediumQuality = true;
            lowQuality = false;
        }

        glPushMatrix();
        glTranslatef(bh.x, bh.y, bh.z);

        if (bh.hasAccretionDisk) {
            int numRings, numSegments, numLayers;
            if (highQuality) {
                numRings = 40;
                numSegments = 128;
                numLayers = 4;
            } else if (mediumQuality) {
                numRings = 20;
                numSegments = 64;
                numLayers = 2;
            } else {
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
                        // intense blue and violet since its extremely hot and near event horizon
                        if (t < 0.12f) {
                            // blue-violet
                            color.r = 0.4f + t * 2.0f;
                            color.g = 0.5f + t * 2.5f;
                            color.b = 1.0f;
                        } else if (t < 0.25f) {
                            // transition to bright blue
                            float s = (t - 0.12f) / 0.13f;
                            color.r = 0.65f + s * 0.35f;
                            color.g = 0.8f + s * 0.2f;
                            color.b = 1.0f;
                        } else if (t < 0.4f) {
                            // blue-white
                            float s = (t - 0.25f) / 0.15f;
                            color.r = 1.0f;
                            color.g = 1.0f;
                            color.b = 1.0f;
                        } else if (t < 0.6f) {
                            // yellow-orange
                            float s = (t - 0.4f) / 0.2f;
                            color.r = 1.0f;
                            color.g = 1.0f - s * 0.2f;
                            color.b = 1.0f - s * 0.6f;
                        } else if (t < 0.8f) {
                            // deep red-orange
                            float s = (t - 0.6f) / 0.2f;
                            color.r = 1.0f;
                            color.g = 0.8f - s * 0.4f;
                            color.b = 0.4f - s * 0.3f;
                        } else {
                            // red-orange with some brown
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
                            } else {
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

            if (bh.type == BlackHoleType::SUPERMASSIVE) {
                if (highQuality) {
                    int numMagneticLines = 8;
                    float magneticRadius = bh.accretionDiskInnerRadius * visualScale * 2.5f;
                    float magneticHeight = bh.accretionDiskOuterRadius * visualScale * 2.0f;

                    glLineWidth(2.0f);
                    for (int line = 0; line < numMagneticLines; line++) {
                        float lineAngle = (line / (float)numMagneticLines) * 2.0f * (float)M_PI + bh.diskRotationAngle * 0.5f;
                        float lineRadius = magneticRadius * (0.6f + 0.4f * (line % 2));

                        glBegin(GL_LINE_STRIP);
                        int arcSegments = 20;
                        for (int seg = 0; seg <= arcSegments; seg++) {
                            float t = seg / (float)arcSegments;
                            float theta = t * (float)M_PI;

                            float x = lineRadius * cos(lineAngle) * sin(theta);
                            float y = magneticHeight * (0.5f - cos(theta) * 0.5f);
                            float z = lineRadius * sin(lineAngle) * sin(theta);

                            float alpha = sin(theta) * 0.7f;
                            glColor4f(1.0f, 1.0f, 1.0f, alpha);
                            glVertex3f(x, y, z);
                        }
                        glEnd();

                        glBegin(GL_LINE_STRIP);
                        for (int seg = 0; seg <= arcSegments; seg++) {
                            float t = seg / (float)arcSegments;
                            float theta = t * (float)M_PI;

                            float x = lineRadius * cos(lineAngle) * sin(theta);
                            float y = -magneticHeight * (0.5f - cos(theta) * 0.5f);
                            float z = lineRadius * sin(lineAngle) * sin(theta);

                            float alpha = sin(theta) * 0.7f;
                            glColor4f(1.0f, 1.0f, 1.0f, alpha);
                            glVertex3f(x, y, z);
                        }
                        glEnd();
                    }
                    glLineWidth(1.0f);
                }

                float jetLength = bh.accretionDiskOuterRadius * visualScale * 2.0f;
                float jetWidth = bh.accretionDiskInnerRadius * visualScale * 0.25f;

                int jetLayers, jetSegments;
                if (highQuality) {
                    jetLayers = 4;
                    jetSegments = 24;
                } else if (mediumQuality) {
                    jetLayers = 3;
                    jetSegments = 16;
                } else {
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
                                   -jetLength * 0.15f, // More collimated base
                                   jetWidth * jetScale * sin(angle));
                    }
                    glEnd();
                }
            }
        }

        float photonSphereRadius = bh.eventHorizonRadius * visualScale * 1.5f;

        int numLensRings, lensSegments;
        if (highQuality) {
            numLensRings = 8;
            lensSegments = 64;
        } else if (mediumQuality) {
            numLensRings = 4;
            lensSegments = 32;
        } else {
            numLensRings = 2;
            lensSegments = 24;
        }

        for (int lensLayer = 0; lensLayer < numLensRings; lensLayer++) {
            float lensRadius = photonSphereRadius * (1.0f + (float)lensLayer * 0.15f);
            float lensAlpha = 0.6f / (1.0f + (float)lensLayer * 0.6f);
            float lensWidth = 3.0f + (float)lensLayer * 0.8f;

            glLineWidth(lensWidth);
            glBegin(GL_LINE_LOOP);

            if (bh.hasAccretionDisk) {
                glColor4f(1.0f, 0.95f, 0.7f, lensAlpha);
            } else {
                glColor4f(1.0f, 1.0f, 1.0f, lensAlpha);
            }

            for (int i = 0; i < lensSegments; i++) {
                float angle = (i / (float)lensSegments) * 2.0f * (float)M_PI;
                glVertex3f(lensRadius * cos(angle), 0.0f, lensRadius * sin(angle));
            }
            glEnd();
        }
        glLineWidth(1.0f);

        float shadowRadius = bh.eventHorizonRadius * visualScale * 2.5f;

        int latSegments, lonSegments;
        if (highQuality) {
            latSegments = 24;
            lonSegments = 32;
        } else if (mediumQuality) {
            latSegments = 16;
            lonSegments = 24;
        } else {
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

        int numGlowLayers;
        if (highQuality) {
            numGlowLayers = (bh.type == BlackHoleType::SUPERMASSIVE) ? 12 : 8;
        } else if (mediumQuality) {
            numGlowLayers = 6;
        } else {
            numGlowLayers = 3;
        }

        for (int i = 0; i < numGlowLayers; i++) {
            float glowSize = shadowRadius * (1.0f + (float)i * 0.3f);
            float glowAlpha = 0.25f / (1.0f + (float)i * 0.5f);

            glPointSize(glowSize);
            glBegin(GL_POINTS);

            if (bh.hasAccretionDisk) {
                glColor4f(1.0f, 0.85f, 0.5f, glowAlpha);
            } else {
                glColor4f(0.6f, 0.5f, 0.8f, glowAlpha);
            } glVertex3f(0.0f, 0.0f, 0.0f);
            glEnd();
        }

        glPopMatrix();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
