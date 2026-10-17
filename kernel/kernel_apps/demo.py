import pygame
import math
import numpy as np

# Initialize Pygame
pygame.init()
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("3D Cube Renderer")
clock = pygame.time.Clock()

# Define cube vertices (8 corners of a unit cube centered at origin)
cube_vertices = np.array([
    [-1, -1, -1],  # 0
    [ 1, -1, -1],  # 1
    [ 1,  1, -1],  # 2
    [-1,  1, -1],  # 3
    [-1, -1,  1],  # 4
    [ 1, -1,  1],  # 5
    [ 1,  1,  1],  # 6
    [-1,  1,  1],  # 7
], dtype=float)

# Define edges connecting vertices
edges = [
    (0, 1), (1, 2), (2, 3), (3, 0),  # Front face
    (4, 5), (5, 6), (6, 7), (7, 4),  # Back face
    (0, 4), (1, 5), (2, 6), (3, 7)   # Connecting edges
]

def rotation_matrix_x(angle):
    """Rotation around X-axis"""
    c, s = math.cos(angle), math.sin(angle)
    return np.array([
        [1, 0, 0],
        [0, c, -s],
        [0, s, c]
    ])

def rotation_matrix_y(angle):
    """Rotation around Y-axis"""
    c, s = math.cos(angle), math.sin(angle)
    return np.array([
        [c, 0, s],
        [0, 1, 0],
        [-s, 0, c]
    ])

def rotation_matrix_z(angle):
    """Rotation around Z-axis"""
    c, s = math.cos(angle), math.sin(angle)
    return np.array([
        [c, -s, 0],
        [s, c, 0],
        [0, 0, 1]
    ])

def project_3d_to_2d(vertex_3d, distance=5):
    """Project 3D point to 2D screen using perspective"""
    z = vertex_3d[2] + distance  # Move camera back
    
    if z <= 0:
        return None
    
    scale = 300 / z  # Perspective scaling
    x = int(vertex_3d[0] * scale + WIDTH / 2)
    y = int(vertex_3d[1] * scale + HEIGHT / 2)
    
    return (x, y)

# Main loop
angle_x = angle_y = angle_z = 0
running = True

while running:
    clock.tick(60)  # 60 FPS
    
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
    
    # Update rotation angles
    angle_x += 0.01
    angle_y += 0.015
    angle_z += 0.005
    
    # Apply rotation transformations
    rotation = (rotation_matrix_z(angle_z) @ 
                rotation_matrix_y(angle_y) @ 
                rotation_matrix_x(angle_x))
    
    rotated_vertices = cube_vertices @ rotation.T
    
    # Project to 2D
    projected_vertices = []
    for vertex in rotated_vertices:
        proj = project_3d_to_2d(vertex)
        projected_vertices.append(proj)
    
    # Clear screen
    screen.fill((0, 0, 0))
    
    # Draw edges
    for edge in edges:
        v1, v2 = edge
        if projected_vertices[v1] and projected_vertices[v2]:
            pygame.draw.line(screen, (0, 255, 0), 
                           projected_vertices[v1], 
                           projected_vertices[v2], 2)
    
    # Draw vertices as small circles
    for proj_vertex in projected_vertices:
        if proj_vertex:
            pygame.draw.circle(screen, (255, 0, 0), proj_vertex, 4)
    
    pygame.display.flip()

pygame.quit()
