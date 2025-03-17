#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <GLFW/glfw3.h>

// Base Terrain class
class Terrain {
public:
    virtual ~Terrain() = default;
    virtual void render() const = 0;  // Render the terrain
    virtual char getSymbol() const = 0;  // Get symbol to print to terminal
};

// Derived Terrain Classes
class Water : public Terrain {
public:
    void render() const override {
        glColor3f(0.0f, 0.0f, 1.0f);  // Blue
        glBegin(GL_QUADS);
        glVertex2f(-0.05f, -0.05f); // bottom left
        glVertex2f( 0.05f, -0.05f); // bottom right
        glVertex2f( 0.05f,  0.05f); // top right
        glVertex2f(-0.05f,  0.05f); // top left
        glEnd();
    }

    char getSymbol() const override { return 'W'; }  // 'W' for Water
};

class Grass : public Terrain {
public:
    int value;  // Value from 1 to 9 for gradient

    Grass(int v) : value(v) {}

    void render() const override {
        // Gradient shades for grass based on value
        float greenShade = 0.1f * value;  // Gradually increasing green for values 1-9
        glColor3f(0.0f, greenShade, 0.0f);  // Dynamic green color

        glBegin(GL_QUADS);
        glVertex2f(-0.05f, -0.05f);
        glVertex2f( 0.05f, -0.05f);
        glVertex2f( 0.05f,  0.05f);
        glVertex2f(-0.05f,  0.05f);
        glEnd();
    }

    char getSymbol() const override {
        return 'G';  // 'G' for Grass (though we'll be using numbers for values)
    }
};

class Tree : public Terrain {
public:
    void render() const override {
        glColor3f(0.5f, 0.25f, 0.0f);  // Brown for the tree trunk
        glBegin(GL_QUADS);
        glVertex2f(-0.025f, -0.05f);
        glVertex2f( 0.025f, -0.05f);
        glVertex2f( 0.025f,  0.05f);
        glVertex2f(-0.025f,  0.05f);
        glEnd();
    }

    char getSymbol() const override { return 'T'; }  // 'T' for Tree
};

class Stone : public Terrain {
public:
    void render() const override {
        glColor3f(0.5f, 0.5f, 0.5f);  // Gray for Stone
        glBegin(GL_QUADS);
        glVertex2f(-0.05f, -0.05f);
        glVertex2f( 0.05f, -0.05f);
        glVertex2f( 0.05f,  0.05f);
        glVertex2f(-0.05f,  0.05f);
        glEnd();
    }

    char getSymbol() const override { return 'S'; }  // 'S' for Stone
};

class Mountain : public Terrain {
public:
    void render() const override {
        glColor3f(0.6f, 0.6f, 0.6f);  // Light gray for Mountain

        // Draw the triangular peak of the mountain
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.05f, -0.05f);  // Left bottom
        glVertex2f( 0.05f, -0.05f);  // Right bottom
        glVertex2f( 0.0f,  0.1f);    // Top peak
        glEnd();

        // Draw the base of the mountain (a square under the triangle)
        glColor3f(0.4f, 0.4f, 0.4f);  // Darker gray for the base of the mountain
        glBegin(GL_QUADS);
        glVertex2f(-0.05f, -0.05f);  // bottom left
        glVertex2f( 0.05f, -0.05f);  // bottom right
        glVertex2f( 0.05f, -0.1f);   // bottom right lower
        glVertex2f(-0.05f, -0.1f);   // bottom left lower
        glEnd();
    }

    char getSymbol() const override { return 'M'; }  // 'M' for Mountain
};

// TerrainTile class to represent each tile in the map
class TerrainTile {
public:
    std::unique_ptr<Terrain> terrain;

    TerrainTile(std::unique_ptr<Terrain> t) : terrain(std::move(t)) {}

    void render() const {
        terrain->render();
    }

    char getSymbol() const {
        return terrain->getSymbol();
    }
};

// MapGenerator class to generate and manage the map
class MapGenerator {
private:
    int width, height;
    std::vector<std::vector<TerrainTile>> grid;

    std::unique_ptr<Terrain> generateRandomTerrain(int x, int y) {
        int randomType = rand() % 5;  // Random terrain type (0-4)
        
        // Ensure adjacency rules are followed
        if (randomType == 0) {  // Water
            return std::make_unique<Water>();
        }
        
        if (randomType == 1) {  // Grass (with gradient from 1-9)
            int gradientValue = 1 + rand() % 9;  // Gradient value between 1 and 9
            return std::make_unique<Grass>(gradientValue);
        }
        
        if (randomType == 2) {  // Tree (just static, no gradient)
            return std::make_unique<Tree>();
        }
        
        if (randomType == 3) {  // Stone (from 10-19)
            int stoneValue = 10 + rand() % 10;  // Values from 10-19
            return std::make_unique<Stone>();
        }

        if (randomType == 4) {  // Mountain (same for now)
            return std::make_unique<Mountain>();
        }
        
        return nullptr;
    }

public:
    MapGenerator(int w, int h) : width(w), height(h) {
        grid.resize(height);
        for (int i = 0; i < height; ++i) {
            grid[i].reserve(width);
            for (int j = 0; j < width; ++j) {
                // Generate terrain
                grid[i].emplace_back(generateRandomTerrain(j, i));
            }
        }
    }

    void generate() {
        srand(static_cast<unsigned>(time(0)));
    }

    void render() const {
        float offsetX = -1.0f;
        float offsetY = 1.0f;

        // Adjust the offset for smaller tiles (0.1 instead of 0.5)
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                glPushMatrix();
                glTranslatef(offsetX + j * 0.1f, offsetY - i * 0.1f, 0.0f);
                grid[i][j].render();
                glPopMatrix();
            }
        }
    }

    void printToTerminal() const {
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                std::cout << grid[i][j].getSymbol() << " ";
            }
            std::cout << std::endl;
        }
    }

    // Walk the map and change terrain based on boundary conditions
    void walkMap() {
        // Start the guy at a random empty position (0)
        int startX = rand() % width;
        int startY = rand() % height;

        // Ensure starting position is water (0)
        grid[startY][startX] = TerrainTile(std::make_unique<Water>());

        // Walking logic
        bool walked = true;
        while (walked) {
            walked = false;

            // Check adjacent tiles and move if possible
            std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};  // up, down, left, right
            for (auto dir : directions) {
                int newX = startX + dir.first;
                int newY = startY + dir.second;

                // Check bounds and if the tile is walkable (boundary values)
                if (newX >= 0 && newX < width && newY >= 0 && newY < height) {
                    int currentTileValue = grid[newY][newX].getSymbol();
                    if (currentTileValue == 'W' || currentTileValue == 'G' || currentTileValue == 'S' || currentTileValue == 'M') {
                        // Move and change terrain
                        startX = newX;
                        startY = newY;

                        // Random terrain generation for the moved tile
                        grid[startY][startX] = generateRandomTerrain(startX, startY);

                        walked = true;  // Continue walking
                        break;
                    }
                }
            }
        }
    }
};

// Main Function
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(800, 800, "Terrain Map", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to open GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);

    // Adjust orthographic projection for more squares
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // Create the map generator (100x100)
    MapGenerator map(100, 100);
    map.generate();

    // Let the guy walk across the map
    map.walkMap();  // The walking algorithm

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Render the map
        glClear(GL_COLOR_BUFFER_BIT);

        map.render();

        // Swap buffers
        glfwSwapBuffers(window);

        // Poll events
        glfwPollEvents();
    }

    // Clean up and exit
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
