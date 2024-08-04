#ifndef SCENE_FILE_H
#define SCENE_FILE_H
#include "../types/ray.h"
#include "../primitives/primitive.h"
#include "shader/shader.h"
#include "types/vector.h"
#include <cstdlib>
#include <string>
#include <vector>

//sceneLoading

// Structure definitions
struct SceneFileModel {
    std::string file;
    Vector3 pos;
    Vector3 scale;
};


struct SceneFileMaterial {
    Material mat;
    std::string name;
};

struct SceneFileCamera {
    Vector3 pos;
    Vector3 forward;
    float focus;
    float dof;
};

struct SceneFile {
    std::vector<SceneFileModel> models;
    std::vector<SceneFileMaterial> materials;
    SceneFileCamera cam;
};

SceneFile &getSceneFile();
void loadScene(const std::string& file);
#endif // !SCENE_FILE_H
