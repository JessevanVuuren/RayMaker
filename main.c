#include <math.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#define HEIGHT 800
#define WIDTH 1200

#define CAMERA_SPEED 0.01
#define ZOOM_SPEED 1

#define SCENE_SIZE 100
#define SCENE_DETAIL 101
#define SCENE_GRID_SPACE 1
#define EDIT_TOOLS_SCALE 1

enum EditMode { MOVE,
                SCALE,
                ROTATE

};

Vector3 camera_start_pos = {10, 10, -10};
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

void draw_xyz_control(Vector3 target, enum EditMode mode, Camera3D cam) {
    float dist = Vector3Distance(cam.position, cam.target);
    float magic_cam_dist_rec = .4 * EDIT_TOOLS_SCALE;
    Vector3 newX = Vector3Add(target, (Vector3){dist * magic_cam_dist_rec, 0, 0});
    Vector3 newY = Vector3Add(target, (Vector3){0, dist * magic_cam_dist_rec, 0});
    Vector3 newZ = Vector3Add(target, (Vector3){0, 0, -dist * magic_cam_dist_rec});

    if (target.x == 0 && target.y == 0 && target.z == 0) {
        target.y += 0.015f;
        newX.y += 0.015f;
        newZ.y += 0.015f;
    }

    float magic_radius_dist_rec = .017 * EDIT_TOOLS_SCALE;
    float magic_height_dist_rec = .07 * EDIT_TOOLS_SCALE;
    float magic_size_dist_rec = .03 * EDIT_TOOLS_SCALE;
    if (mode == MOVE) {
        DrawCylinderEx(newX, (Vector3){newX.x + dist * magic_height_dist_rec, newX.y, newX.z}, dist * magic_radius_dist_rec, 0, 30, GetColor(0xFF0000FF));
        DrawCylinderEx(newY, (Vector3){newY.x, newY.y + dist * magic_height_dist_rec, newY.z}, dist * magic_radius_dist_rec, 0, 30, GetColor(0x00FF00FF));
        DrawCylinderEx(newZ, (Vector3){newZ.x, newZ.y, newZ.z - dist * magic_height_dist_rec}, dist * magic_radius_dist_rec, 0, 30, GetColor(0x0000FFFF));
    };
    if (mode == SCALE) {
        DrawCube(newX, dist * magic_size_dist_rec, dist * magic_size_dist_rec, dist * magic_size_dist_rec, GetColor(0xFF0000FF));
        DrawCube(newY, dist * magic_size_dist_rec, dist * magic_size_dist_rec, dist * magic_size_dist_rec, GetColor(0x00FF00FF));
        DrawCube(newZ, dist * magic_size_dist_rec, dist * magic_size_dist_rec, dist * magic_size_dist_rec, GetColor(0x0000FFFF));
    };
    if (mode == ROTATE) {
        DrawSphere(newX, dist * magic_size_dist_rec, GetColor(0xFF0000FF));
        DrawSphere(newY, dist * magic_size_dist_rec, GetColor(0x00FF00FF));
        DrawSphere(newZ, dist * magic_size_dist_rec, GetColor(0x0000FFFF));
    };

    DrawLine3D(target, newX, GetColor(0xFF0000FF));
    DrawLine3D(target, newY, GetColor(0x00FF00FF));
    DrawLine3D(target, newZ, GetColor(0x0000FFFF));
}

BoundingBox GetBoundingBoxFromRaySegment(Vector3 rayOrigin, Vector3 rayDirection, float rayLength) {
    Vector3 endPoint = { 
        rayOrigin.x + rayDirection.x * rayLength, 
        rayOrigin.y + rayDirection.y * rayLength, 
        rayOrigin.z + rayDirection.z * rayLength 
    };

    Vector3 min = { 
        fmin(rayOrigin.x, endPoint.x), 
        fmin(rayOrigin.y, endPoint.y), 
        fmin(rayOrigin.z, endPoint.z) 
    };
    Vector3 max = { 
        fmax(rayOrigin.x, endPoint.x), 
        fmax(rayOrigin.y, endPoint.y), 
        fmax(rayOrigin.z, endPoint.z) 
    };

    BoundingBox box = { min, max };
    return box;
}

int main() {
    Camera3D cam = {
        .fovy = 90,
        .position = camera_start_pos,
        .target = origin,
        .projection = CAMERA_PERSPECTIVE,
        .up = camera_start_up};

    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "RayMaker");

    Mesh testCube = GenMeshCube(1.0f,1.0f,1.0f);
    Material mat = LoadMaterialDefault();
    // mat.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    Matrix matrix = MatrixTranslate(0.0f,0.0f,0.0f);

    while (!WindowShouldClose()) {

        float dist = GetMouseWheelMove();
        CameraMoveToTarget(&cam, -dist * ZOOM_SPEED);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mouseDelta = GetMouseDelta();
            Vector2 mousePos = GetMousePosition();
            if (mousePos.x > 70) {
                CameraYaw(&cam, -mouseDelta.x * CAMERA_SPEED, true);
                CameraPitch(&cam, -mouseDelta.y * CAMERA_SPEED, true, true, false);
            }
        }

        if (IsKeyPressed(KEY_R)) {
            cam.position = camera_start_pos;
            cam.up = camera_start_up;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(GetMousePosition(), cam);

            RayCollision box = GetRayCollisionBox(ray, GetMeshBoundingBox(testCube));
    
            if (box.hit) {
                printf("box hit, LETS FUCKING GOOOOO\n");
            } else {
                printf("box not hit\n");
            }
        }


        BeginDrawing();
            BeginMode3D(cam);
                ClearBackground(GetColor(0x181818FF));
                draw_graph();
                draw_xyz_control(origin, (enum EditMode)MOVE, cam);

                DrawMesh(testCube, mat, matrix);

            EndMode3D();
            DrawFPS(0, 0);
        EndDrawing();
    }
}