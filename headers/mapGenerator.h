#pragma once
#include "Terrain.h"
#include "../perlin/PerlinNoise.hpp"
#include <vector>
#include <memory>
#include <string>

class MapGenerator {
    int width, height;
    float islandScale;
    std::vector<std::vector<TerrainTile>> grid;
    std::vector<std::vector<float>> heightMap;
    std::vector<std::vector<float>> falloffMap;
    siv::PerlinNoise perlin;
    bool isDirty = true;
    int octaves;
    float persistence;
    float lacunarity;
    float baseScale;

    void generateFalloffMap();
    void generateHeightMap();
    std::unique_ptr<Terrain> generateTerrainFromHeight(float h);

public:
    MapGenerator(int w, int h, float scale, unsigned int seed, 
               int oct, float pers, float lac, float nScale);
    void generateTextureData(std::vector<unsigned char>& data) const;
    bool getIsDirty() const;
    void markClean();
    void setTerrain(int x, int y, std::unique_ptr<Terrain> terrain);
    void invertTextures();
    bool exportToPNG(const std::string& filename) const;
    bool exportToPPM(const std::string& filename) const;
};