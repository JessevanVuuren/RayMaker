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

enum EditMode { MOVE,
                SCALE,
                ROTATE,
                NONE

};

typedef struct {
    int id;
    Mesh mesh;
    Matrix matrix;
    Material material;
} Object;

typedef struct {
    Object x;
    Object y;
    Object z;
    RayCollision x_ray;
    RayCollision y_ray;
    RayCollision z_ray;
    Mesh hidden_box;
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
    Vector3 newX = Vector3Add(target, (Vector3){0, 0, 0});
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
    // #TODO: fix offset, made by my
    xyz->x.matrix = MatrixTranslate(newX.x + 7.5f * .5f, newX.y, newX.z);
    xyz->y.matrix = MatrixTranslate(newY.x, newY.y - 7.5f * .5f, newY.z);
    xyz->z.matrix = MatrixTranslate(newZ.x, newZ.y, newZ.z + 7.5f * .5f);

    DrawMesh(xyz->x.mesh, xyz->x.material, xyz->x.matrix);
    DrawMesh(xyz->y.mesh, xyz->y.material, xyz->y.matrix);
    DrawMesh(xyz->z.mesh, xyz->z.material, xyz->z.matrix);
}

void init_XYZ_controls(XYZcontrol *xyz) {
    Object hit_box_x;
    hit_box_x.id = 1;
    hit_box_x.mesh = GenMeshCube(7.5f, 6.5f, 6.5f);
    hit_box_x.material = LoadMaterialDefault();
    hit_box_x.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0xFF000055);
    hit_box_x.matrix = MatrixTranslate(7.5f / 2, 0.0f, 0.0f);

    Object hit_box_y;
    hit_box_y.id = 1;
    hit_box_y.mesh = GenMeshCube(.5f, 7.5f, .5f);
    hit_box_y.material = LoadMaterialDefault();
    hit_box_y.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x00FF0055);
    hit_box_y.matrix = MatrixTranslate(0.0f, 7.5f / 2, 0.0f);

    Object hit_box_z;
    hit_box_z.id = 1;
    hit_box_z.mesh = GenMeshCube(.5f, .5f, 7.5f);
    hit_box_z.material = LoadMaterialDefault();
    hit_box_z.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x0000FF55);
    hit_box_z.matrix = MatrixTranslate(0.0f, 0.0f, -7.5f / 2);

    xyz->x_ray.hit = false;
    xyz->y_ray.hit = false;
    xyz->z_ray.hit = false;

    xyz->hidden_box = GenMeshCube(20000, 20000, 20000);
    xyz->x = hit_box_x;
    xyz->y = hit_box_y;
    xyz->z = hit_box_z;
}


Selected set_selected(Object object, int index) {
    Selected selected;
    selected.object = object;
    selected.pos = getMatrixTranslation(object.matrix);
    selected.index = index;
    selected.is_selected = true;
    return selected;
}

