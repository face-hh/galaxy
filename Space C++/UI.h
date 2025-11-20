#pragma once
#include "Stars.h"
#include "GalacticGas.h"
#include "BlackHole.h"
#include <string>

struct UIState {
    bool isVisible;

    int hoveredButton;
    int activeInput;
 
    int tempStarCount;
    int tempMolecularClouds;
    int tempColdNeutralClouds;
    int tempWarmNeutralClouds;
    int tempWarmIonizedClouds;
    int tempHotIonizedClouds;
    int tempCoronalClouds;
    bool tempEnableTurbulence;
    bool tempEnableDensityWaves;
    bool tempEnableSupermassive;
    float tempBlackHoleMass;
    float tempSolarSystemScale;
    float tempTimeSpeed;
    
    unsigned int currentSeed;
    bool needsRegeneration;
    
    int defaultStarCount;
    int defaultMolecularClouds;
    int defaultColdNeutralClouds;
    int defaultWarmNeutralClouds;
    int defaultWarmIonizedClouds;
    int defaultHotIonizedClouds;
    int defaultCoronalClouds;
    bool defaultEnableTurbulence;
    bool defaultEnableDensityWaves;
    bool defaultEnableSupermassive;
    float defaultBlackHoleMass;
    float defaultSolarSystemScale;
    float defaultTimeSpeed;
};

void initUI();

void toggleUI(UIState& uiState);
void renderUI(UIState& uiState, int screenWidth, int screenHeight);

void updateUIStateFromConfigs(UIState& uiState, const GalaxyConfig& galaxyConfig, 
    const GasConfig& gasConfig, const BlackHoleConfig& blackHoleConfig);

void applyUIChangesToConfigs(const UIState& uiState, GalaxyConfig& galaxyConfig, 
         GasConfig& gasConfig, BlackHoleConfig& blackHoleConfig);

void handleUIInput(struct GLFWwindow* window, UIState& uiState);

extern float g_currentBlackHoleMass;
extern float g_currentSolarSystemScale;
extern float g_currentTimeSpeed;
