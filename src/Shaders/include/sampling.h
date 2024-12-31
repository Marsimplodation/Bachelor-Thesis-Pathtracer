
#include "types.h"
vec4 sampleTexture(vec4 textureData, vec2 uv) {
    uv = uv - floor(uv);
    // Compute the texel coordinate within the texture
    uint offset = uint(textureData[0]);
    uint width = uint(textureData[1]);
    uint height = uint(textureData[2]);
    vec2 texCoord = uv * vec2(width, height);
    uint x = uint(texCoord.x);
    uint y = uint(texCoord.y);

    // Compute the 1D index into the pixels array
    uint texelIndex = offset + y * width + x;

    return pixels[texelIndex];
}

vec4 getMaterialColor(uint materialIdx, vec2 uv) {
    Material mat = materials[materialIdx];
    if(mat.textureData[0] == -1) return mat.color;
    return sampleTexture(mat.textureData, uv);

}

uint sampleEmissiveIndex() {
    float xi = fastRandom(waveFront[rayPayload.idx].randomState);
    float size = float(camera.emissiveTriangleCount);
    uint index = uint(floor(xi * size));
    return emissive_trianles[index];
}

float surfaceAreaTriangle(uint index){
    uint index0 = indices[index + 0];
    uint index1 = indices[index + 1];
    uint index2 = indices[index + 2];

    // Fetch vertices from your vertex buffer (similarly as discussed earlier)
    Vertex v0 = vertices[index0];
    Vertex v1 = vertices[index1];
    Vertex v2 = vertices[index2];
    // Calculate edge vectors
    vec3 edge1 = v1.position.xyz - v0.position.xyz;
    vec3 edge2 = v2.position.xyz - v0.position.xyz;

    // Cross product of the two edge vectors
    vec3 crossProduct = cross(edge1, edge2);

    // Surface area of the triangle
    float area = length(crossProduct) * 0.5;

    return area;
}

void NEE(vec3 origin, vec3 normal, bool isVolume) {
    uint index = sampleEmissiveIndex();
    // Fetch indices for the triangle
    uint index0 = indices[index + 0];
    uint index1 = indices[index + 1];
    uint index2 = indices[index + 2];

    // Fetch vertices from your vertex buffer (similarly as discussed earlier)
    Vertex v0 = vertices[index0];
    Vertex v1 = vertices[index1];
    Vertex v2 = vertices[index2];
    float xi1 = fastRandom(waveFront[rayPayload.idx].randomState);
    float xi2 = fastRandom(waveFront[rayPayload.idx].randomState);
    // Ensure xi1 + xi2 <= 1 by sorting and scaling
    if (xi1 + xi2 > 1.0f) {
        xi1 = 1.0f - xi1;
        xi2 = 1.0f - xi2;
    }
    float xi3 = 1 - xi1 - xi2;
    vec3 point = (xi3 * v0.position + xi1 * v1.position + xi2 * v2.position).xyz;
    vec3 lightNormal = (v0.normal * xi3 + v1.normal * xi1 + v2.normal * xi2).xyz;
    lightNormal = normalize(lightNormal);

    vec2 uv = v0.uv * xi3 + v1.uv * xi1 + v2.uv * xi2;
    vec3 direction = point - origin;
    float length = length(direction) - EPS;
    direction = normalize(direction);
    

    float cosSurface = dot(normal, direction);
    float cosLight = abs(dot(lightNormal, direction));
    if(isVolume) cosSurface = 1.0;
    if(cosSurface < 0.0) {
        waveFront[rayPayload.idx].terminated = true;
        return;
    }
    
    float inv_square_distance = min(1.0, (1.0/(length * length)));
    float surfaceDistance = rayPayload.hitDistance;
    waveFront[rayPayload.idx].origin.xyz = origin;
    waveFront[rayPayload.idx].shadowRayIndex = index;
    waveFront[rayPayload.idx].direction.xyz = direction;
    vec4 attenuation = vec4(1.0) * cosSurface  * cosLight * inv_square_distance * surfaceAreaTriangle(index) * camera.emissiveTriangleCount;
    waveFront[rayPayload.idx].throughPut.rgb *= attenuation.rgb; 

    if(isVolume) {
        float density = camera.fogDensity; 
        attenuation.rgb *= exp(-length * density);
    }
}
