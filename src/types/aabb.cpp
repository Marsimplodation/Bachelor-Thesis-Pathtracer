#include "types/aabb.h"
#include "types/vector.h"
#include <cmath>
#include <utility>
#include <algorithm>



bool findIntersection(const Ray &r, const AABB &b) {
    // calculate intersection intervals
    float tx1 = (b.min.x - r.origin.x) * r.inv_dir.x;
    float tx2 = (b.max.x - r.origin.x) * r.inv_dir.x;
    float ty1 = (b.min.y - r.origin.y) * r.inv_dir.y;
    float ty2 = (b.max.y - r.origin.y) * r.inv_dir.y;
    float tz1 = (b.min.z - r.origin.z) * r.inv_dir.z;
    float tz2 = (b.max.z - r.origin.z) * r.inv_dir.z;
    // find min and max intersection t’s
    using std::max;
    using std::min;
    float tres[2] = {
        max(max(max(min(tx1, tx2), min(ty1, ty2)), min(tz1, tz2)), r.tmin),
        min(min(min(max(tx1, tx2), max(ty1, ty2)), max(tz1, tz2)), r.tmax)
    };
    // return result
    return tres[0] <= tres[1];
}

float getIntersectDistance(const Ray &r, const AABB &b) {
    // calculate intersection intervals
    float tx1 = (b.min.x - r.origin.x) * r.inv_dir.x;
    float tx2 = (b.max.x - r.origin.x) * r.inv_dir.x;
    float ty1 = (b.min.y - r.origin.y) * r.inv_dir.y;
    float ty2 = (b.max.y - r.origin.y) * r.inv_dir.y;
    float tz1 = (b.min.z - r.origin.z) * r.inv_dir.z;
    float tz2 = (b.max.z - r.origin.z) * r.inv_dir.z;
    // find min and max intersection t’s
    using std::max;
    using std::min;
    float tres[2] = {
        max(max(max(min(tx1, tx2), min(ty1, ty2)), min(tz1, tz2)), r.tmin),
        min(min(min(max(tx1, tx2), max(ty1, ty2)), max(tz1, tz2)), r.tmax)
    };
    // return result
    return tres[0] <= tres[1] ? tres[0] : INFINITY;
}