Matrix move_object(Camera cam, Selected selected, Mesh cube, Vector2 camera_pos, Vector3 axis, float offset_point) {
    Ray ray = GetMouseRay(GetMousePosition(), cam);

    float angle = atan2f(camera_pos.x, camera_pos.y);
    float scale = 10000 / sqrtf(powf(camera_pos.x, 2) + powf(camera_pos.y, 2));

    Vector3 inverse_axis = Vector3MultiplyValue(Vector3AddValue(axis, -1), -1);
    Vector3 opposite_cam = Vector3MultiplyValue(cam.position, -scale);
    Vector3 current_axis = Vector3Add(Vector3Multiply(opposite_cam, inverse_axis), selected.pos);

    Matrix translation = MatrixTranslate(current_axis.x, current_axis.y, current_axis.z);
    Matrix rotation = MatrixRotate(axis, angle);
    Matrix matrix = MatrixMultiply(rotation, translation);

    

    Vector3 hit_point = GetRayCollisionMesh(ray, cube, matrix).point;
    Vector3 hit_point_offset = Vector3AddValue(hit_point, -offset_point);
    Vector3 offset_current_axis = Vector3Multiply(axis, hit_point_offset);
    Vector3 new_position = Vector3Add(selected.pos, offset_current_axis);

    // move
    return MatrixTranslate(new_position.x, new_position.y, new_position.z);


    // scale

    // Vector3 scale_up = {new_position.x, new_position.y, -new_position.z};
    // Vector3 minus_position = Vector3Subtract(scale_up, selected.pos);
    // Vector3 current_axis_scale = Vector3Multiply(minus_position, axis);
    // printf("XYZ: %f, %f, %f\n", current_axis_scale.x, current_axis_scale.y, current_axis_scale.z);
    // Vector3 add_base_one = Vector3AddValue((Vector3){0,0,0}, 1);
    // Matrix matrix_scaled_up = MatrixScale(add_base_one.x, add_base_one.y, add_base_one.z);

    // return MatrixMultiply(matrix_scaled_up, selected.object.matrix);
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
    test_cube1.mesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    test_cube1.material = LoadMaterialDefault();
    test_cube1.material.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    test_cube1.matrix = MatrixTranslate(10.0f, 0.0f, 0.0f);

    Object test_cube2;
    test_cube2.id = 2;
    test_cube2.mesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    test_cube2.material = LoadMaterialDefault();
    test_cube2.material.maps[MATERIAL_MAP_DIFFUSE].color = RED;
    test_cube2.matrix = MatrixTranslate(-10.0f, 0.0f, 0.0f);

    arrput(objects, test_cube1);
    arrput(objects, test_cube2);

    Selected selected = {0};

    selected.index = 1;
    selected.is_selected = true;
    selected.object = test_cube2;
    selected.pos = getMatrixTranslation(MatrixTranslate(-10.0f, 0.0f, 0.0f));

    while (!WindowShouldClose()) {

        float dist = GetMouseWheelMove();
        CameraMoveToTarget(&cam, -dist * ZOOM_SPEED);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_CONTROL)) {
            Vector2 mouseDelta = GetMouseDelta();

            CameraYaw(&cam, -mouseDelta.x * CAMERA_SPEED, true);
            CameraPitch(&cam, -mouseDelta.y * CAMERA_SPEED, true, true, false);
        }


        if (IsKeyPressed(KEY_ESCAPE)) {
            selected.is_selected = false;
        }

        if (IsKeyPressed(KEY_R)) {
            cam.position = camera_start_pos;
            cam.up = camera_start_up;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(GetMousePosition(), cam);

            xyz_control.x_ray = GetRayCollisionMesh(ray, xyz_control.x.mesh, xyz_control.x.matrix);
            xyz_control.y_ray = GetRayCollisionMesh(ray, xyz_control.y.mesh, xyz_control.y.matrix);
            xyz_control.z_ray = GetRayCollisionMesh(ray, xyz_control.z.mesh, xyz_control.z.matrix);

            for (int i = 0; i < arrlen(objects); i++) {
                RayCollision box = GetRayCollisionMesh(ray, objects[i].mesh, objects[i].matrix);
                if (box.hit) selected = set_selected(objects[i], i);
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            selected = set_selected(objects[selected.index], selected.index);
            xyz_control.x_ray.hit = false;
            xyz_control.y_ray.hit = false;
            xyz_control.z_ray.hit = false;
        }

        if (xyz_control.x_ray.hit) {
            Ray ray = GetMouseRay(GetMousePosition(), cam);

            Vector2 camera_pos = {cam.position.z, cam.position.y};
            Mesh cube = xyz_control.hidden_box;
            Vector3 axis = {1, 0, 0};

            float offset_point = xyz_control.x_ray.point.x;


            float angle = atan2f(camera_pos.x, camera_pos.y);
            float scale = 10000 / sqrtf(powf(camera_pos.x, 2) + powf(camera_pos.y, 2));

            Vector3 inverse_axis = Vector3MultiplyValue(Vector3AddValue(axis, -1), -1);
            Vector3 opposite_cam = Vector3MultiplyValue(cam.position, -scale);
            Vector3 current_axis = Vector3Add(Vector3Multiply(opposite_cam, inverse_axis), selected.pos);

            Matrix translation = MatrixTranslate(current_axis.x, current_axis.y, current_axis.z);
            Matrix rotation = MatrixRotate(axis, angle);
            Matrix matrix = MatrixMultiply(rotation, translation);

            float dist_to_origin_angle = Vector3Distance(xyz_control.x_ray.point, selected.pos);
            float dist_to_origin = selected.pos.x + 7.5f;
            // TODO: fix
            printf("%f\n", dist_to_origin_angle - dist_to_origin);



            Vector3 hit_point = GetRayCollisionMesh(ray, cube, matrix).point;
            Vector3 hit_point_offset = Vector3AddValue(hit_point, -offset_point);



            Vector3 offset_current_axis = Vector3Multiply(axis, hit_point_offset);
            Vector3 new_position = Vector3Add(selected.pos, offset_current_axis);

            // move
            objects[selected.index].matrix = MatrixTranslate(new_position.x, new_position.y, new_position.z);

            // Vector2 camera_pos = {cam.position.z, cam.position.y};
            // Mesh cube = xyz_control.hidden_box;

            // Vector3 axis = {1, 0, 0};
            // float offset_point = xyz_control.x_ray.point.x;

            // Matrix new_position = move_object(cam, selected, cube, camera_pos, axis, offset_point);
            // objects[selected.index].matrix = new_position;
        }

        // if (xyz_control.y_ray.hit) {
        //     Vector2 camera_pos = {cam.position.x, cam.position.z};
        //     Mesh cube = xyz_control.hidden_box;

        //     Vector3 axis = {0, 1, 0};
        //     float offset_point = xyz_control.y_ray.point.y;

        //     Matrix new_position = move_object(cam, selected, cube, camera_pos, axis, offset_point);
        //     objects[selected.index].matrix = new_position;
        // }

        // if (xyz_control.z_ray.hit) {
        //     Vector2 camera_pos = {cam.position.y, cam.position.x};
        //     Mesh cube = xyz_control.hidden_box;

        //     Vector3 axis = {0, 0, 1};
        //     float offset_point = xyz_control.z_ray.point.z;

        //     Matrix new_position = move_object(cam, selected, cube, camera_pos, axis, offset_point);
        //     objects[selected.index].matrix = new_position;
        // }

        BeginDrawing();
        BeginMode3D(cam);
        ClearBackground(GetColor(0x181818FF));
        draw_graph();

        Vector3 edit_pos = origin;
        enum EditMode edit = NONE;

        for (int i = 0; i < arrlen(objects); i++)
            DrawMesh(objects[i].mesh, objects[i].material, objects[i].matrix);

        if (selected.is_selected) {
            edit_pos = getMatrixTranslation(objects[selected.index].matrix);
            edit = MOVE;
        }
        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            edit_pos = origin;
            edit = NONE;
        }

        draw_xyz_control(edit_pos, edit, cam, &xyz_control);


        EndMode3D();
        DrawFPS(0, 0);
        EndDrawing();
    }
}