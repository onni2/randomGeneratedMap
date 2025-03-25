#include "../headers/TextureManager.h"
#include <vector>
#include <GLFW/glfw3.h>

bool pointInTriangle(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3) {
    float denominator = ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
    if (denominator == 0) return false;
    float a = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / denominator;
    float b = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / denominator;
    float c = 1 - a - b;
    return a >= 0 && b >= 0 && c >= 0;
}

void TextureManager::generateDefaultTextures() {
    generateTexture(CAVE, {128, 128, 128, 255});
    generateTexture(STOWN, {200, 50, 50, 255});
    generateTexture(CAMP, {139, 69, 19, 255});
}

void TextureManager::generateTexture(MapMarkerType type, std::array<GLubyte, 4> color) {
    const int size = 16;
    std::vector<GLubyte> pixels(size * size * 4, 0);

    for(int y = 0; y < size; y++) {
        for(int x = 0; x < size; x++) {
            int idx = (y * size + x) * 4;
            bool inShape = false;
            float fy = 1.0f - (y + 0.5f) / size;
            float fx = (x + 0.5f) / size;

            switch(type) {
                case CAVE: {
                    float dx = fx - 0.5f;
                    float dy = fy - 0.2f;
                    if (dx*dx + dy*dy <= 0.16f && dy > -0.1f) {
                        if (dx*dx + dy*dy > 0.09f) {
                            pixels[idx+0] = 128;
                            pixels[idx+1] = 128;
                            pixels[idx+2] = 128;
                            pixels[idx+3] = 255;
                        } else {
                            pixels[idx+0] = 0;
                            pixels[idx+1] = 0;
                            pixels[idx+2] = 0;
                            pixels[idx+3] = 255;
                        }
                    }
                    break;
                }
                case STOWN: {
                    if (fy > 0.3f && fy < 0.7f && fx > 0.25f && fx < 0.75f) {
                        pixels[idx+0] = 200;
                        pixels[idx+1] = 200;
                        pixels[idx+2] = 200;
                        pixels[idx+3] = 255;
                    }
                    if (pointInTriangle(x, y, size/4, size/2, 3*size/4, size/2, size/2, size/4)) {
                        inShape = true;
                    }
                    break;
                }
                case CAMP: {
                    bool inTriangle = pointInTriangle(x, y,
                        size*0.35f, size*0.9f,
                        size*0.65f, size*0.9f,
                        size*0.5f, size*0.1f);
                    if (inTriangle) inShape = true;
                    break;
                }
            }

            if (inShape) {
                pixels[idx+0] = color[0];
                pixels[idx+1] = color[1];
                pixels[idx+2] = color[2];
                pixels[idx+3] = 255;
            }
        }
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    textures[type] = texID;
}

GLuint TextureManager::getTexture(MapMarkerType type) const {
    auto it = textures.find(type);
    return (it != textures.end()) ? it->second : 0;
}