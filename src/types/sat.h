#ifndef SAH_H
#define SAH_H

#include "types/ray.h"
#include "types/aabb.h"
#include "types/vector.h"
bool cuboidInAABB(AABB & aabb, Vector3 *verts, Vector3 *edges, Vector3* normals);
bool triInAABB(AABB & aabb, Vector3 *verts);
bool triInChannel(Vector3 *trisVerts, Vector3 * channelVerts, Vector3 *channelEdges, Vector3 *channelNormals);

#endif // !SAH_H
