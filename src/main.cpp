#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <GLFW/glfw3.h>
#include "../perlin/PerlinNoise.hpp"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include <unordered_map>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

int mapWidth = 300;
int mapHeight = 300;

enum MapMarkerType { CAVE, STOWN, CAMP }; 


// Add new MapMarker class
class MapMarker {
    public:
        MapMarkerType type;
        int x, y;
        GLuint texture;
        float size;
    
        MapMarker(MapMarkerType t, int posX, int posY, GLuint tex, float sz = 0.05f)
            : type(t), x(posX), y(posY), texture(tex), size(sz) {}
    
            void render() const {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, texture);
                
                // Adjust size calculation
                float aspect = (float)mapWidth/(float)mapHeight;
                float xSize = size * aspect;
                float ySize = size;
                
                glBegin(GL_QUADS);
                float xPos = -1.0f + (2.0f * x / mapWidth);
                float yPos = -1.0f + (2.0f * y / mapHeight);
                
                // Flip texture coordinates vertically
                glTexCoord2f(0, 1); glVertex2f(xPos - xSize, yPos - ySize);
                glTexCoord2f(1, 1); glVertex2f(xPos + xSize, yPos - ySize);
                glTexCoord2f(1, 0); glVertex2f(xPos + xSize, yPos + ySize);
                glTexCoord2f(0, 0); glVertex2f(xPos - xSize, yPos + ySize);
                glEnd();
                
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
            }
    };
    class TextureManager {
        std::unordered_map<MapMarkerType, GLuint> textures;
        
        bool pointInTriangle(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3) const {
            float denominator = ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
            if (denominator == 0) return false;
            float a = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / denominator;
            float b = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / denominator;
            float c = 1 - a - b;
            return a >= 0 && b >= 0 && c >= 0;
        }
    
    public:
    void generateDefaultTextures() {
        generateTexture(CAVE, {128, 128, 128, 255}); // Grey cave entrance
        generateTexture(STOWN, {200, 50, 50, 255});   // Red roof house
        generateTexture(CAMP, {139, 69, 19, 255});    // Brown tent
    }

    void generateTexture(MapMarkerType type, std::array<GLubyte, 4> color) {
        const int size = 16;
        std::vector<GLubyte> pixels(size * size * 4, 0);
    
        for(int y = 0; y < size; y++) {
            for(int x = 0; x < size; x++) {
                int idx = (y * size + x) * 4;
                bool inShape = false;
    
                // Flip Y coordinate for OpenGL texture space
                float fy = 1.0f - (y + 0.5f) / size;
                float fx = (x + 0.5f) / size;
    
                switch(type) {
                    case CAVE: { // Grey arch with black outline
                        // Outer grey semicircle (larger)
                        float outer_radius = 0.4f;
                        float inner_radius = 0.3f;
                        float dx = fx - 0.5f;
                        float dy = fy - 0.2f;
                        
                        if (dx*dx + dy*dy <= outer_radius*outer_radius && dy > -0.1f) {
                            // Black inner semicircle
                            if (dx*dx + dy*dy > inner_radius*inner_radius) {
                                pixels[idx+0] = 128; // Grey
                                pixels[idx+1] = 128;
                                pixels[idx+2] = 128;
                                pixels[idx+3] = 255;
                            } else { // Black center
                                pixels[idx+0] = 0;
                                pixels[idx+1] = 0;
                                pixels[idx+2] = 0;
                                pixels[idx+3] = 255;
                            }
                        }
                        break;
                    }
                    case STOWN: { // House with red roof
                        // House body (square)
                        if (fy > 0.3f && fy < 0.7f && fx > 0.25f && fx < 0.75f) {
                            pixels[idx+0] = 200; // Gray walls
                            pixels[idx+1] = 200;
                            pixels[idx+2] = 200;
                            pixels[idx+3] = 255;
                        }
                        // Roof triangle
                        if (pointInTriangle(x, y, 
                            size/4, size/2, 
                            3*size/4, size/2, 
                            size/2, size/4)) {
                            inShape = true;
                        }
                        break;
                    }
                    case CAMP: { // Smaller brown tent
                        // Triangle points (centered and smaller)
                        float center_x = 0.5f;
                        float center_y = 0.5f;
                        float tent_width = 0.3f;
                        float tent_height = 0.4f;
                        
                        bool inTriangle = pointInTriangle(
                            x, y,
                            size*(center_x - tent_width/2), size*(center_y + tent_height),  // Left base
                            size*(center_x + tent_width/2), size*(center_y + tent_height),  // Right base
                            size*center_x, size*(center_y - tent_height)                    // Peak
                        );
                        
                        if (inTriangle) {
                            pixels[idx+0] = color[0]; // Brown
                            pixels[idx+1] = color[1];
                            pixels[idx+2] = color[2];
                            pixels[idx+3] = 255;
                        }
                        break;
                    }
                }
    
                if (inShape) {
                    pixels[idx+0] = color[0];
                    pixels[idx+1] = color[1];
                    pixels[idx+2] = color[2];
                    pixels[idx+3] = 255; // Force full opacity
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
    
        GLuint getTexture(MapMarkerType type) const {
            auto it = textures.find(type);
            return (it != textures.end()) ? it->second : 0;
        }
    };

// Terrain Types (removed render methods)
class Terrain {
public:
    virtual ~Terrain() = default;
    virtual char getSymbol() const = 0;
    virtual void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const = 0;
};

class Water : public Terrain {
public:
    char getSymbol() const override { return 'W'; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override {
        r = 0; g = 0; b = 255;
    }
};

class Grass : public Terrain {
public:
    int value;
    Grass(int v) : value(v) {}
    char getSymbol() const override { return 'G'; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override {
        r = 0; g = static_cast<unsigned char>(value * 25); b = 0;
    }
};

class Stone : public Terrain {
public:
    char getSymbol() const override { return 'S'; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override {
        r = 128; g = 128; b = 128;
    }
};

class Beach : public Terrain {
public:
    char getSymbol() const override { return 'B'; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const override {
        r = 245; g = 222; b = 179;
    }
};

class TerrainTile {
public:
    std::unique_ptr<Terrain> terrain;
    TerrainTile(std::unique_ptr<Terrain> t) : terrain(std::move(t)) {}
    char getSymbol() const { return terrain ? terrain->getSymbol() : ' '; }
    void getColor(unsigned char& r, unsigned char& g, unsigned char& b) const {
        if (terrain) terrain->getColor(r, g, b);
        else r = g = b = 0;
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
    bool isDirty = true;

    // Perlin parameters
    int octaves;
    float persistence;
    float lacunarity;
    float baseScale;

    void generateFalloffMap() {
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

    void generateHeightMap() {
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

    std::unique_ptr<Terrain> generateTerrainFromHeight(float h) {
        if (h < 0.3f) return std::make_unique<Water>();
        if (h < 0.35f) return std::make_unique<Beach>();
        if (h < 0.7f) return std::make_unique<Grass>(static_cast<int>(h * 10));
        return std::make_unique<Stone>();
    }

public:
    MapGenerator(int w, int h, float scale, unsigned int seed, 
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

    void generateTextureData(std::vector<unsigned char>& data) const {
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

    bool getIsDirty() const { return isDirty; }
    void markClean() { isDirty = false; }

    void setTerrain(int x, int y, std::unique_ptr<Terrain> terrain) {
        // Flip Y-axis to match OpenGL's coordinate system
        int invertedY = height - 1 - y;
        if (x >= 0 && x < width && invertedY >= 0 && invertedY < height) {
            grid[invertedY][x].terrain = std::move(terrain);
            isDirty = true;
        }
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
                    case 'B': newTerrain = std::make_unique<Grass>(5); break;
                    default: newTerrain = std::make_unique<Water>(); break;
                }
                
                grid[i][j].terrain = std::move(newTerrain);
            }
        }
        isDirty = true;
    }

    void exportToPPM(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) return;

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
    }
};

// Global variables
MapGenerator* map = nullptr;
GLuint textureID;
char currentTerrainType = 'W';
int brushRadius = 3;
bool isMousePressed = false;
TextureManager textureManager;
std::vector<MapMarker> mapMarkers;
MapMarkerType currentMarkerType = CAVE;
bool placementMode = false;
bool removalMode = false;


// Function to edit a circle of tiles around the cursor
void editCircle(int centerX, int centerY) {
    for (int i = -brushRadius; i <= brushRadius; ++i) {
        for (int j = -brushRadius; j <= brushRadius; ++j) {
            if (i * i + j * j <= brushRadius * brushRadius) {
                int tileX = centerX + i;
                int tileY = centerY + j;  // Now using correct Y-axis orientation
                
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

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // Handle single-click actions
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            
            // Convert to map coordinates with corrected Y-axis
            int tileX = static_cast<int>(xpos / (900.0 / mapWidth));
            int tileY = static_cast<int>((900.0 - ypos) / (900.0 / mapHeight));
            tileX = std::clamp(tileX, 0, mapWidth - 1);
            tileY = std::clamp(tileY, 0, mapHeight - 1);

            if (placementMode) {
                // Marker placement
                bool canPlace = true;
                for (const auto& marker : mapMarkers) {
                    if (abs(marker.x - tileX) < 5 && abs(marker.y - tileY) < 5) {
                        canPlace = false;
                        break;
                    }
                }
                if (canPlace) {
                    mapMarkers.emplace_back(
                        currentMarkerType,
                        tileX,
                        tileY,
                        textureManager.getTexture(currentMarkerType),
                        0.04f
                    );
                }
            }
            else if (removalMode) {
                // Marker removal
                auto it = mapMarkers.begin();
                while (it != mapMarkers.end()) {
                    if (abs(it->x - tileX) < 3 && abs(it->y - tileY) < 3) {
                        it = mapMarkers.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
            
            isMousePressed = true;
        }
        else if (action == GLFW_RELEASE) {
            isMousePressed = false;
        }
    }
}


void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    if (ImGui::GetIO().WantCaptureMouse) return;

    if (isMousePressed && !placementMode && !removalMode) {
        // Convert screen coordinates to map coordinates
        int tileX = static_cast<int>(xpos / (900.0 / mapWidth));
        int tileY = static_cast<int>((900.0 - ypos) / (900.0 / mapHeight));  
        
        tileX = std::clamp(tileX, 0, mapWidth - 1);
        tileY = std::clamp(tileY, 0, mapHeight - 1);

        editCircle(tileX, tileY);
    }
}


// Key callback function to switch terrain types
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

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
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(900, 900, "Optimized Terrain Map", nullptr, nullptr);
    glfwMakeContextCurrent(window);  
    textureManager.generateDefaultTextures();  

    // Initialize texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    // Initial parameters
    float islandScale = 1.1f;
    int seed = static_cast<int>(std::time(nullptr));
    int octaves = 9;
    float persistence = 0.5f;
    float lacunarity = 2.0f;
    float noiseScale = 0.03f;

    map = new MapGenerator(mapWidth, mapHeight, islandScale, seed, 
                          octaves, persistence, lacunarity, noiseScale);

    // Initial texture upload
    std::vector<unsigned char> textureData;
    map->generateTextureData(textureData);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mapWidth, mapHeight, 0, 
                GL_RGB, GL_UNSIGNED_BYTE, textureData.data());
    map->markClean();

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetKeyCallback(window, keyCallback);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Update texture if needed
        if (map->getIsDirty()) {
            map->generateTextureData(textureData);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mapWidth, mapHeight,
                           GL_RGB, GL_UNSIGNED_BYTE, textureData.data());
            map->markClean();
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Terrain Controls Window
        ImGui::Begin("Terrain Controls");
        ImGui::SliderFloat("Island Scale", &islandScale, 0.5f, 2.0f);
        ImGui::InputInt("Seed", &seed);
        ImGui::SliderInt("Octaves", &octaves, 1, 16);

        if (ImGui::Button("Regenerate")) {
            delete map;
            map = new MapGenerator(mapWidth, mapHeight, islandScale, seed, 
                                 octaves, persistence, lacunarity, noiseScale);
        }
        ImGui::SameLine();
        if (ImGui::Button("Export to PPM")) map->exportToPPM("map_export.ppm");
        if (ImGui::Button("Invert Textures")) map->invertTextures();

        ImGui::SliderInt("Brush Radius", &brushRadius, 1, 10);
        ImGui::Text("Terrain Type:");
        ImGui::RadioButton("Water", (int*)&currentTerrainType, 'W'); ImGui::SameLine();
        ImGui::RadioButton("Grass", (int*)&currentTerrainType, 'G'); ImGui::SameLine();
        ImGui::RadioButton("Stone", (int*)&currentTerrainType, 'S'); ImGui::SameLine();
        ImGui::RadioButton("Beach", (int*)&currentTerrainType, 'B');
        ImGui::End();

        // Marker Controls Window
        ImGui::Begin("Marker Controls");
        ImGui::Checkbox("Placement Mode", &placementMode);
        if (placementMode) {
            removalMode = false;
            ImGui::RadioButton("Cave", (int*)&currentMarkerType, CAVE);
            ImGui::RadioButton("Village", (int*)&currentMarkerType, STOWN);
            ImGui::RadioButton("Camp", (int*)&currentMarkerType, CAMP);
        }
        ImGui::Checkbox("Removal Mode", &removalMode);
        if (removalMode) placementMode = false;
        ImGui::End();

        // Rendering
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw terrain texture
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBegin(GL_QUADS);
            // Corrected texture coordinates to match OpenGL's coordinate system
            glTexCoord2f(0, 0); glVertex2f(-1, 1);   // Top-left
            glTexCoord2f(1, 0); glVertex2f(1, 1);    // Top-right
            glTexCoord2f(1, 1); glVertex2f(1, -1);   // Bottom-right
            glTexCoord2f(0, 1); glVertex2f(-1, -1);  // Bottom-left
        glEnd();
        glDisable(GL_TEXTURE_2D);

        // Draw markers on top of terrain
        for (const auto& marker : mapMarkers) {
            marker.render();
        }

        // Render ImGUI on top of everything
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    delete map;
    glDeleteTextures(1, &textureID);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}