#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

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

enum EditMode { MOVE,
                SCALE,
                ROTATE,
                NONE

};

typedef struct {
    int id;
    bool is_selected;
    Mesh mesh;
    Matrix matrix;
    Material material;
} Object;

typedef struct {
    Object x;
    Object y;
    Object z;
    bool x_selected;
    RayCollision x_ray;
    bool y_selected;
    bool z_selected;
    Object hidden_plane_x;
    Object hidden_plane_y;
    Object hidden_plane_z;
} XYZcontrol;

typedef struct {
    Object object;
    bool is_selected;
    Vector3 pos;
    int index;
} Selected;

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

void draw_xyz_control(Vector3 target, enum EditMode mode, Camera3D cam, XYZcontrol *xyz) {
    float magic_cam_dist_rec = .4 * EDIT_TOOLS_SCALE;
    Vector3 newX = Vector3Add(target, (Vector3){EDIT_TOOLS_SCALE * magic_cam_dist_rec, 0, 0});
    Vector3 newY = Vector3Add(target, (Vector3){0, EDIT_TOOLS_SCALE * magic_cam_dist_rec, 0});
    Vector3 newZ = Vector3Add(target, (Vector3){0, 0, -EDIT_TOOLS_SCALE * magic_cam_dist_rec});

    if (target.y == 0) {
        target.y += 0.015f;
        newX.y += 0.015f;
        newZ.y += 0.015f;
    }

    float magic_radius_dist_rec = .017 * EDIT_TOOLS_SCALE;
    float magic_height_dist_rec = .07 * EDIT_TOOLS_SCALE;
    float magic_size_dist_rec = .03 * EDIT_TOOLS_SCALE;
    if (mode == MOVE) {
        DrawCylinderEx(newX, (Vector3){newX.x + EDIT_TOOLS_SCALE * magic_height_dist_rec, newX.y, newX.z}, EDIT_TOOLS_SCALE * magic_radius_dist_rec, 0, 30, GetColor(0xFF0000FF));
        DrawCylinderEx(newY, (Vector3){newY.x, newY.y + EDIT_TOOLS_SCALE * magic_height_dist_rec, newY.z}, EDIT_TOOLS_SCALE * magic_radius_dist_rec, 0, 30, GetColor(0x00FF00FF));
        DrawCylinderEx(newZ, (Vector3){newZ.x, newZ.y, newZ.z - EDIT_TOOLS_SCALE * magic_height_dist_rec}, EDIT_TOOLS_SCALE * magic_radius_dist_rec, 0, 30, GetColor(0x0000FFFF));
    };
    if (mode == SCALE) {
        DrawCube(newX, EDIT_TOOLS_SCALE * magic_size_dist_rec, EDIT_TOOLS_SCALE * magic_size_dist_rec, EDIT_TOOLS_SCALE * magic_size_dist_rec, GetColor(0xFF0000FF));
        DrawCube(newY, EDIT_TOOLS_SCALE * magic_size_dist_rec, EDIT_TOOLS_SCALE * magic_size_dist_rec, EDIT_TOOLS_SCALE * magic_size_dist_rec, GetColor(0x00FF00FF));
        DrawCube(newZ, EDIT_TOOLS_SCALE * magic_size_dist_rec, EDIT_TOOLS_SCALE * magic_size_dist_rec, EDIT_TOOLS_SCALE * magic_size_dist_rec, GetColor(0x0000FFFF));
    };
    if (mode == ROTATE) {
        DrawSphere(newX, EDIT_TOOLS_SCALE * magic_size_dist_rec, GetColor(0xFF0000FF));
        DrawSphere(newY, EDIT_TOOLS_SCALE * magic_size_dist_rec, GetColor(0x00FF00FF));
        DrawSphere(newZ, EDIT_TOOLS_SCALE * magic_size_dist_rec, GetColor(0x0000FFFF));
    };

    DrawLine3D(target, newX, GetColor(0xFF0000FF));
    DrawLine3D(target, newY, GetColor(0x00FF00FF));
    DrawLine3D(target, newZ, GetColor(0x0000FFFF));

    xyz->x.matrix = MatrixTranslate(newX.x - 5.5f / 2.0f, newX.y, newX.z);
    xyz->y.matrix = MatrixTranslate(newY.x, newY.y - 5.5f / 2.0f, newY.z);
    xyz->z.matrix = MatrixTranslate(newZ.x, newZ.y, newZ.z + 5.5f / 2.0f);

    DrawMesh(xyz->x.mesh, xyz->x.material, xyz->x.matrix);
    DrawMesh(xyz->y.mesh, xyz->y.material, xyz->y.matrix);
    DrawMesh(xyz->z.mesh, xyz->z.material, xyz->z.matrix);
}

