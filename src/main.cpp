#define NOMINMAX  
#include <windows.h>
#include <algorithm>  
#include <cstring>

#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <GLFW/glfw3.h>
#include "../headers/MapGenerator.h"
#include "../headers/TextureManager.h"
#include "../headers/MapMarker.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"


int mapWidth = 300;
int mapHeight = 300;

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
char exportFileName[256] = "";
bool exportSuccess = false;

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
        ImGui::Separator();
        ImGui::Text("Export Settings:");
        ImGui::InputText("File Name", exportFileName, IM_ARRAYSIZE(exportFileName));

        ImGui::BeginDisabled(strlen(exportFileName) == 0);
        bool pngPressed = ImGui::Button("Export to PNG");
        ImGui::SameLine();
        bool ppmPressed = ImGui::Button("Export to PPM");
        ImGui::EndDisabled();

        if (pngPressed || ppmPressed) {
            std::string fullPath = "maps/" + std::string(exportFileName);
            if (pngPressed) fullPath += ".png";
            if (ppmPressed) fullPath += ".ppm";
            
            bool success = false;
            if (pngPressed) success = map->exportToPNG(fullPath);
            if (ppmPressed) success = map->exportToPPM(fullPath);
            
            if (success) {
                exportSuccess = true;
                memset(exportFileName, 0, sizeof(exportFileName));
            }
            else {
                ImGui::TextColored(ImVec4(1,0,0,1), "Export failed! Check console.");
            }
        }

        if (exportSuccess) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0,1,0,1), "Export successful!");
        }

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