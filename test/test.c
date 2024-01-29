#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#include "extramath.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define HEIGHT 800
#define WIDTH 1200

#define CAMERA_SPEED 0.01
#define ZOOM_SPEED 1

#define SCENE_SIZE 100
#define SCENE_DETAIL 101
#define SCENE_GRID_SPACE 1
#define EDIT_TOOLS_SCALE 4


typedef struct {
    int id;
    float angle;
    Model model;
    Vector3 pos;
    Vector3 axis;
    Vector3 scale;
    Texture2D texture;
} Object;




Vector3 camera_start_pos = {10, 10, 10};
Vector3 camera_start_up = {0, 1, 0};
Vector3 origin = {0};

void draw_graph() {

    for (int i = 0; i < SCENE_DETAIL; i++) {
        float step = SCENE_SIZE - (2 * SCENE_SIZE * i) / (float)(SCENE_DETAIL - 1);
        float point_one = -sqrtf(powf(SCENE_SIZE, 2) - powf(step, 2));
        float point_two = sqrtf(powf(SCENE_SIZE, 2) - powf(step, 2));

        DrawLine3D((Vector3){point_one, 0, step}, (Vector3){point_two, 0, step}, GetColor(0x454545FF));
        DrawLine3D((Vector3){step, 0, point_one}, (Vector3){step, 0, point_two}, GetColor(0x454545FF));
    }
}

void draw_model(Object o) {
    DrawModel(o.model, o.pos, 1, WHITE);
    DrawBoundingBox(GetModelBoundingBox(o.model), GREEN);
}

Mesh CreateHollowCylinder(float outerRadius, float innerRadius, float height, int slices) {
    Mesh mesh = {0};

    // Define the number of vertices and faces
    int vertexCount = (slices + 1) * 2; // top and bottom, inner and outer
    int triangleCount = slices * 2;     // top and bottom, inner and outer

    // Initialize mesh data (vertices, normals, and texcoords)
    mesh.vertices = (float *)malloc(vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)malloc(vertexCount * 2 * sizeof(float));
    mesh.normals = (float *)malloc(vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short *)malloc(triangleCount * 3 * sizeof(unsigned short));

    // for (int i = 0; i < mesh.vertexCount; i++) {
    //     // Fill in vertices, normals, and texcoords
    //     float x = cos(2 * PI * (i / mesh.vertexCount));
    //     float z = cos(2 * PI * (i / mesh.vertexCount));

    //     if (i > mesh.vertexCount / 2) {
    //         mesh.vertices[i * 3 + 1] = 0;
    //     } else {
    //         mesh.vertices[i * 3 + 1] = 1;
    //     }

    //     mesh.vertices[i * 3] = x;
    //     mesh.vertices[i * 3 + 2] = z;
    //     // ...
    // }

    for (int i = 0; i < mesh.vertexCount; i++) {
        // Calculate the angle for this vertex
        float angle = 2 * PI * (float)i / mesh.vertexCount;

        // Calculate x and z using cosine and sine functions
        float x = cos(angle);
        float z = sin(angle);

        // Assign the y coordinate
        float y;
        if (i > mesh.vertexCount / 2) {
            y = 0; // For the second half of the vertices
        } else {
            y = 1; // For the first half of the vertices
        }

        // Assign the calculated coordinates to the mesh vertices
        mesh.vertices[i * 3] = x;     // x-coordinate
        mesh.vertices[i * 3 + 1] = y; // y-coordinate
        mesh.vertices[i * 3 + 2] = z; // z-coordinate

        // If you need to fill in normals and texture coordinates, do it here
        // ...
    }

    for (int i = 0; i < mesh.triangleCount; i++) {
        mesh.indices[i + 0] = i + 0; // First vertex of the first triangle
        mesh.indices[i + 1] = i + 1; // Second vertex of the first triangle
        mesh.indices[i + 2] = i + 2; // Third vertex of the first triangle

        mesh.indices[i + 3] = i + 2; // First vertex of the second triangle
        mesh.indices[i + 4] = i + 3; // Second vertex of the second triangle
        mesh.indices[i + 5] = i + 0; // Third vertex of the second triangle
    }

    return mesh;
}

Mesh CreateTriangle() {
    Mesh mesh = {0};

    mesh.vertexCount = 30;
    mesh.triangleCount = 30;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    for (int i = 0; i < mesh.vertexCount / 2.0f; i++) {
        float x = cosf(2 * PI * ((float)i / (mesh.vertexCount /2.0f))) * 10;
        float z = sinf(2 * PI * ((float)i / (mesh.vertexCount /2.0f))) * 10;

        mesh.vertices[i * 6 + 0] = x;
        mesh.vertices[i * 6 + 1] = 0;
        mesh.vertices[i * 6 + 2] = z;

        mesh.vertices[i * 6 + 3] = x;
        mesh.vertices[i * 6 + 4] = 10;
        mesh.vertices[i * 6 + 5] = z;
    }


    for (int i = 0; i < mesh.triangleCount /2.0f; i++)
    {
            mesh.indices[i * 6 + 0] = 0 + (i * 2);
            mesh.indices[i * 6 + 1] = 1 + (i * 2);
            mesh.indices[i * 6 + 2] = 2 + (i * 2);

            mesh.indices[i * 6 + 3] = 1 + (i * 2);
            mesh.indices[i * 6 + 4] = 3 + (i * 2);
            mesh.indices[i * 6 + 5] = 2 + (i * 2);
    }
  


    UploadMesh(&mesh, false);

    return mesh;
}



int main() {
    Camera3D cam = {.fovy = 90, .position = camera_start_pos, .target = origin, .projection = CAMERA_PERSPECTIVE, .up = camera_start_up};

    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "RayMaker");
    SetExitKey(0);


    Material material = LoadMaterialDefault();
    material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0xFF0000FF);
    Mesh mesh = CreateTriangle();

    while (!WindowShouldClose()) {

        float dist = GetMouseWheelMove();
        CameraMoveToTarget(&cam, -dist * ZOOM_SPEED);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_CONTROL)) {
            Vector2 mouseDelta = GetMouseDelta();

            CameraYaw(&cam, -mouseDelta.x * CAMERA_SPEED, true);
            CameraPitch(&cam, -mouseDelta.y * CAMERA_SPEED, true, true, false);
        }



        if (IsKeyPressed(KEY_R)) {
            cam.position = camera_start_pos;
            cam.up = camera_start_up;
        }


        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));

        BeginMode3D(cam);
        draw_graph();

        DrawMesh(mesh, material, MatrixTranslate(0, 0, 0));
        


        EndMode3D();
        DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
}