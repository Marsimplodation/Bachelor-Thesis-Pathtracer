#include "texture.h"
#include "types/vector.h"
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Vector3 getTextureRGBAt(Texture & texture, int x, int y) {
    auto v = texture.data[y*texture.width + x];
    return {v.x, v.y, v.z};
}



Vector4 getTextureAt(Texture & texture, int x, int y) {
    return texture.data[y*texture.width + x];
}

void setTextureAt(Texture & texture, int x, int y, Vector3 color) {
    texture.data[y*texture.width + x] = {color.x, color.y, color.z, 1.0f};
}

Vector4 getTextureAtUV(Texture & texture, float u, float v) {
    if(u > 1) u = 1;
    if(v > 1) v = 1;
    if(v < 0) v = 0;
    if(u < 0) u = 0;
    if(texture.data.size() != texture.width * texture.height) return {};
    int x = int(roundf(u * (texture.width-1))); 
    int y = int(roundf(v * (texture.height -1))); 
    return texture.data[y*texture.width + x];
}

void loadTexture(Texture & texture, std::string path) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if(!pixels) return;
    // Clear any existing data in the texture
    texture.data.clear();

    // Copy pixel data to the texture
    for (int y = 0; y < texHeight; ++y) {
        for (int x = 0; x < texWidth; ++x) {
            int pixelIndex = (y * texWidth + x) * 4; // RGBA channels
            float r = pixels[pixelIndex] / 255.0f;
            float g = pixels[pixelIndex + 1] / 255.0f;
            float b = pixels[pixelIndex + 2] / 255.0f;
            float a = pixels[pixelIndex + 3] / 255.0f;
            texture.data.push_back({ r, g, b, a});
        }
    }
    // Update texture dimensions
    texture.width = texWidth;
    texture.height = texHeight;

    // Free stb_image allocated memory
    stbi_image_free(pixels);
    printf("loaded texture %s\n", path.c_str());
}

Texture createTexture(int width, int height) {
    Texture t{
        .data = std::vector<Vector4>(width * height),
        .width = width,
        .height = height,
    };
    return t;
}
