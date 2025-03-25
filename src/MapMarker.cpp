#include "../headers/MapMarker.h"
#include <GLFW/glfw3.h>

MapMarker::MapMarker(MapMarkerType t, int posX, int posY, GLuint tex, float sz)
    : type(t), x(posX), y(posY), texture(tex), size(sz) {}

void MapMarker::render() const {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    float aspect = static_cast<float>(mapWidth)/static_cast<float>(mapHeight);
    float xSize = size * aspect;
    float ySize = size;
    
    glBegin(GL_QUADS);
    float xPos = -1.0f + (2.0f * x / mapWidth);
    float yPos = -1.0f + (2.0f * y / mapHeight);
    
    glTexCoord2f(0, 1); glVertex2f(xPos - xSize, yPos - ySize);
    glTexCoord2f(1, 1); glVertex2f(xPos + xSize, yPos - ySize);
    glTexCoord2f(1, 0); glVertex2f(xPos + xSize, yPos + ySize);
    glTexCoord2f(0, 0); glVertex2f(xPos - xSize, yPos + ySize);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}