#include "SceneFile.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <cstdlib>

namespace {
    SceneFile scene;
    #define MAX_LINE_LENGTH 1024
    #define MAX_STRING_LENGTH 256
}

SceneFile &getSceneFile() {
    return scene;
}


void loadScene(const std::string& file) {
    std::ifstream infile(file);
    char line[MAX_LINE_LENGTH];
    FILE* f = fopen(file.c_str(), "r");
    if (!f) {
        perror("Failed to open file");
        return;
    }
    while (std::fgets(line, MAX_LINE_LENGTH, f)) {
        char keyword[MAX_STRING_LENGTH];
        if (sscanf(line, "%s", keyword) != 1) continue;;

        // Parse Models
        if (strstr(line, "[Models") != NULL) {
            SceneFileModel model;
            bool loaded = false;

            char filePath[MAX_LINE_LENGTH]; 
            
            while (fgets(line, MAX_LINE_LENGTH, f) && !strchr(line, '[')) {
                if (sscanf(line, "file: %s", filePath) == 1) {
                    if(loaded) {
                        scene.models.push_back(model);
                    }
                    loaded = true;
                    model.file = std::string(filePath);
                    // file is set
                } else if (sscanf(line, "pos: %f,%f,%f", &model.pos[0], &model.pos[1], &model.pos[2]) == 3) {
                    // position is set
                } else if (sscanf(line, "scale: %f,%f,%f", &model.scale[0], &model.scale[1], &model.scale[2]) == 3) {
                    // scale is set
                }
            }
            scene.models.push_back(model);
        }
        
        // Parse Models
        if (strstr(line, "[Materials") != NULL) {
            SceneFileMaterial mat;
            bool loaded = false;
 
            char name[MAX_LINE_LENGTH]; 
            
            while (fgets(line, MAX_LINE_LENGTH, f) && !strchr(line, '[')) {
                int idx = 0;
                if (sscanf(line, "name: %s", name) == 1) {
                    if(loaded) {
                        scene.materials.push_back(mat);
                    }
                    loaded = true;
                    mat.name = std::string(name);
                    // file is set
                } else if (sscanf(line, "color: %f,%f,%f",&mat.mat.pbr.albedo[0], &mat.mat.pbr.albedo[1], &mat.mat.pbr.albedo[2]) == 3) {
                    // position is set
                }
                else if (sscanf(line, "weights: %f,%f,%f", &mat.mat.weights.lambert, &mat.mat.weights.reflection, &mat.mat.weights.refraction) == 3) {
                    // position is set
                }
                else if (sscanf(line, "emission: %f", &mat.mat.pbr.emmision) == 1) {
                    // scale is set
                }else if (sscanf(line, "roughness: %f", &mat.mat.pbr.roughness) == 1) {
                    // scale is set
                }
            }
            scene.materials.push_back(mat);
        }

         if (strstr(line, "[Camera") != NULL) {
            while (fgets(line, MAX_LINE_LENGTH, f) && !strchr(line, '[')) {
                int idx = 0;
                if (sscanf(line, "pos: %f,%f,%f", &scene.cam.pos[0], &scene.cam.pos[1], &scene.cam.pos[2]) == 3) {
                    // position is set
                } else if (sscanf(line, "forward: %f,%f,%f", &scene.cam.forward[0], &scene.cam.forward[1], &scene.cam.forward[2]) == 3) {
                    // scale is set
                }
                else if (sscanf(line, "dof: %f", &scene.cam.dof) == 1) {
                    // scale is set
                }
                else if (sscanf(line, "focus: %f", &scene.cam.focus) == 1) {
                    // scale is set
                }
            }
        }
    }
    fclose(f);

    // Example of usage
    std::cout << "Loaded " << scene.models.size() << " models, "
              << scene.materials.size() << " materials, and 1 camera.\n";
}
