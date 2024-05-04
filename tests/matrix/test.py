import numpy as np
from mmath import *
import matplotlib.pyplot as plt
import pygame
import sys
# parallelogram:
# 0,0 0,2 5,1 5,3
#               *
# * 
#               *
# *
#first 4 give the cuboid the rest the shape
points = np.array([
[0,0],
[0,200],
[500,100],
[500,300],

[100,0],
[100,100],
[0,100],
])


# Initialize pygame
pygame.init()

# Screen dimensions
WIDTH, HEIGHT = 800, 600
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
#tmp = transform(points)
#print(intersects(tmp))

# Function to draw points on the screen
def draw_points(screen, points, inside):
    screen.fill(WHITE)
    if inside:
        screen.fill(GREEN)
    pygame.draw.line(screen, RED, points[0], points[1])
    pygame.draw.line(screen, RED, points[0], points[2])
    pygame.draw.line(screen, RED, points[2], points[3])
    pygame.draw.line(screen, RED, points[1], points[3])

    pygame.draw.line(screen, BLACK, points[4], points[5])
    pygame.draw.line(screen, BLACK, points[4], points[6])
    pygame.draw.line(screen, BLACK, points[5], points[6])
    for point in points:
        pygame.draw.circle(screen, RED, point, 5)

# Function to check if a point is clicked
def is_point_clicked(point, pos):
    return (point[0]-pos[0])**2 + (point[1]-pos[1])**2 <= 25**2

# Main function
def main():
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Draggable Points")
    clock = pygame.time.Clock()

    # Initial points
    dragging = False
    dragging_point = None

    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.MOUSEBUTTONDOWN:
                # Check if any point is clicked
                for i, point in enumerate(points):
                    if is_point_clicked(point, event.pos):
                        dragging = True
                        dragging_point = i
                        break
            elif event.type == pygame.MOUSEBUTTONUP:
                dragging = False
                dragging_point = None
            elif event.type == pygame.MOUSEMOTION:
                # Move the point being dragged
                if dragging:
                    points[dragging_point] = list(event.pos)

        # Draw points on the screen
        tmp = transform(points)
        inside = intersects(tmp)
        draw_points(screen, points, inside)
        pygame.display.flip()
        clock.tick(60)

if __name__ == "__main__":
    main()
