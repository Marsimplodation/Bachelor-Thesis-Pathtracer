import bpy
import time

# Function to create a plane and align it to a specific axis
def create_plane(name, location, rotation, size=(1,1,1)):
    bpy.ops.mesh.primitive_plane_add(size=2, location=location, rotation=rotation)
    plane = bpy.context.object
    plane.name = name
    plane.scale = size
    return plane

# Create two planes aligned along the X-axis
create_plane('Plane_X1', location=(4, 0, 0), rotation=(0, 1.5708, 0), size=(2,1,0))
create_plane('Plane_X2', location=(-4, 0, 0), rotation=(0, 1.5708, 0), size=(2,1,0))

# Create two planes aligned along the Y-axis
create_plane('Plane_Y1', location=(0, 1, 0), rotation=(1.5708, 0, 0), size=(4,2,1))  # 90 degrees in radians
create_plane('Plane_Y2', location=(0, -1, 0), rotation=(1.5708, 0, 0), size=(4,2,1))

# Create two planes aligned along the Z-axis
create_plane('Plane_Z1', location=(0, 0, 2), rotation=(0, 0, 0), size=(4,1,1))  # 90 degrees in radians
create_plane('Plane_Z2', location=(0, 0, -2), rotation=(0, 0, 0), size=(4,1,1))


# Create two planes aligned along the X-axis
pX1 = create_plane('Plane_X1', location=(4, 0, 0), rotation=(0, 1.5708, 0), size=(2,1,0))
pX2 = create_plane('Plane_X2', location=(-4, 0, 0), rotation=(0, 1.5708, 0), size=(2,1,0))

# Create two planes aligned along the Y-axis
pY1 = create_plane('Plane_Y1', location=(0, 1, 0), rotation=(1.5708, 0, 0), size=(4,2,1))  # 90 degrees in radians
pY2 = create_plane('Plane_Y2', location=(0, -1, 0), rotation=(1.5708, 0, 0), size=(4,2,1))

# Create two planes aligned along the Z-axis
pZ1= create_plane('Plane_Z1', location=(0, 0, 2), rotation=(0, 0, 0), size=(4,1,1))  # 90 degrees in radians
pZ2= create_plane('Plane_Z2', location=(0, 0, -2), rotation=(0, 0, 0), size=(4,1,1))
# Update the scene to reflect the changes
bpy.context.view_layer.update()


deltaZ = pZ1.location[2] - pZ2.location[2]
deltaX = pX1.location[0] - pX2.location[0]
deltaY = pY1.location[1] - pY2.location[1]

#x forward
pX1.location[0] += deltaZ*2 + deltaY*2
pX2.location[0] -= deltaZ*2 + deltaY*2
pX1.scale[1] += (deltaX + deltaZ) * 2
pX2.scale[1] += (deltaX + deltaZ) * 2
pX1.scale[0] += (deltaX + deltaY) * 2
pX2.scale[0] += (deltaX + deltaY) * 2

#x right
pY1.location[1] += deltaZ*2 + deltaX*2
pY2.location[1] -= deltaZ*2 + deltaX*2
pY1.scale[0] += (deltaY + deltaZ) * 2
pY2.scale[0] += (deltaY + deltaZ) * 2
pY1.scale[1] += (deltaY + deltaX) * 2
pY2.scale[1] += (deltaY + deltaX) * 2

#x up
pZ1.location[2] += deltaX*2 + deltaY*2
pZ2.location[2] -= deltaX*2 + deltaY*2
pZ1.scale[1] += (deltaZ + deltaX) * 2
pZ2.scale[1] += (deltaZ + deltaX) * 2
pZ1.scale[0] += (deltaZ + deltaY) * 2
pZ2.scale[0] += (deltaZ + deltaY) * 2



# Update the scene to reflect the changes
bpy.context.view_layer.update()
