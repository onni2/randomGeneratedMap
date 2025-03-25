#pragma once
#include "Enums.h"
#include <GLFW/glfw3.h>
#include <array>
#include <unordered_map>

class TextureManager {
    std::unordered_map<MapMarkerType, GLuint> textures;
    
public:
    void generateDefaultTextures();
    void generateTexture(MapMarkerType type, std::array<GLubyte, 4> color);
    GLuint getTexture(MapMarkerType type) const;
};