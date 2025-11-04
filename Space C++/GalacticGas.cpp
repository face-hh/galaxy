#include "GalacticGas.h"
#include "SolarSystem.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Color4 {
    float r, g, b, a;
};

GasConfig createDefaultGasConfig() {
    GasConfig config;

    config.numMolecularClouds = 2000;
    config.numColdNeutralClouds = 8000;
    config.numWarmNeutralClouds = 12000;
    config.numWarmIonizedClouds = 200;
    config.numHotIonizedClouds = 2000;

    config.numCoronalClouds = 4000;

    config.molecularScaleHeight = 25.0f;
    config.neutralScaleHeight = 100.0f;
    config.ionizedScaleHeight = 400.0f;
    config.coronalScaleHeight = 2000.0f;

    config.enableTurbulence = true;
    config.enableDensityWaves = true;

    return config;
}

// Cubic spline kernel for blurring a gas cloud's spread
float cubicSplineKernel2D(float r, float h) {
    float q = r / h;
    float sigma = 10.0f / (7.0f * M_PI * h * h); // 2D normalization

    if (q >= 0.0f && q < 1.0f) {
        return sigma * (1.0f - 1.5f * q * q + 0.75f * q * q * q);
    } else if (q >= 1.0f && q < 2.0f) {
        float term = 2.0f - q;
        return sigma * 0.25f * term * term * term;
    }
    return 0.0f;
}

Color4 getGasColor(GasType type, float temperature, float density, bool& isDarkLane) {
    Color4 color;
    isDarkLane = false;

    switch (type) {
        case GasType::MOLECULAR:
            // molecular clouds absorb light, so render as dark silhouettes
            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 0.0f;
            color.a = density * 0.5f;
            isDarkLane = true;
            break;

        case GasType::COLD_NEUTRAL:
            color.r = 0.35f;
            color.g = 0.28f;
            color.b = 0.22f;
            color.a = density * 0.03f;
            break;

        case GasType::WARM_NEUTRAL:
            color.r = 0.5f;
            color.g = 0.38f;
            color.b = 0.22f;
            color.a = density * 0.025f;
            break;

        case GasType::WARM_IONIZED:
            // HII (ionized hydrogen) regions are red, but not too bright in visible light
            color.r = 0.9f;
            color.g = 0.25f;
            color.b = 0.35f;
            color.a = density * 0.03f;  // Much more subtle
            break;

        case GasType::HOT_IONIZED:
            // Blue hot gas in supernova remnants
            color.r = 0.45f;
            color.g = 0.6f;
            color.b = 1.0f;
            color.a = density * 0.05f;  // Subtle
            break;

        case GasType::CORONAL:
            // Faint purple coronal gas
            color.r = 0.55f;
            color.g = 0.4f;
            color.b = 0.7f;
            color.a = density * 0.015f;  // Very faint
            break;
    }

    return color;
}

void generateSpiralArmCloud(GasCloud& cloud, std::mt19937& rng, int numArms,
                           double spiralTightness, double armWidth, double diskRadius) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // choose an arm
    int armIndex = rng() % numArms;
    float armAngle = (armIndex * 2.0f * M_PI) / numArms;

    // position along the arm
    float radius = 100.0f + dist(rng) * (diskRadius * 0.8f);
    float spiralAngle = armAngle + spiralTightness * log(radius / 100.0f);

    // add scatter around arm center
    float armOffset = (dist(rng) - 0.5f) * armWidth;
    float perpAngle = spiralAngle + M_PI / 2.0f;

    cloud.x = radius * cos(spiralAngle) + armOffset * cos(perpAngle);
    cloud.z = radius * sin(spiralAngle) + armOffset * sin(perpAngle);

    cloud.orbitalRadius = sqrt(cloud.x * cloud.x + cloud.z * cloud.z);
    cloud.angle = atan2(cloud.z, cloud.x);
}

