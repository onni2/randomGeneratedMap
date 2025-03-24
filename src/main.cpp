#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <fstream>  // Include this header for std::ofstream
#include <GLFW/glfw3.h>
#include "../perlin/PerlinNoise.hpp"  // Adjust path as needed

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

int mapWidth = 300;
int mapHeight = 300;

// Terrain Types
class Terrain {
public:
    virtual ~Terrain() = default;
    virtual void render() const = 0;
    virtual char getSymbol() const = 0;
    virtual void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const = 0;
};

class Water : public Terrain {
public:
    void render() const override {
        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(1.0f, 0.0f);
            glVertex2f(1.0f, 1.0f);
            glVertex2f(0.0f, 1.0f);
        glEnd();
    }
    char getSymbol() const override { return 'W'; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override {
        r = 0; g = 0; b = 255; // Blue for water
    }
};

class Grass : public Terrain {
public:
    int value;
    Grass(int v) : value(v) {}
    void render() const override {
        float greenShade = 0.1f * value;
        glColor3f(0.0f, greenShade, 0.0f);
        glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(1.0f, 0.0f);
            glVertex2f(1.0f, 1.0f);
            glVertex2f(0.0f, 1.0f);
        glEnd();
    }
    char getSymbol() const override { return 'G'; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override {
        r = 0; g = static_cast<unsigned char>(value * 25); b = 0; // Green for grass
    }
};

class Stone : public Terrain {
public:
    void render() const override {
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(1.0f, 0.0f);
            glVertex2f(1.0f, 1.0f);
            glVertex2f(0.0f, 1.0f);
        glEnd();
    }
    char getSymbol() const override { return 'S'; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override {
        r = 128; g = 128; b = 128; // Gray for stone
    }
};

class Beach : public Terrain {
public:
    void render() const override {
        glColor3f(0.96f, 0.87f, 0.7f); // Sandy color
        glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(1.0f, 0.0f);
            glVertex2f(1.0f, 1.0f);
            glVertex2f(0.0f, 1.0f);
        glEnd();
    }
    char getSymbol() const override { return 'B'; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override {
        r = 245; g = 222; b = 179; // Sandy color for beach
    }
};

class TerrainTile {
public:
    std::unique_ptr<Terrain> terrain;
    TerrainTile(std::unique_ptr<Terrain> t) : terrain(std::move(t)) {}
    void render() const {
        if (terrain)
            terrain->render();
        else
            std::cerr << "Error: Null terrain in TerrainTile!" << std::endl;
    }
    char getSymbol() const { return terrain ? terrain->getSymbol() : ' '; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const {
        if (terrain)
            terrain->getColor(r, g, b);
        else
            r = g = b = 0; // Black for null terrain
    }
};

class MapGenerator {
    private:
        int width, height;
        float islandScale;
        std::vector<std::vector<TerrainTile>> grid;
        std::vector<std::vector<float>> heightMap;
        std::vector<std::vector<float>> falloffMap;
        siv::PerlinNoise perlin;
    
        void generateFalloffMap() {
            falloffMap.resize(height, std::vector<float>(width));
            const float centerX = (width - 1) / 2.0f;
            const float centerY = (height - 1) / 2.0f;
    
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    // Use member variable islandScale
                    float dx = (x - centerX) / (centerX * islandScale);
                    float dy = (y - centerY) / (centerY * islandScale);
                    float distance = std::sqrt(dx*dx + dy*dy);
                    
                    distance = std::clamp(distance, 0.0f, 1.0f);
                    distance = distance * distance * (3.0f - 2.0f * distance);
                    falloffMap[y][x] = 1.0f - distance;
                }
            }
        }
    
        void generateHeightMap() {
            const float baseScale = 0.03f;  // More zoomed-out noise
            const int octaves = 9;
            const float persistence = 0.5f;
            const float lacunarity = 2.0f;
    
            // ... rest of height map generation ...
    
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
    
                    // Apply falloff and normalize
                    noiseHeight = (noiseHeight + 1) / 2.0f;  // Remap to [0..1]
                    noiseHeight *= falloffMap[i][j];          // Apply island shape
                    heightMap[i][j] = noiseHeight;
                }
            }
        }
    
        std::unique_ptr<Terrain> generateTerrainFromHeight(float h) {
            if (h < 0.3f) return std::make_unique<Water>();
            if (h < 0.35f) return std::make_unique<Beach>();
            if (h < 0.7f) return std::make_unique<Grass>(static_cast<int>(h * 10));
            return std::make_unique<Stone>();
        }
    
    public:
        MapGenerator(int w, int h, float scale, unsigned int seed = std::time(nullptr))
            : width(w), height(h), islandScale(scale), perlin(seed) {
            generateFalloffMap();
            heightMap.resize(height, std::vector<float>(width, 0.0f));
            generateHeightMap();
    
            grid.resize(height);
            for (int i = 0; i < height; ++i) {
                grid[i].reserve(width);
                for (int j = 0; j < width; ++j) {
                    grid[i].emplace_back(generateTerrainFromHeight(heightMap[i][j]));
                }
            }
        }
    