void init_XYZ_controls(XYZcontrol *xyz) {
    Object hit_box_x;
    hit_box_x.id = 1;
    hit_box_x.is_selected = false;
    hit_box_x.mesh = GenMeshCube(7.5f, .5f, .5f);
    hit_box_x.material = LoadMaterialDefault();
    hit_box_x.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0xFF000055);
    hit_box_x.matrix = MatrixTranslate(7.5f / 2, 0.0f, 0.0f);

    Object hit_box_y;
    hit_box_y.id = 1;
    hit_box_y.is_selected = false;
    hit_box_y.mesh = GenMeshCube(.5f, 7.5f, .5f);
    hit_box_y.material = LoadMaterialDefault();
    hit_box_y.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x00FF0055);
    hit_box_y.matrix = MatrixTranslate(0.0f, 7.5f / 2, 0.0f);

    Object hit_box_z;
    hit_box_z.id = 1;
    hit_box_z.is_selected = false;
    hit_box_z.mesh = GenMeshCube(.5f, .5f, 7.5f);
    hit_box_z.material = LoadMaterialDefault();
    hit_box_z.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x0000FF55);
    hit_box_z.matrix = MatrixTranslate(0.0f, 0.0f, -7.5f / 2);

    xyz->x_ray.hit = false;

    xyz->x_selected = false;
    xyz->y_selected = false;
    xyz->z_selected = false;

    xyz->hidden_plane_x.mesh = GenMeshPlane(100.0f, 100.0f, 1, 1);

    xyz->x = hit_box_x;
    xyz->y = hit_box_y;
    xyz->z = hit_box_z;
}

Vector3 get_matrix_translation(Matrix mat) {
    Vector3 translation;
    translation.x = mat.m12;
    translation.y = mat.m13;
    translation.z = mat.m14;
    return translation;
}

void move_selected_item(Object *objects) {
}

Selected set_selected(Object object, int index) {
    Selected selected;
    selected.object = object;
    selected.pos = get_matrix_translation(object.matrix);
    selected.index = index;
    selected.is_selected = true;
    return selected;
}

int main() {
    Camera3D cam = {.fovy = 90, .position = camera_start_pos, .target = origin, .projection = CAMERA_PERSPECTIVE, .up = camera_start_up};

    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "RayMaker");
    SetExitKey(0);

    Object *objects = NULL;

    XYZcontrol xyz_control;
    init_XYZ_controls(&xyz_control);

    Object test_cube1;
    test_cube1.id = 1;
    test_cube1.is_selected = false;
    test_cube1.mesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    test_cube1.material = LoadMaterialDefault();
    test_cube1.material.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    test_cube1.matrix = MatrixTranslate(10.0f, 0.0f, 0.0f);

    Object test_cube2;
    test_cube2.id = 2;
    test_cube2.is_selected = false;
    test_cube2.mesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    test_cube2.material = LoadMaterialDefault();
    test_cube2.material.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    test_cube2.matrix = MatrixTranslate(-10.0f, 0.0f, 0.0f);

    arrput(objects, test_cube1);
    arrput(objects, test_cube2);

    Selected selected = {0};

    while (!WindowShouldClose()) {

        float dist = GetMouseWheelMove();
        CameraMoveToTarget(&cam, -dist * ZOOM_SPEED);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_CONTROL)) {
            Vector2 mouseDelta = GetMouseDelta();
            Vector2 mousePos = GetMousePosition();
            if (mousePos.x > 70) {
                CameraYaw(&cam, -mouseDelta.x * CAMERA_SPEED, true);
                CameraPitch(&cam, -mouseDelta.y * CAMERA_SPEED, true, true, false);
            }
        }


        if (IsKeyPressed(KEY_ESCAPE)) {
            for (int i = 0; i < arrlen(objects); i++) {
                objects[i].is_selected = false;
            }
        }

        if (IsKeyPressed(KEY_R)) {
            cam.position = camera_start_pos;
            cam.up = camera_start_up;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(GetMousePosition(), cam);

            for (int i = 0; i < arrlen(objects); i++) {
                RayCollision box = GetRayCollisionMesh(ray, objects[i].mesh, objects[i].matrix);
                if (box.hit) selected = set_selected(objects[i], i);
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(GetMousePosition(), cam);

            xyz_control.x_ray = GetRayCollisionMesh(ray, xyz_control.x.mesh, xyz_control.x.matrix);
            xyz_control.y_selected = GetRayCollisionMesh(ray, xyz_control.y.mesh, xyz_control.y.matrix).hit;
            xyz_control.z_selected = GetRayCollisionMesh(ray, xyz_control.z.mesh, xyz_control.z.matrix).hit;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            xyz_control.x_ray.hit = false;
            xyz_control.y_selected = false;
            xyz_control.x_selected = false;
        }

        if (xyz_control.x_ray.hit) {
            Ray ray = GetMouseRay(GetMousePosition(), cam);
            Vector3 pos = selected.pos;

            Mesh plane_x = xyz_control.hidden_plane_x.mesh;
            Matrix matrix_x = MatrixTranslate(pos.x, pos.y, pos.z);

            float point = GetRayCollisionMesh(ray, plane_x, matrix_x).point.x;

            Matrix new_pos = MatrixTranslate(point + (selected.pos.x - xyz_control.x_ray.point.x), pos.y, pos.z);
            objects[selected.index].matrix = new_pos;
        }



        BeginDrawing();
        BeginMode3D(cam);
        ClearBackground(GetColor(0x181818FF));
        draw_graph();

        Vector3 edit_pos = origin;
        enum EditMode edit = NONE;
        for (int i = 0; i < arrlen(objects); i++) {
            DrawMesh(objects[i].mesh, objects[i].material, objects[i].matrix);
        }

        if (selected.is_selected) {
            edit_pos = get_matrix_translation(objects[selected.index].matrix);
            edit = MOVE;
        }
        if (IsKeyDown(KEY_LEFT_CONTROL)) edit_pos = origin;

        draw_xyz_control(edit_pos, edit, cam, &xyz_control);
        EndMode3D();
        DrawFPS(0, 0);
        EndDrawing();
    }
}