void generateGalacticGas(std::vector<GasCloud>& gasClouds, const GasConfig& config,
                         unsigned int seed, double diskRadius, double bulgeRadius) {
    std::mt19937 rng(seed + 12345); // offset seed from stars
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::normal_distribution<float> normalDist(0.0f, 1.0f);

    gasClouds.clear();

    int totalClouds = config.numMolecularClouds + config.numColdNeutralClouds +
                      config.numWarmNeutralClouds + config.numWarmIonizedClouds +
                      config.numHotIonizedClouds + config.numCoronalClouds;
    gasClouds.reserve(totalClouds);

    const int numArms = 2;
    const double spiralTightness = 0.3;
    const double armWidth = 60.0;

    // Spawning gas clouds by type
    // The data on these is approximate and based on what I foudn online
    // so treat it, as everything else in this simulation, as a rough approximation :thumbs_up:
    // MOLECULAR CLOUDS
    for (int i = 0; i < config.numMolecularClouds; i++) {
        GasCloud cloud;
        cloud.type = GasType::MOLECULAR;
        cloud.temperature = MOLECULAR_TEMP;

        generateSpiralArmCloud(cloud, rng, numArms, spiralTightness, armWidth, diskRadius);

        cloud.y = normalDist(rng) * config.molecularScaleHeight;

        cloud.mass = 1000.0f + dist(rng) * 100000.0f;
        cloud.smoothingLength = 10.0f + dist(rng) * 25.0f;
        cloud.density = 0.7f + dist(rng) * 0.3f;

        // orbital motion (slower in spiral arms due to density wave)
        cloud.angularVelocity = 0.3f / (sqrt(cloud.orbitalRadius / bulgeRadius) * (cloud.orbitalRadius + 1.0f));

        cloud.turbulencePhase = dist(rng) * 2.0f * M_PI;
        cloud.turbulenceSpeed = 0.1f + dist(rng) * 0.2f;

        bool isDark;
        Color4 col = getGasColor(cloud.type, cloud.temperature, cloud.density, isDark);
        cloud.r = col.r;
        cloud.g = col.g;
        cloud.b = col.b;
        cloud.alpha = col.a;
        cloud.isDarkLane = isDark;

        cloud.elongation = 5.0f + dist(rng) * 5.0f;
        cloud.rotationAngle = dist(rng) * 2.0f * M_PI;

        gasClouds.push_back(cloud);
    }

    // COLD NEUTRAL
    for (int i = 0; i < config.numColdNeutralClouds; i++) {
        GasCloud cloud;
        cloud.type = GasType::COLD_NEUTRAL;
        cloud.temperature = COLD_NEUTRAL_TEMP;

        // Exponential disk distribution
        float diskScale = diskRadius * 0.3f;
        float u = dist(rng);
        float radius = -diskScale * log(1.0f - u * 0.95f + 1e-8f);

        if (radius > diskRadius * 1.2f) {
            radius = diskRadius * 1.2f;
        }

        float theta = dist(rng) * 2.0f * M_PI;
        cloud.x = radius * cos(theta);
        cloud.z = radius * sin(theta);
        cloud.y = normalDist(rng) * config.neutralScaleHeight;

        cloud.orbitalRadius = radius;
        cloud.angle = theta;
        cloud.angularVelocity = 0.4f / (sqrt(radius / bulgeRadius) * (radius + 1.0f));

        cloud.mass = 100.0f + dist(rng) * 1000.0f;
        cloud.smoothingLength = 8.0f + dist(rng) * 20.0f;
        cloud.density = 0.3f + dist(rng) * 0.4f;

        cloud.turbulencePhase = dist(rng) * 2.0f * M_PI;
        cloud.turbulenceSpeed = 0.2f + dist(rng) * 0.3f;

        bool isDark;
        Color4 col = getGasColor(cloud.type, cloud.temperature, cloud.density, isDark);
        cloud.r = col.r;
        cloud.g = col.g;
        cloud.b = col.b;
        cloud.alpha = col.a;
        cloud.isDarkLane = isDark;

        cloud.elongation = 2.0f + dist(rng) * 2.0f;
        cloud.rotationAngle = dist(rng) * 2.0f * M_PI;

        gasClouds.push_back(cloud);
    }

    // WARM NEUTRAL
    for (int i = 0; i < config.numWarmNeutralClouds; i++) {
        GasCloud cloud;
        cloud.type = GasType::WARM_NEUTRAL;
        cloud.temperature = WARM_NEUTRAL_TEMP;

        // widespread in disk
        float diskScale = diskRadius * 0.35f;
        float u = dist(rng);
        float radius = -diskScale * log(1.0f - u * 0.95f + 1e-8f);

        if (radius > diskRadius * 1.5f) {
            radius = diskRadius * 1.5f;
        }

        float theta = dist(rng) * 2.0f * M_PI;
        cloud.x = radius * cos(theta);
        cloud.z = radius * sin(theta);
        cloud.y = normalDist(rng) * config.neutralScaleHeight * 1.5f;

        cloud.orbitalRadius = radius;
        cloud.angle = theta;
        cloud.angularVelocity = 0.4f / (sqrt(radius / bulgeRadius) * (radius + 1.0f));

        cloud.mass = 50.0f + dist(rng) * 500.0f;
        cloud.smoothingLength = 10.0f + dist(rng) * 30.0f;
        cloud.density = 0.2f + dist(rng) * 0.3f;

        cloud.turbulencePhase = dist(rng) * 2.0f * M_PI;
        cloud.turbulenceSpeed = 0.3f + dist(rng) * 0.4f;

        bool isDark;
        Color4 col = getGasColor(cloud.type, cloud.temperature, cloud.density, isDark);
        cloud.r = col.r;
        cloud.g = col.g;
        cloud.b = col.b;
        cloud.alpha = col.a;
        cloud.isDarkLane = isDark;

        cloud.elongation = 2.0f + dist(rng) * 1.0f;
        cloud.rotationAngle = dist(rng) * 2.0f * M_PI;

        gasClouds.push_back(cloud);
    }

    // WARM IONIZED
    for (int i = 0; i < config.numWarmIonizedClouds; i++) {
        GasCloud cloud;
        cloud.type = GasType::WARM_IONIZED;
        cloud.temperature = WARM_IONIZED_TEMP;

        // concentrated tightly in spiral arms
        generateSpiralArmCloud(cloud, rng, numArms, spiralTightness, armWidth * 0.8f, diskRadius);

        cloud.y = normalDist(rng) * config.molecularScaleHeight * 2.0f;

        cloud.mass = 10.0f + dist(rng) * 100.0f;
        cloud.smoothingLength = 6.0f + dist(rng) * 20.0f;
        cloud.density = 0.6f + dist(rng) * 0.4f;

        cloud.angularVelocity = 0.35f / (sqrt(cloud.orbitalRadius / bulgeRadius) * (cloud.orbitalRadius + 1.0f));

        cloud.turbulencePhase = dist(rng) * 2.0f * M_PI;
        cloud.turbulenceSpeed = 0.4f + dist(rng) * 0.5f;

        bool isDark;
        Color4 col = getGasColor(cloud.type, cloud.temperature, cloud.density, isDark);
        cloud.r = col.r;
        cloud.g = col.g;
        cloud.b = col.b;
        cloud.alpha = col.a;
        cloud.isDarkLane = isDark;

        cloud.elongation = 1.2f + dist(rng) * 0.8f;
        cloud.rotationAngle = dist(rng) * 2.0f * M_PI;

        gasClouds.push_back(cloud);
    }

    // HOT IONIZED
    for (int i = 0; i < config.numHotIonizedClouds; i++) {
        GasCloud cloud;
        cloud.type = GasType::HOT_IONIZED;
        cloud.temperature = HOT_IONIZED_TEMP;

        // scattered throughout disk, some concentration in arms
        float diskScale = diskRadius * 0.4f;
        float u = dist(rng);
        float radius = -diskScale * log(1.0f - u * 0.9f + 1e-8f);

        if (radius > diskRadius * 1.3f) {
            radius = diskRadius * 1.3f;
        }

        float theta = dist(rng) * 2.0f * M_PI;
        cloud.x = radius * cos(theta);
        cloud.z = radius * sin(theta);
        cloud.y = normalDist(rng) * config.ionizedScaleHeight;

        cloud.orbitalRadius = radius;
        cloud.angle = theta;
        cloud.angularVelocity = 0.4f / (sqrt(radius / bulgeRadius) * (radius + 1.0f));

        cloud.mass = 1.0f + dist(rng) * 50.0f;
        cloud.smoothingLength = 12.0f + dist(rng) * 40.0f;
        cloud.density = 0.15f + dist(rng) * 0.25f;

        cloud.turbulencePhase = dist(rng) * 2.0f * M_PI;
        cloud.turbulenceSpeed = 0.6f + dist(rng) * 0.8f;

        bool isDark;
        Color4 col = getGasColor(cloud.type, cloud.temperature, cloud.density, isDark);
        cloud.r = col.r;
        cloud.g = col.g;
        cloud.b = col.b;
        cloud.alpha = col.a;
        cloud.isDarkLane = isDark;

        cloud.elongation = 1.0f + dist(rng) * 1.0f;
        cloud.rotationAngle = dist(rng) * 2.0f * M_PI;

        gasClouds.push_back(cloud);
    }

    // CORONAL GAS
    for (int i = 0; i < config.numCoronalClouds; i++) {
        GasCloud cloud;
        cloud.type = GasType::CORONAL;
        cloud.temperature = CORONAL_TEMP;

        // spherical halo distribution
        float theta = dist(rng) * 2.0f * M_PI;
        float phi = acos(2.0f * dist(rng) - 1.0f);
        float radius = pow(dist(rng), 0.5f) * diskRadius * 2.5f;

        cloud.x = radius * sin(phi) * cos(theta);
        cloud.y = radius * sin(phi) * sin(theta);
        cloud.z = radius * cos(phi);

        cloud.orbitalRadius = sqrt(cloud.x * cloud.x + cloud.z * cloud.z);
        cloud.angle = atan2(cloud.z, cloud.x);
        cloud.angularVelocity = 0.1f / (cloud.orbitalRadius + 1.0f);

        // Very diffuse
        cloud.mass = 0.1f + dist(rng) * 10.0f;
        cloud.smoothingLength = 40.0f + dist(rng) * 120.0f;
        cloud.density = 0.05f + dist(rng) * 0.1f;

        cloud.turbulencePhase = dist(rng) * 2.0f * M_PI;
        cloud.turbulenceSpeed = 0.05f + dist(rng) * 0.1f;

        bool isDark;
        Color4 col = getGasColor(cloud.type, cloud.temperature, cloud.density, isDark);
        cloud.r = col.r;
        cloud.g = col.g;
        cloud.b = col.b;
        cloud.alpha = col.a;
        cloud.isDarkLane = isDark;

        cloud.elongation = 1.0f + dist(rng) * 0.5f;
        cloud.rotationAngle = dist(rng) * 2.0f * M_PI;

        gasClouds.push_back(cloud);
    }
}

