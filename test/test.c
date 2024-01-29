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

Mesh CreateTriangle(float height, float inner_radius, float outer_radius, int slices) {
    Mesh mesh = {0};

    mesh.vertexCount = slices * 4;
    mesh.triangleCount = slices * 8;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount * 3 * sizeof(unsigned short));

    for (int i = 0; i < slices; i++) {
        float x = cosf(2 * PI * ((float)i / (slices)));
        float z = sinf(2 * PI * ((float)i / (slices)));

        mesh.vertices[i * 12 + 0] = x * outer_radius;
        mesh.vertices[i * 12 + 1] = 0;
        mesh.vertices[i * 12 + 2] = z * outer_radius;

        mesh.vertices[i * 12 + 3] = x * outer_radius;
        mesh.vertices[i * 12 + 4] = height;
        mesh.vertices[i * 12 + 5] = z * outer_radius;

        mesh.vertices[i * 12 + 6] = x * inner_radius;
        mesh.vertices[i * 12 + 7] = 0;
        mesh.vertices[i * 12 + 8] = z * inner_radius;

        mesh.vertices[i * 12 + 9] = x * inner_radius;
        mesh.vertices[i * 12 + 10] = height;
        mesh.vertices[i * 12 + 11] = z * inner_radius;
    }


    int mod = slices * 4;
    for (int i = 0; i < slices; i++) {
        int step_size = 4 * i;

        mesh.indices[i * 24 + 0] = (0 + step_size) % mod;
        mesh.indices[i * 24 + 1] = (1 + step_size) % mod;
        mesh.indices[i * 24 + 2] = (4 + step_size) % mod;

        mesh.indices[i * 24 + 3] = (1 + step_size) % mod;
        mesh.indices[i * 24 + 4] = (5 + step_size) % mod;
        mesh.indices[i * 24 + 5] = (4 + step_size) % mod;

        mesh.indices[i * 24 + 6] = (6 + step_size) % mod;
        mesh.indices[i * 24 + 7] = (3 + step_size) % mod;
        mesh.indices[i * 24 + 8] = (2 + step_size) % mod;

        mesh.indices[i * 24 + 9] = (7 + step_size) % mod;
        mesh.indices[i * 24 + 10] = (3 + step_size) % mod;
        mesh.indices[i * 24 + 11] = (6 + step_size) % mod;

        mesh.indices[i * 24 + 12] = (1 + step_size) % mod;
        mesh.indices[i * 24 + 13] = (3 + step_size) % mod;
        mesh.indices[i * 24 + 14] = (7 + step_size) % mod;

        mesh.indices[i * 24 + 15] = (7 + step_size) % mod;
        mesh.indices[i * 24 + 16] = (5 + step_size) % mod;
        mesh.indices[i * 24 + 17] = (1 + step_size) % mod;

        mesh.indices[i * 24 + 18] = (6 + step_size) % mod;
        mesh.indices[i * 24 + 19] = (2 + step_size) % mod;
        mesh.indices[i * 24 + 20] = (0 + step_size) % mod;

        mesh.indices[i * 24 + 21] = (0 + step_size) % mod;
        mesh.indices[i * 24 + 22] = (4 + step_size) % mod;
        mesh.indices[i * 24 + 23] = (6 + step_size) % mod;
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
    Mesh mesh = CreateTriangle(1,1,1,1);

    while (!WindowShouldClose()) {

        float dist = GetMouseWheelMove();
        CameraMoveToTarget(&cam, -dist * ZOOM_SPEED);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
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
    UnloadMesh(mesh);
    CloseWindow();
}