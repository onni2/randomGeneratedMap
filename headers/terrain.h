#pragma once
#include <memory>

class Terrain {
public:
    virtual ~Terrain() = default;
    virtual char getSymbol() const = 0;
    virtual void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const = 0;
};

class Water : public Terrain {
public:
    char getSymbol() const override;
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override;
};

class Grass : public Terrain {
public:
    int value;
    Grass(int v);
    char getSymbol() const override;
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override;
};

class Stone : public Terrain {
public:
    char getSymbol() const override;
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override;
};

class Beach : public Terrain {
public:
    char getSymbol() const override;
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override;
};

class TerrainTile {
public:
    std::unique_ptr<Terrain> terrain;
    TerrainTile(std::unique_ptr<Terrain> t);
    char getSymbol() const;
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const;
};