//----------- Math stuff -----------//
bool triInAABB(AABB & aabb, Vector3 *verts) {
    // Check if at least one vertex is inside the unit cube
    // Unit cube spans from -0.5 to 0.5 in all axes
    // Helper function to project a point onto an axis
    auto projectPoint = [](const Vector3 &p, const Vector3 &axis) -> float {
        return p.x * axis.x + p.y * axis.y + p.z * axis.z;
    };
    Vector3 min = aabb.min; 
    Vector3 max = aabb.max; 

    // Check overlap on the coordinate axes
    for (int axis = 0; axis < 3; axis++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = min[axis], cubeMaxProj = max[axis];

        for (int i = 0; i < 3; i++) {
            float proj = verts[i][axis];
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    // Edge vectors of the triangle and cube
    Vector3 edges[3] = {{verts[1].x - verts[0].x, verts[1].y - verts[0].y,
                         verts[1].z - verts[0].z},
                        {verts[2].x - verts[1].x, verts[2].y - verts[1].y,
                         verts[2].z - verts[1].z},
                        {verts[0].x - verts[2].x, verts[0].y - verts[2].y,
                         verts[0].z - verts[2].z}};
    Vector3 cubeVerts[8] = {
        {min.x, min.y, min.z},
        {min.x, min.y, max.z},
        {min.x, max.y, min.z},
        {min.x, max.y, max.z},
        {max.x, min.y, min.z},
        {max.x, min.y, max.z},
        {max.x, max.y, min.z},
        {max.x, max.y, max.z}};

    // Create the 9 axis on which the test is performed
    Vector3 testAxes[9];
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        for (int axis = 0; axis < 3; axis++) {
            Vector3 coordAxis;
            switch (axis) {
            case 0:
                coordAxis = {1, 0, 0};
                break;
            case 1:
                coordAxis = {0, 1, 0};
                break;
            case 2:
                coordAxis = {0, 0, 1};
                break;
            }
            testAxes[idx++] = crossProduct(edges[i], coordAxis);
        }
    }

    // perform the SAT test
    for (int i = 0; i < 9; i++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = INFINITY, cubeMaxProj = -INFINITY;

        for (int j = 0; j < 3; j++) {
            float proj = projectPoint(verts[j], testAxes[i]);
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        for (int j = 0; j < 8; j++) {
            float proj = projectPoint(cubeVerts[j], testAxes[i]);
            if (proj < cubeMinProj)
                cubeMinProj = proj;
            if (proj > cubeMaxProj)
                cubeMaxProj = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    return true;
}

bool cuboidInAABB(AABB & aabb, Vector3 *verts) {
    // Check if at least one vertex is inside the unit cube
    // Unit cube spans from -0.5 to 0.5 in all axes
    // Helper function to project a point onto an axis
    auto projectPoint = [](const Vector3 &p, const Vector3 &axis) -> float {
        return p.x * axis.x + p.y * axis.y + p.z * axis.z;
    };
    Vector3 min = aabb.min; 
    Vector3 max = aabb.max; 

    // Check overlap on the coordinate axes
    for (int axis = 0; axis < 3; axis++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = min[axis], cubeMaxProj = max[axis];

        for (int i = 0; i < 8; i++) {
            float proj = verts[i][axis];
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    // Edges of the AABB (only need 3 edges since they are axis-aligned)
    Vector3 edgesA[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    // Edges of the cuboid
    Vector3 edgesB[12] = {
        verts[1] - verts[0],
        verts[2] - verts[0],
        verts[4] - verts[0],
        verts[3] - verts[1],
        verts[5] - verts[1],
        verts[3] - verts[2],
        verts[6] - verts[2],
        verts[6] - verts[4],
        verts[5] - verts[4],
        verts[7] - verts[5],
        verts[7] - verts[3],
        verts[7] - verts[6],
    };

    // Create test axes from cross products of edges
    Vector3 testAxes[36];
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 12; j++) {
            testAxes[idx++] = crossProduct(edgesA[i], edgesB[j]);
        }
    }

    Vector3 cubeVerts[8] = {
        {min.x, min.y, min.z},
        {min.x, min.y, max.z},
        {min.x, max.y, min.z},
        {min.x, max.y, max.z},
        {max.x, min.y, min.z},
        {max.x, min.y, max.z},
        {max.x, max.y, min.z},
        {max.x, max.y, max.z}};
    // perform the SAT test
    for (int i = 0; i < 36; i++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = INFINITY, cubeMaxProj = -INFINITY;

        for (int j = 0; j < 8; j++) {
            float proj = projectPoint(verts[j], testAxes[i]);
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        for (int j = 0; j < 8; j++) {
            float proj = projectPoint(cubeVerts[j], testAxes[i]);
            if (proj < cubeMinProj)
                cubeMinProj = proj;
            if (proj > cubeMaxProj)
                cubeMaxProj = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    return true;
}


bool aabbInAABB(AABB & aabbA, AABB & aabbB) { 
    // Check if at least one vertex is inside the unit cube
    // Unit cube spans from -0.5 to 0.5 in all axes
    // Helper function to project a point onto an axis
    auto projectPoint = [](const Vector3 &p, const Vector3 &axis) -> float {
        return p.x * axis.x + p.y * axis.y + p.z * axis.z;
    };
    Vector3 minA = aabbA.min; 
    Vector3 maxA = aabbA.max; 
    Vector3 minB = aabbB.min; 
    Vector3 maxB = aabbB.max; 

    // Check overlap on the coordinate axes
    for (int axis = 0; axis < 3; axis++) {
        float cubeMinProj = minA[axis], cubeMaxProj = maxB[axis];
        float triMin = minB[axis], triMax = maxB[axis];


        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    // Edges of the AABB (only need 3 edges since they are axis-aligned)
    Vector3 edgesA[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    Vector3 edgesB[3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};


    // Create test axes from cross products of edges
    Vector3 testAxes[9];
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            testAxes[idx++] = crossProduct(edgesA[i], edgesB[j]);
        }
    }

    Vector3 cubeAVerts[8] = {
        {minA.x, minA.y, minA.z},
        {minA.x, minA.y, maxA.z},
        {minA.x, maxA.y, minA.z},
        {minA.x, maxA.y, maxA.z},
        {maxA.x, minA.y, minA.z},
        {maxA.x, minA.y, maxA.z},
        {maxA.x, maxA.y, minA.z},
        {maxA.x, maxA.y, maxA.z}};
    
    Vector3 cubeBVerts[8] = {
        {minB.x, minB.y, minB.z},
        {minB.x, minB.y, maxB.z},
        {minB.x, maxB.y, minB.z},
        {minB.x, maxB.y, maxB.z},
        {maxB.x, minB.y, minB.z},
        {maxB.x, minB.y, maxB.z},
        {maxB.x, maxB.y, minB.z},
        {maxB.x, maxB.y, maxB.z}};
    // perform the SAT test
    for (int i = 0; i < 9; i++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = INFINITY, cubeMaxProj = -INFINITY;

        for (int j = 0; j < 8; j++) {
            float proj = projectPoint(cubeAVerts[j], testAxes[i]);
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        for (int j = 0; j < 8; j++) {
            float proj = projectPoint(cubeBVerts[j], testAxes[i]);
            if (proj < cubeMinProj)
                cubeMinProj = proj;
            if (proj > cubeMaxProj)
                cubeMaxProj = proj;
        }

        if (triMax < cubeMinProj || triMin > cubeMaxProj)
            return false;
    }

    return true;
}

bool pointInAABB(AABB &aabb, Vector3 v){
    Vector3 min = aabb.min; 
    Vector3 max = aabb.max; 
    if ((min.x <= v.x && v.x <= max.x) && 
        (min.y <= v.y && v.y <= max.y) && 
        (min.z <= v.z && v.z <= max.z)) return true; 
    return false;
}
