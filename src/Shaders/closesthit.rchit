#version 460
#extension GL_EXT_ray_tracing : require
#include "include/types.h"
#include "include/random.h"
#include "include/layout_chit.h"


//------- Helpers -------//

float calculateFresnelTerm(float dot, float n1, float n2) {
    float r0 = ((n1 - n2) / (n1 + n2));
    r0 *= r0;
    return r0 + (1 - r0) * pow(1 - dot, 5);
}


vec4 sampleTexture(uint materialIdx, vec2 uv) {

    uv = uv - floor(uv);
    Material mat = materials[materialIdx];
    
    // Compute the texel coordinate within the texture
    uint offset = uint(mat.textureData[0]);
    uint width = uint(mat.textureData[1]);
    uint height = uint(mat.textureData[2]);
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
    return sampleTexture(materialIdx, uv);

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
    
    rayPayload.hitDistance = gl_HitTEXT;
    traceRayEXT(
        topLevelAS, 
        gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 
        0xFF, 
        0,    // SBT record offset
        1,    // SBT record stride
        0,    // Miss shader index
        origin, 
        0.0, 
        direction, 
        length, 
        0 // location of the payload
    );
    float distance = rayPayload.hitDistance;
    waveFront[rayPayload.idx].terminated = false;
    if(distance != 0.0) {
        return;
    }
    rayPayload.hitDistance = gl_HitTEXT;
    float inv_square_distance = min(1.0, (1.0/(length * length)));
    float cosSurface = dot(normal, direction);
    if(isVolume) cosSurface = 1.0;
    float cosLight = -dot(lightNormal, direction); 
    if(cosSurface < 0.0) return;
    if(cosLight < 0.0) return;
    Material light = materials[v0.materialID];
    vec4 color = getMaterialColor(v0.materialID, uv); 
    vec4 attenuation = vec4(0.5) * cosSurface  * cosLight * inv_square_distance *  surfaceAreaTriangle(index) * camera.emissiveTriangleCount;
    if(isVolume) {
        float density = camera.fogDensity; 
        attenuation.rgb *= exp(-length * density);
    }
    waveFront[rayPayload.idx].light += light.emission * color * attenuation * waveFront[rayPayload.idx].throughPut;
}

//------- Shaders -------//

void mirror(vec3 origin, vec3 direction, vec4 normal, uint materialIdx, vec2 uv, float t) {
    Material mat = materials[materialIdx];
    vec3 dir = normalize(direction - 2*dot(direction, normal.xyz)*normal.xyz);
    vec3 hitPosition = origin + (t-EPS) * direction; // Compute world-space hit position

    waveFront[rayPayload.idx].origin.xyz = hitPosition;
    waveFront[rayPayload.idx].direction.xyz = dir;
    waveFront[rayPayload.idx].throughPut.rgb *= getMaterialColor(materialIdx, uv).rgb;
}

void refraction(vec3 origin, vec3 direction, vec4 normal, uint materialIdx, vec2 uv, float t) {
    Material material = materials[materialIdx];
    vec3 hitPosition = origin + (t) * direction; // Compute world-space hit position
    float n1 = material.ior1;
    float n2 = material.ior2;
    float eta = n1 / n2;
    float cos = dot(direction, normal.xyz);
    if (cos >= EPS) { // in object
        cos *= -1;
        eta = 1.0f / eta;
        normal = normal * -1;
        float tmp = n2;
    }
    vec3 refractDirection;
    float discriminator = 1.0f - (eta * eta) * (1.0f - (cos * cos));

    // internal relection
    float xi = fastRandom(waveFront[rayPayload.idx].randomState);
    float reflectance = calculateFresnelTerm(-cos, n1, n2);
    if (reflectance > 1)
        reflectance = 1;
    if (reflectance < 0)
        reflectance = 0;
    if (discriminator < EPS || xi < reflectance) {
        refractDirection = normalize(direction - 2.0f * dot(direction, normal.xyz) * normal.xyz);
        // if(discriminator > eps)
        // r.throughPut *= reflectance * 1.0f / (reflectance);  // r * 1/r = 1
    } else {
        refractDirection = normalize(eta * (direction - cos * normal.xyz) -
                                      normal.xyz * sqrt(discriminator + EPS));
        // r.throughPut *= 1 - reflectance;
        // r.throughPut *= 1.0f / (1-reflectance);
    }


    waveFront[rayPayload.idx].origin.xyz = hitPosition + refractDirection * EPS;
    waveFront[rayPayload.idx].direction.xyz = refractDirection;
    waveFront[rayPayload.idx].throughPut.rgb *= getMaterialColor(materialIdx, uv).rgb;
}

