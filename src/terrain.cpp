#include "../headers/Terrain.h"

char Water::getSymbol() const { return 'W'; }
void Water::getColor(unsigned char& r, unsigned char& g, unsigned char& b) const {
    r = 0; g = 0; b = 255;
}

Grass::Grass(int v) : value(v) {}
char Grass::getSymbol() const { return 'G'; }
void Grass::getColor(unsigned char& r, unsigned char& g, unsigned char& b) const {
    r = 0; g = static_cast<unsigned char>(value * 25); b = 0;
}

char Stone::getSymbol() const { return 'S'; }
void Stone::getColor(unsigned char& r, unsigned char& g, unsigned char& b) const {
    r = 128; g = 128; b = 128;
}

char Beach::getSymbol() const { return 'B'; }
void Beach::getColor(unsigned char& r, unsigned char& g, unsigned char& b) const {
    r = 245; g = 222; b = 179;
}

TerrainTile::TerrainTile(std::unique_ptr<Terrain> t) : terrain(std::move(t)) {}
char TerrainTile::getSymbol() const { return terrain ? terrain->getSymbol() : ' '; }
void TerrainTile::getColor(unsigned char& r, unsigned char& g, unsigned char& b) const {
    if (terrain) terrain->getColor(r, g, b);
    else r = g = b = 0;
}