        void render() const {
            const float tileWidth = 2.0f / width;
            const float tileHeight = 2.0f / height;
        
            // Main terrain rendering
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    glPushMatrix();
                    // Calculate position with proper centering
                    const float xPos = -1.0f + (x * tileWidth) + (tileWidth / 2.0f);
                    const float yPos = 1.0f - (y * tileHeight) - (tileHeight / 2.0f);
                    
                    glTranslatef(xPos, yPos, 0.0f);
                    glScalef(tileWidth, tileHeight, 1.0f);
                    grid[y][x].render();
                    glPopMatrix();
                }
            }
        }

    void printToTerminal() const {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j)
                std::cout << grid[i][j].getSymbol() << " ";
            std::cout << std::endl;
        }
    }

    // Add this method to modify the terrain at a specific coordinate
    void setTerrain(int x, int y, std::unique_ptr<Terrain> terrain) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            grid[y][x].terrain = std::move(terrain);
        } else {
            std::cerr << "Error: Coordinates out of bounds!" << std::endl;
        }
    }

    // Export the map to a PPM file
    void exportToPPM(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for writing!" << std::endl;
            return;
        }

        // Write PPM header
        file << "P3\n"; // PPM magic number (text-based RGB format)
        file << width << " " << height << "\n"; // Image dimensions
        file << "255\n"; // Maximum color value

        // Write pixel data
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
        std::cout << "Map exported to " << filename << std::endl;
    }

    void invertTextures() {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                char current = grid[i][j].getSymbol();
                std::unique_ptr<Terrain> newTerrain;

                switch (current) {
                    case 'W': newTerrain = std::make_unique<Stone>(); break;
                    case 'S': newTerrain = std::make_unique<Water>(); break;
                    case 'G': newTerrain = std::make_unique<Beach>(); break;
                    case 'B': newTerrain = std::make_unique<Grass>(5); break; // Default grass value
                    default: newTerrain = std::make_unique<Water>(); break;
                }
                
                grid[i][j].terrain = std::move(newTerrain);
            }
        }
    }

};

// Global variables for map and editing state
MapGenerator* map = nullptr;
char currentTerrainType = 'W'; // Default to water
int brushRadius = 3; // Radius of the brush in tiles
bool isMousePressed = false; // Track if the mouse button is pressed

// Function to edit a circle of tiles around the cursor
void editCircle(int centerX, int centerY) {
    for (int i = -brushRadius; i <= brushRadius; ++i) {
        for (int j = -brushRadius; j <= brushRadius; ++j) {
            if (i * i + j * j <= brushRadius * brushRadius) { // Check if within circle
                int tileX = centerX + i;
                int tileY = centerY + j;
                switch (currentTerrainType) {
                    case 'W': map->setTerrain(tileX, tileY, std::make_unique<Water>()); break;
                    case 'G': map->setTerrain(tileX, tileY, std::make_unique<Grass>(5)); break;
                    case 'S': map->setTerrain(tileX, tileY, std::make_unique<Stone>()); break;
                    case 'B': map->setTerrain(tileX, tileY, std::make_unique<Beach>()); break;
                    default: std::cerr << "Unknown terrain type!" << std::endl;
                }
            }
        }
    }
}

// Mouse callback function
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isMousePressed = true;
        } else if (action == GLFW_RELEASE) {
            isMousePressed = false;
        }
    }
}

// Cursor position callback function
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (isMousePressed) {
        // Convert screen coordinates to map coordinates
        int tileX = static_cast<int>(xpos / (900.0 / mapWidth));
        int tileY = static_cast<int>(ypos / (900.0 / mapHeight));
        editCircle(tileX, tileY); // Edit a circle of tiles
    }
}

// Key callback function to switch terrain types
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_W: currentTerrainType = 'W'; std::cout << "Selected: Water" << std::endl; break;
            case GLFW_KEY_G: currentTerrainType = 'G'; std::cout << "Selected: Grass" << std::endl; break;
            case GLFW_KEY_S: currentTerrainType = 'S'; std::cout << "Selected: Stone" << std::endl; break;
            case GLFW_KEY_B: currentTerrainType = 'B'; std::cout << "Selected: Beach" << std::endl; break;
            case GLFW_KEY_UP: brushRadius = std::min(brushRadius + 1, 10); std::cout << "Brush Radius: " << brushRadius << std::endl; break;
            case GLFW_KEY_DOWN: brushRadius = std::max(brushRadius - 1, 1); std::cout << "Brush Radius: " << brushRadius << std::endl; break;
            case GLFW_KEY_E: map->exportToPPM("map_export.ppm"); break; // Export the map to a PPM file
        }
    }
}

int main() {
    std::cout << "Choose island size (S/M/L):\n";
    std::cout << "S - Small (0.6)\nM - Medium (1.1)\nL - Large (1.6)\n> ";
    char sizeChoice;
    std::cin >> sizeChoice;

    float islandScale = 1.1f; // Default to Medium
    switch (toupper(sizeChoice)) {
        case 'S': islandScale = 0.6f; break;
        case 'M': islandScale = 1.1f; break;
        case 'L': islandScale = 1.6f; break;
        default: std::cout << "Invalid choice. Using Medium.\n";
    }
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(900, 900, "Island Terrain Map", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to open GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspectRatio = static_cast<float>(fbWidth) / fbHeight;
    if (aspectRatio > 1.0f) {
        glOrtho(-aspectRatio, aspectRatio, -1, 1, -1, 1);
    } else {
        glOrtho(-1, 1, -1 / aspectRatio, 1 / aspectRatio, -1, 1);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    
    // Create generator with custom falloff settings
    map = new MapGenerator(mapWidth, mapHeight, islandScale, std::time(nullptr));

    // Set up input callbacks
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetKeyCallback(window, keyCallback);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        map->render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete map;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}