void lambert(vec3 origin, vec3 direction, vec4 normal, uint materialIdx, vec2 uv, float t) {
    vec3 hitPosition = origin + (t-EPS) * direction; // Compute world-space hit position
    hitPosition += EPS * normal.xyz;
    waveFront[rayPayload.idx].throughPut.rgb *= getMaterialColor(materialIdx, uv).rgb;
    
    if(camera.nee && camera.emissiveTriangleCount != 0) {
        NEE(hitPosition, normal.xyz, false);
    }

    //next bounce
    vec3 arbitrary = abs(normal.z) < 0.99 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(normal.xyz, arbitrary));
    vec3 bitangent = cross(normal.xyz, tangent);
    vec3 randomDir = randomCosineWeightedDirection(waveFront[rayPayload.idx].randomState);
    
    waveFront[rayPayload.idx].direction.xyz = normalize(randomDir.x * tangent +
                                            randomDir.y * bitangent +
                                            randomDir.z * normal.xyz);
    waveFront[rayPayload.idx].origin.xyz = hitPosition;
}


//---- entry point -----//

bool hitVolume(vec3 origin, vec3 direction, vec4 normal) {
    float density = camera.fogDensity; 
    float xi1 = fastRandom(waveFront[rayPayload.idx].randomState);
    float xi2 = fastRandom(waveFront[rayPayload.idx].randomState);
    
    float t = -log(1-xi1)/density;
    if(t > gl_HitTEXT) return false;
    
    vec3 hitPosition = origin + (t-EPS) * direction; // Compute world-space hit position
    //do the volume Shading
    waveFront[rayPayload.idx].throughPut.rgb *= exp(-t * density);
    
    if(camera.emissiveTriangleCount != 0) {
        NEE(hitPosition, normal.xyz, true);
    }
    waveFront[rayPayload.idx].terminated = true;
    return true;
}


void main() {
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    uint primitiveID = gl_PrimitiveID;
    vec3 origin = gl_WorldRayOriginEXT;      // Ray origin in world space
    vec3 direction = gl_WorldRayDirectionEXT; // Ray direction in world space
    float t = gl_HitTEXT;                      // Distance to the hit point
    rayPayload.hitDistance = t;

    // Fetch indices for the triangle
    uint index0 = indices[primitiveID * 3 + 0];
    uint index1 = indices[primitiveID * 3 + 1];
    uint index2 = indices[primitiveID * 3 + 2];

    // Fetch vertices from your vertex buffer (similarly as discussed earlier)
    Vertex v0 = vertices[index0];
    Vertex v1 = vertices[index1];
    Vertex v2 = vertices[index2];
    Material m = materials[v1.materialID];
    vec4 color = m.color; 
    vec4 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
    vec2 uv = v0.uv * barycentrics.x + v1.uv * barycentrics.y + v2.uv * barycentrics.z;

    if(hitVolume(origin, direction, normal)) return;
    float weight = 1.0; 
    if(m.shaderFlag == 0x00) {
        lambert(origin, direction,normal, v0.materialID, uv, t);
        if(camera.nee)weight = 0.5;
    }
    if(m.shaderFlag == 0x01) {
        mirror(origin, direction,normal, v0.materialID, uv, t);
    }
    if(m.shaderFlag == 0x02) {
        refraction(origin, direction,normal, v0.materialID, uv, t);
    }

    waveFront[rayPayload.idx].light.rgb += m.emission * waveFront[rayPayload.idx].throughPut.rgb * weight;
    
}
