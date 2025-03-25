#include "../headers/MapGenerator.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <cstring>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write/stb-master/stb_image_write.h"


bool MapGenerator::getIsDirty() const { return isDirty; }
void MapGenerator::markClean() { isDirty = false; }
void  MapGenerator::generateFalloffMap() {
    falloffMap.resize(height, std::vector<float>(width));
    const float centerX = (width - 1) / 2.0f;
    const float centerY = (height - 1) / 2.0f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = (x - centerX) / (centerX * islandScale);
            float dy = (y - centerY) / (centerY * islandScale);
            float distance = std::sqrt(dx*dx + dy*dy);
            
            distance = std::clamp(distance, 0.0f, 1.0f);
            distance = distance * distance * (3.0f - 2.0f * distance);
            falloffMap[y][x] = 1.0f - distance;
        }
    }
}


void MapGenerator::generateHeightMap() {
    heightMap.resize(height, std::vector<float>(width, 0.0f));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            float amplitude = 1.0f;
            float frequency = 1.0f;
            float noiseHeight = 0.0f;

            for (int o = 0; o < octaves; ++o) {
                float sampleX = j * baseScale * frequency;
                float sampleY = i * baseScale * frequency;
                float perlinValue = perlin.noise2D(sampleX, sampleY);
                
                noiseHeight += perlinValue * amplitude;
                amplitude *= persistence;
                frequency *= lacunarity;
            }

            noiseHeight = (noiseHeight + 1) / 2.0f;
            noiseHeight *= falloffMap[i][j];
            heightMap[i][j] = noiseHeight;
        }
    }
}

std::unique_ptr<Terrain> MapGenerator::generateTerrainFromHeight(float h) {
    if (h < 0.3f) return std::make_unique<Water>();
    if (h < 0.35f) return std::make_unique<Beach>();
    if (h < 0.7f) return std::make_unique<Grass>(static_cast<int>(h * 10));
    return std::make_unique<Stone>();
}

MapGenerator::MapGenerator(int w, int h, float scale, unsigned int seed, 
                         int oct, float pers, float lac, float nScale)
    : width(w), height(h), islandScale(scale), perlin(seed),
      octaves(oct), persistence(pers), lacunarity(lac), baseScale(nScale) {
    generateFalloffMap();
    generateHeightMap();

    grid.resize(height);
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            grid[i].emplace_back(generateTerrainFromHeight(heightMap[i][j]));
        }
    }
}

void MapGenerator::generateTextureData(std::vector<unsigned char>& data) const {
    data.resize(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char r, g, b;
            grid[y][x].getColor(r, g, b);
            int index = (y * width + x) * 3;
            data[index] = r;
            data[index + 1] = g;
            data[index + 2] = b;
        }
    }
}

void MapGenerator::setTerrain(int x, int y, std::unique_ptr<Terrain> terrain) {
    int invertedY = height - 1 - y;
    if (x >= 0 && x < width && invertedY >= 0 && invertedY < height) {
        grid[invertedY][x].terrain = std::move(terrain);
        isDirty = true;
    }
}

void MapGenerator::invertTextures() {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            char current = grid[i][j].getSymbol();
            std::unique_ptr<Terrain> newTerrain;

            switch (current) {
                case 'W': newTerrain = std::make_unique<Stone>(); break;
                case 'S': newTerrain = std::make_unique<Water>(); break;
                case 'G': newTerrain = std::make_unique<Beach>(); break;
                case 'B': newTerrain = std::make_unique<Grass>(5); break;
                default: newTerrain = std::make_unique<Water>(); break;
            }
            
            grid[i][j].terrain = std::move(newTerrain);
        }
    }
    isDirty = true;
}

bool MapGenerator::exportToPNG(const std::string& filename) const {
    std::vector<unsigned char> data;
    generateTextureData(data);
    
    // Flip the image vertically
    std::vector<unsigned char> flippedData(data.size());
    const int rowSize = width * 3;
    for(int y = 0; y < height; y++) {
        const int srcY = height - 1 - y;
        memcpy(&flippedData[y * rowSize], &data[srcY * rowSize], rowSize);
    }
    
    int result = stbi_write_png(filename.c_str(), width, height, 3, flippedData.data(), width * 3);
    return result != 0;
}

bool MapGenerator::exportToPPM(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << "P3\n" << width << " " << height << "\n255\n";
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            unsigned char r, g, b;
            grid[i][j].getColor(r, g, b);
            file << static_cast<int>(r) << " "
                 << static_cast<int>(g) << " "
                 << static_cast<int>(b) << " ";
        }
        file << "\n";
    }
    file.close();
    return true;
}