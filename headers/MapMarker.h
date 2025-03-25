#pragma once
#include "Enums.h"
#include <GLFW/glfw3.h>

class MapMarker {
public:
    MapMarkerType type;
    int x, y;
    GLuint texture;
    float size;
    
    MapMarker(MapMarkerType t, int posX, int posY, GLuint tex, float sz = 0.05f);
    void render() const;
};