#include "types/sat.h"
#include "types/vector.h"
#include <cmath>
#include <utility>
#include <algorithm>


bool cuboidInAABB(AABB & aabb, Vector3 *verts, Vector3 *edges, Vector3* normals) {
    // Check if at least one vertex is inside the unit cube
    // Unit cube spans from -0.5 to 0.5 in all axes
    // Helper function to project a point onto an axis
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
    // Create test axes from cross products of edges
    Vector3 testAxes[9+3+3]; //crossproducts + edges + normals
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            testAxes[idx++] = crossProduct(edgesA[i], edges[j]);
        }
    }
    for (int i = 0; i < 3; i++) {
        testAxes[idx++] = edges[i];
    }
    for (int i = 0; i < 3; i++) {
        testAxes[idx++] = normals[i];
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
    for (int i = 0; i < 15; i++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = INFINITY, cubeMaxProj = -INFINITY;

        for (int j = 0; j < 8; j++) {
            float proj = dotProduct(verts[j], testAxes[i]);
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        for (int j = 0; j < 8; j++) {
            float proj = dotProduct(cubeVerts[j], testAxes[i]);
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

bool triInAABB(AABB & aabb, Vector3 *verts) {
    // Check if at least one vertex is inside the unit cube
    // Unit cube spans from -0.5 to 0.5 in all axes
    // Helper function to project a point onto an axis
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
            float proj = dotProduct(verts[j], testAxes[i]);
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        for (int j = 0; j < 8; j++) {
            float proj = dotProduct(cubeVerts[j], testAxes[i]);
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


bool triInChannel(Vector3 *trisVerts, Vector3 *trisNormals, Vector3 * channelVerts, Vector3 *channelEdges, Vector3 *channelNormals) {
    // Check if at least one vertex is inside the unit cube
    // Unit cube spans from -0.5 to 0.5 in all axes
    // Check overlap on the coordinate axes
    for (int axis = 0; axis < 3; axis++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float channelMin = INFINITY, channelMax = -INFINITY;

        for (int i = 0; i < 3; i++) {
            float proj = trisVerts[i][axis];
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }
        
        for (int i = 0; i < 8; i++) {
            float proj = channelVerts[i][axis];
            if (proj < channelMin)
                channelMin = proj;
            if (proj > channelMax)
                channelMax = proj;
        }

        if (triMax < channelMin || triMin > channelMax)
            return false;
    }

    // Edge vectors of the triangle 
    Vector3 edges[3] = {{trisVerts[1].x - trisVerts[0].x, trisVerts[1].y - trisVerts[0].y,
                         trisVerts[1].z - trisVerts[0].z},
                        {trisVerts[2].x - trisVerts[1].x, trisVerts[2].y - trisVerts[1].y,
                         trisVerts[2].z - trisVerts[1].z},
                        {trisVerts[0].x - trisVerts[2].x, trisVerts[0].y - trisVerts[2].y,
                         trisVerts[0].z - trisVerts[2].z}};
    // Create the 9 axis on which the test is performed
    Vector3 testAxes[9 + 3 + 3 + 3 + 3]; //cross products + edges + normals
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            testAxes[idx++] = crossProduct(edges[i], channelEdges[j]);
        }
    }
    for (int i = 0; i < 3; i++) {
        testAxes[idx++] = edges[i];
    }
    for (int i = 0; i < 3; i++) {
        testAxes[idx++] = trisNormals[i];
    }
    for (int i = 0; i < 3; i++) {
        testAxes[idx++] = channelEdges[i];
    }
    for (int i = 0; i < 3; i++) {
        testAxes[idx++] = channelNormals[i]; 
    }

    // perform the SAT test
    for (int i = 0; i < 21; i++) {
        float triMin = INFINITY, triMax = -INFINITY;
        float cubeMinProj = INFINITY, cubeMaxProj = -INFINITY;

        for (int j = 0; j < 3; j++) {
            float proj = dotProduct(trisVerts[j], testAxes[i]);
            if (proj < triMin)
                triMin = proj;
            if (proj > triMax)
                triMax = proj;
        }

        for (int j = 0; j < 8; j++) {
            float proj = dotProduct(channelVerts[j], testAxes[i]);
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

