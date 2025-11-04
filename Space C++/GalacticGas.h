#pragma once
#include <vector>

struct RenderZone;

enum class GasType {
    MOLECULAR,
    COLD_NEUTRAL,
    WARM_NEUTRAL,
    WARM_IONIZED,
    HOT_IONIZED,
    CORONAL
};

struct GasCloud {
    float x, y, z;
    GasType type;
    float mass;              // solar masses
    float smoothingLength;   // SPH-style smoothing kernel radius
    float temperature;       // In Kelvin
    float density;           // Relative density (0.0-1.0)

    float r, g, b;
    float alpha;

    float orbitalRadius;     // distance from galactic center
    float angle;             // current angle in XZ plane
    float angularVelocity;   // rotation speed (radians per second)

    // turbulence = random small-scale motion
    float turbulencePhase;   // random phase for animated turbulence
    float turbulenceSpeed;   // how fast the turbulence evolves

    bool isDarkLane;         // true for molecular clouds that absorb light (render as dark)
    float elongation;        // stretch factor
    float rotationAngle;     // orientation angle for elongated clouds
};

struct GasConfig {
    int numMolecularClouds;
    int numColdNeutralClouds;
    int numWarmNeutralClouds;
    int numWarmIonizedClouds;
    int numHotIonizedClouds;
    int numCoronalClouds;
    
    // distribution
    float molecularScaleHeight;
    float neutralScaleHeight;
    float ionizedScaleHeight;
    float coronalScaleHeight;

    bool enableTurbulence;
    bool enableDensityWaves;
};

GasConfig createDefaultGasConfig();

void generateGalacticGas(std::vector<GasCloud>& gasClouds, const GasConfig& config, unsigned int seed, double diskRadius, double bulgeRadius);
void updateGalacticGas(std::vector<GasCloud>& gasClouds, double deltaTime);
void renderGalacticGas(const std::vector<GasCloud>& gasClouds, const RenderZone& zone);

const float MOLECULAR_TEMP = 20.0f;          // 10-50 K
const float COLD_NEUTRAL_TEMP = 80.0f;       // 50-100 K
const float WARM_NEUTRAL_TEMP = 8000.0f;     // 6000-10000 K
const float WARM_IONIZED_TEMP = 8000.0f;     // ~8000 K
const float HOT_IONIZED_TEMP = 1e6f;         // ~1 million K
const float CORONAL_TEMP = 5e6f;             // 1-10 million K
