#ifndef TEXTURE_H
#define TEXTURE_H

#include "types/vector.h"
#include <string>
#include <vector>
struct Texture {
    std::vector<Vector4> data;
    int width;
    int height;
};

Vector3 getTextureRGBAt(Texture & texture, int x, int y);
Vector4 getTextureAt(Texture & texture, int x, int y);
Vector4 getTextureAtUV(Texture & texture, float u, float v);
void setTextureAt(Texture & texture, int x, int y, Vector3 color);
void loadTexture(Texture & texture, std::string path);
Texture createTexture(int width, int height);

#endif // !TEXTURE_H
