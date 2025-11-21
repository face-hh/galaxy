#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace FontRenderer {
    void initFont();
 
    void renderText(const std::string& text, float x, float y, float scale, 
    float r, float g, float b, float a = 1.0f);
    
    float getTextWidth(const std::string& text, float scale);
    
    void cleanup();
}