void updateGalacticGas(std::vector<GasCloud>& gasClouds, double deltaTime) {
    const float TWO_PI = 2.0f * M_PI;

    for (auto& cloud : gasClouds) {
        cloud.angle += cloud.angularVelocity * deltaTime;

        cloud.angle = fmodf(cloud.angle, TWO_PI);
        if (cloud.angle < 0.0f) cloud.angle += TWO_PI;

        float oldY = cloud.y;
        cloud.x = cloud.orbitalRadius * cos(cloud.angle);
        cloud.z = cloud.orbitalRadius * sin(cloud.angle);

        if (cloud.type != GasType::CORONAL) {
            cloud.y = oldY;
        }

        cloud.turbulencePhase += cloud.turbulenceSpeed * deltaTime;
        cloud.turbulencePhase = fmodf(cloud.turbulencePhase, TWO_PI);
        if (cloud.turbulencePhase < 0.0f) cloud.turbulencePhase += TWO_PI;
    }
}

void renderGalacticGas(const std::vector<GasCloud>& gasClouds, const RenderZone& zone) {
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SMOOTH);

    // LOD based on zoom
    int numFilaments = 3;
    int numLayersPerFilament = 4;
    if (zone.zoomLevel < 0.5) {
        numFilaments = 2;
        numLayersPerFilament = 3;
    }

    // culling at high zoom
    int skipFactor = 1;
    if (zone.zoomLevel > 100.0) skipFactor = 4;
    else if (zone.zoomLevel > 50.0) skipFactor = 3;
    else if (zone.zoomLevel > 20.0) skipFactor = 2;

    const int MAX_SIZE_BINS = 40;
    const float SIZE_BIN = 5.0f;

    static std::vector<float> verticesBySize[MAX_SIZE_BINS];
    static std::vector<float> colorsBySize[MAX_SIZE_BINS];

    static std::vector<int> darkLaneIndices;
    static std::vector<int> emissiveIndices;

    darkLaneIndices.clear();
    emissiveIndices.clear();
    darkLaneIndices.reserve(gasClouds.size() / 10);
    emissiveIndices.reserve(gasClouds.size());

    for (size_t i = 0; i < gasClouds.size(); i++) {
        if (gasClouds[i].isDarkLane) {
            darkLaneIndices.push_back(i);
        } else {
            emissiveIndices.push_back(i);
        }
    }

    for (int i = 0; i < MAX_SIZE_BINS; i++) {
        verticesBySize[i].clear();
        colorsBySize[i].clear();
    }

    int estimatedVerticesPerBin = (gasClouds.size() / MAX_SIZE_BINS) * 4 * 3;
    int estimatedColorsPerBin = (gasClouds.size() / MAX_SIZE_BINS) * 4 * 4;
    for (int i = 0; i < MAX_SIZE_BINS; i++) {
        verticesBySize[i].reserve(estimatedVerticesPerBin);
        colorsBySize[i].reserve(estimatedColorsPerBin);
    }

    glBlendFunc(GL_ZERO, GL_SRC_COLOR);

    if (zone.zoomLevel >= 0.1) {
        int numLayers = (zone.zoomLevel < 2.0) ? 3 : 4;

        for (size_t idx = 0; idx < darkLaneIndices.size(); idx++) {
            if (skipFactor > 1 && (idx % skipFactor) != 0) continue;

            const auto& cloud = gasClouds[darkLaneIndices[idx]];

            const float smoothingLength2x = cloud.smoothingLength * 2.0f;
            const float alphaW06 = cloud.alpha * 0.6f;

            for (int i = 0; i < numLayers; i++) {
                float t = i / (float)(numLayers - 1);
                float r = t * smoothingLength2x;
                float w = cubicSplineKernel2D(r, cloud.smoothingLength);

                float extinction = alphaW06 * w;
                float darken = 1.0f - extinction;
                float size = smoothingLength2x * (1.0f + t * 0.3f);

                int sizeBin = (int)(size / SIZE_BIN);
                if (sizeBin < 0) sizeBin = 0;
                if (sizeBin >= MAX_SIZE_BINS) sizeBin = MAX_SIZE_BINS - 1;

                verticesBySize[sizeBin].push_back(cloud.x);
                verticesBySize[sizeBin].push_back(cloud.y);
                verticesBySize[sizeBin].push_back(cloud.z);

                colorsBySize[sizeBin].push_back(darken);
                colorsBySize[sizeBin].push_back(darken);
                colorsBySize[sizeBin].push_back(darken);
                colorsBySize[sizeBin].push_back(1.0f);
            }
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        for (int sizeBin = 0; sizeBin < MAX_SIZE_BINS; sizeBin++) {
            auto& vertices = verticesBySize[sizeBin];
            auto& colors = colorsBySize[sizeBin];

            if (vertices.empty()) continue;

            glPointSize(sizeBin * SIZE_BIN);
            glVertexPointer(3, GL_FLOAT, 0, vertices.data());
            glColorPointer(4, GL_FLOAT, 0, colors.data());
            glDrawArrays(GL_POINTS, 0, vertices.size() / 3);
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (int i = 0; i < MAX_SIZE_BINS; i++) {
        verticesBySize[i].clear();
        colorsBySize[i].clear();
    }

    for (size_t idx = 0; idx < emissiveIndices.size(); idx++) {
        if (skipFactor > 1 && (idx % skipFactor) != 0) continue;

        const auto& cloud = gasClouds[emissiveIndices[idx]];

        if (zone.zoomLevel < 0.001 && cloud.type == GasType::CORONAL) continue;

        const float smoothingLength04 = cloud.smoothingLength * 0.4f;
        const float cosRotation = cos(cloud.rotationAngle);
        const float sinRotation = sin(cloud.rotationAngle);
        const float baseSize = cloud.smoothingLength * 1.2f;
        const float baseSizeElongated = baseSize * (1.0f + cloud.elongation * 0.5f);
        const float alpha08 = cloud.alpha * 0.8f;
        const int numFilamentsHalf = numFilaments / 2;

        for (int f = 0; f < numFilaments; f++) {
            const float filamentOffset = (f - numFilamentsHalf) * smoothingLength04;
            const float offsetX = filamentOffset * cosRotation;
            const float offsetZ = filamentOffset * sinRotation;
            const float filamentFalloff = exp(-f * f * 0.8f);

            for (int i = 0; i < numLayersPerFilament; i++) {
                float t = i / (float)(numLayersPerFilament - 1);

                float gaussian = exp(-t * t * 2.5f);
                float alpha = alpha08 * gaussian * filamentFalloff;

                float size = baseSizeElongated * (1.0f + t * 0.2f);

                int sizeBin = (int)(size / SIZE_BIN);
                if (sizeBin < 0) sizeBin = 0;
                if (sizeBin >= MAX_SIZE_BINS) sizeBin = MAX_SIZE_BINS - 1;

                verticesBySize[sizeBin].push_back(cloud.x + offsetX);
                verticesBySize[sizeBin].push_back(cloud.y);
                verticesBySize[sizeBin].push_back(cloud.z + offsetZ);

                colorsBySize[sizeBin].push_back(cloud.r);
                colorsBySize[sizeBin].push_back(cloud.g);
                colorsBySize[sizeBin].push_back(cloud.b);
                colorsBySize[sizeBin].push_back(alpha);
            }
        }
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    for (int sizeBin = 0; sizeBin < MAX_SIZE_BINS; sizeBin++) {
        auto& vertices = verticesBySize[sizeBin];
        auto& colors = colorsBySize[sizeBin];

        if (vertices.empty()) continue;

        glPointSize(sizeBin * SIZE_BIN);
        glVertexPointer(3, GL_FLOAT, 0, vertices.data());
        glColorPointer(4, GL_FLOAT, 0, colors.data());
        glDrawArrays(GL_POINTS, 0, vertices.size() / 3);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_POINT_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
