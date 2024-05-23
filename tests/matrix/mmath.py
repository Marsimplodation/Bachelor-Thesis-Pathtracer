import numpy as np
def pointIsideTriangle(triangle: list[list[float]], point: list[float]) -> bool:
    #calculate barycentric coordinates:
    A = triangle[0]
    B = triangle[1]
    C = triangle[2]
    a = ((B[0] - C[0]) * (point[0] - C[0])) + ((B[1] - C[1]) * (point[1] - C[1]))
    a /= ((B[0] - C[0]) * (A[0] - C[0])) + ((B[1] - C[1]) * (A[1] - C[1]))
    b = ((C[0] - A[0]) * (point[0] - A[0])) + ((C[1] - A[1]) * (point[1] - A[1]))
    b /= ((C[0] - A[0]) * (B[0] - A[0])) + ((C[1] - A[1]) * (B[1] - A[1]))
    y = 1 -a -b
    return a >= 0 and b>=0 and y>=0 and a+b+y==1 


def intersects(points: list[list[float]]) -> bool:
    #for i in range(0,4):
    #    if pointIsideTriangle(points[4:], points[i]):
    #        return True
    ##check if triangle completly in cube/square
    inside = False
    for p in points[4:]:
        if p[0] < 0 or p[1] < 0:
            continue
        if p[0] > 1 or p[1] > 1:
            continue
        inside = True
    return inside
# Define your matrix M and vector p'
# Matrix:
# x1 y1 1  0  0 0
# 0  0  0 x1 y1 1 
# x2 y2 1  0  0 0
# 0  0  0 x2 y2 1 
# x3 y3 1  0  0 0
# 0  0  0 x3 y3 1 
def transform(points, debug = False):
    M = np.array([[points[0][0], points[0][1], 1, 0, 0, 0],
                  [0, 0, 0, points[0][0], points[0][1], 1],
                  [points[1][0], points[1][1], 1, 0, 0, 0],
                  [0, 0, 0, points[1][0], points[1][1], 1],
                  [points[2][0], points[2][1], 1, 0, 0, 0],
                  [0, 0, 0, points[2][0], points[2][1], 1],
    ])
    #p:
    # x1' y1' x2' y2' x3' y3'

    p_prime = np.array([0, 0, 0, 1, 1, 0])

    # Solve for p
    p = np.linalg.solve(M, p_prime)

    M = np.array([
        [p[0], p[1], p[2]],
        [p[3], p[4], p[5]],
        [0, 0, 1]]
    )
    points2=[]
    for i in range(0, len(points)):
        p4 = np.array([points[i][0],points[i][1],1])
        tmp = np.dot(M, p4)
        points2.append([tmp[0],tmp[1]])
    return points2 

